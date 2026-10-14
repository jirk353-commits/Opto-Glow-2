// Offline test runner: DSP sanity checks + UI screenshot. Not part of the plugin.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cstdio>

using namespace juce;

static int failures = 0;
static void check (bool ok, const String& what)
{
    std::printf ("[%s] %s\n", ok ? " OK " : "FAIL", what.toRawUTF8());
    if (! ok) ++failures;
}

static void setP (BKPOptoGlowProcessor& p, const char* id, float v)
{
    auto* param = p.apvts.getParameter (id);
    param->setValueNotifyingHost (param->convertTo0to1 (v));
}

struct RunResult { float outRms = 0, peak = 0, gr = 0; bool finite = true; std::vector<float> tail; };

static RunResult run (BKPOptoGlowProcessor& p, std::function<float (int)> gen, int numSamples, int block = 512, int tailLen = 0)
{
    RunResult r;
    AudioBuffer<float> buf (2, block);
    MidiBuffer midi;
    double sum = 0; int count = 0;
    for (int pos = 0; pos < numSamples; pos += block)
    {
        const int n = jmin (block, numSamples - pos);
        AudioBuffer<float> b (buf.getArrayOfWritePointers(), 2, 0, n);
        for (int i = 0; i < n; ++i) { const float x = gen (pos + i); b.setSample (0, i, x); b.setSample (1, i, x); }
        p.processBlock (b, midi);
        for (int i = 0; i < n; ++i)
        {
            const float y = b.getSample (0, i);
            if (! std::isfinite (y)) r.finite = false;
            r.peak = jmax (r.peak, std::abs (y));
            if (pos + i >= numSamples / 2) { sum += (double) y * y; ++count; }
            if (pos + i >= numSamples - tailLen) r.tail.push_back (y);
        }
    }
    r.outRms = (float) std::sqrt (sum / jmax (1, count));
    r.gr = p.meterGr.load();
    return r;
}

static float harmonicDb (const std::vector<float>& x, double fs, double f)
{
    double re = 0, im = 0;
    for (size_t i = 0; i < x.size(); ++i)
    {
        const double w = 0.5 - 0.5 * std::cos (MathConstants<double>::twoPi * (double) i / (double) x.size());
        const double ph = MathConstants<double>::twoPi * f * (double) i / fs;
        re += x[i] * w * std::cos (ph); im += x[i] * w * std::sin (ph);
    }
    return (float) Decibels::gainToDecibels (std::sqrt (re * re + im * im) / (double) x.size() * 4.0, -200.0);
}

int main (int argc, char** argv)
{
    ScopedJuceInitialiser_GUI init;
    const double fs = 48000.0;
    auto sine = [fs] (float amp, double f) { return [=] (int i) { return amp * (float) std::sin (MathConstants<double>::twoPi * f * i / fs); }; };

    // ---------------- compression amount per mode ----------------
    for (int mode : { 0, 1 })
        for (bool lim : { false, true })
        {
            BKPOptoGlowProcessor p;
            p.prepareToPlay (fs, 512);
            setP (p, ParamIDs::mode, (float) mode); setP (p, ParamIDs::limit, lim ? 1.0f : 0.0f);
            setP (p, ParamIDs::peak, 50.0f); setP (p, ParamIDs::gain, 0.0f); setP (p, ParamIDs::satType, 0);
            setP (p, ParamIDs::warmth, 0); setP (p, ParamIDs::bite, 0);
            p.prepareToPlay (fs, 512);
            auto r = run (p, sine (0.5f, 1000.0), (int) fs * 3);
            const float inRms = 0.5f / std::sqrt (2.0f);
            std::printf ("  mode=%s limit=%d  GR=%.2f dB  out/in=%.2f dB  peak=%.3f\n", mode ? "PUNCH " : "SMOOTH", (int) lim,
                         r.gr, Decibels::gainToDecibels (r.outRms / inRms), r.peak);
            check (r.finite && r.gr > 2.0f && r.gr < 30.0f, String ("compressor engages, mode ") + String (mode) + (lim ? " limit" : ""));
        }

    // ---------------- peak reduction 0 = no compression --------------
    {
        BKPOptoGlowProcessor p; p.prepareToPlay (fs, 512);
        setP (p, ParamIDs::peak, 0.0f);
        auto r = run (p, sine (0.25f, 1000.0), (int) fs * 2);
        check (r.gr < 0.3f, "peak reduction 0 gives ~no gain reduction (" + String (r.gr, 2) + " dB)");
    }

    // ---------------- release behaviour ------------------------------
    for (int mode : { 0, 1 })
    {
        BKPOptoGlowProcessor p; p.prepareToPlay (fs, 64);
        setP (p, ParamIDs::mode, (float) mode); setP (p, ParamIDs::peak, 70.0f);
        run (p, sine (0.7f, 500.0), (int) fs * 3, 64);
        const float startGr = p.meterGr.load();
        AudioBuffer<float> b (2, 64); MidiBuffer m;
        int t = 0;
        while (p.meterGr.load() > 1.0f && t < (int) fs * 10) { b.clear(); p.processBlock (b, m); t += 64; }
        std::printf ("  release %s: from %.1f dB to <1 dB in %.0f ms\n", mode ? "PUNCH " : "SMOOTH", startGr, 1000.0 * t / fs);
        check (mode == 0 ? (t > fs * 0.3) : (t < fs * 1.5), String ("release character mode ") + String (mode));
    }

    // ---------------- saturation harmonics ---------------------------
    {
        const char* names[] = { "CLEAN", "WARM", "TRIODE", "RETRO", "TAPE" };
        for (int type = 0; type < 5; ++type)
        {
            BKPOptoGlowProcessor p; p.prepareToPlay (fs, 512);
            setP (p, ParamIDs::peak, 0.0f); setP (p, ParamIDs::gain, 0.0f); setP (p, ParamIDs::warmth, 0); setP (p, ParamIDs::bite, 0);
            setP (p, ParamIDs::satType, (float) type); setP (p, ParamIDs::drive, 70.0f);
            p.prepareToPlay (fs, 512);
            auto r = run (p, sine (0.5f, 1000.0), 48000, 512, 16384);
            const float h1 = harmonicDb (r.tail, fs, 1000), h2 = harmonicDb (r.tail, fs, 2000), h3 = harmonicDb (r.tail, fs, 3000);
            std::printf ("  %-6s fund=%6.1f dB  H2=%6.1f dBc  H3=%6.1f dBc  peak=%.3f\n", names[type], h1, h2 - h1, h3 - h1, r.peak);
            check (r.finite && r.peak < 2.0f, String ("saturation ") + names[type] + " stable");
            if (type == 2) check (h2 - h1 > -40.0f, "triode produces strong 2nd harmonic");
        }
    }

    // ---------------- presets, noise, odd block sizes ----------------
    {
        BKPOptoGlowProcessor p; p.prepareToPlay (fs, 256);
        Random rnd (7);
        bool allOk = true;
        for (int i = 0; i < p.getNumPrograms(); ++i)
        {
            p.setCurrentProgram (i);
            auto r = run (p, [&] (int) { return (rnd.nextFloat() * 2 - 1) * 0.6f; }, 24000, 997);  // bigger than prepared
            std::printf ("  preset %-18s GR=%5.1f dB  outRms=%6.1f dBFS  peak=%.2f\n", p.getProgramName (i).toRawUTF8(), r.gr,
                         Decibels::gainToDecibels (r.outRms), r.peak);
            allOk = allOk && r.finite && r.peak < 4.0f;
        }
        check (allOk, "all presets stable on noise with oversized blocks");
    }

    // ---------------- state save / restore --------------------------
    {
        BKPOptoGlowProcessor a, b;
        a.setCurrentProgram (5);
        MemoryBlock mb; a.getStateInformation (mb);
        b.setStateInformation (mb.getData(), (int) mb.getSize());
        check (std::abs (b.apvts.getRawParameterValue (ParamIDs::warmth)->load() - 60.0f) < 0.01f && b.getCurrentProgram() == 5,
               "state save/restore");
    }

    // ---------------- latency ---------------------------------------
    {
        BKPOptoGlowProcessor p; p.prepareToPlay (fs, 512);
        std::printf ("  reported latency: %d samples\n", p.getLatencySamples());
        // impulse through clean, no compression, 50% mix: dry and wet must line up (single peak)
        setP (p, ParamIDs::peak, 0); setP (p, ParamIDs::satType, 0); setP (p, ParamIDs::warmth, 0); setP (p, ParamIDs::bite, 0);
        setP (p, ParamIDs::gain, 0); setP (p, ParamIDs::mix, 50);
        p.prepareToPlay (fs, 512);
        auto r = run (p, [] (int i) { return i == 100 ? 0.5f : 0.0f; }, 512, 512, 512);
        int argmax = 0; for (int i = 0; i < 512; ++i) if (std::abs (r.tail[(size_t) i]) > std::abs (r.tail[(size_t) argmax])) argmax = i;
        std::printf ("  impulse peak at %d (expected ~%d), value %.3f\n", argmax, 100 + p.getLatencySamples(), r.tail[(size_t) argmax]);
        check (std::abs (argmax - (100 + p.getLatencySamples())) <= 1 && std::abs (r.tail[(size_t) argmax]) > 0.3f, "dry/wet alignment");
    }

    // ---------------- UI screenshot ---------------------------------
    if (argc > 1)
    {
        BKPOptoGlowProcessor p; p.prepareToPlay (fs, 512);
        p.setCurrentProgram (std::atoi (argc > 2 ? argv[2] : "2"));
        run (p, sine (0.6f, 300.0), (int) fs * 2);
        std::unique_ptr<AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (980, 560);
        MessageManager::getInstance()->runDispatchLoopUntil (900);
        const float scale = argc > 3 ? (float) std::atof (argv[3]) : 1.0f;
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, scale);
        File out (argv[1]);
        out.deleteFile();
        FileOutputStream os (out);
        PNGImageFormat png;
        check (png.writeImageToStream (img, os), "wrote UI screenshot " + out.getFullPathName());
    }

    std::printf ("\n%s (%d failures)\n", failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED", failures);
    return failures == 0 ? 0 : 1;
}
