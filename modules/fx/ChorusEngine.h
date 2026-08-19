/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#pragma once
#define MAX_DELAY_TIME 1
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
 */
class WayloChorusEngine  : public juce::AudioProcessor
{
public:
    //==============================================================================
    WayloChorusEngine();
    ~WayloChorusEngine() override;
    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif
    
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
    float linInterp(float sample_x, float sample_x1, float inPhase);

    // Added for use as a headless internal effect embedded in another
    // processor (see WayloPianoAudioProcessor) - mDryGainParameter/
    // mWetGainParameter are otherwise private.
    void setDryGain (float value) { *mDryGainParameter = value; }
    void setWetGain (float value) { *mWetGainParameter = value; }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WayloChorusEngine)
    float mDelayTimeInSamples;
    float mDelayReadHead;
    float feedback;
    float mLFOphase;
    float mLFOrate;
    float mPanOne;
    
    float mDelayTwoTimeInSamples;
    float mDelayTwoReadHead;
    float feedbackTwo;
    float mLFOphaseTwo;
    float mLFOrateTwo;
    float mPanTwo;
    
    float mDelayThreeTimeInSamples;
    float mDelayThreeReadHead;
    float feedbackThree;
    float mLFOphaseThree;
    float mLFOrateThree;
    float mPanThree;
    
    float mDelayFourTimeInSamples;
    float mDelayFourReadHead;
    float feedbackFour;
    float mLFOphaseFour;
    float mLFOrateFour;
    float mPanFour;
    
    float mDelayFiveTimeInSamples;
    float mDelayFiveReadHead;
    float feedbackFive;
    float mLFOphaseFive;
    float mLFOrateFive;
    float mPanFive;
    
    float mDelaySixTimeInSamples;
    float mDelaySixReadHead;
    float feedbackSix;
    float mLFOphaseSix;
    float mLFOrateSix;
    float mPanSix;
    
    float mDelaySevenTimeInSamples;
    float mDelaySevenReadHead;
    float feedbackSeven;
    float mLFOphaseSeven;
    float mLFOrateSeven;
    float mPanSeven;
    
    float mDelayEightTimeInSamples;
    float mDelayEightReadHead;
    float feedbackEight;
    float mLFOphaseEight;
    float mLFOrateEight;
    float mPanEight;
    
    
    
    
    
    
    
    
    float mDelayOneTimeParameter;
    juce::AudioParameterFloat* mDryGainParameter;
    juce::AudioParameterFloat* mWetGainParameter;
    float mRateParameter;
    float mDepthParameter;
    float mDelayOneGainParameter;
    float mDelayOneModDepthParameter;
    float mDelayOneModRateParameter;
    float mDelayOneFeedbackParameter;
    float mDelayOnePanParameter;
    
    
    float mDelayTwoTimeParameter;
    float mDelayTwoGainParameter;
    float mDelayTwoModDepthParameter;
    float mDelayTwoModRateParameter;
    float mDelayTwoFeedbackParameter;
    float mDelayTwoPanParameter;
    
    float mDelayThreeTimeParameter;
    float mDelayThreeGainParameter;
    float mDelayThreeModDepthParameter;
    float mDelayThreeModRateParameter;
    float mDelayThreeFeedbackParameter;
    float mDelayThreePanParameter;
    
    float mDelayFourTimeParameter;
    float mDelayFourGainParameter;
    float mDelayFourModDepthParameter;
    float mDelayFourModRateParameter;
    float mDelayFourFeedbackParameter;
    float mDelayFourPanParameter;
    
    float mDelayFiveTimeParameter;
    float mDelayFiveGainParameter;
    float mDelayFiveModDepthParameter;
    float mDelayFiveModRateParameter;
    float mDelayFiveFeedbackParameter;
    float mDelayFivePanParameter;
    
    float mDelaySixTimeParameter;
    float mDelaySixGainParameter;
    float mDelaySixModDepthParameter;
    float mDelaySixModRateParameter;
    float mDelaySixFeedbackParameter;
    float mDelaySixPanParameter;
    
    float mDelaySevenTimeParameter;
    float mDelaySevenGainParameter;
    float mDelaySevenModDepthParameter;
    float mDelaySevenModRateParameter;
    float mDelaySevenFeedbackParameter;
    float mDelaySevenPanParameter;
    
    float mDelayEightTimeParameter;
    float mDelayEightGainParameter;
    float mDelayEightModDepthParameter;
    float mDelayEightModRateParameter;
    float mDelayEightFeedbackParameter;
    float mDelayEightPanParameter;
    
    float mCircularBufferLeft[96000] = {  };
    float mCircularBufferRight[96000] = {  };
    float mCircularBufferLeftTwo[96000] = {  };
    float mCircularBufferRightTwo[96000] = {  };
    float mCircularBufferLeftThree[96000] = {  };
    float mCircularBufferRightThree[96000] = {  };
    float mCircularBufferLeftFour[96000] = {  };
    float mCircularBufferRightFour[96000] = {  };
    float mCircularBufferLeftFive[96000] = {  };
    float mCircularBufferRightFive[96000] = {  };
    float mCircularBufferLeftSix[96000] = {  };
    float mCircularBufferRightSix[96000] = {  };
    float mCircularBufferLeftSeven[96000] = {  };
    float mCircularBufferRightSeven[96000] = {  };
    float mCircularBufferLeftEight[96000] = {  };
    float mCircularBufferRightEight[96000] = {  };
    
    
    
    
    
    float mLastInputGain  = 0.0f;
    float mDelayTimeSmoothed;
    
    // float* mCircularBufferRight;
    //  float* mCircularBufferLeft;
    
    
    float arr2[10] = { 0 };
    
    int mCircularBufferWriteHead;
    
    int mCircularBufferLength;
    
    float mfeedbackLeft;
    float mfeedbackRight;
    
    float mLastInputGainTwo  = 0.0f;
    float mDelayTimeSmoothedTwo;
    
    //   float* mCircularBufferRightTwo;
    //  float* mCircularBufferLeftTwo;
    
    int mCircularBufferWriteHeadTwo;
    
    
    float mfeedbackLeftTwo;
    float mfeedbackRightTwo;
    
    float mLastInputGainThree  = 0.0f;
    float mDelayTimeSmoothedThree;
    
    //   float* mCircularBufferRightThree;
    //   float* mCircularBufferLeftThree;
    
    int mCircularBufferWriteHeadThree;
    
    
    
    float mfeedbackLeftThree;
    float mfeedbackRightThree;
    
    float mLastInputGainFour  = 0.0f;
    float mDelayTimeSmoothedFour;
    
    //   float* mCircularBufferRightFour;
    //   float* mCircularBufferLeftFour;
    
    int mCircularBufferWriteHeadFour;
    
    
    float mfeedbackLeftFour;
    float mfeedbackRightFour;
    
    float mLastInputGainFive  = 0.0f;
    float mDelayTimeSmoothedFive;
    
    //   float* mCircularBufferRightFive;
    //   float* mCircularBufferLeftFive;
    
    int mCircularBufferWriteHeadFive;
    
    
    
    float mfeedbackLeftFive;
    float mfeedbackRightFive;
    
    float mLastInputGainSix  = 0.0f;
    float mDelayTimeSmoothedSix;
    
    //    float* mCircularBufferRightSix;
    //    float* mCircularBufferLeftSix;
    
    int mCircularBufferWriteHeadSix;
    
    
    
    float mfeedbackLeftSix;
    float mfeedbackRightSix;
    
    
    float mLastInputGainSeven  = 0.0f;
    float mDelayTimeSmoothedSeven;
    
    //   float* mCircularBufferRightSeven;
    //    float* mCircularBufferLeftSeven;
    
    int mCircularBufferWriteHeadSeven;
    
    
    
    float mfeedbackLeftSeven;
    float mfeedbackRightSeven;
    
    
    
    float mLastInputGainEight  = 0.0f;
    float mDelayTimeSmoothedEight;
    
    //   float* mCircularBufferRightEight;
    //   float* mCircularBufferLeftEight;
    
    int mCircularBufferWriteHeadEight;
    
    
    
    float mfeedbackLeftEight;
    float mfeedbackRightEight;
    
    float panLevelOne;
    float panLevelTwo;
    float panLevelThree;
    float panLevelFour;
    float panLevelFive;
    float panLevelSix;
    float panLevelSeven;
    float panLevelEight;
    
    float panOneL;
    float panOneR;
    float panTwoL;
    float panTwoR;
    float panThreeL;
    float panThreeR;
    float panFourL;
    float panFourR;
    float panFiveL;
    float panFiveR;
    float panSixL;
    float panSixR;
    float panSevenL;
    float panSevenR;
    float panEightL;
    float panEightR;
    
    
};
