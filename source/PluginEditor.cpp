#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
WayloPianoAudioProcessorEditor::WayloPianoAudioProcessorEditor (WayloPianoAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      keyboardComponent (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lookAndFeel);
    logoImage = juce::ImageCache::getFromMemory (BinaryData::waylopiano_png, BinaryData::waylopiano_pngSize);
    wayloLogoImage = juce::ImageCache::getFromMemory (BinaryData::waylologo_png, BinaryData::waylologo_pngSize);

    auto& patchChoices = audioProcessor.patchParam->choices;
    for (int idx = 0; idx < patchChoices.size(); ++idx)
        patchCombo.addItem (patchChoices[idx], idx + 1);
    patchCombo.setSelectedItemIndex (audioProcessor.patchParam->getIndex(), juce::dontSendNotification);
    patchCombo.onChange = [this] { *audioProcessor.patchParam = patchCombo.getSelectedItemIndex(); };
    addAndMakeVisible (patchCombo);

    patchLabel.setText ("Patch", juce::dontSendNotification);
    patchLabel.setJustificationType (juce::Justification::centred);
    patchLabel.attachToComponent (&patchCombo, false);
    addAndMakeVisible (patchLabel);

    overdriveButton.setButtonText ("Overdrive");
    overdriveButton.setToggleState (audioProcessor.overdriveParam->get(), juce::dontSendNotification);
    overdriveButton.onClick = [this] { *audioProcessor.overdriveParam = overdriveButton.getToggleState(); };
    addAndMakeVisible (overdriveButton);

    int i = 0;
    setupKnob (i++, *audioProcessor.outputGainParam, "Volume");
    setupKnob (i++, *audioProcessor.nativeDelayParam, "Delay");
    setupKnob (i++, *audioProcessor.toneParam, "Tone");
    setupKnob (i++, *audioProcessor.tremoloPanParam, "Tremolo Pan");
    setupKnob (i++, *audioProcessor.chorusWetParam, "Chorus Wet");
    setupKnob (i++, *audioProcessor.extraDelayTimeLeftParam, "Delay2 Time L");
    setupKnob (i++, *audioProcessor.extraDelayTimeMidParam, "Delay2 Time M");
    setupKnob (i++, *audioProcessor.extraDelayTimeRightParam, "Delay2 Time R");
    setupKnob (i++, *audioProcessor.extraDelayFeedbackParam, "Delay2 Feedback");
    setupKnob (i++, *audioProcessor.extraDelayWetParam, "Delay2 Wet");
    jassert (i == kNumKnobs);

    addAndMakeVisible (keyboardComponent);
    keyboardComponent.setAvailableRange (24, 96);

    setSize (975, 545);
}

WayloPianoAudioProcessorEditor::~WayloPianoAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void WayloPianoAudioProcessorEditor::setupKnob (int index, juce::AudioParameterFloat& param, const juce::String& labelText)
{
    knobParams[index] = &param;
    knobNames[index] = labelText;

    auto& slider = knobs[index];
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 68, 17);
    slider.setRange (param.range.start, param.range.end, 0.0);
    slider.setValue (param.get(), juce::dontSendNotification);
    slider.onValueChange = [&param, &slider] { param.setValueNotifyingHost (param.convertTo0to1 ((float) slider.getValue())); };
    slider.onDragStart = [&param] { param.beginChangeGesture(); };
    slider.onDragEnd = [&param] { param.endChangeGesture(); };
    addAndMakeVisible (slider);

    auto& label = knobLabels[index];
    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (12.0f));
    addAndMakeVisible (label);
}

void WayloPianoAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    if (logoImage.isValid())
    {
        auto area = getLocalBounds().reduced (11);
        auto logoArea = area.removeFromTop (logoHeight);
        const float aspect = (float) logoImage.getWidth() / (float) logoImage.getHeight();
        const int drawWidth = juce::jmin (logoArea.getWidth(), juce::roundToInt ((float) logoHeight * aspect));
        const int x = logoArea.getCentreX() - drawWidth / 2;
        g.drawImage (logoImage, x, logoArea.getY(), drawWidth, logoHeight, 0, 0, logoImage.getWidth(), logoImage.getHeight());
    }

    if (wayloLogoImage.isValid())
    {
        constexpr int badgeHeight = 60;
        const float aspect = (float) wayloLogoImage.getWidth() / (float) wayloLogoImage.getHeight();
        const int badgeWidth = juce::roundToInt ((float) badgeHeight * aspect);
        const int x = getWidth() - badgeWidth - 11;
        const int y = 11;
        g.drawImage (wayloLogoImage, x, y, badgeWidth, badgeHeight, 0, 0, wayloLogoImage.getWidth(), wayloLogoImage.getHeight());
    }
}

void WayloPianoAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (11);
    area.removeFromTop (logoHeight);

    area.removeFromTop (20); // room for patch combo label
    auto topRow = area.removeFromTop (24);
    patchCombo.setBounds (topRow.removeFromLeft (topRow.getWidth() / 2).reduced (5, 0));
    overdriveButton.setBounds (topRow.reduced (5, 0));

    area.removeFromTop (11);

    constexpr int kCols = 5;
    constexpr int kRows = (kNumKnobs + kCols - 1) / kCols;
    constexpr int kCellHeight = 120;
    const int knobAreaHeight = kCellHeight * kRows;
    auto knobArea = area.removeFromTop (knobAreaHeight);
    const int colWidth = knobArea.getWidth() / kCols;

    for (int i = 0; i < kNumKnobs; ++i)
    {
        const int row = i / kCols;
        const int col = i % kCols;
        juce::Rectangle<int> cell (knobArea.getX() + col * colWidth, knobArea.getY() + row * kCellHeight, colWidth, kCellHeight);
        knobLabels[i].setBounds (cell.removeFromTop (16));
        knobs[i].setBounds (cell.reduced (8));
    }

    area.removeFromTop (11);
    keyboardComponent.setBounds (area.removeFromTop (105));
}
