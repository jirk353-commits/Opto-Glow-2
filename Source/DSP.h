#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <array>

namespace bkp
{
using Coeffs = juce::dsp::IIR::Coefficients<float>;
using ArrCoeffs = juce::dsp::IIR::ArrayCoefficients<float>;
using Biquad = juce::dsp::IIR::Filter<float>;

inline float coefFromTime (double seconds, double sampleRate)
{
    return (float) (1.0 - std::exp (-1.0 / (seconds * sampleRate)));
}

//==============================================================================
/** Stereo pair of biquads sharing one coefficient set (allocation-free updates). */
struct StereoBiquad
{
    Coeffs::Ptr coeffs { new Coeffs (1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f) };
    std::array<Biquad, 2> f;

    void prepare()
    {
        for (auto& x : f) { x.coefficients = coeffs; x.reset(); }
    }

    template <size_t N>
    void set (const std::array<float, N>& c) { *coeffs = c; }

    void reset() { for (auto& x : f) x.reset(); }

    float process (int ch, float x) noexcept { return f[(size_t) ch].processSample (x); }
};

//==============================================================================
/**
    Program-dependent opto compressor model.

    SMOOTH  : LA-2A style - slow optical attack (~10 ms), two-stage release
              (fast ~60 ms + slow 0.5..4 s that lengthens with "memory"),
              soft knee, HF-emphasised sidechain.
    PUNCH   : LA-3A style - faster attack (~1 ms), quicker release,
              firmer knee and higher ratio, more forward character.
*/
class OptoCompressor
{
public:
    enum Mode { Smooth = 0, Punch = 1 };

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        for (auto& h : scHpf) h.coefficients = scHpfCoeffs;
        for (auto& e : scEmph) e.coefficients = scEmphCoeffs;
        *scEmphCoeffs = ArrCoeffs::makeHighShelf (fs, 2500.0f, 0.7f, juce::Decibels::decibelsToGain (4.0f));
        lastHpf = -1.0f;
        lastMode = -1;
        setParams (Smooth, 30.0f, false, 20.0f);
        reset();
    }

    void reset()
    {
        env = 0.0f; s1 = s2 = 0.0f; memory = 0.0f; gr = 0.0f;
        for (auto& h : scHpf) h.reset();
        for (auto& e : scEmph) e.reset();
    }

    void setParams (int newMode, float peakReduction, bool limit, float scHpfHz)
    {
        if (newMode != lastMode || limit != lastLimit)
        {
            lastMode = newMode; lastLimit = limit;
            mode = newMode;

            if (mode == Smooth)
            {
                ratio = limit ? 12.0f : 3.0f;
                kneeDb = limit ? 6.0f : 12.0f;
                aAtt   = coefFromTime (0.010, fs);
                aRel1  = coefFromTime (0.060, fs);
                aRms   = coefFromTime (0.010, fs);
                weight = 0.5f;
                slowMin = 0.5f; slowRange = 3.5f;
            }
            else
            {
                ratio = limit ? 20.0f : 4.0f;
                kneeDb = limit ? 3.0f : 7.0f;
                aAtt   = coefFromTime (0.0012, fs);
                aRel1  = coefFromTime (0.045, fs);
                aRms   = coefFromTime (0.003, fs);
                weight = 0.65f;
                slowMin = 0.15f; slowRange = 0.6f;
            }
            aMemUp   = coefFromTime (2.5, fs);
            aMemDown = coefFromTime (6.0, fs);
        }

        thresholdDb = -0.42f * juce::jlimit (0.0f, 100.0f, peakReduction);

        if (std::abs (scHpfHz - lastHpf) > 0.5f)
        {
            lastHpf = scHpfHz;
            *scHpfCoeffs = ArrCoeffs::makeHighPass (fs, scHpfHz, 0.707f);
        }
    }

    /** Feeds one multichannel frame, returns the linear gain to apply (linked). */
    float processFrame (const float* frame, int numCh) noexcept
    {
        float p = 0.0f;
        for (int c = 0; c < numCh; ++c)
        {
            float s = scHpf[(size_t) c].processSample (frame[c]);
            if (mode == Smooth)
                s = scEmph[(size_t) c].processSample (s);
            p = juce::jmax (p, s * s);
        }

        env += aRms * (p - env);
        const float levelDb = 10.0f * std::log10 (env + 1.0e-20f) + 3.01f;

        // soft-knee gain computer
        const float over = levelDb - thresholdDb;
        float target = 0.0f;
        const float slope = 1.0f - 1.0f / ratio;
        if (2.0f * over <= -kneeDb)       target = 0.0f;
        else if (2.0f * over < kneeDb)    target = slope * (over + 0.5f * kneeDb) * (over + 0.5f * kneeDb) / (2.0f * kneeDb);
        else                              target = slope * over;

        // optical memory: long heavy compression -> longer slow release
        memory += (target > 1.5f ? aMemUp * (1.0f - memory) : aMemDown * (0.0f - memory));
        const float slowSeconds = slowMin + slowRange * memory;
        const float aRel2 = (float) (1.0 / (slowSeconds * fs)); // small-coef approximation

        // opto cell reacts faster to bigger overshoot
        auto stage = [&] (float& s, float aRel)
        {
            const float d = target - s;
            if (d > 0.0f) s += juce::jmin (1.0f, aAtt * (1.0f + 0.08f * d)) * d;
            else          s += aRel * d;
        };
        stage (s1, aRel1);
        stage (s2, aRel2);

        gr = weight * s1 + (1.0f - weight) * s2;
        return juce::Decibels::decibelsToGain (-gr, -120.0f);
    }

    float getGainReductionDb() const noexcept { return gr; }
    int getMode() const noexcept { return mode; }

private:
    double fs = 44100.0;
    int mode = Smooth, lastMode = -1;
    bool lastLimit = false;
    float lastHpf = -1.0f;

    float ratio = 3.0f, kneeDb = 12.0f, thresholdDb = -12.0f;
    float aAtt = 0.01f, aRel1 = 0.001f, aRms = 0.01f, weight = 0.5f;
    float slowMin = 0.5f, slowRange = 3.5f, aMemUp = 0.0f, aMemDown = 0.0f;

    float env = 0.0f, s1 = 0.0f, s2 = 0.0f, memory = 0.0f, gr = 0.0f;

    Coeffs::Ptr scHpfCoeffs  { new Coeffs (1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f) };
    Coeffs::Ptr scEmphCoeffs { new Coeffs (1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f) };
    std::array<Biquad, 2> scHpf, scEmph;
};

//==============================================================================
/** Saturation shapers. All are normalised to unity small-signal gain. */
namespace shapers
{
    inline float warm (float x) noexcept
    {
        constexpr float b = 0.30f;
        static const float tb = std::tanh (b), norm = 1.0f / (1.0f - tb * tb);
        return (std::tanh (x + b) - tb) * norm;
    }

    inline float triode (float x) noexcept
    {
        // asymmetric: positive side clips early, negative side later -> strong 2nd harmonic
        constexpr float k = 0.55f;
        return x >= 0.0f ? 1.0f - std::exp (-x)
                         : -(1.0f - std::exp (k * x)) / k;
    }

    inline float retro (float x) noexcept
    {
        const float a = std::abs (x);
        return x / std::cbrt (1.0f + a * a * a);
    }

    inline float tape (float x) noexcept
    {
        constexpr float b = 0.06f;
        static const float tb = std::tanh (b), norm = 1.0f / (1.0f - tb * tb);
        return (std::tanh (x + b) - tb) * norm;
    }

    /** subtle valve colour for the SMOOTH mode */
    inline float tubeColour (float x) noexcept
    {
        constexpr float k = 0.5f, b = 0.25f;
        static const float tb = std::tanh (b), norm = 1.0f / (k * (1.0f - tb * tb));
        return (std::tanh (k * x + b) - tb) * norm;
    }

    /** subtle solid-state/transformer colour for the PUNCH mode */
    inline float transformerColour (float x) noexcept
    {
        constexpr float k = 0.9f;
        return std::tanh (k * x) / k;
    }
}

} // namespace bkp
