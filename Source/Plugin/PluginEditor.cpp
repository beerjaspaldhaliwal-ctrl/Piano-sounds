// Keyverse - plug-in editor implementation.
#include "PluginEditor.h"

using namespace kvui;

namespace
{
juce::Font font (float size, bool bold = false)
{
    return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
}

const char* engineGroupName (kv::Engine e)
{
    switch (e)
    {
        case kv::Engine::Piano:  return "Acoustic Piano (modal strings)";
        case kv::Engine::EPiano: return "Electric Piano (tine / reed)";
        case kv::Engine::Pluck:  return "Plucked String (waveguide)";
        case kv::Engine::Mallet: return "Mallet & Bell (modal)";
        case kv::Engine::Organ:  return "Organ";
        case kv::Engine::Analog: return "Analog Synth";
        case kv::Engine::FM:     return "FM Synth";
        case kv::Engine::None:
        case kv::Engine::Count:  break;
    }
    return "No Engine";
}

const char* engineShortName (kv::Engine e)
{
    switch (e)
    {
        case kv::Engine::Piano:  return "Piano";
        case kv::Engine::EPiano: return "E.Piano";
        case kv::Engine::Pluck:  return "String";
        case kv::Engine::Mallet: return "Mallet";
        case kv::Engine::Organ:  return "Organ";
        case kv::Engine::Analog: return "Analog";
        case kv::Engine::FM:     return "FM";
        case kv::Engine::None:
        case kv::Engine::Count:  break;
    }
    return "-";
}
} // namespace

//==============================================================================
LookAndFeel::LookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, Colours::bg);
    setColour (juce::ListBox::backgroundColourId, Colours::panel);
    setColour (juce::ListBox::outlineColourId, Colours::outline);
    setColour (juce::Label::textColourId, Colours::text);
    setColour (juce::TextEditor::backgroundColourId, Colours::panelHi);
    setColour (juce::TextEditor::textColourId, Colours::text);
    setColour (juce::TextEditor::outlineColourId, Colours::outline);
    setColour (juce::TextEditor::focusedOutlineColourId, Colours::accent);
    setColour (juce::CaretComponent::caretColourId, Colours::accent);
    setColour (juce::TextButton::buttonColourId, Colours::panelHi);
    setColour (juce::TextButton::buttonOnColourId, Colours::accent);
    setColour (juce::TextButton::textColourOffId, Colours::text);
    setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    setColour (juce::Slider::textBoxTextColourId, Colours::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::backgroundColourId, Colours::panelHi);
    setColour (juce::ComboBox::textColourId, Colours::text);
    setColour (juce::ComboBox::outlineColourId, Colours::outline);
    setColour (juce::ComboBox::arrowColourId, Colours::accent);
    setColour (juce::PopupMenu::backgroundColourId, Colours::panel);
    setColour (juce::PopupMenu::textColourId, Colours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Colours::accent);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::black);
    setColour (juce::ToggleButton::textColourId, Colours::text);
    setColour (juce::ToggleButton::tickColourId, Colours::accent);
    setColour (juce::ToggleButton::tickDisabledColourId, Colours::dim);
    setColour (juce::ScrollBar::thumbColourId, Colours::outline);
    setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Colours::accent.withAlpha (0.8f));
    setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, Colours::accent.withAlpha (0.3f));
    setColour (juce::AlertWindow::backgroundColourId, Colours::panel);
    setColour (juce::AlertWindow::textColourId, Colours::text);
}

void LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end, juce::Slider& s)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
    const float r = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto c = bounds.getCentre();
    const float angle = start + pos * (end - start);
    const float thick = juce::jmax (3.0f, r * 0.13f);

    g.setColour (Colours::panelHi);
    g.fillEllipse (c.x - r * 0.72f, c.y - r * 0.72f, r * 1.44f, r * 1.44f);
    g.setColour (Colours::outline);
    g.drawEllipse (c.x - r * 0.72f, c.y - r * 0.72f, r * 1.44f, r * 1.44f, 1.2f);

    juce::Path track;
    track.addCentredArc (c.x, c.y, r - thick, r - thick, 0.0f, start, end, true);
    g.setColour (Colours::outline);
    g.strokePath (track, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // bipolar macros (0.5 = preset) draw from the top
    const bool bipolar = s.getProperties().contains ("bipolar");
    const float from = bipolar ? (start + end) * 0.5f : start;
    juce::Path arc;
    arc.addCentredArc (c.x, c.y, r - thick, r - thick, 0.0f, juce::jmin (from, angle), juce::jmax (from, angle), true);
    g.setColour (s.findColour (juce::Slider::rotarySliderFillColourId).getAlpha() > 0 && s.getProperties().contains ("alt") ? Colours::accent2 : Colours::accent);
    g.strokePath (arc, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path ptr;
    ptr.addRoundedRectangle (-1.5f, -r * 0.68f, 3.0f, r * 0.38f, 1.5f);
    g.setColour (Colours::text);
    g.fillPath (ptr, juce::AffineTransform::rotation (angle).translated (c.x, c.y));
}

void LookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float,
                                    juce::Slider::SliderStyle style, juce::Slider& s)
{
    if (style == juce::Slider::LinearVertical)
    {
        const auto r = juce::Rectangle<float> ((float) x + (float) w * 0.3f, (float) y, (float) w * 0.4f, (float) h);
        g.setColour (Colours::panelHi);
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (Colours::accent.withAlpha (0.85f));
        const bool centre = s.getProperties().contains ("centre");
        const float mid = r.getCentreY();
        if (centre) g.fillRoundedRectangle (r.withTop (juce::jmin (pos, mid)).withBottom (juce::jmax (pos, mid)), 3.0f);
        else        g.fillRoundedRectangle (r.withTop (pos), 3.0f);
        g.setColour (Colours::text);
        g.fillRoundedRectangle (r.getX() - 3.0f, pos - 3.0f, r.getWidth() + 6.0f, 6.0f, 2.0f);
        return;
    }
    const auto r = juce::Rectangle<float> ((float) x, (float) y + (float) h * 0.5f - 3.0f, (float) w, 6.0f);
    g.setColour (Colours::panelHi);
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (Colours::accent);
    g.fillRoundedRectangle (r.withRight (pos), 3.0f);
    g.setColour (Colours::text);
    g.fillEllipse (pos - 5.0f, r.getCentreY() - 5.0f, 10.0f, 10.0f);
}

void LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    auto col = b.getToggleState() ? Colours::accent : Colours::panelHi;
    if (over) col = col.brighter (0.08f);
    if (down) col = col.darker (0.15f);
    g.setColour (col);
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (r, 5.0f, 1.0f);
}

void LookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) w, (float) h).reduced (0.5f);
    g.setColour (Colours::panelHi);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (r, 4.0f, 1.0f);
    juce::Path arrow;
    const float ax = (float) w - 14.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
    g.setColour (Colours::accent);
    g.fillPath (arrow);
}

//==============================================================================
ParamRow::ParamRow (const kv::PD& d, float* target, std::function<void (bool)> changed)
    : desc (d), value (target), onChange (std::move (changed))
{
    label.setText (desc.label, juce::dontSendNotification);
    label.setFont (font (13.0f));
    label.setColour (juce::Label::textColourId, Colours::dim);
    addAndMakeVisible (label);
    const bool structural = juce::String (desc.id) == "engine";

    if (desc.kind == kv::PK::Choice)
    {
        combo = std::make_unique<juce::ComboBox>();
        juce::StringArray items;
        items.addTokens (desc.choices, "|", "");
        for (int i = 0; i < items.size(); ++i) combo->addItem (items[i], i + 1);
        combo->setSelectedId ((int) std::lround (*value) + 1, juce::dontSendNotification);
        combo->onChange = [this, structural]
        {
            *value = (float) (combo->getSelectedId() - 1);
            if (onChange) onChange (structural);
        };
        addAndMakeVisible (*combo);
    }
    else if (desc.kind == kv::PK::Toggle)
    {
        toggle = std::make_unique<juce::ToggleButton>();
        toggle->setToggleState (*value > 0.5f, juce::dontSendNotification);
        toggle->onClick = [this]
        {
            *value = toggle->getToggleState() ? 1.0f : 0.0f;
            if (onChange) onChange (false);
        };
        addAndMakeVisible (*toggle);
    }
    else
    {
        slider = std::make_unique<juce::Slider> (juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
        slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 74, 20);
        slider->setColour (juce::Slider::textBoxBackgroundColourId, Colours::bg);
        slider->setColour (juce::Slider::textBoxTextColourId, Colours::text);
        slider->setColour (juce::Slider::textBoxOutlineColourId, Colours::outline);
        slider->setColour (juce::Slider::textBoxHighlightColourId, Colours::accent.withAlpha (0.4f));
        juce::NormalisableRange<double> range (desc.lo, desc.hi);
        if (desc.kind == kv::PK::Int) range.interval = 1.0;
        if (desc.kind == kv::PK::Log)
        {
            if (desc.lo > 0.0f) range.setSkewForCentre (std::sqrt ((double) desc.lo * desc.hi));
            else range.skew = 0.35;
        }
        slider->setNormalisableRange (range);
        const float span = std::fabs (desc.hi - desc.lo);
        slider->setNumDecimalPlacesToDisplay (desc.kind == kv::PK::Int ? 0 : (span <= 2.0f ? 3 : (span <= 60.0f ? 1 : 0)));
        if (juce::String (desc.unit).isNotEmpty()) slider->setTextValueSuffix (juce::String (" ") + desc.unit);
        slider->setValue (*value, juce::dontSendNotification);
        slider->setDoubleClickReturnValue (false, 0.0);
        slider->onValueChange = [this, structural]
        {
            *value = (float) slider->getValue();
            if (onChange) onChange (structural);
        };
        addAndMakeVisible (*slider);
    }
}

void ParamRow::resized()
{
    auto r = getLocalBounds().reduced (6, 2);
    label.setBounds (r.removeFromLeft (juce::jmin (150, r.getWidth() / 2)));
    if (slider) slider->setBounds (r);
    if (combo) combo->setBounds (r.reduced (0, 1));
    if (toggle) toggle->setBounds (r.removeFromLeft (30));
}

void ParamRow::paint (juce::Graphics& g)
{
    g.setColour (Colours::outline.withAlpha (0.35f));
    g.drawHorizontalLine (getHeight() - 1, 6.0f, (float) getWidth() - 6.0f);
}

void SectionHeader::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().reduced (2, 3);
    g.setColour (Colours::panelHi);
    g.fillRoundedRectangle (r.toFloat(), 4.0f);
    g.setColour (Colours::accent);
    g.fillRect (r.removeFromLeft (3));
    g.setColour (Colours::text);
    g.setFont (font (13.5f, true));
    g.drawText (title, r.reduced (8, 0), juce::Justification::centredLeft);
}

//==============================================================================
KeyverseEditor::KeyverseEditor (KeyverseProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);

    logo.setText ("KEYVERSE", juce::dontSendNotification);
    logo.setFont (font (24.0f, true));
    logo.setColour (juce::Label::textColourId, Colours::accent);
    addAndMakeVisible (logo);

    presetName.setFont (font (20.0f, true));
    presetName.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (presetName);
    presetInfo.setFont (font (12.5f));
    presetInfo.setColour (juce::Label::textColourId, Colours::dim);
    presetInfo.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (presetInfo);
    voicesLabel.setFont (font (12.0f));
    voicesLabel.setColour (juce::Label::textColourId, Colours::dim);
    voicesLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (voicesLabel);

    prevButton.onClick = [this] { proc.step (-1, visible); };
    nextButton.onClick = [this] { proc.step (1, visible); };
    saveButton.onClick = [this] { savePresetDialog(); };
    favButton.onClick = [this]
    {
        const auto& es = proc.entries();
        const int i = proc.currentEntryIndex();
        if (juce::isPositiveAndBelow (i, (int) es.size())) proc.toggleFavourite (es[(size_t) i]);
        refreshHeader();
        if (currentCategory == "Favourites") refilter();
        presetList.repaint();
    };
    for (auto* b : { &prevButton, &nextButton, &saveButton, &favButton }) addAndMakeVisible (*b);

    performTab.setClickingTogglesState (false);
    editTab.setClickingTogglesState (false);
    performTab.onClick = [this] { showPage (false); };
    editTab.onClick = [this] { showPage (true); };
    addAndMakeVisible (performTab);
    addAndMakeVisible (editTab);

    // browser
    categoryNames.push_back ("All");
    categoryNames.push_back ("Favourites");
    for (const auto& c : kv::categories()) categoryNames.push_back (c);
    categoryNames.push_back ("User");
    categoryList.setRowHeight (27);
    categoryList.setColour (juce::ListBox::backgroundColourId, Colours::panel);
    addAndMakeVisible (categoryList);
    presetList.setRowHeight (26);
    addAndMakeVisible (presetList);
    search.setTextToShowWhenEmpty ("Search presets (name, type, tag)...", Colours::dim);
    search.setFont (font (14.0f));
    search.onTextChange = [this] { refilter(); };
    addAndMakeVisible (search);
    countLabel.setFont (font (12.0f));
    countLabel.setColour (juce::Label::textColourId, Colours::dim);
    addAndMakeVisible (countLabel);

    // perform page
    for (int i = 0; i < KeyverseProcessor::numMacros; ++i)
    {
        auto& k = macroKnobs[i];
        k.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        k.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        k.getProperties().set ("bipolar", true);
        if (i >= 4) k.getProperties().set ("alt", true);
        k.setDoubleClickReturnValue (true, 0.5);
        k.setPopupDisplayEnabled (true, true, this);
        performPage.addAndMakeVisible (k);
        attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, KeyverseProcessor::macroId (i), k));
        auto& l = macroLabels[i];
        l.setText (KeyverseProcessor::macroName (i), juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setFont (font (i < 4 ? 14.0f : 12.5f, i < 4));
        performPage.addAndMakeVisible (l);
    }
    volumeKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    volumeKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    volumeKnob.setPopupDisplayEnabled (true, true, this);
    volumeKnob.setDoubleClickReturnValue (true, 0.0);
    performPage.addAndMakeVisible (volumeKnob);
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "volume", volumeKnob));
    volumeLabel.setText ("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType (juce::Justification::centred);
    volumeLabel.setFont (font (12.5f));
    performPage.addAndMakeVisible (volumeLabel);
    soundInfo.setFont (font (13.0f));
    soundInfo.setColour (juce::Label::textColourId, Colours::dim);
    soundInfo.setJustificationType (juce::Justification::topLeft);
    performPage.addAndMakeVisible (soundInfo);
    addAndMakeVisible (performPage);

    // edit page
    const char* tabNames[] = { "Layer 1", "Layer 2", "Global FX" };
    for (int i = 0; i < 3; ++i)
    {
        layerTabs[i].setButtonText (tabNames[i]);
        layerTabs[i].onClick = [this, i] { editLayer = i; rebuildEditPanel(); };
        addChildComponent (layerTabs[i]);
    }
    editViewport.setViewedComponent (&editContent, false);
    editViewport.setScrollBarsShown (true, false);
    addChildComponent (editViewport);

    // performance strip
    keyboard.setAvailableRange (21, 108);
    keyboard.setLowestVisibleKey (21);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xfff2f0eb));
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff15161a));
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff6a6a6a));
    keyboard.setOctaveForMiddleC (4);
    addAndMakeVisible (keyboard);

    modWheel.setSliderStyle (juce::Slider::LinearVertical);
    modWheel.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    modWheel.setRange (0, 127, 1);
    modWheel.onValueChange = [this] { sendCC (1, (int) modWheel.getValue()); };
    addAndMakeVisible (modWheel);
    pitchWheel.setSliderStyle (juce::Slider::LinearVertical);
    pitchWheel.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    pitchWheel.setRange (-8192, 8191, 1);
    pitchWheel.setValue (0, juce::dontSendNotification);
    pitchWheel.getProperties().set ("centre", true);
    pitchWheel.onValueChange = [this]
    {
        proc.uiMidi.addMessageToQueue (juce::MidiMessage::pitchWheel (1, (int) pitchWheel.getValue() + 8192)
                                           .withTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001));
    };
    pitchWheel.onDragEnd = [this] { pitchWheel.setValue (0); };
    addAndMakeVisible (pitchWheel);
    sustainButton.setClickingTogglesState (true);
    sustainButton.onClick = [this] { sendCC (64, sustainButton.getToggleState() ? 127 : 0); };
    addAndMakeVisible (sustainButton);

    setResizable (true, true);
    setResizeLimits (980, 640, 2600, 1700);
    setSize (1200, 780);

    categoryList.selectRow (0);
    refilter();
    refreshHeader();
    showPage (false);
    startTimerHz (15);
}

KeyverseEditor::~KeyverseEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void KeyverseEditor::sendCC (int cc, int value)
{
    proc.uiMidi.addMessageToQueue (juce::MidiMessage::controllerEvent (1, cc, value)
                                       .withTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001));
}

//==============================================================================
void KeyverseEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colours::bg);
    auto r = getLocalBounds();
    auto header = r.removeFromTop (64);
    g.setColour (Colours::panel);
    g.fillRect (header);
    g.setColour (Colours::outline);
    g.drawHorizontalLine (header.getBottom() - 1, 0.0f, (float) getWidth());

    auto strip = getLocalBounds().removeFromBottom (118);
    g.setColour (Colours::panel);
    g.fillRect (strip);
    g.setColour (Colours::outline);
    g.drawHorizontalLine (strip.getY(), 0.0f, (float) getWidth());

    g.setColour (Colours::dim);
    g.setFont (font (10.5f));
    g.drawText ("MOD", modWheel.getBounds().withY (strip.getY() + 2).withHeight (14), juce::Justification::centred);
    g.drawText ("PITCH", pitchWheel.getBounds().withY (strip.getY() + 2).withHeight (14), juce::Justification::centred);

    // centre display frame
    const auto disp = presetName.getBounds().getUnion (presetInfo.getBounds()).expanded (10, 4);
    g.setColour (Colours::bg);
    g.fillRoundedRectangle (disp.toFloat(), 8.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (disp.toFloat(), 8.0f, 1.0f);

    // right panel background
    const auto rp = performPage.getBounds().getUnion (editViewport.getBounds()).expanded (0, 0);
    g.setColour (Colours::panel);
    g.fillRoundedRectangle (rp.toFloat().expanded (6.0f), 8.0f);
}

void KeyverseEditor::resized()
{
    auto r = getLocalBounds();
    auto header = r.removeFromTop (64).reduced (14, 10);
    logo.setBounds (header.removeFromLeft (170));
    voicesLabel.setBounds (header.removeFromRight (110));
    header.removeFromRight (8);
    saveButton.setBounds (header.removeFromRight (64).reduced (0, 6));
    header.removeFromRight (6);
    favButton.setBounds (header.removeFromRight (40).reduced (0, 6));
    header.removeFromRight (16);
    nextButton.setBounds (header.removeFromRight (36).reduced (0, 6));
    header.removeFromLeft (16);
    prevButton.setBounds (header.removeFromLeft (36).reduced (0, 6));
    header.reduce (16, 0);
    presetName.setBounds (header.removeFromTop (header.getHeight() * 6 / 10));
    presetInfo.setBounds (header);

    auto strip = r.removeFromBottom (118).reduced (10, 10);
    auto wheels = strip.removeFromLeft (110);
    sustainButton.setBounds (wheels.removeFromBottom (24).reduced (2, 0));
    wheels.removeFromTop (14);
    modWheel.setBounds (wheels.removeFromLeft (52).reduced (6, 2));
    pitchWheel.setBounds (wheels.removeFromLeft (52).reduced (6, 2));
    strip.removeFromLeft (8);
    keyboard.setBounds (strip);
    keyboard.setKeyWidth ((float) strip.getWidth() / 52.0f);

    r.reduce (10, 10);
    auto left = r.removeFromLeft (210);
    categoryList.setBounds (left);
    r.removeFromLeft (10);
    auto right = r.removeFromRight (juce::jlimit (380, 520, getWidth() / 3));
    r.removeFromRight (16);
    auto top = r.removeFromTop (34);
    countLabel.setBounds (top.removeFromRight (90));
    search.setBounds (top.reduced (0, 2));
    r.removeFromTop (6);
    presetList.setBounds (r);

    right.reduce (6, 6);
    auto tabs = right.removeFromTop (30);
    performTab.setBounds (tabs.removeFromLeft (tabs.getWidth() / 2).reduced (2, 0));
    editTab.setBounds (tabs.reduced (2, 0));
    right.removeFromTop (8);
    performPage.setBounds (right);
    auto lt = right.removeFromTop (28);
    const int tw = lt.getWidth() / 3;
    for (int i = 0; i < 3; ++i) layerTabs[i].setBounds (lt.removeFromLeft (tw).reduced (2, 0));
    right.removeFromTop (6);
    editViewport.setBounds (right);

    // perform page layout
    auto pr = performPage.getLocalBounds();
    auto row1 = pr.removeFromTop (juce::jmin (170, pr.getHeight() / 3));
    const int kw = row1.getWidth() / 4;
    for (int i = 0; i < 4; ++i)
    {
        auto cell = row1.removeFromLeft (kw);
        macroLabels[i].setBounds (cell.removeFromBottom (22));
        macroKnobs[i].setBounds (cell.reduced (6));
    }
    pr.removeFromTop (12);
    auto row2 = pr.removeFromTop (juce::jmin (120, pr.getHeight() / 2));
    for (int i = 4; i < 7; ++i)
    {
        auto cell = row2.removeFromLeft (kw);
        macroLabels[i].setBounds (cell.removeFromBottom (20));
        macroKnobs[i].setBounds (cell.reduced (12));
    }
    volumeLabel.setBounds (row2.removeFromBottom (20));
    volumeKnob.setBounds (row2.reduced (12));
    pr.removeFromTop (14);
    soundInfo.setBounds (pr.reduced (8, 0));

    if (editVisible) rebuildEditPanel();
}

//==============================================================================
void KeyverseEditor::CategoryModel::paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected)
{
    if (! juce::isPositiveAndBelow (row, (int) ed.categoryNames.size())) return;
    const auto& name = ed.categoryNames[(size_t) row];
    const bool header = row < 2 || name == "User";
    if (selected)
    {
        g.setColour (Colours::panelHi);
        g.fillRect (0, 0, w, h);
        g.setColour (Colours::accent);
        g.fillRect (0, 3, 3, h - 6);
    }
    g.setColour (selected ? Colours::text : (header ? Colours::accent2 : Colours::text.withAlpha (0.85f)));
    g.setFont (font (14.0f, selected));
    g.drawText (name, 12, 0, w - 50, h, juce::Justification::centredLeft);
    g.setColour (Colours::dim);
    g.setFont (font (11.5f));
    if (juce::isPositiveAndBelow (row, (int) ed.categoryCounts.size()))
        g.drawText (juce::String (ed.categoryCounts[(size_t) row]), w - 44, 0, 36, h, juce::Justification::centredRight);
}

void KeyverseEditor::PresetModel::paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected)
{
    if (! juce::isPositiveAndBelow (row, (int) ed.visible.size())) return;
    const int idx = ed.visible[(size_t) row];
    const auto& e = ed.proc.entries()[(size_t) idx];
    const bool current = idx == ed.proc.currentEntryIndex();
    if (selected || current)
    {
        g.setColour (current ? Colours::accent.withAlpha (0.16f) : Colours::panelHi);
        g.fillRect (0, 0, w, h);
    }
    else if (row % 2)
    {
        g.setColour (Colours::panelHi.withAlpha (0.35f));
        g.fillRect (0, 0, w, h);
    }
    g.setColour (current ? Colours::accent : Colours::text);
    g.setFont (font (14.0f, current));
    g.drawText (e.name, 12, 0, w / 2, h, juce::Justification::centredLeft, true);
    g.setColour (Colours::dim);
    g.setFont (font (12.0f));
    g.drawText (e.subcategory, w / 2, 0, w / 2 - 70, h, juce::Justification::centredLeft, true);
    if (ed.currentCategory == "All" || ed.currentCategory == "Favourites")
        g.drawText (e.category.upToFirstOccurrenceOf (" ", false, false), w / 2 + (w / 2 - 70) / 2, 0, (w / 2 - 70) / 2, h, juce::Justification::centredRight, true);
    const bool fav = ed.proc.isFavourite (e);
    g.setColour (fav ? Colours::accent : Colours::outline);
    g.setFont (font (16.0f));
    g.drawText (fav ? juce::String::fromUTF8 ("\xe2\x98\x85") : juce::String::fromUTF8 ("\xe2\x98\x86"), w - 34, 0, 26, h, juce::Justification::centred);
}

void KeyverseEditor::PresetModel::listBoxItemClicked (int row, const juce::MouseEvent& e)
{
    if (! juce::isPositiveAndBelow (row, (int) ed.visible.size())) return;
    const int idx = ed.visible[(size_t) row];
    if (e.x > ed.presetList.getVisibleRowWidth() - 38)
    {
        ed.proc.toggleFavourite (ed.proc.entries()[(size_t) idx]);
        if (ed.currentCategory == "Favourites") ed.refilter();
        ed.presetList.repaint();
        ed.refreshHeader();
        return;
    }
    ed.proc.loadEntry (idx);
}

void KeyverseEditor::PresetModel::returnKeyPressed (int row)
{
    if (juce::isPositiveAndBelow (row, (int) ed.visible.size())) ed.proc.loadEntry (ed.visible[(size_t) row]);
}

void KeyverseEditor::categoryChanged (int row)
{
    if (! juce::isPositiveAndBelow (row, (int) categoryNames.size())) return;
    currentCategory = categoryNames[(size_t) row];
    refilter();
}

void KeyverseEditor::refilter()
{
    const auto& es = proc.entries();
    juce::StringArray words;
    words.addTokens (search.getText().toLowerCase(), " ", "");
    words.removeEmptyStrings();

    categoryCounts.assign (categoryNames.size(), 0);
    visible.clear();
    for (int i = 0; i < (int) es.size(); ++i)
    {
        const auto& e = es[(size_t) i];
        const juce::String hay = (e.name + " " + e.category + " " + e.subcategory + " " + e.tags).toLowerCase();
        bool match = true;
        for (const auto& w : words) if (! hay.contains (w)) { match = false; break; }
        if (! match) continue;
        const bool fav = proc.isFavourite (e);
        for (int c = 0; c < (int) categoryNames.size(); ++c)
        {
            const auto& cn = categoryNames[(size_t) c];
            if (cn == "All" || (cn == "Favourites" && fav) || cn == e.category) ++categoryCounts[(size_t) c];
        }
        if (currentCategory == "All" || (currentCategory == "Favourites" && fav) || currentCategory == e.category)
            visible.push_back (i);
    }
    presetList.updateContent();
    presetList.repaint();
    categoryList.repaint();
    countLabel.setText (juce::String ((int) visible.size()) + " sounds", juce::dontSendNotification);
    countLabel.setJustificationType (juce::Justification::centredRight);
    selectCurrentInList();
}

void KeyverseEditor::selectCurrentInList()
{
    for (int r = 0; r < (int) visible.size(); ++r)
        if (visible[(size_t) r] == proc.currentEntryIndex())
        {
            presetList.selectRow (r, true, true);
            return;
        }
    presetList.deselectAllRows();
}

void KeyverseEditor::refreshHeader()
{
    presetName.setText (proc.currentPresetName() + (proc.isModified() ? " *" : ""), juce::dontSendNotification);
    const auto& es = proc.entries();
    const int i = proc.currentEntryIndex();
    juce::String sub;
    bool fav = false;
    if (juce::isPositiveAndBelow (i, (int) es.size()))
    {
        sub = es[(size_t) i].subcategory;
        fav = proc.isFavourite (es[(size_t) i]);
    }
    auto& snd = proc.editableSound();
    juce::String engines = engineShortName (snd.layer[0].engine());
    if (snd.numLayers > 1 && snd.layer[1].engine() != kv::Engine::None)
        engines += juce::String (" + ") + engineShortName (snd.layer[1].engine());
    presetInfo.setText (proc.currentPresetCategory() + "  |  " + sub + "  |  " + engines, juce::dontSendNotification);
    favButton.setButtonText (fav ? juce::String::fromUTF8 ("\xe2\x98\x85") : juce::String::fromUTF8 ("\xe2\x98\x86"));
    favButton.setToggleState (fav, juce::dontSendNotification);

    juce::String info;
    for (int l = 0; l < snd.numLayers; ++l)
    {
        const auto& L = snd.layer[l];
        if (L.engine() == kv::Engine::None) continue;
        info << "Layer " << (l + 1) << ": " << engineGroupName (L.engine());
        if ((int) L.common.keyLo > 0 || (int) L.common.keyHi < 127)
            info << "  [keys " << juce::MidiMessage::getMidiNoteName ((int) L.common.keyLo, true, true, 4)
                 << " - " << juce::MidiMessage::getMidiNoteName ((int) L.common.keyHi, true, true, 4) << "]";
        if ((int) L.common.arpMode > 0) info << "  [arpeggiator]";
        if ((int) L.common.poly == 1) info << "  [mono]";
        info << "\n";
    }
    info << "\nMacros: Brightness, Timbre, Time and Movement reshape the sound;"
            "\nMod FX, Delay and Reverb scale the effects. Centre = as designed."
            "\nDouble-click a knob to reset. Mod wheel: vibrato / rotary speed / wah.";
    soundInfo.setText (info, juce::dontSendNotification);
}

void KeyverseEditor::showPage (bool edit)
{
    editVisible = edit;
    performTab.setToggleState (! edit, juce::dontSendNotification);
    editTab.setToggleState (edit, juce::dontSendNotification);
    performPage.setVisible (! edit);
    editViewport.setVisible (edit);
    for (auto& t : layerTabs) t.setVisible (edit);
    if (edit) rebuildEditPanel();
}

void KeyverseEditor::rebuildEditPanel()
{
    for (int i = 0; i < 3; ++i) layerTabs[i].setToggleState (i == editLayer, juce::dontSendNotification);
    editRows.clear();
    editContent.removeAllChildren();
    auto& snd = proc.editableSound();
    const int width = juce::jmax (200, editViewport.getWidth() - editViewport.getScrollBarThickness() - 2);
    int y = 0;

    const juce::Component::SafePointer<KeyverseEditor> safeThis (this);
    auto rebuildLater = [safeThis]
    {
        juce::MessageManager::callAsync ([safeThis]
        {
            if (safeThis != nullptr) { safeThis->rebuildEditPanel(); safeThis->refreshHeader(); }
        });
    };
    auto changed = [this, rebuildLater] (bool structural)
    {
        proc.soundEdited (structural);
        refreshHeader();
        if (structural) rebuildLater();
    };
    auto header = [&] (const juce::String& t)
    {
        auto h = std::make_unique<SectionHeader> (t);
        h->setBounds (0, y, width, 30);
        editContent.addAndMakeVisible (*h);
        editRows.push_back (std::move (h));
        y += 32;
    };
    auto addRow = [&] (const kv::PD& d, float& v)
    {
        auto row = std::make_unique<ParamRow> (d, &v, changed);
        row->setBounds (0, y, width, 27);
        editContent.addAndMakeVisible (*row);
        editRows.push_back (std::move (row));
        y += 27;
    };

    if (editLayer < 2)
    {
        auto& L = snd.layer[editLayer];
        if (editLayer == 1)
        {
            header ("Layer 2");
            static float layer2Proxy = 0.0f;
            layer2Proxy = snd.numLayers > 1 ? 1.0f : 0.0f;
            auto row = std::make_unique<ParamRow> (kv::tog ("numLayers", "Layer 2 Active"), &layer2Proxy,
                                                   [this, rebuildLater] (bool)
                                                   {
                                                       auto& s = proc.editableSound();
                                                       s.numLayers = layer2Proxy > 0.5f ? 2 : 1;
                                                       if (s.numLayers == 2 && s.layer[1].engine() == kv::Engine::None)
                                                           s.layer[1].common.engine = (float) (int) kv::Engine::Analog;
                                                       proc.soundEdited (true);
                                                       refreshHeader();
                                                       rebuildLater();
                                                   });
            row->setBounds (0, y, width, 27);
            editContent.addAndMakeVisible (*row);
            editRows.push_back (std::move (row));
            y += 27;
            if (snd.numLayers < 2) { editContent.setSize (width, y + 10); return; }
        }
        header ("Layer " + juce::String (editLayer + 1) + " - Performance & Range");
        L.common.visit (addRow);
        header (engineGroupName (L.engine()));
        switch (L.engine())
        {
            case kv::Engine::Piano:  L.piano.visit (addRow); break;
            case kv::Engine::EPiano: L.ep.visit (addRow); break;
            case kv::Engine::Pluck:  L.pluck.visit (addRow); break;
            case kv::Engine::Mallet: L.mallet.visit (addRow); break;
            case kv::Engine::Organ:  L.organ.visit (addRow); break;
            case kv::Engine::Analog: L.analog.visit (addRow); break;
            case kv::Engine::FM:     L.fm.visit (addRow); break;
            case kv::Engine::None:
            case kv::Engine::Count:  break;
        }
        header ("Insert Effects");
        L.fx.visit (addRow);
    }
    else
    {
        header ("Delay / Reverb / Lo-Fi / Master");
        snd.gfx.visit (addRow);
    }
    editContent.setSize (width, y + 10);
}

void KeyverseEditor::savePresetDialog()
{
    auto* w = new juce::AlertWindow ("Save User Preset", "Name your sound:", juce::MessageBoxIconType::NoIcon, this);
    w->addTextEditor ("name", proc.currentPresetName().upToFirstOccurrenceOf (" *", false, false) + " (User)");
    w->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    w->enterModalState (true, juce::ModalCallbackFunction::create ([this, w] (int result)
    {
        if (result == 1)
        {
            const auto name = w->getTextEditorContents ("name");
            if (proc.saveUserPreset (name))
            {
                refilter();
                refreshHeader();
            }
        }
    }), true);
}

void KeyverseEditor::timerCallback()
{
    voicesLabel.setText (juce::String (proc.activeVoices()) + " voices", juce::dontSendNotification);
    if (proc.presetVersion() != lastVersion)
    {
        lastVersion = proc.presetVersion();
        refreshHeader();
        if (currentCategory == "User" || currentCategory == "Favourites") refilter();
        selectCurrentInList();
        presetList.repaint();
        if (editVisible) rebuildEditPanel();
    }
}
