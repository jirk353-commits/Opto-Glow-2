#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "ReverbDSP.h"

namespace IDs
{
    inline constexpr auto type      = "type";
    inline constexpr auto predelay  = "predelay";
    inline constexpr auto decay     = "decay";
    inline constexpr auto size      = "size";
    inline constexpr auto diffusion = "diffusion";
    inline constexpr auto damping   = "damping";
    inline constexpr auto mod       = "mod";
    inline constexpr auto early     = "early";
    inline constexpr auto width     = "width";
    inline constexpr auto lowCut    = "lowcut";
    inline constexpr auto highCut   = "highcut";
    inline constexpr auto midFreq   = "midfreq";
    inline constexpr auto midGain   = "midgain";
    inline constexpr auto satType   = "sattype";
    inline constexpr auto drive     = "drive";
    inline constexpr auto air       = "air";
    inline constexpr auto scSource  = "scsource";
    inline constexpr auto duck      = "duck";
    inline constexpr auto threshold = "threshold";
    inline constexpr auto release   = "release";
    inline constexpr auto mix       = "mix";
    inline constexpr auto output    = "output";
}

class GlowVerbProcessor : public juce::AudioProcessor
{
public:
    GlowVerbProcessor();
    ~GlowVerbProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Glow Verb"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 20.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    /** Picks an environment and loads its typical decay / size / pre-delay settings. */
    void applyEnvironment (int typeIndex);

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    // metering for the editor
    std::atomic<float> meterIn { -100.0f }, meterOut { -100.0f }, meterWet { -100.0f }, meterDuck { 0.0f };
    std::atomic<bool> sidechainConnected { false };

private:
    void updateFilters();
    void setParam (const char* id, float value);

    gv::Reverb reverb;

    using Biquad = juce::dsp::IIR::Filter<float>;
    using Coeffs = juce::dsp::IIR::Coefficients<float>;
    struct Stereo
    {
        Coeffs::Ptr c { new Coeffs (1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f) };
        Biquad f[2];
        void prepare() { for (auto& x : f) { x.coefficients = c; x.reset(); } }
        void set (const std::array<float, 6>& a) { *c = a; }
        float process (int ch, float x) noexcept { return f[ch].processSample (x); }
    };
    Stereo lowCut1, lowCut2, highCut1, highCut2, midBell, airHp, airShelf;

    juce::SmoothedValue<float> mixSm, outSm, driveSm, airSm, widthSm;
    float duckEnv = 0.0f, duckGainDb = 0.0f;
    double fs = 48000.0;
    float lastLow = -1, lastHigh = -1, lastMidF = -1, lastMidG = -100, lastAir = -1;
    int currentProgram = 0;

    std::atomic<float>* p[22] {};
    enum PIdx { pType, pPre, pDecay, pSize, pDiff, pDamp, pMod, pEarly, pWidth, pLow, pHigh, pMidF, pMidG,
                pSat, pDrive, pAir, pScSrc, pDuck, pThr, pRel, pMix, pOut };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlowVerbProcessor)
};
