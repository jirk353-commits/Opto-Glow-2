#include "PluginEditor.h"

using namespace juce;

namespace ui
{
//==============================================================================
static Font uiFont (float size, bool bold = false, float kerning = 0.0f)
{
    Font f { FontOptions (size, bold ? Font::bold : Font::plain) };
    return kerning > 0.0f ? f.withExtraKerningFactor (kerning) : f;
}

//==============================================================================
GlowLookAndFeel::GlowLookAndFeel()
{
    setColour (PopupMenu::backgroundColourId, Colour (0xff1c1713));
    setColour (PopupMenu::textColourId, col::text);
    setColour (PopupMenu::highlightedBackgroundColourId, col::amberDeep.withAlpha (0.85f));
    setColour (PopupMenu::highlightedTextColourId, Colours::black);
    setColour (ComboBox::textColourId, col::amber);
    setColour (ComboBox::arrowColourId, col::gold);
}

void GlowLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                        float startAngle, float endAngle, Slider& slider)
{
    const auto area = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float size = jmin (area.getWidth(), area.getHeight());
    const auto c = area.getCentre();
    const float tickR = size * 0.5f - 2.0f;
    const float arcR  = tickR - size * 0.07f;
    const float arcW  = jmax (2.5f, size * 0.03f);
    const float bodyR = arcR - size * 0.085f;
    const float angle = startAngle + pos * (endAngle - startAngle);

    // tick dots
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (float) i / 10.0f * (endAngle - startAngle);
        const auto p = c.getPointOnCircumference (tickR, a);
        const float r = (i == 0 || i == 5 || i == 10) ? 1.6f : 1.0f;
        g.setColour (col::gold.withAlpha (i == 0 || i == 5 || i == 10 ? 0.75f : 0.4f));
        g.fillEllipse (p.x - r, p.y - r, 2 * r, 2 * r);
    }

    // arc track
    Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (Colour (0xff0b0907));
    g.strokePath (track, PathStrokeType (arcW + 2.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour (Colour (0xff2e261f));
    g.strokePath (track, PathStrokeType (arcW, PathStrokeType::curved, PathStrokeType::rounded));

    // value arc (bipolar parameters grow from zero)
    float fromAngle = startAngle;
    if (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0)
        fromAngle = startAngle + (float) slider.valueToProportionOfLength (0.0) * (endAngle - startAngle);

    if (std::abs (angle - fromAngle) > 0.01f)
    {
        Path value;
        value.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, jmin (fromAngle, angle), jmax (fromAngle, angle), true);
        g.setColour (col::amber.withAlpha (0.16f));
        g.strokePath (value, PathStrokeType (arcW * 3.2f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setGradientFill (ColourGradient (col::amberDeep, c.x - arcR, c.y + arcR, col::amber, c.x + arcR, c.y - arcR, false));
        g.strokePath (value, PathStrokeType (arcW, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // knob body with shadow
    Path body;
    body.addEllipse (c.x - bodyR, c.y - bodyR, bodyR * 2, bodyR * 2);
    DropShadow (Colours::black.withAlpha (0.75f), (int) (size * 0.12f), { 0, (int) (size * 0.04f) }).drawForPath (g, body);

    g.setGradientFill (ColourGradient (Colour (0xff5d4e41), c.x - bodyR * 0.6f, c.y - bodyR * 0.8f,
                                       Colour (0xff110d0b), c.x + bodyR * 0.5f, c.y + bodyR, true));
    g.fillPath (body);

    // knurled edge
    for (int i = 0; i < 48; ++i)
    {
        const float a = (float) i / 48.0f * MathConstants<float>::twoPi;
        const auto p1 = c.getPointOnCircumference (bodyR * 0.84f, a);
        const auto p2 = c.getPointOnCircumference (bodyR * 0.98f, a);
        g.setColour ((i % 2 == 0 ? Colours::black.withAlpha (0.45f) : Colours::white.withAlpha (0.06f)));
        g.drawLine ({ p1, p2 }, 1.0f);
    }

    // gold rim
    g.setGradientFill (ColourGradient (col::gold, c.x, c.y - bodyR, col::goldDim.darker (0.6f), c.x, c.y + bodyR, false));
    g.drawEllipse (c.x - bodyR, c.y - bodyR, bodyR * 2, bodyR * 2, jmax (1.0f, size * 0.012f));

    // machined cap
    const float capR = bodyR * 0.70f;
    g.setGradientFill (ColourGradient (Colour (0xff4a3f35), c.x, c.y - capR, Colour (0xff17120f), c.x, c.y + capR, false));
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2, capR * 2);
    for (float r = capR * 0.25f; r < capR; r += 2.2f)
    {
        g.setColour (Colours::white.withAlpha (0.025f));
        g.drawEllipse (c.x - r, c.y - r, r * 2, r * 2, 0.6f);
    }
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.16f), c.x, c.y - capR,
                                       Colours::transparentWhite, c.x, c.y, false));
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2, capR * 2);
    g.setColour (Colours::black.withAlpha (0.6f));
    g.drawEllipse (c.x - capR, c.y - capR, capR * 2, capR * 2, 1.0f);

    // pointer
    Path pointer;
    pointer.startNewSubPath (c.getPointOnCircumference (bodyR * 0.30f, angle));
    pointer.lineTo (c.getPointOnCircumference (bodyR * 0.90f, angle));
    g.setColour (col::amber.withAlpha (0.28f));
    g.strokePath (pointer, PathStrokeType (size * 0.055f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour (col::amber);
    g.strokePath (pointer, PathStrokeType (jmax (2.0f, size * 0.024f), PathStrokeType::curved, PathStrokeType::rounded));
}

void GlowLookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const float cs = 5.0f;
    const bool L = ! b.isConnectedOnLeft(), R = ! b.isConnectedOnRight();
    Path p;
    p.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), cs, cs, L, R, L, R);

    if (b.getToggleState())
    {
        g.setGradientFill (ColourGradient (Colour (0xffffd283), 0, r.getY(), Colour (0xffe57f2c), 0, r.getBottom(), false));
        g.fillPath (p);
        g.setColour (Colours::white.withAlpha (0.35f));
        g.drawLine (r.getX() + 3, r.getY() + 1.5f, r.getRight() - 3, r.getY() + 1.5f, 1.0f);
        g.setColour (Colour (0xff7a3f10));
        g.strokePath (p, PathStrokeType (1.0f));
    }
    else
    {
        g.setGradientFill (ColourGradient (Colour (down ? 0xff15110e : 0xff2c241e), 0, r.getY(),
                                           Colour (0xff17130f), 0, r.getBottom(), false));
        g.fillPath (p);
        g.setColour (highlighted ? col::amber.withAlpha (0.55f) : Colour (0xff4b3d31));
        g.strokePath (p, PathStrokeType (1.0f));
    }
}

void GlowLookAndFeel::drawButtonText (Graphics& g, TextButton& b, bool highlighted, bool)
{
    g.setFont (uiFont (jmin (14.0f, b.getHeight() * 0.52f), true, 0.08f));
    g.setColour (b.getToggleState() ? Colour (0xff2a1606) : (highlighted ? col::text : col::textDim));
    g.drawText (b.getButtonText(), b.getLocalBounds(), Justification::centred, false);
}

void GlowLookAndFeel::drawComboBox (Graphics& g, int w, int h, bool, int, int, int, int, ComboBox& box)
{
    auto r = Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
    g.setGradientFill (ColourGradient (Colour (0xff0e0b09), 0, 0, Colour (0xff1e1814), 0, (float) h, false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (box.isMouseOver (true) ? col::amber.withAlpha (0.6f) : col::panelEdge.brighter (0.2f));
    g.drawRoundedRectangle (r, 6.0f, 1.0f);

    Path arrow;
    const float ax = (float) w - 18.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax - 5, ay - 2.5f, ax + 5, ay - 2.5f, ax, ay + 3.5f);
    g.setColour (col::gold);
    g.fillPath (arrow);
}

Font GlowLookAndFeel::getComboBoxFont (ComboBox&)  { return uiFont (14.0f, true, 0.04f); }
Font GlowLookAndFeel::getPopupMenuFont()            { return uiFont (14.0f); }

void GlowLookAndFeel::positionComboBoxText (ComboBox& box, Label& label)
{
    label.setBounds (12, 1, box.getWidth() - 36, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

//==============================================================================
Knob::Knob (AudioProcessorValueTreeState& state, const String& paramId, const String& t, bool b)
    : title (t), big (b)
{
    slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (MathConstants<float>::pi * 1.23f, MathConstants<float>::pi * 2.77f, true);
    slider.setMouseDragSensitivity (big ? 260 : 220);
    slider.setVelocityBasedMode (false);
    addAndMakeVisible (slider);

    attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (state, paramId, slider);

    if (auto* p = state.getParameter (paramId))
        slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));

    slider.onValueChange = [this] { repaint(); };
}

void Knob::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop (big ? 20 : 18);
    r.removeFromBottom (big ? 26 : 24);
    const int s = jmin (r.getWidth(), r.getHeight());
    slider.setBounds (r.withSizeKeepingCentre (s, s));
}

void Knob::paint (Graphics& g)
{
    auto r = getLocalBounds();
    g.setColour (col::text);
    g.setFont (uiFont (big ? 14.0f : 12.5f, true, 0.12f));
    g.drawText (title, r.removeFromTop (big ? 20 : 18), Justification::centred, false);

    // value pill
    auto pill = r.removeFromBottom (big ? 26 : 24).withSizeKeepingCentre (big ? 84 : 72, big ? 22 : 20).toFloat();
    g.setColour (Colour (0xff0c0907));
    g.fillRoundedRectangle (pill, 4.0f);
    g.setColour (col::panelEdge);
    g.drawRoundedRectangle (pill, 4.0f, 1.0f);
    g.setColour (col::amber);
    g.setFont (uiFont (big ? 14.5f : 13.5f, true));
    g.drawText (slider.getTextFromValue (slider.getValue()), pill, Justification::centred, false);
}

//==============================================================================
ChoiceButtons::ChoiceButtons (RangedAudioParameter& param, const StringArray& labels)
{
    for (int i = 0; i < labels.size(); ++i)
    {
        auto* b = buttons.add (new TextButton (labels[i]));
        b->setClickingTogglesState (false);
        int edges = 0;
        if (i > 0) edges |= Button::ConnectedOnLeft;
        if (i < labels.size() - 1) edges |= Button::ConnectedOnRight;
        b->setConnectedEdges (edges);
        b->onClick = [this, i] { if (attachment) attachment->setValueAsCompleteGesture ((float) i); };
        addAndMakeVisible (b);
    }

    attachment = std::make_unique<ParameterAttachment> (param, [this] (float v) { update (roundToInt (v)); });
    attachment->sendInitialUpdate();
}

void ChoiceButtons::update (int index)
{
    selected = index;
    for (int i = 0; i < buttons.size(); ++i)
        buttons[i]->setToggleState (i == index, dontSendNotification);
    if (onChange) onChange (index);
}

void ChoiceButtons::resized()
{
    const int n = buttons.size();
    if (n == 0) return;
    const float w = (float) getWidth() / (float) n;
    for (int i = 0; i < n; ++i)
        buttons[i]->setBounds (roundToInt ((float) i * w), 0, roundToInt ((float) (i + 1) * w) - roundToInt ((float) i * w), getHeight());
}

//==============================================================================
float VUMeter::vuPosition (float vu)
{
    static const float table[][2] = { { -20.0f, 0.0f }, { -10.0f, 0.20f }, { -7.0f, 0.33f }, { -5.0f, 0.45f },
                                      { -3.0f, 0.58f }, { -2.0f, 0.65f }, { -1.0f, 0.72f }, { 0.0f, 0.79f },
                                      { 1.0f, 0.86f }, { 2.0f, 0.93f }, { 3.0f, 1.0f } };
    if (vu <= table[0][0]) return 0.0f;
    for (size_t i = 1; i < std::size (table); ++i)
        if (vu <= table[i][0])
        {
            const float t = (vu - table[i - 1][0]) / (table[i][0] - table[i - 1][0]);
            return table[i - 1][1] + t * (table[i][1] - table[i - 1][1]);
        }
    return 1.03f; // pinned
}

void VUMeter::setValues (float gr, float inDb, float outDb)
{
    grDb = gr;
    float vu = 0.0f;
    switch (source)
    {
        case GR:  vu = -gr; break;
        case IN:  vu = inDb + 18.0f; break;   // 0 VU = -18 dBFS RMS
        case OUT: vu = outDb + 18.0f; break;
    }
    const float target = vuPosition (vu);
    const float old = needle;
    needle += (target - needle) * 0.16f;
    if (std::abs (needle - old) > 0.0005f)
        repaint();
}

void VUMeter::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // bezel
    g.setGradientFill (ColourGradient (Colour (0xff3e342c), 0, b.getY(), Colour (0xff0f0c0a), 0, b.getBottom(), false));
    g.fillRoundedRectangle (b, 9.0f);
    g.setColour (Colours::black);
    g.drawRoundedRectangle (b.reduced (0.5f), 9.0f, 1.0f);
    g.setColour (col::gold.withAlpha (0.35f));
    g.drawRoundedRectangle (b.reduced (2.0f), 8.0f, 0.8f);

    auto face = b.reduced (9.0f);
    Path facePath;
    facePath.addRoundedRectangle (face, 5.0f);

    // backlit cream face
    g.setGradientFill (ColourGradient (Colour (0xfffff2cf), face.getCentreX(), face.getBottom(),
                                       Colour (0xffd9bd86), face.getX(), face.getY(), true));
    g.fillPath (facePath);
    g.setGradientFill (ColourGradient (col::amber.withAlpha (0.28f), face.getCentreX(), face.getBottom() + 10,
                                       Colours::transparentWhite, face.getCentreX(), face.getY(), true));
    g.fillPath (facePath);

    Graphics::ScopedSaveState save (g);
    g.reduceClipRegion (facePath);

    const Point<float> pivot { face.getCentreX(), face.getBottom() + face.getHeight() * 0.28f };
    const float R = face.getHeight() * 0.98f;
    const float span = 0.74f;
    auto posToAngle = [span] (float p) { return -span + p * 2.0f * span; };

    const Colour ink (0xff2a1d12);

    // scale arc
    Path arc;
    arc.addCentredArc (pivot.x, pivot.y, R, R, 0.0f, posToAngle (0.0f), posToAngle (0.79f), true);
    g.setColour (ink);
    g.strokePath (arc, PathStrokeType (1.4f));
    Path redArc;
    redArc.addCentredArc (pivot.x, pivot.y, R + 2.5f, R + 2.5f, 0.0f, posToAngle (0.79f), posToAngle (1.0f), true);
    g.setColour (col::red);
    g.strokePath (redArc, PathStrokeType (5.0f));

    // ticks + labels
    const float marks[] = { -20, -10, -7, -5, -3, -2, -1, 0, 1, 2, 3 };
    g.setFont (uiFont (11.0f, true));
    for (float m : marks)
    {
        const float a = posToAngle (vuPosition (m));
        const auto p1 = pivot.getPointOnCircumference (R, a);
        const auto p2 = pivot.getPointOnCircumference (R + 9.0f, a);
        g.setColour (m > 0 ? col::red : ink);
        g.drawLine ({ p1, p2 }, 1.4f);
        const auto tp = pivot.getPointOnCircumference (R + 19.0f, a);
        const String label = m > 0 ? "+" + String ((int) m) : String ((int) std::abs (m));
        g.drawText (label, Rectangle<float> (34, 14).withCentre (tp), Justification::centred, false);
    }
    // minor ticks
    for (float m = -20.0f; m < 3.0f; m += 1.0f)
    {
        const float a = posToAngle (vuPosition (m));
        g.setColour ((m >= 0 ? col::red : ink).withAlpha (0.55f));
        g.drawLine ({ pivot.getPointOnCircumference (R, a), pivot.getPointOnCircumference (R + 4.0f, a) }, 0.8f);
    }

    // legends
    g.setColour (ink);
    g.setFont (uiFont (22.0f, true, 0.05f));
    g.drawText ("VU", Rectangle<float> (face.getX(), face.getY() + face.getHeight() * 0.50f, face.getWidth(), 24), Justification::centred);
    g.setFont (uiFont (9.5f, true, 0.25f));
    const char* src = source == GR ? "GAIN REDUCTION" : (source == IN ? "INPUT LEVEL" : "OUTPUT LEVEL");
    g.drawText (src, Rectangle<float> (face.getX(), face.getY() + face.getHeight() * 0.50f + 24, face.getWidth(), 14), Justification::centred);

    g.setFont (uiFont (10.0f, true));
    g.setColour (ink.withAlpha (0.8f));
    g.drawText ("GR " + String (-grDb, 1) + " dB", face.reduced (10, 6), Justification::bottomRight);
    g.drawText ("ROHIT JI", face.reduced (10, 6), Justification::bottomLeft);

    // needle
    const float a = posToAngle (jlimit (-0.02f, 1.03f, needle));
    const auto tip = pivot.getPointOnCircumference (R + 6.0f, a);
    g.setColour (Colours::black.withAlpha (0.18f));
    g.drawLine ({ pivot.translated (3, 2), tip.translated (3, 2) }, 2.0f);
    g.setColour (Colour (0xff1a120c));
    g.drawLine ({ pivot, tip }, 1.7f);
    g.setColour (col::red.darker (0.2f));
    g.drawLine ({ pivot.getPointOnCircumference (R - 30.0f, a), tip }, 1.7f);

    // inner shadow + glass
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.35f), 0, face.getY(), Colours::transparentBlack, 0, face.getY() + 14, false));
    g.fillRect (face);
    Path glass;
    glass.startNewSubPath (face.getX(), face.getY());
    glass.lineTo (face.getRight(), face.getY());
    glass.lineTo (face.getRight(), face.getY() + face.getHeight() * 0.22f);
    glass.quadraticTo (face.getCentreX(), face.getY() + face.getHeight() * 0.48f, face.getX(), face.getY() + face.getHeight() * 0.30f);
    glass.closeSubPath();
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.30f), 0, face.getY(), Colours::white.withAlpha (0.02f), 0, face.getY() + face.getHeight() * 0.4f, false));
    g.fillPath (glass);
}

//==============================================================================
namespace
{
    const Rectangle<int> vuPanel   { 24, 80, 330, 262 };
    const Rectangle<int> compPanel { 370, 80, 320, 262 };
    const Rectangle<int> tonePanel { 706, 80, 250, 262 };
    const Rectangle<int> satPanel  { 24, 358, 616, 176 };
    const Rectangle<int> outPanel  { 656, 358, 300, 176 };
    const Rectangle<int> curveBox  { 300, 428, 104, 96 };
    const Rectangle<int> descBox   { 414, 428, 206, 96 };
    const Rectangle<int> lampArea  { 26, 20, 30, 30 };

    const char* satDescriptions[] =
    {
        "Transparent. No extra saturation - just the opto colour of the compressor.",
        "Soft valve warmth. Rounds the low end and adds gentle even harmonics.",
        "Asymmetric triode glow. Rich 2nd harmonic, forward and present mids.",
        "Band-limited vintage grit. Old radio, lo-fi and character vocals.",
        "Tape saturation. Smooth peak rounding, low head-bump, softened highs."
    };

    float shapeFor (int type, float x)
    {
        using namespace bkp::shapers;
        switch (type)
        {
            case 1: return warm (x);
            case 2: return triode (x);
            case 3: return retro (x);
            case 4: return tape (x);
            default: return x;
        }
    }

    void drawScrew (Graphics& g, Point<float> c, float r, float angle)
    {
        g.setGradientFill (ColourGradient (Colour (0xff8d7a66), c.x - r, c.y - r, Colour (0xff2a221c), c.x + r, c.y + r, false));
        g.fillEllipse (c.x - r, c.y - r, 2 * r, 2 * r);
        g.setColour (Colours::black.withAlpha (0.7f));
        g.drawEllipse (c.x - r, c.y - r, 2 * r, 2 * r, 0.8f);
        const auto d = Point<float> (std::cos (angle), std::sin (angle)) * (r * 0.75f);
        g.setColour (Colour (0xff1a1410));
        g.drawLine ({ c - d, c + d }, 1.6f);
    }
}

MainPanel::MainPanel (BKPOptoGlowProcessor& p)
    : proc (p),
      modeButtons (*p.apvts.getParameter (ParamIDs::mode), { "SMOOTH", "PUNCH" }),
      limitButtons (*p.apvts.getParameter (ParamIDs::limit), { "COMPRESS", "LIMIT" }),
      satButtons (*p.apvts.getParameter (ParamIDs::satType), BKPOptoGlowProcessor::satTypeNames()),
      peak   (p.apvts, ParamIDs::peak,   "PEAK REDUCTION", true),
      gain   (p.apvts, ParamIDs::gain,   "GAIN", true),
      warmth (p.apvts, ParamIDs::warmth, "WARMTH"),
      bite   (p.apvts, ParamIDs::bite,   "BITE"),
      scHpf  (p.apvts, ParamIDs::scHpf,  "SC HPF"),
      drive  (p.apvts, ParamIDs::drive,  "DRIVE"),
      satMix (p.apvts, ParamIDs::satMix, "SAT MIX"),
      mix    (p.apvts, ParamIDs::mix,    "MIX"),
      output (p.apvts, ParamIDs::output, "OUTPUT")
{
    setLookAndFeel (&lnf);
    setOpaque (true);

    for (auto* c : std::initializer_list<Component*> { &vu, &meterGr, &meterIn, &meterOut, &modeButtons, &limitButtons,
                                                       &satButtons, &peak, &gain, &warmth, &bite, &scHpf, &drive,
                                                       &satMix, &mix, &output, &presetBox })
        addAndMakeVisible (c);

    // meter source selector
    TextButton* srcButtons[] = { &meterGr, &meterIn, &meterOut };
    for (int i = 0; i < 3; ++i)
    {
        auto* b = srcButtons[i];
        b->setRadioGroupId (4242);
        b->setClickingTogglesState (true);
        b->setConnectedEdges ((i > 0 ? Button::ConnectedOnLeft : 0) | (i < 2 ? Button::ConnectedOnRight : 0));
        b->onClick = [this, i] { vu.setSource ((VUMeter::Source) i); };
    }
    meterGr.setToggleState (true, dontSendNotification);

    satButtons.onChange = [this] (int) { repaint (curveBox.getUnion (descBox).expanded (4)); };

    refreshPresetBox();
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id > 0 && id - 1 != proc.getCurrentProgram())
            proc.setCurrentProgram (id - 1);
    };

    startTimerHz (60);
}

MainPanel::~MainPanel()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void MainPanel::refreshPresetBox()
{
    presetBox.clear (dontSendNotification);
    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, dontSendNotification);
}

void MainPanel::resized()
{
    renderBackground();

    vu.setBounds (40, 106, 298, 184);
    const int bw = 60, bx = vuPanel.getX() + (vuPanel.getWidth() - bw * 3) / 2;
    meterGr.setBounds  (bx,          302, bw, 26);
    meterIn.setBounds  (bx + bw,     302, bw, 26);
    meterOut.setBounds (bx + bw * 2, 302, bw, 26);

    modeButtons.setBounds (compPanel.getX() + 40, 106, 240, 28);
    peak.setBounds (compPanel.getX() + 14, 140, 146, 154);
    gain.setBounds (compPanel.getX() + 160, 140, 146, 154);
    limitButtons.setBounds (compPanel.getX() + 70, 302, 180, 26);

    warmth.setBounds (tonePanel.getX() + 10, 106, 115, 124);
    bite.setBounds (tonePanel.getX() + 125, 106, 115, 124);
    scHpf.setBounds (tonePanel.getX() + 70, 232, 110, 104);

    satButtons.setBounds (satPanel.getX() + 20, 388, satPanel.getWidth() - 40, 28);
    drive.setBounds  (satPanel.getX() + 14, 422, 126, 108);
    satMix.setBounds (satPanel.getX() + 140, 422, 126, 108);

    mix.setBounds    (outPanel.getX() + 14, 384, 136, 144);
    output.setBounds (outPanel.getX() + 150, 384, 136, 144);

    presetBox.setBounds (W - 24 - 250, 20, 250, 30);
}

void MainPanel::renderBackground()
{
    const float scale = 2.0f;
    background = Image (Image::RGB, roundToInt (W * scale), roundToInt (H * scale), true);
    Graphics g (background);
    g.addTransform (AffineTransform::scale (scale));

    // base
    g.setGradientFill (ColourGradient (col::bgTop, 0, 0, col::bgBottom, 0, (float) H, false));
    g.fillAll();

    // brushed metal texture
    Random rnd (1234);
    for (int y = 0; y < H * 2; ++y)
    {
        const float a = rnd.nextFloat() * 0.035f;
        g.setColour ((rnd.nextBool() ? Colours::white : Colours::black).withAlpha (a));
        g.fillRect (0.0f, (float) y * 0.5f, (float) W, 0.5f);
    }

    // vignette
    g.setGradientFill (ColourGradient (Colours::transparentBlack, W * 0.5f, H * 0.45f,
                                       Colours::black.withAlpha (0.55f), 0.0f, 0.0f, true));
    g.fillAll();

}

void MainPanel::drawStatic (Graphics& g)
{
    // header
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.35f), 0, 0, Colours::transparentBlack, 0, 70, false));
    g.fillRect (0, 0, W, 70);
    {
        ColourGradient line (col::gold.withAlpha (0.0f), 0, 0, col::gold.withAlpha (0.0f), (float) W, 0, false);
        line.addColour (0.15, col::gold.withAlpha (0.55f));
        line.addColour (0.85, col::gold.withAlpha (0.55f));
        g.setGradientFill (line);
        g.fillRect (0.0f, 66.0f, (float) W, 1.0f);
    }

    // title
    g.setGradientFill (ColourGradient (Colour (0xffffe1a3), 0, 14, Colour (0xffc98b3a), 0, 44, false));
    g.setFont (uiFont (30.0f, true, 0.14f));
    g.drawText ("OPTO GLOW", 66, 10, 300, 34, Justification::centredLeft, false);
    g.setColour (col::textDim);
    g.setFont (uiFont (12.0f, true, 0.22f));
    g.drawText ("ROHIT JI  |  VINTAGE OPTO COMPRESSOR", 68, 42, 420, 16, Justification::centredLeft, false);

    g.setColour (col::textDim);
    g.setFont (uiFont (12.0f, true, 0.2f));
    g.drawText ("PRESET", W - 24 - 250 - 70, 20, 62, 30, Justification::centredRight, false);

    // panels
    auto drawPanel = [&] (Rectangle<int> r, const String& title)
    {
        auto rf = r.toFloat();
        DropShadow (Colours::black.withAlpha (0.6f), 14, { 0, 4 }).drawForRectangle (g, r);
        g.setGradientFill (ColourGradient (Colour (0xff221c17), 0, rf.getY(), Colour (0xff171310), 0, rf.getBottom(), false));
        g.fillRoundedRectangle (rf, 10.0f);
        g.setColour (Colours::white.withAlpha (0.06f));
        g.drawLine (rf.getX() + 10, rf.getY() + 1, rf.getRight() - 10, rf.getY() + 1, 1.0f);
        g.setColour (col::panelEdge);
        g.drawRoundedRectangle (rf.reduced (0.5f), 10.0f, 1.0f);

        g.setFont (uiFont (12.0f, true, 0.24f));
        const int tw = GlyphArrangement::getStringWidthInt (g.getCurrentFont(), title) + 8;
        g.setColour (col::gold);
        g.drawText (title, r.getX() + 16, r.getY() + 6, tw + 10, 16, Justification::centredLeft, false);
        g.setColour (col::goldDim.withAlpha (0.45f));
        g.fillRect ((float) (r.getX() + 24 + tw), (float) r.getY() + 14.0f, (float) (r.getRight() - 16 - (r.getX() + 24 + tw)), 1.0f);
    };

    drawPanel (vuPanel, "METER");
    drawPanel (compPanel, "OPTO COMPRESSOR");
    drawPanel (tonePanel, "TONE");
    drawPanel (satPanel, "SATURATION");
    drawPanel (outPanel, "OUTPUT");

    // character boxes (recessed)
    for (auto r : { curveBox, descBox })
    {
        g.setColour (Colour (0xff0d0a08));
        g.fillRoundedRectangle (r.toFloat(), 6.0f);
        g.setColour (col::panelEdge);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 6.0f, 1.0f);
    }

    // screws
    drawScrew (g, { 10.0f, 10.0f }, 4.5f, 0.6f);
    drawScrew (g, { W - 10.0f, 10.0f }, 4.5f, 2.1f);
    drawScrew (g, { 10.0f, H - 10.0f }, 4.5f, 1.2f);
    drawScrew (g, { W - 10.0f, H - 10.0f }, 4.5f, 0.2f);

    // footer
    g.setColour (col::textDim.withAlpha (0.7f));
    g.setFont (uiFont (11.0f, true, 0.2f));
    g.drawText ("VOCALS  -  DRUMS  -  BASS  -  MIX BUS", 26, H - 22, 400, 16, Justification::centredLeft, false);
    g.drawText ("v1.0  |  ROHIT JI", W - 26 - 300, H - 22, 300, 16, Justification::centredRight, false);
}

void MainPanel::paint (Graphics& g)
{
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    g.drawImage (background, getLocalBounds().toFloat());
    drawStatic (g);

    // power / activity lamp
    {
        const auto c = lampArea.toFloat().getCentre();
        const float r = 8.0f;
        g.setGradientFill (ColourGradient (col::amber.withAlpha (0.45f * lamp), c.x, c.y,
                                           Colours::transparentBlack, c.x + 20, c.y, true));
        g.fillEllipse (c.x - 20, c.y - 20, 40, 40);
        g.setColour (Colour (0xff120d09));
        g.fillEllipse (c.x - r - 2, c.y - r - 2, 2 * r + 4, 2 * r + 4);
        g.setGradientFill (ColourGradient (Colour (0xffffe6a8).interpolatedWith (Colour (0xff5a3510), 1.0f - lamp), c.x - 2, c.y - 3,
                                           Colour (0xffe0782a).interpolatedWith (Colour (0xff2a170a), 1.0f - lamp), c.x + r, c.y + r, true));
        g.fillEllipse (c.x - r, c.y - r, 2 * r, 2 * r);
        g.setColour (Colours::white.withAlpha (0.35f));
        g.fillEllipse (c.x - 4, c.y - 5, 4, 3);
    }

    // saturation character: transfer curve + description
    const int type = jlimit (0, 4, satButtons.getSelected());
    {
        auto r = curveBox.toFloat().reduced (10.0f);
        g.setColour (col::goldDim.withAlpha (0.25f));
        g.drawLine (r.getX(), r.getCentreY(), r.getRight(), r.getCentreY(), 0.6f);
        g.drawLine (r.getCentreX(), r.getY(), r.getCentreX(), r.getBottom(), 0.6f);
        g.setColour (col::goldDim.withAlpha (0.35f));
        g.drawLine (r.getX(), r.getBottom(), r.getRight(), r.getY(), 0.6f);

        const float driveDb = proc.apvts.getRawParameterValue (ParamIDs::drive)->load() * 0.24f;
        const float hs[] = { 1.0f, 0.40f, 0.35f, 0.50f, 0.45f };
        const float k = jmax (0.05f, hs[type] * Decibels::decibelsToGain (driveDb) * 2.2f);
        const float norm = type == 0 ? 1.0f : 1.0f / jmax (1.0e-3f, shapeFor (type, k));

        Path curve;
        for (int i = 0; i <= 80; ++i)
        {
            const float x = -1.0f + 2.0f * (float) i / 80.0f;
            const float y = jlimit (-1.6f, 1.6f, (type == 0 ? x : shapeFor (type, k * x) * norm));
            const Point<float> pt { r.getCentreX() + x * r.getWidth() * 0.5f, r.getCentreY() - y * r.getHeight() * 0.5f / 1.6f };
            if (i == 0) curve.startNewSubPath (pt); else curve.lineTo (pt);
        }
        g.setColour (col::amber.withAlpha (0.25f));
        g.strokePath (curve, PathStrokeType (5.0f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour (col::amber);
        g.strokePath (curve, PathStrokeType (1.8f, PathStrokeType::curved, PathStrokeType::rounded));
    }
    {
        auto r = descBox.reduced (12, 8);
        g.setColour (col::textDim);
        g.setFont (uiFont (11.0f, true, 0.24f));
        g.drawText ("CHARACTER", r.removeFromTop (14), Justification::centredLeft, false);
        g.setColour (col::amber);
        g.setFont (uiFont (17.0f, true, 0.12f));
        g.drawText (BKPOptoGlowProcessor::satTypeNames()[type], r.removeFromTop (24), Justification::centredLeft, false);
        g.setColour (col::text.withAlpha (0.85f));
        g.setFont (uiFont (13.0f));
        g.drawFittedText (satDescriptions[type], r.reduced (0, 2), Justification::topLeft, 4, 1.0f);
    }
}

void MainPanel::timerCallback()
{
    const float gr = proc.meterGr.load();
    vu.setValues (gr, proc.meterIn.load(), proc.meterOut.load());

    const float target = 0.25f + 0.75f * jlimit (0.0f, 1.0f, gr / 8.0f);
    const float newLamp = lamp + (target - lamp) * 0.2f;
    if (std::abs (newLamp - lamp) > 0.002f)
    {
        lamp = newLamp;
        repaint (lampArea.expanded (14));
    }

    // redraw the curve when drive changes
    const float d = proc.apvts.getRawParameterValue (ParamIDs::drive)->load();
    if (std::abs (d - lastDrive) > 0.05f)
    {
        lastDrive = d;
        repaint (curveBox.expanded (2));
    }

    if (presetBox.getSelectedId() != proc.getCurrentProgram() + 1)
        presetBox.setSelectedId (proc.getCurrentProgram() + 1, dontSendNotification);
}

} // namespace ui

//==============================================================================
BKPOptoGlowEditor::BKPOptoGlowEditor (BKPOptoGlowProcessor& p)
    : AudioProcessorEditor (&p), panel (p)
{
    addAndMakeVisible (panel);
    setResizable (true, true);
    setResizeLimits (ui::MainPanel::W * 3 / 4, ui::MainPanel::H * 3 / 4,
                     ui::MainPanel::W * 2, ui::MainPanel::H * 2);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) ui::MainPanel::W / (double) ui::MainPanel::H);
    setSize (ui::MainPanel::W, ui::MainPanel::H);
}

void BKPOptoGlowEditor::resized()
{
    const float scale = (float) getWidth() / (float) ui::MainPanel::W;
    panel.setBounds (0, 0, ui::MainPanel::W, ui::MainPanel::H);
    panel.setTransform (AffineTransform::scale (scale));
}
