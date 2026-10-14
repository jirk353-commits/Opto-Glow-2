// Offline test runner for Glow Verb: DSP checks + UI screenshot.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdio>

using namespace juce;

static int failures = 0;
static void check (bool ok, const String& what)
{
    std::printf ("[%s] %s\n", ok ? " OK " : "FAIL", what.toRawUTF8());
    if (! ok) ++failures;
}
static void setP (GlowVerbProcessor& p, const char* id, float v)
{
    auto* prm = p.apvts.getParameter (id);
    prm->setValueNotifyingHost (prm->convertTo0to1 (v));
}

/** runs the processor; in(i) main input, sc(i) sidechain input. Returns left output. */
static std::vector<float> run (GlowVerbProcessor& p, std::function<float (int)> in, std::function<float (int)> sc,
                               int total, int block = 512)
{
    std::vector<float> out;
    out.reserve ((size_t) total);
    AudioBuffer<float> buf (4, block);
    MidiBuffer midi;
    for (int pos = 0; pos < total; pos += block)
    {
        const int n = jmin (block, total - pos);
        AudioBuffer<float> b (buf.getArrayOfWritePointers(), 4, 0, n);
        for (int i = 0; i < n; ++i)
        {
            const float x = in (pos + i), s = sc ? sc (pos + i) : 0.0f;
            b.setSample (0, i, x); b.setSample (1, i, x);
            b.setSample (2, i, s); b.setSample (3, i, s);
        }
        p.processBlock (b, midi);
        for (int i = 0; i < n; ++i) out.push_back (b.getSample (0, i));
    }
    return out;
}

static float rms (const std::vector<float>& x, size_t a, size_t b)
{
    double s = 0; for (size_t i = a; i < b; ++i) s += (double) x[i] * x[i];
    return (float) std::sqrt (s / (double) jmax<size_t> (1, b - a));
}

static float measureT60 (const std::vector<float>& ir, double fs)
{
    // Schroeder backward integration, fit -5 .. -25 dB, extrapolate to -60
    std::vector<double> e (ir.size());
    double acc = 0;
    for (size_t i = ir.size(); i-- > 0;) { acc += (double) ir[i] * ir[i]; e[i] = acc; }
    const double e0 = e[0];
    int i5 = -1, i25 = -1;
    for (size_t i = 0; i < e.size(); ++i)
    {
        const double db = 10.0 * std::log10 (e[i] / e0 + 1e-30);
        if (i5 < 0 && db <= -5.0) i5 = (int) i;
        if (i25 < 0 && db <= -25.0) { i25 = (int) i; break; }
    }
    if (i5 < 0 || i25 < 0) return -1.0f;
    return (float) (3.0 * (i25 - i5) / fs);
}

int main (int argc, char** argv)
{
    ScopedJuceInitialiser_GUI init;
    const double fs = 48000.0;
    Random rnd (3);

    // ---------- decay time per environment ----------------------------------
    for (int t = 0; t < 9; ++t)
    {
        GlowVerbProcessor p;
        p.prepareToPlay (fs, 512);
        p.applyEnvironment (t);
        setP (p, IDs::mix, 100); setP (p, IDs::lowCut, 20); setP (p, IDs::highCut, 20000);
        p.prepareToPlay (fs, 512);
        const float target = p.apvts.getRawParameterValue (IDs::decay)->load();
        const int len = (int) (fs * (target * 1.6 + 0.6));
        auto ir = run (p, [] (int i) { return i == 0 ? 1.0f : 0.0f; }, nullptr, len);
        bool finite = true; float peak = 0;
        for (auto v : ir) { finite &= std::isfinite (v); peak = jmax (peak, std::abs (v)); }
        const float t60 = measureT60 (ir, fs);

        GlowVerbProcessor q; q.prepareToPlay (fs, 512); q.applyEnvironment (t);
        setP (q, IDs::mix, 100); setP (q, IDs::lowCut, 20); setP (q, IDs::highCut, 20000);
        q.prepareToPlay (fs, 512);
        auto noise = run (q, [&] (int) { return (rnd.nextFloat() * 2 - 1) * 0.3f; }, nullptr, (int) fs * 3);
        const float gainDb = Decibels::gainToDecibels (rms (noise, (size_t) fs, noise.size()) / (0.3f / std::sqrt (3.0f)));

        std::printf ("  %-10s target %5.2fs  measured T60 %5.2fs  IR peak %.3f  noise wet/in %+5.1f dB\n",
                     gv::envTypes()[(size_t) t].name, target, t60, peak, gainDb);
        check (finite && t60 > target * 0.55f && t60 < target * 1.6f, String ("decay matches for ") + gv::envTypes()[(size_t) t].name);
        check (gainDb > -12.0f && gainDb < 6.0f, String ("wet level sane for ") + gv::envTypes()[(size_t) t].name);
    }

    // ---------- worst-case stability ----------------------------------------
    {
        GlowVerbProcessor p; p.prepareToPlay (fs, 256);
        setP (p, IDs::type, 6); setP (p, IDs::decay, 20); setP (p, IDs::damping, 0); setP (p, IDs::diffusion, 100);
        setP (p, IDs::mod, 100); setP (p, IDs::size, 100); setP (p, IDs::mix, 100); setP (p, IDs::drive, 100);
        setP (p, IDs::satType, 2); setP (p, IDs::air, 100);
        p.prepareToPlay (fs, 256);
        auto y = run (p, [&] (int i) { return i < (int) fs * 2 ? (rnd.nextFloat() * 2 - 1) * 0.8f : 0.0f; }, nullptr, (int) fs * 8, 256);
        bool finite = true; for (auto v : y) finite &= std::isfinite (v);
        const float a = rms (y, (size_t) fs * 3, (size_t) fs * 4), b = rms (y, (size_t) fs * 7, (size_t) fs * 8);
        std::printf ("  20s cathedral tail: %.4f -> %.4f\n", a, b);
        check (finite && b <= a * 1.01f && a < 2.0f, "stable with extreme settings");
    }

    // ---------- external sidechain ducking ----------------------------------
    {
        auto wetWith = [&] (bool key)
        {
            GlowVerbProcessor p; p.prepareToPlay (fs, 512);
            setP (p, IDs::mix, 100); setP (p, IDs::duck, 100); setP (p, IDs::scSource, 1); setP (p, IDs::threshold, -40);
            p.prepareToPlay (fs, 512);
            auto y = run (p, [&] (int) { return (rnd.nextFloat() * 2 - 1) * 0.3f; },
                          [&] (int i) { return key ? 0.5f * (float) std::sin (i * 0.03) : 0.0f; }, (int) fs * 3);
            return rms (y, (size_t) fs, y.size());
        };
        const float noKey = wetWith (false), withKey = wetWith (true);
        std::printf ("  ext sidechain: wet %.4f (no key) vs %.4f (key)  = %.1f dB\n", noKey, withKey,
                     Decibels::gainToDecibels (withKey / noKey));
        check (withKey < noKey * 0.25f, "external sidechain ducks the reverb");
    }

    // ---------- mix 0 = dry --------------------------------------------------
    {
        GlowVerbProcessor p; p.prepareToPlay (fs, 512); setP (p, IDs::mix, 0); p.prepareToPlay (fs, 512);
        std::vector<float> in; auto y = run (p, [&] (int i) { float v = (float) std::sin (i * 0.01); return v; }, nullptr, 4800);
        float err = 0; for (int i = 0; i < 4800; ++i) err = jmax (err, std::abs (y[(size_t) i] - (float) std::sin (i * 0.01)));
        check (err < 1e-4f, "mix 0 passes dry signal unchanged");
    }

    // ---------- presets ------------------------------------------------------
    {
        GlowVerbProcessor p; p.prepareToPlay (fs, 300);
        bool ok = true;
        for (int i = 0; i < p.getNumPrograms(); ++i)
        {
            p.setCurrentProgram (i);
            auto y = run (p, [&] (int) { return (rnd.nextFloat() * 2 - 1) * 0.5f; }, nullptr, 48000, 300);
            float pk = 0; bool fin = true; for (auto v : y) { fin &= std::isfinite (v); pk = jmax (pk, std::abs (v)); }
            std::printf ("  preset %-22s peak %.2f  rms %.3f\n", p.getProgramName (i).toRawUTF8(), pk, rms (y, 24000, y.size()));
            ok &= fin && pk < 3.0f;
        }
        check (ok, "all presets stable");
    }

    // ---------- state --------------------------------------------------------
    {
        GlowVerbProcessor a, b;
        a.setCurrentProgram (4);
        MemoryBlock mb; a.getStateInformation (mb);
        b.setStateInformation (mb.getData(), (int) mb.getSize());
        check (b.getCurrentProgram() == 4 && std::abs (b.apvts.getRawParameterValue (IDs::decay)->load() - 7.5f) < 0.01f, "state save/restore");
    }

    // ---------- screenshot ---------------------------------------------------
    if (argc > 1)
    {
        GlowVerbProcessor p; p.prepareToPlay (fs, 512);
        p.setCurrentProgram (argc > 2 ? std::atoi (argv[2]) : 2);
        run (p, [&] (int i) { return (i / 12000) % 2 == 0 ? (rnd.nextFloat() * 2 - 1) * 0.4f : 0.0f; }, nullptr, (int) fs * 2);
        std::unique_ptr<AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (gvui::MainPanel::W, gvui::MainPanel::H);
        MessageManager::getInstance()->runDispatchLoopUntil (800);
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, argc > 3 ? (float) std::atof (argv[3]) : 1.0f);
        File out (argv[1]); out.deleteFile();
        FileOutputStream os (out);
        check (PNGImageFormat().writeImageToStream (img, os), "wrote screenshot");
    }

    std::printf ("\n%s (%d failures)\n", failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED", failures);
    return failures == 0 ? 0 : 1;
}
