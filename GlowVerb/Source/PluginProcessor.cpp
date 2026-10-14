#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;
using AC = dsp::IIR::ArrayCoefficients<float>;

//==============================================================================
namespace
{
    struct Preset
    {
        const char* name;
        int type; float pre, decay, size, diff, damp, mod, early, width;
        float low, high, midF, midG; int sat; float drive, air;
        int src; float duck, thr, rel, mix, out;
    };

    // type: 0 Ambience 1 Room 2 Studio 3 Chamber 4 Plate 5 Hall 6 Cathedral 7 Arena 8 Vintage
    // sat : 0 Clean 1 Warm 2 Tube 3 Tape        src: 0 Input 1 External sidechain
    const Preset presets[] =
    {
        { "Init - Hall",            5, 22,  2.8f, 50, 75, 40, 50, 50, 100, 200, 12000, 1000,  0.0f, 0,  0,  0, 0,  0, -30, 300, 30, 0 },
        { "Vocal Plate",            4, 20,  1.9f, 50, 85, 22, 40, 40, 110, 250, 11000, 2500,  1.5f, 1, 15, 35, 0, 30, -30, 250, 25, 0 },
        { "Ducked Vocal Hall",      5, 30,  2.6f, 50, 75, 40, 45, 45, 110, 220, 10000, 1200, -1.0f, 1, 15, 25, 0, 60, -28, 350, 30, 0 },
        { "Bhakti Hall",            5, 35,  3.4f, 60, 78, 45, 50, 50, 115, 180, 10000,  900,  1.0f, 2, 20, 20, 0, 35, -30, 400, 28, 0 },
        { "Big Cathedral",          6, 40,  7.5f, 60, 80, 55, 50, 40, 120, 150,  9000,  800,  0.0f, 0,  0, 15, 0,  0, -30, 300, 35, 0 },
        { "Drum Room",              1,  3,  0.8f, 45, 65, 35, 30, 70, 100, 150, 14000, 3000,  1.5f, 3, 35, 20, 0,  0, -30, 200, 22, 0 },
        { "Snare Plate",            4, 12,  1.6f, 45, 85, 15, 35, 30, 100, 300, 13000, 3500,  2.0f, 2, 25, 40, 0,  0, -30, 200, 25, 0 },
        { "Studio Live",            2,  8,  1.1f, 50, 62, 35, 40, 60, 100, 200, 13000, 1500,  0.0f, 0,  0, 15, 0,  0, -30, 200, 20, 0 },
        { "Warm Chamber",           3, 14,  1.9f, 50, 72, 50, 45, 55, 100, 160,  9000,  600,  1.5f, 3, 30, 10, 0,  0, -30, 300, 25, 0 },
        { "Tight Ambience",         0,  0,  0.5f, 45, 60, 45, 30, 90, 110, 200, 14000, 2000,  0.0f, 0,  0, 20, 0,  0, -30, 150, 18, 0 },
        { "Arena Clap",             7, 60,  3.8f, 50, 60, 45, 40, 70, 110, 300, 11000, 2000,  1.0f, 1, 20, 25, 0,  0, -30, 300, 30, 0 },
        { "Vintage 80s",            8, 20,  2.6f, 50, 70, 50, 70, 40, 130, 250,  8000, 1500,  2.0f, 3, 40, 10, 0,  0, -30, 300, 30, 0 },
        { "Sidechain Pump Verb",    5, 10,  3.0f, 55, 75, 40, 50, 40, 110, 250, 11000, 1000,  0.0f, 1, 10, 20, 1, 80, -35, 220, 40, 0 },
        { "Ambient Pad Wash",       6, 50, 12.0f, 75, 85, 50, 90, 30, 140, 200,  8000,  700, -2.0f, 1, 15, 30, 0,  0, -30, 300, 50, 0 },
    };
}

//==============================================================================
AudioProcessorValueTreeState::ParameterLayout GlowVerbProcessor::createLayout()
{
    using F = AudioParameterFloat;
    using A = AudioParameterFloatAttributes;
    std::vector<std::unique_ptr<RangedAudioParameter>> v;

    auto pct = A().withLabel ("%").withStringFromValueFunction ([] (float x, int) { return String (roundToInt (x)); });
    auto db  = A().withLabel ("dB").withStringFromValueFunction ([] (float x, int) { return (x > 0.05f ? "+" : "") + String (x, 1); });
    auto ms  = A().withLabel ("ms").withStringFromValueFunction ([] (float x, int) { return String (roundToInt (x)) + " ms"; });
    auto hz  = [] (bool offLow, bool offHigh)
    {
        return A().withLabel ("Hz").withStringFromValueFunction ([=] (float x, int)
        {
            if (offLow && x <= 20.5f) return String ("OFF");
            if (offHigh && x >= 19900.0f) return String ("OFF");
            return x >= 1000.0f ? String (x / 1000.0f, x >= 10000.0f ? 1 : 2) + " k" : String (roundToInt (x)) + " Hz";
        });
    };
    auto range = [] (float lo, float hi, float centre)
    {
        NormalisableRange<float> r (lo, hi);
        r.setSkewForCentre (centre);
        return r;
    };

    StringArray types;
    for (auto& t : gv::envTypes()) types.add (String (t.name).toLowerCase().replaceSection (0, 1, String (t.name).substring (0, 1)));

    v.push_back (std::make_unique<AudioParameterChoice> (ParameterID { IDs::type, 1 }, "Environment", types, 5));
    v.push_back (std::make_unique<F> (ParameterID { IDs::predelay, 1 }, "Pre-Delay", range (0.0f, 250.0f, 40.0f), 22.0f, ms));
    v.push_back (std::make_unique<F> (ParameterID { IDs::decay, 1 }, "Decay", range (0.1f, 20.0f, 2.5f), 2.8f,
                                      A().withLabel ("s").withStringFromValueFunction ([] (float x, int) { return String (x, x < 10.0f ? 2 : 1) + " s"; })));
    v.push_back (std::make_unique<F> (ParameterID { IDs::size, 1 }, "Size", NormalisableRange<float> (0, 100), 50.0f, pct));
    v.push_back (std::make_unique<F> (ParameterID { IDs::diffusion, 1 }, "Diffusion", NormalisableRange<float> (0, 100), 75.0f, pct));
    v.push_back (std::make_unique<F> (ParameterID { IDs::damping, 1 }, "Damping", NormalisableRange<float> (0, 100), 40.0f, pct));
    v.push_back (std::make_unique<F> (ParameterID { IDs::mod, 1 }, "Modulation", NormalisableRange<float> (0, 100), 50.0f, pct));
    v.push_back (std::make_unique<F> (ParameterID { IDs::early, 1 }, "Early Reflections", NormalisableRange<float> (0, 100), 50.0f, pct));
    v.push_back (std::make_unique<F> (ParameterID { IDs::width, 1 }, "Width", NormalisableRange<float> (0, 150), 100.0f, pct));

    v.push_back (std::make_unique<F> (ParameterID { IDs::lowCut, 1 }, "Low Cut", range (20.0f, 1000.0f, 150.0f), 200.0f, hz (true, false)));
    v.push_back (std::make_unique<F> (ParameterID { IDs::highCut, 1 }, "High Cut", range (1000.0f, 20000.0f, 6000.0f), 12000.0f, hz (false, true)));
    v.push_back (std::make_unique<F> (ParameterID { IDs::midFreq, 1 }, "Mid Freq", range (200.0f, 8000.0f, 1200.0f), 1000.0f, hz (false, false)));
    v.push_back (std::make_unique<F> (ParameterID { IDs::midGain, 1 }, "Mid Gain", NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f, db));

    v.push_back (std::make_unique<AudioParameterChoice> (ParameterID { IDs::satType, 1 }, "Saturation", StringArray { "Clean", "Warm", "Tube", "Tape" }, 0));
    v.push_back (std::make_unique<F> (ParameterID { IDs::drive, 1 }, "Drive", NormalisableRange<float> (0, 100), 0.0f, pct));
    v.push_back (std::make_unique<F> (ParameterID { IDs::air, 1 }, "Air", NormalisableRange<float> (0, 100), 0.0f, pct));

    v.push_back (std::make_unique<AudioParameterChoice> (ParameterID { IDs::scSource, 1 }, "Duck Source", StringArray { "Input", "Sidechain" }, 0));
    v.push_back (std::make_unique<F> (ParameterID { IDs::duck, 1 }, "Duck", NormalisableRange<float> (0, 100), 0.0f, pct));
    v.push_back (std::make_unique<F> (ParameterID { IDs::threshold, 1 }, "Duck Threshold", NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -30.0f, db));
    v.push_back (std::make_unique<F> (ParameterID { IDs::release, 1 }, "Duck Release", range (20.0f, 1500.0f, 250.0f), 300.0f, ms));

    v.push_back (std::make_unique<F> (ParameterID { IDs::mix, 1 }, "Mix", NormalisableRange<float> (0, 100), 30.0f, pct));
    v.push_back (std::make_unique<F> (ParameterID { IDs::output, 1 }, "Output", NormalisableRange<float> (-18.0f, 12.0f, 0.1f), 0.0f, db));

    return { v.begin(), v.end() };
}

//==============================================================================
GlowVerbProcessor::GlowVerbProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",     AudioChannelSet::stereo(), true)
                        .withOutput ("Output",    AudioChannelSet::stereo(), true)
                        .withInput  ("Sidechain", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    const char* ids[] = { IDs::type, IDs::predelay, IDs::decay, IDs::size, IDs::diffusion, IDs::damping, IDs::mod,
                          IDs::early, IDs::width, IDs::lowCut, IDs::highCut, IDs::midFreq, IDs::midGain, IDs::satType,
                          IDs::drive, IDs::air, IDs::scSource, IDs::duck, IDs::threshold, IDs::release, IDs::mix, IDs::output };
    for (int i = 0; i < 22; ++i)
        p[i] = apvts.getRawParameterValue (ids[i]);
}

bool GlowVerbProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != AudioChannelSet::mono() && out != AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != out)
        return false;
    if (layouts.inputBuses.size() > 1)
    {
        const auto sc = layouts.getChannelSet (true, 1);
        if (! sc.isDisabled() && sc != AudioChannelSet::mono() && sc != AudioChannelSet::stereo())
            return false;
    }
    return true;
}

//==============================================================================
void GlowVerbProcessor::prepareToPlay (double sampleRate, int)
{
    fs = sampleRate;
    reverb.prepare (sampleRate);
    for (auto* f : { &lowCut1, &lowCut2, &highCut1, &highCut2, &midBell, &airHp, &airShelf })
        f->prepare();
    lastLow = lastHigh = lastMidF = lastAir = -1.0f; lastMidG = -100.0f;
    updateFilters();

    mixSm.reset (fs, 0.05);   mixSm.setCurrentAndTargetValue (p[pMix]->load() * 0.01f);
    outSm.reset (fs, 0.05);   outSm.setCurrentAndTargetValue (Decibels::decibelsToGain (p[pOut]->load()));
    driveSm.reset (fs, 0.05); driveSm.setCurrentAndTargetValue (p[pDrive]->load() * 0.01f);
    airSm.reset (fs, 0.05);   airSm.setCurrentAndTargetValue (p[pAir]->load() * 0.01f);
    widthSm.reset (fs, 0.05); widthSm.setCurrentAndTargetValue (p[pWidth]->load() * 0.01f);
    duckEnv = 0.0f; duckGainDb = 0.0f;
}

void GlowVerbProcessor::updateFilters()
{
    const float lo = p[pLow]->load(), hi = p[pHigh]->load(), mf = p[pMidF]->load(), mg = p[pMidG]->load();
    const float air = p[pAir]->load();
    const float nyq = (float) fs * 0.45f;

    if (std::abs (lo - lastLow) > 0.01f)
    {
        lastLow = lo;   // 24 dB/oct Butterworth
        lowCut1.set (AC::makeHighPass (fs, lo, 0.5412f));
        lowCut2.set (AC::makeHighPass (fs, lo, 1.3066f));
    }
    if (std::abs (hi - lastHigh) > 0.01f)
    {
        lastHigh = hi;
        const float f = jmin (hi, nyq);
        highCut1.set (AC::makeLowPass (fs, f, 0.5412f));
        highCut2.set (AC::makeLowPass (fs, f, 1.3066f));
    }
    if (std::abs (mf - lastMidF) > 0.01f || std::abs (mg - lastMidG) > 0.01f)
    {
        lastMidF = mf; lastMidG = mg;
        midBell.set (AC::makePeakFilter (fs, jmin (mf, nyq), 0.8f, Decibels::decibelsToGain (mg)));
    }
    if (std::abs (air - lastAir) > 0.01f)
    {
        lastAir = air;
        airHp.set (AC::makeHighPass (fs, jmin (4500.0f, nyq), 0.7f));
        airShelf.set (AC::makeHighShelf (fs, jmin (10000.0f, nyq), 0.7f, Decibels::decibelsToGain (air * 0.06f)));
    }
}

//==============================================================================
void GlowVerbProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    auto mainIn  = getBusBuffer (buffer, true, 0);
    auto mainOut = getBusBuffer (buffer, false, 0);
    const int n = buffer.getNumSamples();
    const int numCh = jmin (2, mainOut.getNumChannels());
    if (numCh == 0 || n == 0) return;

    // sidechain
    const float* scPtr[2] = { nullptr, nullptr };
    int scCh = 0;
    if (getBusCount (true) > 1)
        if (auto* bus = getBus (true, 1); bus != nullptr && bus->isEnabled())
        {
            auto sc = getBusBuffer (buffer, true, 1);
            scCh = jmin (2, sc.getNumChannels());
            for (int c = 0; c < scCh; ++c) scPtr[c] = sc.getReadPointer (c);
        }
    sidechainConnected.store (scCh > 0);

    // parameters
    updateFilters();
    gv::Reverb::Params rp;
    rp.type = (int) p[pType]->load();
    rp.predelayMs = p[pPre]->load(); rp.decay = p[pDecay]->load(); rp.size = p[pSize]->load();
    rp.diffusion = p[pDiff]->load(); rp.damping = p[pDamp]->load(); rp.mod = p[pMod]->load(); rp.early = p[pEarly]->load();
    reverb.setParams (rp);

    mixSm.setTargetValue (p[pMix]->load() * 0.01f);
    outSm.setTargetValue (Decibels::decibelsToGain (p[pOut]->load()));
    driveSm.setTargetValue (p[pDrive]->load() * 0.01f);
    airSm.setTargetValue (p[pAir]->load() * 0.01f);
    widthSm.setTargetValue (p[pWidth]->load() * 0.01f);

    const int satType = (int) p[pSat]->load();
    const bool useExt = (int) p[pScSrc]->load() == 1 && scCh > 0;
    const float duckDepthDb = p[pDuck]->load() * 0.30f;          // up to 30 dB
    const float thrDb = p[pThr]->load();
    const float relCoef = std::exp (-1.0f / (0.001f * p[pRel]->load() * (float) fs));
    const float attCoef = std::exp (-1.0f / (0.004f * (float) fs));

    float* L = mainOut.getWritePointer (0);
    float* R = numCh > 1 ? mainOut.getWritePointer (1) : nullptr;
    const float* inL = mainIn.getReadPointer (0);
    const float* inR = mainIn.getNumChannels() > 1 ? mainIn.getReadPointer (1) : inL;

    float inSq = 0, outSq = 0, wetSq = 0, maxDuck = 0;

    for (int i = 0; i < n; ++i)
    {
        const float dl = inL[i];
        const float dr = numCh > 1 ? inR[i] : dl;

        float wl, wr;
        reverb.process (dl, dr, wl, wr);

        // EQ
        for (int c = 0; c < 2; ++c)
        {
            float& w = c == 0 ? wl : wr;
            w = lowCut2.process (c, lowCut1.process (c, w));
            w = highCut2.process (c, highCut1.process (c, w));
            w = midBell.process (c, w);
        }

        // colour: saturation + air
        const float drive = driveSm.getNextValue();
        const float air = airSm.getNextValue();
        if (satType > 0 && drive > 0.001f)
        {
            const float g = Decibels::decibelsToGain (drive * 18.0f);
            for (int c = 0; c < 2; ++c)
            {
                float& w = c == 0 ? wl : wr;
                const float x = w * g * 1.5f;
                const float y = satType == 1 ? gv::shapers::warm (x) : satType == 2 ? gv::shapers::tube (x) : gv::shapers::tape (x);
                w = y / (1.5f * std::sqrt (g));        // partial level compensation
            }
        }
        for (int c = 0; c < 2; ++c)
        {
            float& w = c == 0 ? wl : wr;
            const float h = airHp.process (c, w);
            w += air * 0.8f * std::tanh (h * 3.0f) / 3.0f * 3.0f;   // added "sparkle" harmonics
            w = airShelf.process (c, w);
        }

        // width
        const float width = widthSm.getNextValue();
        const float mid = 0.5f * (wl + wr), side = 0.5f * (wl - wr) * width;
        wl = mid + side; wr = mid - side;

        // ducking
        float det;
        if (useExt) det = jmax (std::abs (scPtr[0][i]), scCh > 1 ? std::abs (scPtr[1][i]) : 0.0f);
        else        det = jmax (std::abs (dl), std::abs (dr));
        duckEnv = det > duckEnv ? attCoef * duckEnv + (1.0f - attCoef) * det : relCoef * duckEnv + (1.0f - relCoef) * det;
        float duckDb = 0.0f;
        if (duckDepthDb > 0.01f)
        {
            const float lvl = Decibels::gainToDecibels (duckEnv, -100.0f);
            duckDb = jlimit (0.0f, 1.0f, (lvl - thrDb) / 12.0f) * duckDepthDb;
        }
        duckGainDb += 0.01f * (duckDb - duckGainDb);
        const float dg = Decibels::decibelsToGain (-duckGainDb);
        wl *= dg; wr *= dg;
        maxDuck = jmax (maxDuck, duckGainDb);

        // mix (equal power) + output
        const float m = mixSm.getNextValue();
        const float dryG = std::cos (m * MathConstants<float>::halfPi);
        const float wetG = std::sin (m * MathConstants<float>::halfPi);
        const float o = outSm.getNextValue();
        const float yl = (dl * dryG + wl * wetG) * o;
        const float yr = (dr * dryG + wr * wetG) * o;

        inSq = jmax (inSq, dl * dl);
        wetSq += wl * wl;
        outSq += yl * yl;

        if (R != nullptr) { L[i] = yl; R[i] = yr; }
        else              { L[i] = 0.5f * (yl + yr); }
    }

    for (int c = numCh; c < mainOut.getNumChannels(); ++c)
        mainOut.clear (c, 0, n);

    meterIn.store (Decibels::gainToDecibels (std::sqrt (inSq), -100.0f));
    meterWet.store (Decibels::gainToDecibels (std::sqrt (wetSq / (float) n), -100.0f));
    meterOut.store (Decibels::gainToDecibels (std::sqrt (outSq / (float) n), -100.0f));
    meterDuck.store (maxDuck);
}

//==============================================================================
void GlowVerbProcessor::setParam (const char* id, float value)
{
    if (auto* param = apvts.getParameter (id))
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost (param->convertTo0to1 (value));
        param->endChangeGesture();
    }
}

void GlowVerbProcessor::applyEnvironment (int typeIndex)
{
    const auto& t = gv::envTypes()[(size_t) jlimit (0, 8, typeIndex)];
    setParam (IDs::type, (float) typeIndex);
    setParam (IDs::decay, t.defDecay);
    setParam (IDs::predelay, t.defPredelay);
    setParam (IDs::size, t.defSize);
    setParam (IDs::diffusion, t.defDiffusion);
    setParam (IDs::damping, t.defDamping);
}

int GlowVerbProcessor::getNumPrograms() { return (int) std::size (presets); }

const String GlowVerbProcessor::getProgramName (int index)
{
    return isPositiveAndBelow (index, getNumPrograms()) ? String (presets[index].name) : String();
}

void GlowVerbProcessor::setCurrentProgram (int index)
{
    if (! isPositiveAndBelow (index, getNumPrograms())) return;
    currentProgram = index;
    const auto& s = presets[index];
    setParam (IDs::type, (float) s.type);   setParam (IDs::predelay, s.pre);  setParam (IDs::decay, s.decay);
    setParam (IDs::size, s.size);           setParam (IDs::diffusion, s.diff); setParam (IDs::damping, s.damp);
    setParam (IDs::mod, s.mod);             setParam (IDs::early, s.early);   setParam (IDs::width, s.width);
    setParam (IDs::lowCut, s.low);          setParam (IDs::highCut, s.high);  setParam (IDs::midFreq, s.midF);
    setParam (IDs::midGain, s.midG);        setParam (IDs::satType, (float) s.sat); setParam (IDs::drive, s.drive);
    setParam (IDs::air, s.air);             setParam (IDs::scSource, (float) s.src); setParam (IDs::duck, s.duck);
    setParam (IDs::threshold, s.thr);       setParam (IDs::release, s.rel);   setParam (IDs::mix, s.mix);
    setParam (IDs::output, s.out);
}

void GlowVerbProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void GlowVerbProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = ValueTree::fromXml (*xml);
            currentProgram = (int) tree.getProperty ("program", 0);
            apvts.replaceState (tree);
        }
}

AudioProcessorEditor* GlowVerbProcessor::createEditor() { return new GlowVerbEditor (*this); }

#if ! GV_TEST_BUILD
AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GlowVerbProcessor(); }
#endif
