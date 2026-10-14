#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "DSP.h"

namespace ParamIDs
{
    inline constexpr auto mode      = "mode";
    inline constexpr auto peak      = "peak";
    inline constexpr auto gain      = "gain";
    inline constexpr auto limit     = "limit";
    inline constexpr auto scHpf     = "schpf";
    inline constexpr auto warmth    = "warmth";
    inline constexpr auto bite      = "bite";
    inline constexpr auto satType   = "sattype";
    inline constexpr auto drive     = "drive";
    inline constexpr auto satMix    = "satmix";
    inline constexpr auto mix       = "mix";
    inline constexpr auto output    = "output";
}

class BKPOptoGlowProcessor : public juce::AudioProcessor
{
public:
    BKPOptoGlowProcessor();
    ~BKPOptoGlowProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Opto Glow"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    static const juce::StringArray& satTypeNames();

    // metering (read by the editor)
    std::atomic<float> meterGr { 0.0f }, meterIn { -100.0f }, meterOut { -100.0f };

private:
    void updateFilters();
    void processChunk (juce::AudioBuffer<float>&);
    float processSaturation (int type, int ch, float x, float g) noexcept;

    bkp::OptoCompressor comp;

    // colour + tone EQ (base rate)
    bkp::StereoBiquad modeEq1, modeEq2;
    bkp::StereoBiquad warmShelf, warmBody, biteBell, biteShelf;

    // saturation type filters (oversampled rate)
    bkp::StereoBiquad retroHp, retroLp, retroMid, tapeBump, tapeTop;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> dryDelay { 1024 };
    juce::AudioBuffer<float> dryBuffer;

    juce::SmoothedValue<float> makeupSm, mixSm, outSm, driveSm, satMixSm;

    float dcX1[2] {}, dcY1[2] {}, dcR = 0.999f;

    double fs = 44100.0, osFs = 176400.0;
    int lastMode = -1, lastSatType = -1;
    float lastWarmth = -1.0f, lastBite = -1.0f;
    int currentProgram = 0;
    int maxBlockSize = 512;

    std::atomic<float>* pMode = nullptr; std::atomic<float>* pPeak = nullptr;
    std::atomic<float>* pGain = nullptr; std::atomic<float>* pLimit = nullptr;
    std::atomic<float>* pScHpf = nullptr; std::atomic<float>* pWarmth = nullptr;
    std::atomic<float>* pBite = nullptr; std::atomic<float>* pSatType = nullptr;
    std::atomic<float>* pDrive = nullptr; std::atomic<float>* pSatMix = nullptr;
    std::atomic<float>* pMix = nullptr; std::atomic<float>* pOutput = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKPOptoGlowProcessor)
};
