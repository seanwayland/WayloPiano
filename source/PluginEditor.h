#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "WayloLookAndFeel.h"

//==============================================================================
class WayloPianoAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit WayloPianoAudioProcessorEditor (WayloPianoAudioProcessor&);
    ~WayloPianoAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    WayloPianoAudioProcessor& audioProcessor;

    WayloLookAndFeel lookAndFeel;
    juce::Image logoImage;
    juce::Image wayloLogoImage;
    static constexpr int logoHeight = 90;

    juce::ComboBox patchCombo;
    juce::Label patchLabel;

    juce::ToggleButton overdriveButton;

    static constexpr int kNumKnobs = 9;
    juce::Slider knobs[kNumKnobs];
    juce::Label knobLabels[kNumKnobs];
    juce::AudioParameterFloat* knobParams[kNumKnobs] {};
    juce::String knobNames[kNumKnobs];

    juce::MidiKeyboardComponent keyboardComponent;

    void setupKnob (int index, juce::AudioParameterFloat& param, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WayloPianoAudioProcessorEditor)
};
