#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
namespace
{
    struct Preset
    {
        const char* name;
        int mode; float peak, gain; bool limit; float hpf, warmth, bite; int sat; float drive, satMix, mix, out;
    };

    // mode: 0 Smooth / 1 Punch     sat: 0 Clean 1 Warm 2 Triode 3 Retro 4 Tape
    const Preset presets[] =
    {
        { "Init - Glow",          0, 35.0f, 3.0f, false,  20.0f, 25.0f, 25.0f, 1, 20.0f, 100.0f, 100.0f,  0.0f },
        { "Vocal Silk",           0, 50.0f, 6.0f, false,  80.0f, 35.0f, 30.0f, 1, 25.0f, 100.0f, 100.0f,  0.0f },
        { "Vocal Bite",           1, 45.0f, 5.0f, false, 100.0f, 20.0f, 60.0f, 2, 30.0f, 100.0f, 100.0f,  0.0f },
        { "Devotional Lead",      0, 55.0f, 7.0f, false,  90.0f, 45.0f, 40.0f, 2, 22.0f,  80.0f, 100.0f, -1.0f },
        { "Drum Bus Glue",        1, 40.0f, 3.0f, false,  90.0f, 30.0f, 35.0f, 4, 35.0f, 100.0f,  70.0f,  0.0f },
        { "Bass Thick",           0, 55.0f, 6.0f, true,   20.0f, 60.0f, 10.0f, 2, 40.0f, 100.0f, 100.0f, -1.0f },
        { "Retro Radio",          1, 60.0f, 6.0f, false, 150.0f, 10.0f, 40.0f, 3, 55.0f, 100.0f, 100.0f, -2.0f },
        { "Parallel Smash",       1, 80.0f, 10.0f, true,  60.0f, 25.0f, 30.0f, 4, 40.0f, 100.0f,  40.0f,  0.0f },
        { "Mix Bus Warm",         0, 20.0f, 1.5f, false,  60.0f, 30.0f, 20.0f, 4, 20.0f, 100.0f, 100.0f,  0.0f },
        { "Acoustic Gentle",      0, 30.0f, 3.0f, false,  70.0f, 25.0f, 35.0f, 0,  0.0f, 100.0f, 100.0f,  0.0f },
    };
}

//==============================================================================
const StringArray& BKPOptoGlowProcessor::satTypeNames()
{
    static const StringArray names { "CLEAN", "WARM", "TRIODE", "RETRO", "TAPE" };
    return names;
}

AudioProcessorValueTreeState::ParameterLayout BKPOptoGlowProcessor::createLayout()
{
    using F = AudioParameterFloat;
    using Attr = AudioParameterFloatAttributes;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto pct = Attr().withLabel ("%").withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v)); });
    auto db  = Attr().withLabel ("dB").withStringFromValueFunction ([] (float v, int)
                                       { return (v > 0.05f ? "+" : "") + String (v, 1); });

    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::mode, 1 }, "Mode",
                                                         StringArray { "Smooth", "Punch" }, 0));
    p.push_back (std::make_unique<F> (ParameterID { ParamIDs::peak, 1 }, "Peak Reduction",
                                      NormalisableRange<float> (0.0f, 100.0f, 0.1f), 35.0f,
                                      Attr().withStringFromValueFunction ([] (float v, int) { return String (v, 1); })));
    p.push_back (std::make_unique<F> (ParameterID { ParamIDs::gain, 1 }, "Gain",
                                      NormalisableRange<float> (-6.0f, 24.0f, 0.1f), 3.0f, db));
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { ParamIDs::limit, 1 }, "Limit", false));

    NormalisableRange<float> hpfRange (20.0f, 300.0f, 1.0f);
    hpfRange.setSkewForCentre (80.0f);
    p.push_back (std::make_unique<F> (ParameterID { ParamIDs::scHpf, 1 }, "Sidechain HPF", hpfRange, 20.0f,
                                      Attr().withLabel ("Hz").withStringFromValueFunction ([] (float v, int)
                                          { return v <= 20.5f ? String ("OFF") : String (roundToInt (v)) + " Hz"; })));

    p.push_back (std::make_unique<F> (ParameterID { ParamIDs::warmth, 1 }, "Warmth",
                                      NormalisableRange<float> (0.0f, 100.0f, 0.1f), 25.0f, pct));
    p.push_back (std::make_unique<F> (ParameterID { ParamIDs::bite, 1 }, "Bite",
                                      NormalisableRange<float> (0.0f, 100.0f, 0.1f), 25.0f, pct));

    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::satType, 1 }, "Saturation Type",
                                                         StringArray { "Clean", "Warm", "Triode", "Retro", "Tape" }, 1));
    p.push_back (std::make_unique<F> (ParameterID { ParamIDs::drive, 1 }, "Drive",
                                      NormalisableRange<float> (0.0f, 100.0f, 0.1f), 20.0f, pct));
    p.push_back (std::make_unique<F> (ParameterID { ParamIDs::satMix, 1 }, "Saturation Mix",
                                      NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, pct));
    p.push_back (std::make_unique<F> (ParameterID { ParamIDs::mix, 1 }, "Mix",
                                      NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, pct));
    p.push_back (std::make_unique<F> (ParameterID { ParamIDs::output, 1 }, "Output",
                                      NormalisableRange<float> (-18.0f, 12.0f, 0.1f), 0.0f, db));

    return { p.begin(), p.end() };
}

//==============================================================================
BKPOptoGlowProcessor::BKPOptoGlowProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    pMode    = apvts.getRawParameterValue (ParamIDs::mode);
    pPeak    = apvts.getRawParameterValue (ParamIDs::peak);
    pGain    = apvts.getRawParameterValue (ParamIDs::gain);
    pLimit   = apvts.getRawParameterValue (ParamIDs::limit);
    pScHpf   = apvts.getRawParameterValue (ParamIDs::scHpf);
    pWarmth  = apvts.getRawParameterValue (ParamIDs::warmth);
    pBite    = apvts.getRawParameterValue (ParamIDs::bite);
    pSatType = apvts.getRawParameterValue (ParamIDs::satType);
    pDrive   = apvts.getRawParameterValue (ParamIDs::drive);
    pSatMix  = apvts.getRawParameterValue (ParamIDs::satMix);
    pMix     = apvts.getRawParameterValue (ParamIDs::mix);
    pOutput  = apvts.getRawParameterValue (ParamIDs::output);
}

bool BKPOptoGlowProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != AudioChannelSet::mono() && out != AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

//==============================================================================
void BKPOptoGlowProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fs = sampleRate;

    oversampler = std::make_unique<dsp::Oversampling<float>> (
        2, 2, dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true);
    oversampler->initProcessing ((size_t) jmax (1, samplesPerBlock));
    osFs = fs * (double) oversampler->getOversamplingFactor();

    const float latency = oversampler->getLatencyInSamples();
    dryDelay.prepare ({ fs, (uint32) samplesPerBlock, 2 });
    dryDelay.setMaximumDelayInSamples (256);
    dryDelay.setDelay (latency);
    dryDelay.reset();
    setLatencySamples (roundToInt (latency));

    maxBlockSize = jmax (1, samplesPerBlock);
    dryBuffer.setSize (2, maxBlockSize, false, true, false);

    comp.prepare (fs);

    for (auto* f : { &modeEq1, &modeEq2, &warmShelf, &warmBody, &biteBell, &biteShelf,
                     &retroHp, &retroLp, &retroMid, &tapeBump, &tapeTop })
        f->prepare();

    // fixed type filters at oversampled rate
    retroHp.set  (bkp::ArrCoeffs::makeHighPass (osFs, 110.0f, 0.75f));
    retroLp.set  (bkp::ArrCoeffs::makeLowPass  (osFs, 6200.0f, 0.8f));
    retroMid.set (bkp::ArrCoeffs::makePeakFilter (osFs, 1400.0f, 0.8f, Decibels::decibelsToGain (3.5f)));
    tapeBump.set (bkp::ArrCoeffs::makePeakFilter (osFs, 75.0f, 1.0f, Decibels::decibelsToGain (1.8f)));
    tapeTop.set  (bkp::ArrCoeffs::makeHighShelf (osFs, 11000.0f, 0.7f, Decibels::decibelsToGain (-2.5f)));

    dcR = (float) (1.0 - MathConstants<double>::twoPi * 8.0 / osFs);
    for (int c = 0; c < 2; ++c) dcX1[c] = dcY1[c] = 0.0f;

    lastMode = -1; lastSatType = -1; lastWarmth = lastBite = -1.0f;
    updateFilters();

    makeupSm.reset (fs, 0.05);  makeupSm.setCurrentAndTargetValue (Decibels::decibelsToGain (pGain->load()));
    mixSm.reset (fs, 0.05);     mixSm.setCurrentAndTargetValue (pMix->load() * 0.01f);
    outSm.reset (fs, 0.05);     outSm.setCurrentAndTargetValue (Decibels::decibelsToGain (pOutput->load()));
    driveSm.reset (osFs, 0.05); driveSm.setCurrentAndTargetValue (Decibels::decibelsToGain (pDrive->load() * 0.24f));
    satMixSm.reset (osFs, 0.05); satMixSm.setCurrentAndTargetValue (pSatMix->load() * 0.01f);
}

void BKPOptoGlowProcessor::updateFilters()
{
    const int mode = (int) pMode->load();
    if (mode != lastMode)
    {
        lastMode = mode;
        if (mode == 0) // Smooth: rounder lows, silky top
        {
            modeEq1.set (bkp::ArrCoeffs::makeLowShelf  (fs, 110.0f, 0.7f, Decibels::decibelsToGain (0.8f)));
            modeEq2.set (bkp::ArrCoeffs::makeHighShelf (fs, 10000.0f, 0.7f, Decibels::decibelsToGain (-1.2f)));
        }
        else           // Punch: forward upper mids
        {
            modeEq1.set (bkp::ArrCoeffs::makePeakFilter (fs, 2800.0f, 0.9f, Decibels::decibelsToGain (1.5f)));
            modeEq2.set (bkp::ArrCoeffs::makeLowShelf   (fs, 90.0f, 0.7f, Decibels::decibelsToGain (-0.5f)));
        }
    }

    const float w = pWarmth->load();
    if (std::abs (w - lastWarmth) > 0.01f)
    {
        lastWarmth = w;
        warmShelf.set (bkp::ArrCoeffs::makeLowShelf  (fs, 140.0f, 0.7f, Decibels::decibelsToGain (w * 0.05f)));
        warmBody.set  (bkp::ArrCoeffs::makePeakFilter (fs, 380.0f, 0.8f, Decibels::decibelsToGain (w * 0.018f)));
    }

    const float b = pBite->load();
    if (std::abs (b - lastBite) > 0.01f)
    {
        lastBite = b;
        biteBell.set  (bkp::ArrCoeffs::makePeakFilter (fs, 3200.0f, 1.1f, Decibels::decibelsToGain (b * 0.025f)));
        biteShelf.set (bkp::ArrCoeffs::makeHighShelf  (fs, 5500.0f, 0.7f, Decibels::decibelsToGain (b * 0.05f)));
    }
}

//==============================================================================
float BKPOptoGlowProcessor::processSaturation (int type, int ch, float x, float g) noexcept
{
    using namespace bkp::shapers;
    switch (type)
    {
        case 1: { const float h = 0.40f * g; return warm (h * x) / h; }
        case 2: { const float h = 0.35f * g; return triode (h * x) / h; }
        case 3:
        {
            float y = retroMid.process (ch, retroLp.process (ch, retroHp.process (ch, x)));
            const float h = 0.50f * g;
            return retro (h * y) / h;
        }
        case 4:
        {
            const float h = 0.45f * g;
            float y = tape (h * x) / h;
            return tapeTop.process (ch, tapeBump.process (ch, y));
        }
        default: return x;
    }
}

void BKPOptoGlowProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    // Hosts may occasionally send bigger blocks than announced: split them up.
    const int total = buffer.getNumSamples();
    if (total > maxBlockSize)
    {
        for (int start = 0; start < total; start += maxBlockSize)
        {
            const int len = jmin (maxBlockSize, total - start);
            AudioBuffer<float> slice (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), start, len);
            processChunk (slice);
        }
        return;
    }
    processChunk (buffer);
}

void BKPOptoGlowProcessor::processChunk (AudioBuffer<float>& buffer)
{

    const int numCh = jmin (2, getTotalNumInputChannels());
    const int n = buffer.getNumSamples();
    for (int c = numCh; c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, n);
    if (numCh == 0 || n == 0)
        return;

    // ---- parameters -----------------------------------------------------
    updateFilters();
    comp.setParams ((int) pMode->load(), pPeak->load(), pLimit->load() > 0.5f, pScHpf->load());
    makeupSm.setTargetValue (Decibels::decibelsToGain (pGain->load()));
    mixSm.setTargetValue (pMix->load() * 0.01f);
    outSm.setTargetValue (Decibels::decibelsToGain (pOutput->load()));
    driveSm.setTargetValue (Decibels::decibelsToGain (pDrive->load() * 0.24f));
    satMixSm.setTargetValue (pSatMix->load() * 0.01f);

    const int satType = (int) pSatType->load();
    if (satType != lastSatType)
    {
        lastSatType = satType;
        for (auto* f : { &retroHp, &retroLp, &retroMid, &tapeBump, &tapeTop })
            f->reset();
    }

    // ---- input meter ----------------------------------------------------
    float inSq = 0.0f;
    for (int c = 0; c < numCh; ++c)
    {
        const float r = buffer.getRMSLevel (c, 0, n);
        inSq = jmax (inSq, r * r);
    }

    // ---- compressor + colour + tone (base rate) -------------------------
    const bool smooth = comp.getMode() == bkp::OptoCompressor::Smooth;
    float maxGr = 0.0f;
    float* ch[2] = { buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr };
    float* dry[2] = { dryBuffer.getWritePointer (0), dryBuffer.getWritePointer (1) };

    for (int i = 0; i < n; ++i)
    {
        float frame[2];
        for (int c = 0; c < numCh; ++c)
        {
            frame[c] = ch[c][i];
            dryDelay.pushSample (c, frame[c]);
            dry[c][i] = dryDelay.popSample (c);
        }

        const float g = comp.processFrame (frame, numCh);
        maxGr = jmax (maxGr, comp.getGainReductionDb());
        const float makeup = makeupSm.getNextValue();

        for (int c = 0; c < numCh; ++c)
        {
            float y = frame[c] * g;
            y = smooth ? y + 0.3f * (bkp::shapers::tubeColour (y) - y)
                       : y + 0.3f * (bkp::shapers::transformerColour (y) - y);
            y = modeEq2.process (c, modeEq1.process (c, y));
            y *= makeup;
            y = warmBody.process (c, warmShelf.process (c, y));
            y = biteShelf.process (c, biteBell.process (c, y));
            ch[c][i] = y;
        }
    }

    // ---- saturation (4x oversampled) ------------------------------------
    {
        dsp::AudioBlock<float> block (buffer.getArrayOfWritePointers(), (size_t) numCh, (size_t) n);
        auto up = oversampler->processSamplesUp (block);
        const int upN = (int) up.getNumSamples();

        for (int i = 0; i < upN; ++i)
        {
            const float drive = driveSm.getNextValue();
            const float sm = satMixSm.getNextValue();
            for (int c = 0; c < numCh; ++c)
            {
                float* d = up.getChannelPointer ((size_t) c);
                const float x = d[i];
                float y = satType == 0 ? x : x + sm * (processSaturation (satType, c, x, drive) - x);

                // DC blocker (removes offset from asymmetric shapers)
                const float out = y - dcX1[c] + dcR * dcY1[c];
                dcX1[c] = y; dcY1[c] = out;
                d[i] = out;
            }
        }
        oversampler->processSamplesDown (block);
    }

    // ---- parallel mix + output ------------------------------------------
    float outSq[2] { 0.0f, 0.0f };
    for (int i = 0; i < n; ++i)
    {
        const float m = mixSm.getNextValue();
        const float o = outSm.getNextValue();
        for (int c = 0; c < numCh; ++c)
        {
            const float y = (m * ch[c][i] + (1.0f - m) * dry[c][i]) * o;
            ch[c][i] = y;
            outSq[c] += y * y;
        }
    }

    if (getTotalNumOutputChannels() > 1 && numCh == 1)
        buffer.copyFrom (1, 0, buffer, 0, 0, n);

    const float outMs = jmax (outSq[0], outSq[1]) / (float) n;
    meterGr.store (maxGr);
    meterIn.store (Decibels::gainToDecibels (std::sqrt (inSq), -100.0f));
    meterOut.store (Decibels::gainToDecibels (std::sqrt (outMs), -100.0f));
}

//==============================================================================
int BKPOptoGlowProcessor::getNumPrograms() { return (int) std::size (presets); }

const String BKPOptoGlowProcessor::getProgramName (int index)
{
    return isPositiveAndBelow (index, getNumPrograms()) ? String (presets[index].name) : String();
}

void BKPOptoGlowProcessor::setCurrentProgram (int index)
{
    if (! isPositiveAndBelow (index, getNumPrograms()))
        return;

    currentProgram = index;
    const auto& p = presets[index];

    auto set = [this] (const char* id, float value)
    {
        if (auto* param = apvts.getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (value));
    };

    set (ParamIDs::mode, (float) p.mode);
    set (ParamIDs::peak, p.peak);
    set (ParamIDs::gain, p.gain);
    set (ParamIDs::limit, p.limit ? 1.0f : 0.0f);
    set (ParamIDs::scHpf, p.hpf);
    set (ParamIDs::warmth, p.warmth);
    set (ParamIDs::bite, p.bite);
    set (ParamIDs::satType, (float) p.sat);
    set (ParamIDs::drive, p.drive);
    set (ParamIDs::satMix, p.satMix);
    set (ParamIDs::mix, p.mix);
    set (ParamIDs::output, p.out);
}

//==============================================================================
void BKPOptoGlowProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void BKPOptoGlowProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = ValueTree::fromXml (*xml);
            currentProgram = (int) tree.getProperty ("program", 0);
            apvts.replaceState (tree);
        }
}

AudioProcessorEditor* BKPOptoGlowProcessor::createEditor()
{
    return new BKPOptoGlowEditor (*this);
}

#if ! BKP_TEST_BUILD
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BKPOptoGlowProcessor();
}
#endif
