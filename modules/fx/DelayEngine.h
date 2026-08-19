#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/** WayloDelay: three independent delay taps (Left / Mid / Right), each with
    its own adjustable delay time and a fixed pan position, sharing a single
    global feedback amount plus Wet/Dry mix controls.
 */
class WayloDelayEngine : public juce::AudioProcessor
{
public:
    //==============================================================================
    WayloDelayEngine();
    ~WayloDelayEngine() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Delay time range, shared by all three taps
    static constexpr float minDelayMs = 1.0f;
    static constexpr float maxDelayMs = 2000.0f;

    juce::AudioParameterFloat* delayTimeLeftParam = nullptr;
    juce::AudioParameterFloat* delayTimeMidParam = nullptr;
    juce::AudioParameterFloat* delayTimeRightParam = nullptr;
    juce::AudioParameterFloat* feedbackParam = nullptr;
    juce::AudioParameterFloat* wetParam = nullptr;
    juce::AudioParameterFloat* dryParam = nullptr;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WayloDelayEngine)

    // One mono delay line per tap. Input is a mono sum of the dry signal
    // plus that tap's own feedback; output is panned into the stereo field
    // in processBlock (hard left / centre / hard right).
    juce::dsp::DelayLine<float> delayLineLeft;
    juce::dsp::DelayLine<float> delayLineMid;
    juce::dsp::DelayLine<float> delayLineRight;

    float feedbackStateLeft = 0.0f;
    float feedbackStateMid = 0.0f;
    float feedbackStateRight = 0.0f;

    double currentSampleRate = 44100.0;

    float msToSamples (float ms) const noexcept
    {
        return static_cast<float> (currentSampleRate) * ms * 0.001f;
    }
};
