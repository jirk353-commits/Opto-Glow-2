#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace gvui
{
namespace col
{
    const juce::Colour bgTop     { 0xff141b2a };
    const juce::Colour bgBottom  { 0xff080b12 };
    const juce::Colour panelTop  { 0xff161e2d };
    const juce::Colour panelBot  { 0xff0e131d };
    const juce::Colour edge      { 0xff26324a };
    const juce::Colour cyan      { 0xff4fd8ff };
    const juce::Colour cyanDeep  { 0xff2a8cff };
    const juce::Colour violet    { 0xff9a7bff };
    const juce::Colour silver    { 0xffaebbd0 };
    const juce::Colour text      { 0xffdce6f2 };
    const juce::Colour textDim   { 0xff7d8aa0 };
}

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();
    void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool, bool) override;
    void drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Font getPopupMenuFont() override;
};

class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState&, const juce::String& id, const juce::String& title, bool big = false);
    void paint (juce::Graphics&) override;
    void resized() override;
    juce::Slider slider;
private:
    juce::String title; bool big;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
};

class ChoiceButtons : public juce::Component
{
public:
    ChoiceButtons (juce::RangedAudioParameter&, const juce::StringArray& labels, int columns = 0);
    void resized() override;
    int getSelected() const { return selected; }
    std::function<void (int)> onChange;
private:
    void update (int);
    juce::OwnedArray<juce::TextButton> buttons;
    std::unique_ptr<juce::ParameterAttachment> att;
    int selected = -1, cols = 0;
};

class MainPanel : public juce::Component, private juce::Timer
{
public:
    explicit MainPanel (GlowVerbProcessor&);
    ~MainPanel() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    static constexpr int W = 1180, H = 660;

private:
    void timerCallback() override;
    void renderBackground();
    void paintDisplay (juce::Graphics&);
    void paintMeters (juce::Graphics&);
    void paintSidechain (juce::Graphics&);
    float param (const char* id) const { return proc.apvts.getRawParameterValue (id)->load(); }

    GlowVerbProcessor& proc;
    LookAndFeel lnf;
    juce::Image background;

    juce::OwnedArray<juce::TextButton> envButtons;
    std::unique_ptr<juce::ParameterAttachment> typeAtt;
    int currentType = 5;

    ChoiceButtons satButtons, srcButtons;
    Knob pre, decay, size, diffusion, damping, mod, early, width;
    Knob lowCut, highCut, midFreq, midGain, drive, air, duck, thresh, release, mix, output;
    juce::ComboBox presetBox;

    float wetLvl = -100, inLvl = -100, outLvl = -100, duckLvl = 0, glow = 0;
    bool scConnected = false;
    juce::String lastSignature;
};
} // namespace gvui

class GlowVerbEditor : public juce::AudioProcessorEditor
{
public:
    explicit GlowVerbEditor (GlowVerbProcessor&);
    void resized() override;
private:
    gvui::MainPanel panel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlowVerbEditor)
};
