#include "PluginEditor.h"

using namespace juce;

namespace gvui
{
static Font uiFont (float size, bool bold = false, float kerning = 0.0f)
{
    Font f { FontOptions (size, bold ? Font::bold : Font::plain) };
    return kerning > 0.0f ? f.withExtraKerningFactor (kerning) : f;
}

//==============================================================================
LookAndFeel::LookAndFeel()
{
    setColour (PopupMenu::backgroundColourId, Colour (0xff111724));
    setColour (PopupMenu::textColourId, col::text);
    setColour (PopupMenu::highlightedBackgroundColourId, col::cyanDeep.withAlpha (0.85f));
    setColour (PopupMenu::highlightedTextColourId, Colours::white);
    setColour (ComboBox::textColourId, col::cyan);
}

void LookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                    float startAngle, float endAngle, Slider& slider)
{
    const auto area = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float size = jmin (area.getWidth(), area.getHeight());
    const auto c = area.getCentre();
    const float arcR = size * 0.5f - size * 0.06f;
    const float arcW = jmax (2.5f, size * 0.035f);
    const float bodyR = arcR - size * 0.10f;
    const float angle = startAngle + pos * (endAngle - startAngle);

    Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (Colour (0xff05070c));
    g.strokePath (track, PathStrokeType (arcW + 2.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour (Colour (0xff1d2638));
    g.strokePath (track, PathStrokeType (arcW, PathStrokeType::curved, PathStrokeType::rounded));

    float from = startAngle;
    if (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0)
        from = startAngle + (float) slider.valueToProportionOfLength (0.0) * (endAngle - startAngle);
    if (std::abs (angle - from) > 0.01f)
    {
        Path v;
        v.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, jmin (from, angle), jmax (from, angle), true);
        g.setColour (col::cyan.withAlpha (0.15f));
        g.strokePath (v, PathStrokeType (arcW * 3.4f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setGradientFill (ColourGradient (col::violet, c.x - arcR, c.y + arcR, col::cyan, c.x + arcR, c.y - arcR, false));
        g.strokePath (v, PathStrokeType (arcW, PathStrokeType::curved, PathStrokeType::rounded));
    }

    Path body;
    body.addEllipse (c.x - bodyR, c.y - bodyR, bodyR * 2, bodyR * 2);
    DropShadow (Colours::black.withAlpha (0.8f), (int) (size * 0.12f), { 0, (int) (size * 0.04f) }).drawForPath (g, body);
    g.setGradientFill (ColourGradient (Colour (0xff3a4558), c.x - bodyR * 0.6f, c.y - bodyR * 0.8f,
                                       Colour (0xff0a0d14), c.x + bodyR * 0.5f, c.y + bodyR, true));
    g.fillPath (body);
    for (int i = 0; i < 40; ++i)
    {
        const float a = (float) i / 40.0f * MathConstants<float>::twoPi;
        g.setColour (i % 2 == 0 ? Colours::black.withAlpha (0.45f) : Colours::white.withAlpha (0.05f));
        g.drawLine ({ c.getPointOnCircumference (bodyR * 0.84f, a), c.getPointOnCircumference (bodyR * 0.98f, a) }, 1.0f);
    }
    g.setGradientFill (ColourGradient (col::silver, c.x, c.y - bodyR, Colour (0xff2a3446), c.x, c.y + bodyR, false));
    g.drawEllipse (c.x - bodyR, c.y - bodyR, bodyR * 2, bodyR * 2, jmax (1.0f, size * 0.012f));

    const float capR = bodyR * 0.68f;
    g.setGradientFill (ColourGradient (Colour (0xff2c3548), c.x, c.y - capR, Colour (0xff0d1119), c.x, c.y + capR, false));
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2, capR * 2);
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.14f), c.x, c.y - capR, Colours::transparentWhite, c.x, c.y, false));
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2, capR * 2);

    Path ptr;
    ptr.startNewSubPath (c.getPointOnCircumference (bodyR * 0.30f, angle));
    ptr.lineTo (c.getPointOnCircumference (bodyR * 0.90f, angle));
    g.setColour (col::cyan.withAlpha (0.3f));
    g.strokePath (ptr, PathStrokeType (size * 0.055f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour (col::cyan);
    g.strokePath (ptr, PathStrokeType (jmax (2.0f, size * 0.024f), PathStrokeType::curved, PathStrokeType::rounded));
}

void LookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool hi, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool L = ! b.isConnectedOnLeft(), R = ! b.isConnectedOnRight();
    Path p;
    p.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), 5.0f, 5.0f, L, R, L, R);
    if (b.getToggleState())
    {
        g.setGradientFill (ColourGradient (Colour (0xff7fe6ff), 0, r.getY(), Colour (0xff2a8cff), 0, r.getBottom(), false));
        g.fillPath (p);
        g.setColour (Colours::white.withAlpha (0.35f));
        g.drawLine (r.getX() + 3, r.getY() + 1.5f, r.getRight() - 3, r.getY() + 1.5f, 1.0f);
        g.setColour (Colour (0xff0b3a7a));
        g.strokePath (p, PathStrokeType (1.0f));
    }
    else
    {
        g.setGradientFill (ColourGradient (Colour (down ? 0xff0b0f17 : 0xff1c2536), 0, r.getY(), Colour (0xff0f141e), 0, r.getBottom(), false));
        g.fillPath (p);
        g.setColour (hi ? col::cyan.withAlpha (0.55f) : col::edge.brighter (0.15f));
        g.strokePath (p, PathStrokeType (1.0f));
    }
}

void LookAndFeel::drawButtonText (Graphics& g, TextButton& b, bool hi, bool)
{
    g.setFont (uiFont (jmin (12.5f, b.getHeight() * 0.46f), true, 0.12f));
    g.setColour (b.getToggleState() ? Colour (0xff061428) : (hi ? col::text : col::textDim));
    g.drawText (b.getButtonText(), b.getLocalBounds().reduced (2, 0), Justification::centred, false);
}

void LookAndFeel::drawComboBox (Graphics& g, int w, int h, bool, int, int, int, int, ComboBox& box)
{
    auto r = Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
    g.setGradientFill (ColourGradient (Colour (0xff070a10), 0, 0, Colour (0xff141b28), 0, (float) h, false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (box.isMouseOver (true) ? col::cyan.withAlpha (0.6f) : col::edge.brighter (0.2f));
    g.drawRoundedRectangle (r, 6.0f, 1.0f);
    Path a;
    const float ax = (float) w - 18.0f, ay = (float) h * 0.5f;
    a.addTriangle (ax - 5, ay - 2.5f, ax + 5, ay - 2.5f, ax, ay + 3.5f);
    g.setColour (col::cyan);
    g.fillPath (a);
}

Font LookAndFeel::getComboBoxFont (ComboBox&) { return uiFont (14.0f, true, 0.04f); }
Font LookAndFeel::getPopupMenuFont()          { return uiFont (14.0f); }
void LookAndFeel::positionComboBoxText (ComboBox& box, Label& l)
{
    l.setBounds (12, 1, box.getWidth() - 36, box.getHeight() - 2);
    l.setFont (getComboBoxFont (box));
}

//==============================================================================
Knob::Knob (AudioProcessorValueTreeState& s, const String& id, const String& t, bool b) : title (t), big (b)
{
    slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (MathConstants<float>::pi * 1.23f, MathConstants<float>::pi * 2.77f, true);
    slider.setMouseDragSensitivity (big ? 260 : 200);
    addAndMakeVisible (slider);
    att = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (s, id, slider);
    if (auto* p = s.getParameter (id))
        slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
    slider.onValueChange = [this] { repaint(); };
}

void Knob::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop (big ? 20 : 16);
    r.removeFromBottom (big ? 26 : 22);
    const int s = jmin (r.getWidth(), r.getHeight());
    slider.setBounds (r.withSizeKeepingCentre (s, s));
}

void Knob::paint (Graphics& g)
{
    auto r = getLocalBounds();
    g.setColour (col::text);
    g.setFont (uiFont (big ? 12.5f : 10.5f, true, 0.16f));
    g.drawText (title, r.removeFromTop (big ? 20 : 16), Justification::centred, false);
    auto pill = r.removeFromBottom (big ? 26 : 22).withSizeKeepingCentre (big ? 80 : 64, big ? 20 : 17).toFloat();
    g.setColour (Colour (0xff05070c));
    g.fillRoundedRectangle (pill, 4.0f);
    g.setColour (col::edge);
    g.drawRoundedRectangle (pill, 4.0f, 1.0f);
    g.setColour (col::cyan);
    g.setFont (uiFont (big ? 13.0f : 11.5f, true));
    g.drawText (slider.getTextFromValue (slider.getValue()), pill, Justification::centred, false);
}

//==============================================================================
ChoiceButtons::ChoiceButtons (RangedAudioParameter& param, const StringArray& labels, int columns) : cols (columns)
{
    const int perRow = cols > 0 ? cols : labels.size();
    for (int i = 0; i < labels.size(); ++i)
    {
        auto* b = buttons.add (new TextButton (labels[i]));
        const int c = i % perRow;
        int edges = 0;
        if (c > 0) edges |= Button::ConnectedOnLeft;
        if (c < perRow - 1 && i < labels.size() - 1) edges |= Button::ConnectedOnRight;
        b->setConnectedEdges (edges);
        b->onClick = [this, i] { if (att) att->setValueAsCompleteGesture ((float) i); };
        addAndMakeVisible (b);
    }
    att = std::make_unique<ParameterAttachment> (param, [this] (float v) { update (roundToInt (v)); });
    att->sendInitialUpdate();
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
    const int perRow = cols > 0 ? cols : n;
    const int rows = (n + perRow - 1) / perRow;
    const float w = (float) getWidth() / (float) perRow;
    const float gap = 4.0f;
    const float h = ((float) getHeight() - gap * (float) (rows - 1)) / (float) rows;
    for (int i = 0; i < n; ++i)
    {
        const int c = i % perRow, r = i / perRow;
        const int x0 = roundToInt ((float) c * w), x1 = roundToInt ((float) (c + 1) * w);
        const int y0 = roundToInt ((float) r * (h + gap));
        buttons[i]->setBounds (x0, y0, x1 - x0, roundToInt (h));
    }
}

//==============================================================================
namespace
{
    const Rectangle<int> envPanel   { 24, 80, 330, 290 };
    const Rectangle<int> dispPanel  { 370, 80, 440, 290 };
    const Rectangle<int> outPanel   { 826, 80, 330, 290 };
    const Rectangle<int> spacePanel { 24, 386, 420, 250 };
    const Rectangle<int> eqPanel    { 460, 386, 220, 250 };
    const Rectangle<int> colPanel   { 696, 386, 210, 250 };
    const Rectangle<int> scPanel    { 922, 386, 234, 250 };

    const Rectangle<int> envGraph   { 386, 108, 408, 150 };
    const Rectangle<int> eqGraph    { 386, 266, 408, 92 };
    const Rectangle<int> meterArea  { 846, 316, 290, 44 };
    const Rectangle<int> duckArea   { 936, 566, 206, 58 };
    const Rectangle<int> lampArea   { 26, 20, 30, 30 };

    const char* envBlurb[] =
    {
        "Short, close space - adds life without a tail",
        "Natural small room - drums, guitars, vocals",
        "Tight studio live room",
        "Dense, warm chamber - classic vocal space",
        "Bright, smooth plate - vocals & snares",
        "Concert hall - wide and lush",
        "Huge stone church - long majestic tail",
        "Stadium - big slapback and long tail",
        "80s digital reverb - lo-fi, chorused"
    };
}

MainPanel::MainPanel (GlowVerbProcessor& p)
    : proc (p),
      satButtons (*p.apvts.getParameter (IDs::satType), { "CLEAN", "WARM", "TUBE", "TAPE" }, 2),
      srcButtons (*p.apvts.getParameter (IDs::scSource), { "INPUT", "SIDECHAIN" }),
      pre (p.apvts, IDs::predelay, "PRE-DELAY"), decay (p.apvts, IDs::decay, "DECAY"),
      size (p.apvts, IDs::size, "SIZE"), diffusion (p.apvts, IDs::diffusion, "DIFFUSION"),
      damping (p.apvts, IDs::damping, "DAMPING"), mod (p.apvts, IDs::mod, "MODULATION"),
      early (p.apvts, IDs::early, "EARLY REF"), width (p.apvts, IDs::width, "WIDTH"),
      lowCut (p.apvts, IDs::lowCut, "LOW CUT"), highCut (p.apvts, IDs::highCut, "HIGH CUT"),
      midFreq (p.apvts, IDs::midFreq, "MID FREQ"), midGain (p.apvts, IDs::midGain, "MID GAIN"),
      drive (p.apvts, IDs::drive, "DRIVE"), air (p.apvts, IDs::air, "AIR"),
      duck (p.apvts, IDs::duck, "DUCK"), thresh (p.apvts, IDs::threshold, "THRESH"),
      release (p.apvts, IDs::release, "RELEASE"),
      mix (p.apvts, IDs::mix, "MIX", true), output (p.apvts, IDs::output, "OUTPUT", true)
{
    setLookAndFeel (&lnf);
    setOpaque (true);

    for (int i = 0; i < (int) gv::envTypes().size(); ++i)
    {
        auto* b = envButtons.add (new TextButton (gv::envTypes()[(size_t) i].name));
        b->onClick = [this, i] { proc.applyEnvironment (i); };
        addAndMakeVisible (b);
    }
    typeAtt = std::make_unique<ParameterAttachment> (*p.apvts.getParameter (IDs::type), [this] (float v)
    {
        currentType = jlimit (0, 8, roundToInt (v));
        for (int i = 0; i < envButtons.size(); ++i)
            envButtons[i]->setToggleState (i == currentType, dontSendNotification);
        repaint (dispPanel);
    });
    typeAtt->sendInitialUpdate();

    for (auto* c : std::initializer_list<Component*> { &satButtons, &srcButtons, &pre, &decay, &size, &diffusion, &damping,
                                                       &mod, &early, &width, &lowCut, &highCut, &midFreq, &midGain, &drive,
                                                       &air, &duck, &thresh, &release, &mix, &output, &presetBox })
        addAndMakeVisible (c);

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, dontSendNotification);
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id > 0 && id - 1 != proc.getCurrentProgram())
            proc.setCurrentProgram (id - 1);
    };

    startTimerHz (30);
}

MainPanel::~MainPanel()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void MainPanel::resized()
{
    renderBackground();

    // environment grid 3x3
    {
        auto r = envPanel.reduced (16).withTrimmedTop (22);
        r.removeFromBottom (34);
        const int bw = (r.getWidth() - 16) / 3, bh = (r.getHeight() - 16) / 3;
        for (int i = 0; i < envButtons.size(); ++i)
            envButtons[i]->setBounds (r.getX() + (i % 3) * (bw + 8), r.getY() + (i / 3) * (bh + 8), bw, bh);
    }

    // output
    mix.setBounds    (outPanel.getX() + 14,  104, 150, 206);
    output.setBounds (outPanel.getX() + 166, 104, 150, 206);

    // space: 2 rows x 4
    {
        Knob* k[] = { &pre, &decay, &size, &diffusion, &damping, &mod, &early, &width };
        const int x0 = spacePanel.getX() + 10, y0 = spacePanel.getY() + 28, cw = 100, ch = 108;
        for (int i = 0; i < 8; ++i)
            k[i]->setBounds (x0 + (i % 4) * cw, y0 + (i / 4) * (ch + 4), cw, ch);
    }
    // eq 2x2
    {
        Knob* k[] = { &lowCut, &highCut, &midFreq, &midGain };
        const int x0 = eqPanel.getX() + 10, y0 = eqPanel.getY() + 28;
        for (int i = 0; i < 4; ++i)
            k[i]->setBounds (x0 + (i % 2) * 100, y0 + (i / 2) * 112, 100, 108);
    }
    // colour
    satButtons.setBounds (colPanel.getX() + 14, colPanel.getY() + 32, colPanel.getWidth() - 28, 52);
    drive.setBounds (colPanel.getX() + 8,   colPanel.getY() + 96, 97, 146);
    air.setBounds   (colPanel.getX() + 105, colPanel.getY() + 96, 97, 146);
    // sidechain
    srcButtons.setBounds (scPanel.getX() + 14, scPanel.getY() + 32, scPanel.getWidth() - 28, 26);
    duck.setBounds    (scPanel.getX() + 6,   scPanel.getY() + 66, 74, 106);
    thresh.setBounds  (scPanel.getX() + 80,  scPanel.getY() + 66, 74, 106);
    release.setBounds (scPanel.getX() + 154, scPanel.getY() + 66, 74, 106);

    presetBox.setBounds (W - 24 - 260, 20, 260, 30);
}

void MainPanel::renderBackground()
{
    const float sc = 2.0f;
    background = Image (Image::RGB, roundToInt (W * sc), roundToInt (H * sc), true);
    Graphics g (background);
    g.addTransform (AffineTransform::scale (sc));

    g.setGradientFill (ColourGradient (col::bgTop, 0, 0, col::bgBottom, 0, (float) H, false));
    g.fillAll();
    // soft colour washes
    g.setGradientFill (ColourGradient (col::violet.withAlpha (0.10f), W * 0.15f, H * 0.1f, Colours::transparentBlack, W * 0.5f, H * 0.6f, true));
    g.fillAll();
    g.setGradientFill (ColourGradient (col::cyan.withAlpha (0.07f), W * 0.85f, H * 0.95f, Colours::transparentBlack, W * 0.4f, H * 0.3f, true));
    g.fillAll();
    // fine grain
    Random rnd (77);
    for (int i = 0; i < 9000; ++i)
    {
        g.setColour ((rnd.nextBool() ? Colours::white : Colours::black).withAlpha (rnd.nextFloat() * 0.05f));
        g.fillRect (rnd.nextFloat() * W, rnd.nextFloat() * H, 1.0f, 1.0f);
    }

    // header
    {
        ColourGradient line (col::cyan.withAlpha (0.0f), 0, 0, col::cyan.withAlpha (0.0f), (float) W, 0, false);
        line.addColour (0.15, col::cyan.withAlpha (0.45f));
        line.addColour (0.85, col::violet.withAlpha (0.45f));
        g.setGradientFill (line);
        g.fillRect (0.0f, 66.0f, (float) W, 1.0f);
    }
    g.setGradientFill (ColourGradient (Colour (0xffbff3ff), 0, 12, col::cyanDeep, 0, 46, false));
    g.setFont (uiFont (30.0f, true, 0.16f));
    g.drawText ("GLOW VERB", 66, 10, 320, 34, Justification::centredLeft, false);
    g.setColour (col::textDim);
    g.setFont (uiFont (10.5f, true, 0.28f));
    g.drawText ("ROHIT JI  |  ALGORITHMIC REVERB", 68, 42, 420, 16, Justification::centredLeft, false);
    g.drawText ("PRESET", W - 24 - 260 - 74, 20, 64, 30, Justification::centredRight, false);

    auto drawPanel = [&] (Rectangle<int> r, const String& title)
    {
        auto rf = r.toFloat();
        DropShadow (Colours::black.withAlpha (0.65f), 16, { 0, 5 }).drawForRectangle (g, r);
        g.setGradientFill (ColourGradient (col::panelTop, 0, rf.getY(), col::panelBot, 0, rf.getBottom(), false));
        g.fillRoundedRectangle (rf, 10.0f);
        g.setColour (Colours::white.withAlpha (0.05f));
        g.drawLine (rf.getX() + 10, rf.getY() + 1, rf.getRight() - 10, rf.getY() + 1, 1.0f);
        g.setColour (col::edge);
        g.drawRoundedRectangle (rf.reduced (0.5f), 10.0f, 1.0f);
        g.setFont (uiFont (10.5f, true, 0.3f));
        const int tw = GlyphArrangement::getStringWidthInt (g.getCurrentFont(), title) + 8;
        g.setColour (col::cyan);
        g.drawText (title, r.getX() + 16, r.getY() + 6, tw + 10, 16, Justification::centredLeft, false);
        g.setColour (col::edge.brighter (0.2f));
        g.fillRect ((float) (r.getX() + 24 + tw), (float) r.getY() + 14.0f, (float) (r.getRight() - 16 - (r.getX() + 24 + tw)), 1.0f);
    };
    drawPanel (envPanel, "ENVIRONMENT");
    drawPanel (dispPanel, "SPACE VIEW");
    drawPanel (outPanel, "OUTPUT");
    drawPanel (spacePanel, "SPACE");
    drawPanel (eqPanel, "REVERB EQ");
    drawPanel (colPanel, "COLOUR");
    drawPanel (scPanel, "SIDECHAIN DUCK");

    for (auto r : { envGraph, eqGraph, duckArea })
    {
        g.setColour (Colour (0xff05070c));
        g.fillRoundedRectangle (r.toFloat(), 6.0f);
        g.setColour (col::edge);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 6.0f, 1.0f);
    }

    g.setColour (col::textDim.withAlpha (0.75f));
    g.setFont (uiFont (9.5f, true, 0.25f));
    g.drawText ("VOCALS  -  DRUMS  -  INSTRUMENTS  -  PADS", 26, H - 22, 420, 16, Justification::centredLeft, false);
    g.drawText ("v1.0  |  ROHIT JI", W - 26 - 300, H - 22, 300, 16, Justification::centredRight, false);
}

//==============================================================================
void MainPanel::paintDisplay (Graphics& g)
{
    const auto& t = gv::envTypes()[(size_t) currentType];
    const float decayS = param (IDs::decay), preMs = param (IDs::predelay);
    const float early = param (IDs::early) * 0.02f * t.erLevel;

    // ---- decay envelope -------------------------------------------------
    {
        auto r = envGraph.toFloat().reduced (10.0f, 8.0f);
        const float total = jmax (0.6f, preMs * 0.001f + decayS * 1.15f);
        auto tx = [&] (float sec) { return r.getX() + sec / total * r.getWidth(); };

        g.setColour (col::edge.withAlpha (0.6f));
        for (int i = 1; i < 4; ++i)
            g.drawHorizontalLine (roundToInt (r.getY() + r.getHeight() * (float) i / 4.0f), r.getX(), r.getRight());

        const float x0 = tx (preMs * 0.001f);
        Path env;
        env.startNewSubPath (r.getX(), r.getBottom());
        env.lineTo (x0, r.getBottom());
        env.lineTo (x0 + 3.0f, r.getY() + r.getHeight() * 0.12f);
        for (int i = 0; i <= 80; ++i)
        {
            const float sec = preMs * 0.001f + decayS * 1.15f * (float) i / 80.0f;
            const float lvlDb = -60.0f * (sec - preMs * 0.001f) / decayS;   // T60
            const float y = jmap (jlimit (-60.0f, 0.0f, lvlDb), -60.0f, 0.0f, r.getBottom(), r.getY() + r.getHeight() * 0.12f);
            env.lineTo (jmax (x0 + 3.0f, tx (sec)), y);
        }
        env.lineTo (r.getRight(), r.getBottom());
        env.closeSubPath();

        const float a = 0.35f + 0.5f * glow;
        g.setGradientFill (ColourGradient (col::cyan.withAlpha (a), r.getX(), r.getY(), col::violet.withAlpha (0.05f), r.getX(), r.getBottom(), false));
        g.fillPath (env);
        g.setColour (col::cyan.withAlpha (0.9f));
        g.strokePath (env, PathStrokeType (1.6f));

        // early reflection spikes
        if (early > 0.01f && t.erSpreadMs > 0.0f)
        {
            static const float tL[10] = { 0.043f, 0.097f, 0.151f, 0.229f, 0.307f, 0.398f, 0.519f, 0.633f, 0.781f, 0.947f };
            const float sizeMul = 0.5f + param (IDs::size) * 0.01f;
            for (int k = 0; k < 10; ++k)
            {
                const float sec = preMs * 0.001f + tL[k] * t.erSpreadMs * sizeMul * 0.001f;
                const float x = tx (sec);
                const float h = r.getHeight() * 0.8f * jmin (1.0f, early) * (1.0f - 0.07f * (float) k);
                g.setColour (Colours::white.withAlpha (0.75f));
                g.drawLine (x, r.getBottom(), x, r.getBottom() - h, 1.4f);
            }
        }

        // labels
        g.setColour (col::text);
        g.setFont (uiFont (22.0f, true, 0.12f));
        g.drawText (t.name, r.reduced (8, 4), Justification::topRight, false);
        g.setColour (col::textDim);
        g.setFont (uiFont (11.0f));
        g.drawText (envBlurb[currentType], r.reduced (8, 4).withTrimmedTop (28), Justification::topRight, false);
        g.setColour (col::cyan);
        g.setFont (uiFont (12.0f, true, 0.06f));
        g.drawText ("DECAY " + String (decayS, 2) + " s   PRE " + String (roundToInt (preMs)) + " ms",
                    r.reduced (8, 4), Justification::bottomRight, false);
    }

    // ---- EQ curve ------------------------------------------------------
    {
        auto r = eqGraph.toFloat().reduced (8.0f, 6.0f);
        using AC = dsp::IIR::ArrayCoefficients<float>;
        using C = dsp::IIR::Coefficients<float>;
        const double fs = 48000.0;
        C lc1, lc2, hc1, hc2, mb, sh;
        lc1 = AC::makeHighPass (fs, param (IDs::lowCut), 0.5412f);
        lc2 = AC::makeHighPass (fs, param (IDs::lowCut), 1.3066f);
        hc1 = AC::makeLowPass (fs, jmin (param (IDs::highCut), 21000.0f), 0.5412f);
        hc2 = AC::makeLowPass (fs, jmin (param (IDs::highCut), 21000.0f), 1.3066f);
        mb  = AC::makePeakFilter (fs, param (IDs::midFreq), 0.8f, Decibels::decibelsToGain (param (IDs::midGain)));
        sh  = AC::makeHighShelf (fs, 10000.0f, 0.7f, Decibels::decibelsToGain (param (IDs::air) * 0.06f));

        auto fx = [&] (double f) { return r.getX() + (float) (std::log (f / 20.0) / std::log (1000.0)) * r.getWidth(); };
        g.setColour (col::edge.withAlpha (0.6f));
        for (double f : { 100.0, 1000.0, 10000.0 })
            g.drawVerticalLine (roundToInt (fx (f)), r.getY(), r.getBottom());
        g.drawHorizontalLine (roundToInt (r.getCentreY()), r.getX(), r.getRight());

        Path p;
        for (int i = 0; i <= 120; ++i)
        {
            const double f = 20.0 * std::pow (1000.0, (double) i / 120.0);
            const double m = lc1.getMagnitudeForFrequency (f, fs) * lc2.getMagnitudeForFrequency (f, fs)
                           * hc1.getMagnitudeForFrequency (f, fs) * hc2.getMagnitudeForFrequency (f, fs)
                           * mb.getMagnitudeForFrequency (f, fs) * sh.getMagnitudeForFrequency (f, fs);
            const float db = jlimit (-24.0f, 12.0f, (float) Decibels::gainToDecibels (m, -60.0));
            const float y = jmap (db, -24.0f, 12.0f, r.getBottom(), r.getY());
            const float x = fx (f);
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        Path fill (p);
        fill.lineTo (r.getRight(), r.getBottom());
        fill.lineTo (r.getX(), r.getBottom());
        fill.closeSubPath();
        g.setGradientFill (ColourGradient (col::violet.withAlpha (0.35f), 0, r.getY(), col::violet.withAlpha (0.02f), 0, r.getBottom(), false));
        g.fillPath (fill);
        g.setColour (col::violet.brighter (0.3f));
        g.strokePath (p, PathStrokeType (1.6f));
        g.setColour (col::textDim);
        g.setFont (uiFont (9.5f, true, 0.2f));
        g.drawText ("WET EQ", r.reduced (4, 2), Justification::topLeft, false);
        g.drawText ("100", Rectangle<float> (fx (100) + 3, r.getBottom() - 12, 30, 12), Justification::left, false);
        g.drawText ("1k", Rectangle<float> (fx (1000) + 3, r.getBottom() - 12, 30, 12), Justification::left, false);
        g.drawText ("10k", Rectangle<float> (fx (10000) + 3, r.getBottom() - 12, 30, 12), Justification::left, false);
    }
}

void MainPanel::paintMeters (Graphics& g)
{
    struct M { const char* name; float db; };
    const M ms[] = { { "IN", inLvl }, { "WET", wetLvl }, { "OUT", outLvl } };
    auto r = meterArea.toFloat();
    const float rowH = r.getHeight() / 3.0f;
    for (int i = 0; i < 3; ++i)
    {
        auto row = Rectangle<float> (r.getX(), r.getY() + rowH * (float) i, r.getWidth(), rowH).reduced (0, 2.5f);
        g.setColour (col::textDim);
        g.setFont (uiFont (9.5f, true, 0.2f));
        g.drawText (ms[i].name, row.removeFromLeft (36), Justification::centredLeft, false);
        g.setColour (Colour (0xff05070c));
        g.fillRoundedRectangle (row, 3.0f);
        const float frac = jlimit (0.0f, 1.0f, (ms[i].db + 60.0f) / 60.0f);
        if (frac > 0.0f)
        {
            auto fillR = row.withWidth (row.getWidth() * frac);
            g.setGradientFill (ColourGradient (col::cyanDeep, row.getX(), 0, ms[i].db > -3.0f ? Colour (0xffff5a6e) : col::cyan, row.getRight(), 0, false));
            g.fillRoundedRectangle (fillR, 3.0f);
        }
    }
}

void MainPanel::paintSidechain (Graphics& g)
{
    auto r = duckArea.toFloat().reduced (10.0f, 8.0f);
    g.setColour (col::textDim);
    g.setFont (uiFont (9.5f, true, 0.2f));
    g.drawText ("DUCKING", r.removeFromTop (12), Justification::centredLeft, false);
    const bool ext = (int) param (IDs::scSource) == 1;
    const String status = ext ? (scConnected ? "EXTERNAL KEY" : "NO SIDECHAIN - USING INPUT") : "KEY: OWN INPUT";
    g.setColour (ext && ! scConnected ? Colour (0xffffb15a) : col::cyan);
    g.drawText (status, Rectangle<float> (r.getX(), r.getY() - 12, r.getWidth(), 12), Justification::centredRight, false);

    auto bar = r.removeFromTop (12).translated (0, 4);
    g.setColour (Colour (0xff111724));
    g.fillRoundedRectangle (bar, 3.0f);
    const float frac = jlimit (0.0f, 1.0f, duckLvl / 30.0f);
    if (frac > 0.0f)
    {
        auto f = bar.withLeft (bar.getRight() - bar.getWidth() * frac);
        g.setGradientFill (ColourGradient (col::violet, f.getX(), 0, col::cyan, bar.getRight(), 0, false));
        g.fillRoundedRectangle (f, 3.0f);
    }
    g.setColour (col::text);
    g.setFont (uiFont (11.0f, true));
    g.drawText ("-" + String (duckLvl, 1) + " dB", r.translated (0, 6), Justification::centredLeft, false);
}

void MainPanel::paint (Graphics& g)
{
    g.drawImage (background, getLocalBounds().toFloat());

    // lamp
    {
        const auto c = lampArea.toFloat().getCentre();
        g.setGradientFill (ColourGradient (col::cyan.withAlpha (0.25f + 0.45f * glow), c.x, c.y, Colours::transparentBlack, c.x + 22, c.y, true));
        g.fillEllipse (c.x - 22, c.y - 22, 44, 44);
        g.setColour (Colour (0xff06090f));
        g.fillEllipse (c.x - 10, c.y - 10, 20, 20);
        g.setGradientFill (ColourGradient (Colour (0xffd9f7ff).interpolatedWith (Colour (0xff12405a), 0.6f - 0.6f * glow), c.x - 2, c.y - 3,
                                           col::cyanDeep, c.x + 8, c.y + 8, true));
        g.fillEllipse (c.x - 8, c.y - 8, 16, 16);
    }

    // environment hint
    g.setColour (col::textDim);
    g.setFont (uiFont (10.5f));
    g.drawText ("Click an environment to load its space", envPanel.reduced (16).removeFromBottom (24), Justification::centred, false);

    paintDisplay (g);
    paintMeters (g);
    paintSidechain (g);
}

void MainPanel::timerCallback()
{
    const float w = proc.meterWet.load(), i = proc.meterIn.load(), o = proc.meterOut.load(), d = proc.meterDuck.load();
    auto smooth = [] (float& v, float target) { v = target > v ? target : v + (target - v) * 0.25f; };
    smooth (wetLvl, w); smooth (inLvl, i); smooth (outLvl, o);
    duckLvl += (d - duckLvl) * 0.3f;
    const float g = jlimit (0.0f, 1.0f, (wetLvl + 45.0f) / 40.0f);
    glow += (g - glow) * 0.2f;
    scConnected = proc.sidechainConnected.load();

    // repaint the display when a relevant parameter changed
    String sig;
    for (auto* id : { IDs::decay, IDs::predelay, IDs::size, IDs::early, IDs::lowCut, IDs::highCut, IDs::midFreq, IDs::midGain, IDs::air, IDs::scSource })
        sig << String (param (id), 2) << ";";
    if (sig != lastSignature) { lastSignature = sig; repaint (dispPanel); repaint (scPanel); }

    repaint (dispPanel.getIntersection (envGraph.expanded (2)));
    repaint (meterArea.expanded (2));
    repaint (duckArea.expanded (2));
    repaint (lampArea.expanded (16));

    if (presetBox.getSelectedId() != proc.getCurrentProgram() + 1)
        presetBox.setSelectedId (proc.getCurrentProgram() + 1, dontSendNotification);
}
} // namespace gvui

//==============================================================================
GlowVerbEditor::GlowVerbEditor (GlowVerbProcessor& p) : AudioProcessorEditor (&p), panel (p)
{
    addAndMakeVisible (panel);
    setResizable (true, true);
    setResizeLimits (gvui::MainPanel::W * 3 / 5, gvui::MainPanel::H * 3 / 5, gvui::MainPanel::W * 2, gvui::MainPanel::H * 2);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) gvui::MainPanel::W / (double) gvui::MainPanel::H);
    setSize (gvui::MainPanel::W * 17 / 20, gvui::MainPanel::H * 17 / 20);
}

void GlowVerbEditor::resized()
{
    panel.setBounds (0, 0, gvui::MainPanel::W, gvui::MainPanel::H);
    panel.setTransform (AffineTransform::scale ((float) getWidth() / (float) gvui::MainPanel::W));
}
