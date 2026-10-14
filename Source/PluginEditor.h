#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace ui
{
namespace col
{
    const juce::Colour bgTop       { 0xff2b231d };
    const juce::Colour bgBottom    { 0xff15110e };
    const juce::Colour panel       { 0xff1d1814 };
    const juce::Colour panelEdge   { 0xff3d3128 };
    const juce::Colour amber       { 0xffffb444 };
    const juce::Colour amberDeep   { 0xffe0782a };
    const juce::Colour gold        { 0xffd9a95b };
    const juce::Colour goldDim     { 0xff8a6a3c };
    const juce::Colour cream       { 0xfff4e6c3 };
    const juce::Colour text        { 0xffe9dcc4 };
    const juce::Colour textDim     { 0xff9c8b74 };
    const juce::Colour red         { 0xffc8432f };
}

//==============================================================================
class GlowLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GlowLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool highlighted, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh,
                       juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Font getPopupMenuFont() override;
};

//==============================================================================
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState&, const juce::String& paramId, const juce::String& title, bool big = false);
    void paint (juce::Graphics&) override;
    void resized() override;

    juce::Slider slider;

private:
    juce::String title;
    bool big;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

//==============================================================================
class ChoiceButtons : public juce::Component
{
public:
    ChoiceButtons (juce::RangedAudioParameter&, const juce::StringArray& labels);
    void resized() override;
    std::function<void (int)> onChange;
    int getSelected() const { return selected; }

private:
    void update (int index);
    juce::OwnedArray<juce::TextButton> buttons;
    std::unique_ptr<juce::ParameterAttachment> attachment;
    int selected = -1;
};

//==============================================================================
class VUMeter : public juce::Component
{
public:
    enum Source { GR = 0, IN, OUT };
    void setSource (Source s) { source = s; repaint(); }
    void setValues (float grDb, float inDb, float outDb);
    void paint (juce::Graphics&) override;

private:
    static float vuPosition (float vu);
    Source source = GR;
    float needle = 0.79f;   // 0..1 across the scale
    float grDb = 0.0f;
};

//==============================================================================
class MainPanel : public juce::Component, private juce::Timer
{
public:
    explicit MainPanel (BKPOptoGlowProcessor&);
    ~MainPanel() override;
    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int W = 980, H = 560;

private:
    void timerCallback() override;
    void renderBackground();
    void refreshPresetBox();

    BKPOptoGlowProcessor& proc;
    GlowLookAndFeel lnf;
    juce::Image background;

    VUMeter vu;
    juce::TextButton meterGr { "GR" }, meterIn { "IN" }, meterOut { "OUT" };

    ChoiceButtons modeButtons, limitButtons, satButtons;
    Knob peak, gain, warmth, bite, scHpf, drive, satMix, mix, output;
    juce::ComboBox presetBox;

    float lamp = 0.0f;
    float lastDrive = -1.0f;
};

} // namespace ui

//==============================================================================
class BKPOptoGlowEditor : public juce::AudioProcessorEditor
{
public:
    explicit BKPOptoGlowEditor (BKPOptoGlowProcessor&);
    void resized() override;

private:
    ui::MainPanel panel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKPOptoGlowEditor)
};
