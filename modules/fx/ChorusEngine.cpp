/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "ChorusEngine.h"

//==============================================================================
WayloChorusEngine::WayloChorusEngine()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
                  // Always a stereo-in/stereo-out effect regardless of the
                  // outer plugin's JucePlugin_IsSynth/IsMidiEffect (this is
                  // embedded as a headless internal effect in WayloPiano,
                  // whose own plugin type those macros actually describe) -
                  // so this can't be macro-gated.
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                  )
#endif
{
    
    
    // DECLARE PLUGIN PARAMETERS
    
    
    mDelayOneTimeParameter = 0.0236f; // fixed WayloChorus value
    
    addParameter (mDryGainParameter = new juce::AudioParameterFloat ("gain",                                      // parameter ID
                                                                     "Gain",                                      // parameter name
                                                                     juce::NormalisableRange<float> (0.0f, 1.0f), // parameter range
                                                                     0.29f));                                     // default value (WayloChorus)
    
    
    mDelayOneGainParameter = 0.9f; // fixed WayloChorus value
    mDelayOneModDepthParameter = 0.9f; // fixed WayloChorus value
    mDelayOneModRateParameter = 0.65f; // fixed WayloChorus value
    mDelayOneFeedbackParameter = 0.0f; // fixed WayloChorus value
    
    mDelayTwoTimeParameter = 0.03f; // fixed WayloChorus value
    mDelayTwoGainParameter = 0.9f; // fixed WayloChorus value
    mDelayTwoModDepthParameter = 0.9f; // fixed WayloChorus value
    mDelayTwoModRateParameter = 0.57f; // fixed WayloChorus value
    mDelayTwoFeedbackParameter = 0.0f; // fixed WayloChorus value
    
    mDelayThreeTimeParameter = 0.036f; // fixed WayloChorus value
    mDelayThreeGainParameter = 0.9f; // fixed WayloChorus value
    mDelayThreeModDepthParameter = 0.9f; // fixed WayloChorus value
    mDelayThreeModRateParameter = 0.48f; // fixed WayloChorus value
    mDelayThreeFeedbackParameter = 0.0f; // fixed WayloChorus value
    
    mDelayFourTimeParameter = 0.028f; // fixed WayloChorus value
    mDelayFourGainParameter = 0.9f; // fixed WayloChorus value
    mDelayFourModDepthParameter = 0.9f; // fixed WayloChorus value
    mDelayFourModRateParameter = 0.44f; // fixed WayloChorus value
    mDelayFourFeedbackParameter = 0.0f; // fixed WayloChorus value
    
    mDelayFiveTimeParameter = 0.0f; // fixed WayloChorus value
    mDelayFiveGainParameter = 0.0f; // fixed WayloChorus value
    mDelayFiveModDepthParameter = 0.4f; // fixed WayloChorus value
    mDelayFiveModRateParameter = 0.0f; // fixed WayloChorus value
    mDelayFiveFeedbackParameter = 0.0f; // fixed WayloChorus value
    
    mDelaySixTimeParameter = 0.0f; // fixed WayloChorus value
    mDelaySixGainParameter = 0.0f; // fixed WayloChorus value
    mDelaySixModDepthParameter = 0.4f; // fixed WayloChorus value
    mDelaySixModRateParameter = 0.0f; // fixed WayloChorus value
    mDelaySixFeedbackParameter = 0.0f; // fixed WayloChorus value
    
    mDelaySevenTimeParameter = 0.0f; // fixed WayloChorus value
    mDelaySevenGainParameter = 0.0f; // fixed WayloChorus value
    mDelaySevenModDepthParameter = 0.4f; // fixed WayloChorus value
    mDelaySevenModRateParameter = 0.0f; // fixed WayloChorus value
    mDelaySevenFeedbackParameter = 0.0f; // fixed WayloChorus value
    
    mDelayEightTimeParameter = 0.0f; // fixed WayloChorus value
    mDelayEightGainParameter = 0.0f; // fixed WayloChorus value
    mDelayEightModDepthParameter = 0.4f; // fixed WayloChorus value
    mDelayEightModRateParameter = 0.0f; // fixed WayloChorus value
    mDelayEightFeedbackParameter = 0.0f; // fixed WayloChorus value
    
    
    addParameter (mWetGainParameter = new juce::AudioParameterFloat ("wetgain",                                      // parameter ID
                                                                     "Wet Gain",                                      // parameter name
                                                                     juce::NormalisableRange<float> (0.0f, 1.0f), // parameter range
                                                                     0.5f));                                      // default value
    
    mRateParameter = 1.0f; // fixed WayloChorus value
    
    mDepthParameter = 1.0f; // fixed WayloChorus value
    
    mDelayOnePanParameter = 0.0f; // fixed WayloChorus value
    mDelayTwoPanParameter = 1.0f; // fixed WayloChorus value
    mDelayThreePanParameter = 0.0f; // fixed WayloChorus value
    mDelayFourPanParameter = 1.0f; // fixed WayloChorus value
    mDelayFivePanParameter = 0.0f; // fixed WayloChorus value
    mDelaySixPanParameter = 1.0f; // fixed WayloChorus value
    mDelaySevenPanParameter = 0.0f; // fixed WayloChorus value
    mDelayEightPanParameter = 1.0f; // fixed WayloChorus value
    
    
    
    
    
    // initialize buffer values
    mCircularBufferWriteHead = mCircularBufferWriteHeadTwo =  mCircularBufferWriteHeadThree = mCircularBufferWriteHeadFour = 0;
    
    mCircularBufferWriteHeadFive = mCircularBufferWriteHeadSix = mCircularBufferWriteHeadSeven = mCircularBufferWriteHeadEight = 0;
    
    mDelayTimeInSamples = mDelayTwoTimeInSamples = mDelayThreeTimeInSamples = mDelayFourTimeInSamples = 0.0;
    
    mDelayFiveTimeInSamples = mDelaySixTimeInSamples = mDelaySevenTimeInSamples = mDelayEightTimeInSamples = 0.0;
    
    mDelayReadHead = mDelayTwoReadHead = mDelayThreeReadHead = mDelayFourReadHead = mDelayFiveReadHead = mDelaySixReadHead = mDelaySevenReadHead = mDelayEightReadHead = 0.0;
    
    mfeedbackLeft = mfeedbackRight = mfeedbackLeftTwo = mfeedbackRightTwo = mfeedbackLeftThree = mfeedbackLeftFour = mfeedbackRightFour = 0.0;
    
    mfeedbackLeftFive = mfeedbackRightFive = mfeedbackLeftSix = mfeedbackRightSix = mfeedbackLeftSeven = mfeedbackRightSeven = mfeedbackLeftEight = mfeedbackRightEight = 0.0;
    
    mLFOphase = mLFOphaseTwo = mLFOphaseThree = mLFOphaseFour = mLFOphaseFive = mLFOphaseSix = mLFOphaseSeven = mLFOphaseEight = 0;
    
    mLFOrate = mLFOrateTwo= mLFOrateThree = mLFOrateFour = mLFOrateFive = mLFOrateSix = mLFOrateSeven = mLFOrateEight = 0.3f;
    
    
    
}

WayloChorusEngine::~WayloChorusEngine()
{
}

//==============================================================================
const juce::String WayloChorusEngine::getName() const
{
    return JucePlugin_Name;
}

bool WayloChorusEngine::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool WayloChorusEngine::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool WayloChorusEngine::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double WayloChorusEngine::getTailLengthSeconds() const
{
    return 0.0;
}

int WayloChorusEngine::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int WayloChorusEngine::getCurrentProgram()
{
    return 0;
}

void WayloChorusEngine::setCurrentProgram (int index)
{
}

const juce::String WayloChorusEngine::getProgramName (int index)
{
    return {};
}

void WayloChorusEngine::changeProgramName (int index, const juce::String& newName)
{
}

float WayloChorusEngine::linInterp(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase * sample_x1;
    
}

//==============================================================================
void WayloChorusEngine::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    
    
    mLFOphase = mLFOphaseTwo = mLFOphaseThree = mLFOphaseFour =  mLFOphaseFive =  mLFOphaseSix = mLFOphaseSeven = mLFOphaseEight = 0;
    
    mLFOrateTwo = mLFOrateTwo = mLFOrateThree = mLFOrateFour =  mLFOrateFive =  mLFOrateSix =  mLFOrateSeven = mLFOrateEight = 0.3f  ;
    
    
    
    
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // mCircularBufferLength = sampleRate*MAX_DELAY_TIME;
    mCircularBufferLength = 96000;
    mCircularBufferWriteHead = 0;
    mCircularBufferWriteHeadTwo = 0;
    mCircularBufferWriteHeadThree = 0;
    mCircularBufferWriteHeadFour = 0;
    mCircularBufferWriteHeadFive = 0;
    mCircularBufferWriteHeadSix = 0;
    mCircularBufferWriteHeadSeven = 0;
    mCircularBufferWriteHeadEight = 0;
    
    
    mDelayReadHead = 0.0;
    mDelayTwoReadHead = 0.0;
    mDelayThreeReadHead = 0.0;
    mDelayFourReadHead =0.0;
    mDelayFiveReadHead =0.0;
    mDelaySixReadHead =0.0;
    mDelaySevenReadHead =0.0;
    mDelayEightReadHead =0.0;
    
    
    
    
}

void WayloChorusEngine::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WayloChorusEngine::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo() &&
        layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()) {
        return true;
    } else {
        return false;
    }
    
    
    /***
     #if JucePlugin_IsMidiEffect
     juce::ignoreUnused (layouts);
     return true;
     #else
     // This is the place where you check if the layout is supported.
     // In this template code we only support mono or stereo.
     if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
     return false;
     
     // This checks if the input layout matches the output layout
     #if ! JucePlugin_IsSynth
     if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
     return false;
     #endif
     
     return true;
     #endif
     ***/
}
#endif

void WayloChorusEngine::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    float* LeftChannel;
    float* RightChannel;
    
    if (totalNumInputChannels >1){
        LeftChannel = buffer.getWritePointer(0);
        RightChannel = buffer.getWritePointer(0);
    }
    else
    {
        LeftChannel = buffer.getWritePointer(0);
        RightChannel = buffer.getWritePointer(1);
        
    }
    
    
    
    
    
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        
        // generate the next LFO value
        
        float lfoOut = sin(2*juce::MathConstants<float>::pi * mLFOphase);
        float lfoOutTwo = sin(2*juce::MathConstants<float>::pi * mLFOphaseTwo);
        float lfoOutThree = sin(2*juce::MathConstants<float>::pi * mLFOphaseThree);
        float lfoOutFour = sin(2*juce::MathConstants<float>::pi * mLFOphaseFour);
        float lfoOutFive = sin(2*juce::MathConstants<float>::pi * mLFOphaseFive);
        float lfoOutSix = sin(2*juce::MathConstants<float>::pi * mLFOphaseSix);
        float lfoOutSeven = sin(2*juce::MathConstants<float>::pi * mLFOphaseSeven);
        float lfoOutEight = sin(2*juce::MathConstants<float>::pi * mLFOphaseEight);
        
        
        
        // move the LFO phase thru the sine wave
        
        mLFOphase += mDelayOneModRateParameter* mRateParameter / getSampleRate();
        mLFOphaseTwo += mDelayTwoModRateParameter* mRateParameter / getSampleRate();
        mLFOphaseThree += mDelayThreeModRateParameter* mRateParameter / getSampleRate();
        mLFOphaseFour += mDelayFourModRateParameter* mRateParameter / getSampleRate();
        mLFOphaseFive += mDelayFiveModRateParameter* mRateParameter / getSampleRate();
        mLFOphaseSix += mDelaySixModRateParameter* mRateParameter / getSampleRate();
        mLFOphaseSeven += mDelaySevenModRateParameter* mRateParameter / getSampleRate();
        mLFOphaseEight += mDelayEightModRateParameter* mRateParameter / getSampleRate();
        
        
        
        
        // LFO phase is moving between zero and one
        if ( mLFOphase > 1){
            mLFOphase -= 1;
        }
        if ( mLFOphaseTwo > 1){
            mLFOphaseTwo -= 1;
        }
        
        if ( mLFOphaseThree > 1){
            mLFOphaseThree -= 1;
        }
        
        if ( mLFOphaseFour > 1){
            mLFOphaseFour -= 1;
        }
        if ( mLFOphaseFive > 1){
            mLFOphaseFive -= 1;
        }
        if ( mLFOphaseSix > 1){
            mLFOphaseSix -= 1;
        }
        
        if ( mLFOphaseSeven > 1){
            mLFOphaseSeven -= 1;
        }
        if ( mLFOphaseEight > 1){
            mLFOphaseEight -= 1;
        }
        
        // attenuate the "depth" of the modulaton
        lfoOut *= mDelayOneModDepthParameter;
        lfoOutTwo *= mDelayTwoModDepthParameter;
        lfoOutThree *= mDelayThreeModDepthParameter;
        lfoOutFour *= mDelayFourModDepthParameter;
        lfoOutFive *= mDelayFiveModDepthParameter;
        lfoOutSix *= mDelaySixModDepthParameter;
        lfoOutSeven *= mDelaySevenModDepthParameter;
        lfoOutEight *= mDelayEightModDepthParameter;
        
        
        //convert -1 to 1 to changes in delay time of .005 min and .03 max
        
        float lfoOutMapped = juce::jmap(lfoOut,-1.f,1.f,0.001f, 0.1f* mDepthParameter);
        float lfoOutMappedTwo = juce::jmap(lfoOutTwo,-1.f,1.f,0.001f, 0.1f* mDepthParameter);
        float lfoOutMappedThree = juce::jmap(lfoOutThree,-1.f,1.f,0.001f, 0.1f* mDepthParameter);
        float lfoOutMappedFour = juce::jmap(lfoOutFour,-1.f,1.f,0.001f, 0.1f* mDepthParameter);
        float lfoOutMappedFive = juce::jmap(lfoOutFive,-1.f,1.f,0.001f, 0.1f* mDepthParameter);
        float lfoOutMappedSix = juce::jmap(lfoOutSix,-1.f,1.f,0.001f, 0.1f* mDepthParameter);
        float lfoOutMappedSeven = juce::jmap(lfoOutSeven,-1.f,1.f,0.001f, 0.1f* mDepthParameter);
        float lfoOutMappedEight = juce::jmap(lfoOutEight,-1.f,1.f,0.001f, 0.1f* mDepthParameter);
        
        
        
        
        
        // get the value of the delay time knob as an in in samples
        int dtime = static_cast<int>(mDelayOneTimeParameter*getSampleRate());
        int dtimeTwo = static_cast<int>(mDelayTwoTimeParameter*getSampleRate());
        int dtimeThree = static_cast<int>(mDelayThreeTimeParameter*getSampleRate());
        int dtimeFour = static_cast<int>(mDelayFourTimeParameter*getSampleRate());
        int dtimeFive = static_cast<int>(mDelayFiveTimeParameter*getSampleRate());
        int dtimeSix = static_cast<int>(mDelaySixTimeParameter*getSampleRate());
        int dtimeSeven = static_cast<int>(mDelaySevenTimeParameter*getSampleRate());
        int dtimeEight = static_cast<int>(mDelayEightTimeParameter*getSampleRate());
        
        
        /// add the modulated delay time to the value the delay time is set to with the slider
        
        // Clamp to the circular buffer's capacity: dtime alone can reach
        // mCircularBufferLength at high sample rates (e.g. 96kHz), and the
        // LFO modulation above adds up to another 10% on top, which can
        // push the read head past what the single-wraparound logic below
        // corrects for, causing an out-of-bounds buffer access.
        auto maxDelaySamples = static_cast<float> (mCircularBufferLength - 1);
        mDelayTimeInSamples = juce::jlimit (0.0f, maxDelaySamples, dtime*(1+ lfoOutMapped)) ;
        mDelayTwoTimeInSamples = juce::jlimit (0.0f, maxDelaySamples, dtimeTwo*(1+ lfoOutMappedTwo));
        mDelayThreeTimeInSamples = juce::jlimit (0.0f, maxDelaySamples, dtimeThree*(1+ lfoOutMappedThree));
        mDelayFourTimeInSamples = juce::jlimit (0.0f, maxDelaySamples, dtimeFour*(1+ lfoOutMappedFour)) ;
        mDelayFiveTimeInSamples = juce::jlimit (0.0f, maxDelaySamples, dtimeFive*(1+ lfoOutMappedFive)) ;
        mDelaySixTimeInSamples = juce::jlimit (0.0f, maxDelaySamples, dtimeSix*(1+ lfoOutMappedSix));
        mDelaySevenTimeInSamples = juce::jlimit (0.0f, maxDelaySamples, dtimeSeven*(1+ lfoOutMappedSeven));
        mDelayEightTimeInSamples = juce::jlimit (0.0f, maxDelaySamples, dtimeEight*(1+ lfoOutMappedEight)) ;
        
        
        
        
        // shove some of the input into the circular buffer also add some of the feedback
        mCircularBufferLeft[mCircularBufferWriteHead] = LeftChannel[i] + mfeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = RightChannel[i] + mfeedbackRight;
        mCircularBufferLeftTwo[mCircularBufferWriteHeadTwo] = LeftChannel[i] + mfeedbackLeftTwo;
        mCircularBufferRightTwo[mCircularBufferWriteHeadTwo] = RightChannel[i] + mfeedbackRightTwo;
        mCircularBufferLeftThree[mCircularBufferWriteHeadThree] = LeftChannel[i] + mfeedbackLeftThree;
        mCircularBufferRightThree[mCircularBufferWriteHeadThree] = RightChannel[i] + mfeedbackRightThree;
        mCircularBufferLeftFour[mCircularBufferWriteHeadFour] = LeftChannel[i] + mfeedbackLeftFour;
        mCircularBufferRightFour[mCircularBufferWriteHeadFour] = RightChannel[i] + mfeedbackRightFour;
        mCircularBufferLeftFive[mCircularBufferWriteHeadFive] = LeftChannel[i] + mfeedbackLeftFive;
        mCircularBufferRightFive[mCircularBufferWriteHeadFive] = RightChannel[i] +mfeedbackRightFive;
        mCircularBufferLeftSix[mCircularBufferWriteHeadSix] = LeftChannel[i] + mfeedbackLeftSix;
        mCircularBufferRightSix[mCircularBufferWriteHeadSix] = RightChannel[i] + mfeedbackRightSix;
        mCircularBufferLeftSeven[mCircularBufferWriteHeadSeven] = LeftChannel[i] + mfeedbackLeftSeven;
        mCircularBufferRightSeven[mCircularBufferWriteHeadSeven] = RightChannel[i] + mfeedbackRightSeven;
        mCircularBufferLeftEight[mCircularBufferWriteHeadEight] = LeftChannel[i] + mfeedbackLeftEight;
        mCircularBufferRightEight[mCircularBufferWriteHeadEight] = RightChannel[i] + mfeedbackRightEight;
        
        
        
        // move the read head on the circular to the new delay position
        mDelayReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;
        mDelayTwoReadHead = mCircularBufferWriteHeadTwo - mDelayTwoTimeInSamples;
        mDelayThreeReadHead = mCircularBufferWriteHeadThree - mDelayThreeTimeInSamples;
        mDelayFourReadHead = mCircularBufferWriteHeadFour - mDelayFourTimeInSamples;
        mDelayFiveReadHead = mCircularBufferWriteHeadFive - mDelayFiveTimeInSamples;
        mDelaySixReadHead = mCircularBufferWriteHeadSix - mDelaySixTimeInSamples;
        mDelaySevenReadHead = mCircularBufferWriteHeadSeven - mDelaySevenTimeInSamples;
        mDelayEightReadHead = mCircularBufferWriteHeadEight - mDelayEightTimeInSamples;
        
        
        // if read head is below zero wrap around
        if (mDelayReadHead < 0){
            mDelayReadHead += mCircularBufferLength;
        }
        
        if (mDelayTwoReadHead < 0){
            mDelayTwoReadHead += mCircularBufferLength;
        }
        if (mDelayThreeReadHead < 0){
            mDelayThreeReadHead += mCircularBufferLength;
        }
        if (mDelayFourReadHead < 0){
            mDelayFourReadHead += mCircularBufferLength;
        }
        
        if (mDelayFiveReadHead < 0){
            mDelayFiveReadHead += mCircularBufferLength;
        }
        
        if (mDelaySixReadHead < 0){
            mDelaySixReadHead += mCircularBufferLength;
        }
        
        if (mDelaySevenReadHead < 0){
            mDelaySevenReadHead += mCircularBufferLength;
        }
        
        if (mDelayEightReadHead < 0){
            mDelayEightReadHead += mCircularBufferLength;
        }
        
        
        // get the integer part of the read head
        int readHeadX = (int)mDelayReadHead;
        // get the part of the readHead after the decimal point
        float readHeadFloat = mDelayReadHead - readHeadX;
        // next integer sample position
        int readHeadX1 = readHeadX + 1;
        
        // get the integer part of the read head
        int readHeadXTwo = (int)mDelayTwoReadHead;
        // get the part of the readHead after the decimal point
        float readHeadFloatTwo = mDelayTwoReadHead - readHeadXTwo;
        // next integer sample position
        int readHeadX1Two = readHeadXTwo + 1;
        
        // get the integer part of the read head
        int readHeadXThree = (int)mDelayThreeReadHead;
        // get the part of the readHead after the decimal point
        float readHeadFloatThree = mDelayThreeReadHead - readHeadXThree;
        // next integer sample position
        int readHeadX1Three = readHeadXThree + 1;
        
        // get the integer part of the read head
        int readHeadXFour = (int)mDelayFourReadHead;
        // get the part of the readHead after the decimal point
        float readHeadFloatFour = mDelayFourReadHead - readHeadXFour;
        // next integer sample position
        int readHeadX1Four = readHeadXFour + 1;
        
        // get the integer part of the read head
        int readHeadXFive = (int)mDelayFiveReadHead;
        // get the part of the readHead after the decimal point
        float readHeadFloatFive = mDelayFiveReadHead - readHeadXFive;
        // next integer sample position
        int readHeadX1Five = readHeadXFive + 1;
        
        // get the integer part of the read head
        int readHeadXSix = (int)mDelaySixReadHead;
        // get the part of the readHead after the decimal point
        float readHeadFloatSix = mDelaySixReadHead - readHeadXSix;
        // next integer sample position
        int readHeadX1Six = readHeadXSix + 1;
        
        // get the integer part of the read head
        int readHeadXSeven = (int)mDelaySevenReadHead;
        // get the part of the readHead after the decimal point
        float readHeadFloatSeven = mDelaySevenReadHead - readHeadXSeven;
        // next integer sample position
        int readHeadX1Seven = readHeadXSeven + 1;
        
        // get the integer part of the read head
        int readHeadXEight = (int)mDelayEightReadHead;
        // get the part of the readHead after the decimal point
        float readHeadFloatEight = mDelayEightReadHead - readHeadXEight;
        // next integer sample position
        int readHeadX1Eight = readHeadXEight + 1;
        
        
        
        
        
        // if next sample position is outside the buffer
        if ( readHeadX1 >= mCircularBufferLength){
            readHeadX1 -= mCircularBufferLength;
        }
        
        if ( readHeadX1Two >= mCircularBufferLength){
            readHeadX1Two -= mCircularBufferLength;
        }
        
        if ( readHeadX1Three >= mCircularBufferLength){
            readHeadX1Three -= mCircularBufferLength;
        }
        
        if ( readHeadX1Four >= mCircularBufferLength){
            readHeadX1Four -= mCircularBufferLength;
        }
        
        if ( readHeadX1Five >= mCircularBufferLength){
            readHeadX1Five -= mCircularBufferLength;
        }
        
        if ( readHeadX1Six >= mCircularBufferLength){
            readHeadX1Six -= mCircularBufferLength;
        }
        
        if ( readHeadX1Seven >= mCircularBufferLength){
            readHeadX1Seven -= mCircularBufferLength;
        }
        
        if ( readHeadX1Eight >= mCircularBufferLength){
            readHeadX1Eight -= mCircularBufferLength;
        }
        
        
        
        
        // get the interpolated value of the delayed sample from the circular buffer
        float delay_sample_Left = WayloChorusEngine::linInterp(mCircularBufferLeft[readHeadX], mCircularBufferLeft[readHeadX1], readHeadFloat);
        float delay_sample_Right = WayloChorusEngine::linInterp(mCircularBufferRight[readHeadX], mCircularBufferRight[readHeadX1], readHeadFloat);
        float delay_sample_LeftTwo = WayloChorusEngine::linInterp(mCircularBufferLeftTwo[readHeadXTwo], mCircularBufferLeftTwo[readHeadX1Two], readHeadFloatTwo);
        float delay_sample_RightTwo = WayloChorusEngine::linInterp(mCircularBufferRightTwo[readHeadXTwo], mCircularBufferRightTwo[readHeadX1Two], readHeadFloatTwo);
        float delay_sample_LeftThree = WayloChorusEngine::linInterp(mCircularBufferLeftThree[readHeadXThree], mCircularBufferLeftThree[readHeadX1Three], readHeadFloatThree);
        float delay_sample_RightThree = WayloChorusEngine::linInterp(mCircularBufferRightThree[readHeadXThree], mCircularBufferRightThree[readHeadX1Three], readHeadFloatThree);
        float delay_sample_LeftFour = WayloChorusEngine::linInterp(mCircularBufferLeftFour[readHeadXFour], mCircularBufferLeftFour[readHeadX1Four], readHeadFloatFour);
        float delay_sample_RightFour = WayloChorusEngine::linInterp(mCircularBufferRightFour[readHeadXFour], mCircularBufferRightFour[readHeadX1Four], readHeadFloatFour);
        float delay_sample_LeftFive = WayloChorusEngine::linInterp(mCircularBufferLeftFive[readHeadXFive], mCircularBufferLeftFive[readHeadX1Five], readHeadFloatFive);
        float delay_sample_RightFive = WayloChorusEngine::linInterp(mCircularBufferRightFive[readHeadXFive], mCircularBufferRightFive[readHeadX1Five], readHeadFloatFive);
        float delay_sample_LeftSix = WayloChorusEngine::linInterp(mCircularBufferLeftSix[readHeadXSix], mCircularBufferLeftSix[readHeadX1Six], readHeadFloatSix);
        float delay_sample_RightSix = WayloChorusEngine::linInterp(mCircularBufferRightSix[readHeadXSix], mCircularBufferRightSix[readHeadX1Six], readHeadFloatSix);
        float delay_sample_LeftSeven = WayloChorusEngine::linInterp(mCircularBufferLeftSeven[readHeadXSeven], mCircularBufferLeftSeven[readHeadX1Seven], readHeadFloatSeven);
        float delay_sample_RightSeven = WayloChorusEngine::linInterp(mCircularBufferRightSeven[readHeadXSeven], mCircularBufferRightSeven[readHeadX1Seven], readHeadFloatSeven);
        float delay_sample_LeftEight = WayloChorusEngine::linInterp(mCircularBufferLeftEight[readHeadXEight], mCircularBufferLeftEight[readHeadX1Eight], readHeadFloatEight);
        float delay_sample_RightEight = WayloChorusEngine::linInterp(mCircularBufferRightEight[readHeadXEight], mCircularBufferRightEight[readHeadX1Eight], readHeadFloatEight);
        
        
        
        
        // store some delay for feedback
        
        mfeedbackLeft = delay_sample_Left* mDelayOneFeedbackParameter;
        mfeedbackRight = delay_sample_Right* mDelayOneFeedbackParameter;
        mfeedbackLeftTwo = delay_sample_LeftTwo* mDelayTwoFeedbackParameter;
        mfeedbackRightTwo = delay_sample_RightTwo* mDelayTwoFeedbackParameter;
        mfeedbackLeftThree = delay_sample_LeftThree* mDelayThreeFeedbackParameter;
        mfeedbackRightThree = delay_sample_RightThree* mDelayThreeFeedbackParameter;
        mfeedbackLeftFour = delay_sample_LeftFour* mDelayFourFeedbackParameter;
        mfeedbackRightFour = delay_sample_RightFour* mDelayFourFeedbackParameter;
        mfeedbackLeftFive = delay_sample_LeftFive* mDelayFiveFeedbackParameter;
        mfeedbackRightFive = delay_sample_RightFive* mDelayFiveFeedbackParameter;
        mfeedbackLeftSix = delay_sample_LeftSix* mDelaySixFeedbackParameter;
        mfeedbackRightSix = delay_sample_RightSix* mDelaySixFeedbackParameter;
        mfeedbackLeftSeven = delay_sample_LeftSeven* mDelaySevenFeedbackParameter;
        mfeedbackRightSeven = delay_sample_RightSeven* mDelaySevenFeedbackParameter;
        mfeedbackLeftEight = delay_sample_LeftEight* mDelayEightFeedbackParameter;
        mfeedbackRightEight = delay_sample_RightEight* mDelayEightFeedbackParameter;
        
        //mfeedbackLeftSeven = mfeedbackRightSeven = mfeedbackLeftEight = mfeedbackRightEight = 0;
        
        
        
        
        // add delay into live audio buffer
        
        /**
         At mid pan, both L and R come through 100% (as you’d expect!), and then you just cut the left as you go from 0.5->0, and cut the right as you go from 0.5->1.
         
         if (level < 0.5) { // cut right
         right = level;
         left = 0.5;  }
         if (level >= 0.5) { // cut left
         right = 0.5;
         left = 1 - level;
         }
         
         **/
        
        panLevelOne = mDelayOnePanParameter;
        panOneL = 1 - panLevelOne;
        panOneR = panLevelOne;
        panLevelTwo = mDelayTwoPanParameter;
        panTwoL = 1 - panLevelTwo;
        panTwoR = panLevelTwo;
        panLevelThree = mDelayThreePanParameter;
        panThreeL = 1 - panLevelThree;
        panThreeR = panLevelThree;
        panLevelFour = mDelayFourPanParameter;
        panFourL = 1 - panLevelFour;
        panFourR = panLevelFour;
        panLevelFive = mDelayFivePanParameter;
        panFiveL = 1 - panLevelFive;
        panFiveR = panLevelFive;
        panLevelSix = mDelaySixPanParameter;
        panSixL = 1 - panLevelSix;
        panSixR = panLevelSix;
        panLevelSeven = mDelaySevenPanParameter;
        panSevenL = 1 - panLevelSeven;
        panSevenR = panLevelSeven;
        panLevelEight = mDelayEightPanParameter;
        panEightL = 1 - panLevelEight;
        panEightR = panLevelEight;
        
        
        
        
        
        
        
        
        
        buffer.setSample(0, i, buffer.getSample(0, i)* *mDryGainParameter
                         
                         
                         /***
                          + delay_sample_Left* mDelayOneGainParameter* *mWetGainParameter + delay_sample_Right* mDelayOneGainParameter* *mWetGainParameter
                          +delay_sample_LeftThree* mDelayThreeGainParameter* *mWetGainParameter+ delay_sample_RightThree* mDelayThreeGainParameter* *mWetGainParameter
                          +delay_sample_LeftFive* mDelayFiveGainParameter* *mWetGainParameter+ delay_sample_RightFive* mDelayFiveGainParameter* *mWetGainParameter
                          +delay_sample_LeftSeven* mDelaySevenGainParameter* *mWetGainParameter+ delay_sample_RightSeven* mDelaySevenGainParameter* *mWetGainParameter
                          ***/
                         
                         + delay_sample_Left* mDelayOneGainParameter* *mWetGainParameter*panOneL
                         +delay_sample_LeftThree* mDelayThreeGainParameter* *mWetGainParameter*panThreeL
                         +delay_sample_LeftFive* mDelayFiveGainParameter* *mWetGainParameter*panFiveL
                         +delay_sample_LeftSeven* mDelaySevenGainParameter* *mWetGainParameter*panSevenL
                         + delay_sample_LeftTwo* mDelayTwoGainParameter* *mWetGainParameter*panTwoL
                         + delay_sample_LeftFour* mDelayFourGainParameter* *mWetGainParameter*panFourL
                         + delay_sample_LeftSix* mDelaySixGainParameter* *mWetGainParameter*panSixL
                         + delay_sample_LeftEight* mDelayEightGainParameter* *mWetGainParameter*panEightL
                         
                         
                         
                        
                         
                         );
        
        buffer.setSample(1, i, buffer.getSample(1, i)* *mDryGainParameter
                         
                         
                         
                         /***
                          + delay_sample_LeftTwo* mDelayTwoGainParameter* *mWetGainParameter+ delay_sample_RightTwo* mDelayTwoGainParameter* *mWetGainParameter
                          + delay_sample_LeftFour* mDelayFourGainParameter* *mWetGainParameter+ delay_sample_RightFour* mDelayFourGainParameter* *mWetGainParameter
                          + delay_sample_LeftSix* mDelaySixGainParameter* *mWetGainParameter+ delay_sample_RightSix* mDelaySixGainParameter* *mWetGainParameter
                          + delay_sample_LeftEight* mDelayEightGainParameter* *mWetGainParameter+ delay_sample_RightEight* mDelayEightGainParameter* *mWetGainParameter
                          ***/
                         
                         + delay_sample_Right* mDelayOneGainParameter* *mWetGainParameter*panOneR
                         + delay_sample_RightThree* mDelayThreeGainParameter* *mWetGainParameter*panThreeR
                         + delay_sample_RightFive* mDelayFiveGainParameter* *mWetGainParameter*panFiveR
                         + delay_sample_RightSeven* mDelaySevenGainParameter* *mWetGainParameter*panSevenR
                         + delay_sample_RightTwo* mDelayTwoGainParameter* *mWetGainParameter*panTwoR
                         + delay_sample_RightFour* mDelayFourGainParameter* *mWetGainParameter*panFourR
                         + delay_sample_RightSix* mDelaySixGainParameter* *mWetGainParameter*panSixR
                         + delay_sample_RightEight* mDelayEightGainParameter* *mWetGainParameter*panEightR
                         
                         
                         
                         
                         
                         );
        
        
        
        
        // increment the buffer write head
        mCircularBufferWriteHead ++;
        mCircularBufferWriteHeadTwo ++;
        mCircularBufferWriteHeadThree ++;
        mCircularBufferWriteHeadFour ++;
        mCircularBufferWriteHeadFive ++;
        mCircularBufferWriteHeadSix ++;
        mCircularBufferWriteHeadSeven ++;
        mCircularBufferWriteHeadEight ++;
        
        
        // wrap around if needed
        if (mCircularBufferWriteHead == mCircularBufferLength){
            mCircularBufferWriteHead = 0;
        }
        if (mCircularBufferWriteHeadTwo == mCircularBufferLength){
            mCircularBufferWriteHeadTwo = 0;
        }
        if (mCircularBufferWriteHeadThree == mCircularBufferLength){
            mCircularBufferWriteHeadThree = 0;
        }
        if (mCircularBufferWriteHeadFour == mCircularBufferLength){
            mCircularBufferWriteHeadFour = 0;
        }
        if (mCircularBufferWriteHeadFive == mCircularBufferLength){
            mCircularBufferWriteHeadFive = 0;
        }
        
        if (mCircularBufferWriteHeadSix == mCircularBufferLength){
            mCircularBufferWriteHeadSix = 0;
        }
        
        if (mCircularBufferWriteHeadSeven == mCircularBufferLength){
            mCircularBufferWriteHeadSeven = 0;
        }
        if (mCircularBufferWriteHeadEight == mCircularBufferLength){
            mCircularBufferWriteHeadEight = 0;
        }
        
        
        
        //end main processing loop
    }
    
    
    
    
    // end process block
}

//==============================================================================
bool WayloChorusEngine::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WayloChorusEngine::createEditor()
{
    return nullptr; // WayloPiano owns the only editor; this engine is a headless internal effect
}

//==============================================================================
void WayloChorusEngine::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement ("waylodelayUD"));
    xml->setAttribute("drygain", *mDryGainParameter);
    xml->setAttribute("delayonetime", mDelayOneTimeParameter);
    xml->setAttribute("delayonegain", mDelayOneGainParameter);
    xml->setAttribute("delayonedepth", mDelayOneModDepthParameter);
    xml->setAttribute("delayonerate", mDelayOneModRateParameter);
    xml->setAttribute("delayonefeedback", mDelayOneFeedbackParameter);
    xml->setAttribute("delaytwotime", mDelayTwoTimeParameter);
    xml->setAttribute("delaytwogain", mDelayTwoGainParameter);
    xml->setAttribute("delaytwodepth", mDelayTwoModDepthParameter);
    xml->setAttribute("delaytworate", mDelayTwoModRateParameter);
    xml->setAttribute("delaytwofeedback", mDelayTwoFeedbackParameter);
    xml->setAttribute("delaythreetime", mDelayThreeTimeParameter);
    xml->setAttribute("delaythreegain", mDelayThreeGainParameter);
    xml->setAttribute("delaythreedepth", mDelayThreeModDepthParameter);
    xml->setAttribute("delaythreerate", mDelayThreeModRateParameter);
    xml->setAttribute("delaythreefeedback", mDelayThreeFeedbackParameter);
    xml->setAttribute("delayfourtime", mDelayFourTimeParameter);
    xml->setAttribute("delayfourgain", mDelayFourGainParameter);
    xml->setAttribute("delayfourdepth", mDelayFourModDepthParameter);
    xml->setAttribute("delayfourrate", mDelayFourModRateParameter);
    xml->setAttribute("delayfourfeedback", mDelayFourFeedbackParameter);
    xml->setAttribute("delayfivetime", mDelayFiveTimeParameter);
    xml->setAttribute("delayfivegain", mDelayFiveGainParameter);
    xml->setAttribute("delayfivedepth", mDelayFiveModDepthParameter);
    xml->setAttribute("delayfiverate", mDelayFiveModRateParameter);
    xml->setAttribute("delayfivefeedback", mDelayFiveFeedbackParameter);
    xml->setAttribute("delaysixtime", mDelaySixTimeParameter);
    xml->setAttribute("delaysixgain", mDelaySixGainParameter);
    xml->setAttribute("delaysixdepth", mDelaySixModDepthParameter);
    xml->setAttribute("delaysixrate", mDelaySixModRateParameter);
    xml->setAttribute("delaysixfeedback", mDelaySixFeedbackParameter);
    xml->setAttribute("delayseventime", mDelaySevenTimeParameter);
    xml->setAttribute("delaysevengain", mDelaySevenGainParameter);
    xml->setAttribute("delaysevendepth", mDelaySevenModDepthParameter);
    xml->setAttribute("delaysevenrate", mDelaySevenModRateParameter);
    xml->setAttribute("delaysevenfeedback", mDelaySevenFeedbackParameter);
    xml->setAttribute("delayeighttime", mDelayEightTimeParameter);
    xml->setAttribute("delayeightgain", mDelayEightGainParameter);
    xml->setAttribute("delayeightdepth", mDelayEightModDepthParameter);
    xml->setAttribute("delayeightrate", mDelayEightModRateParameter);
    xml->setAttribute("delayeightfeedback", mDelayEightFeedbackParameter);
    xml->setAttribute("wetgain", *mWetGainParameter);
    xml->setAttribute("rate", mRateParameter);
    xml->setAttribute("depth", mDepthParameter);
    
    xml->setAttribute("panOne", mDelayOnePanParameter);
    xml->setAttribute("panTwo", mDelayTwoPanParameter);
    xml->setAttribute("panThree", mDelayThreePanParameter);
    xml->setAttribute("panFour", mDelayFourPanParameter);
    xml->setAttribute("panFive", mDelayFivePanParameter);
    xml->setAttribute("panSix", mDelaySixPanParameter);
    xml->setAttribute("panSeven", mDelaySevenPanParameter);
    xml->setAttribute("panEight", mDelayEightPanParameter);
    
    
    copyXmlToBinary(*xml, destData);
}

void WayloChorusEngine::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName("waylodelayUD"))
    {
        
        
        
        *mDryGainParameter = xml->getDoubleAttribute("drygain");
        //this->setGainSlider(xml->getDoubleAttribute("drygain"));
        
        
        
        mDelayOneTimeParameter = xml->getDoubleAttribute("delayonetime");
        mDelayOneGainParameter = xml->getDoubleAttribute("delayonegain");
        mDelayOneModDepthParameter = xml->getDoubleAttribute("delayonedepth");
        mDelayOneModRateParameter = xml->getDoubleAttribute("delayonerate");
        mDelayOneFeedbackParameter = xml->getDoubleAttribute("delayonefeedback");
        
        mDelayTwoTimeParameter = xml->getDoubleAttribute("delaytwotime");
        mDelayTwoGainParameter = xml->getDoubleAttribute("delaytwogain");
        mDelayTwoModDepthParameter = xml->getDoubleAttribute("delaytwodepth");
        mDelayTwoModRateParameter = xml->getDoubleAttribute("delaytworate");
        mDelayTwoFeedbackParameter = xml->getDoubleAttribute("delayonefeedback");
        
        mDelayThreeTimeParameter = xml->getDoubleAttribute("delaythreetime");
        mDelayThreeGainParameter = xml->getDoubleAttribute("delaythreegain");
        mDelayThreeModDepthParameter = xml->getDoubleAttribute("delaythreedepth");
        mDelayThreeModRateParameter = xml->getDoubleAttribute("delaythreerate");
        mDelayThreeFeedbackParameter = xml->getDoubleAttribute("delaythreefeedback");
        
        mDelayFourTimeParameter = xml->getDoubleAttribute("delayfourtime");
        mDelayFourGainParameter = xml->getDoubleAttribute("delayfourgain");
        mDelayFourModDepthParameter = xml->getDoubleAttribute("delayfourdepth");
        mDelayFourModRateParameter = xml->getDoubleAttribute("delayfourrate");
        mDelayFourFeedbackParameter = xml->getDoubleAttribute("delayfourfeedback");
        
        mDelayFiveTimeParameter = xml->getDoubleAttribute("delayfivetime");
        mDelayFiveGainParameter = xml->getDoubleAttribute("delayfivegain");
        mDelayFiveModDepthParameter = xml->getDoubleAttribute("delayfivedepth");
        mDelayFiveModRateParameter = xml->getDoubleAttribute("delayfiverate");
        mDelayFiveFeedbackParameter = xml->getDoubleAttribute("delayfivefeedback");
        
        mDelaySixTimeParameter = xml->getDoubleAttribute("delaysixtime");
        mDelaySixGainParameter = xml->getDoubleAttribute("delaysixgain");
        mDelaySixModDepthParameter = xml->getDoubleAttribute("delaysixdepth");
        mDelaySixModRateParameter = xml->getDoubleAttribute("delaysixrate");
        mDelaySixFeedbackParameter = xml->getDoubleAttribute("delaysixfeedback");
        
        mDelaySevenTimeParameter = xml->getDoubleAttribute("delayseventime");
        mDelaySevenGainParameter = xml->getDoubleAttribute("delaysevengain");
        mDelaySevenModDepthParameter = xml->getDoubleAttribute("delaysevendepth");
        mDelaySevenModRateParameter = xml->getDoubleAttribute("delaysevenrate");
        mDelaySevenFeedbackParameter = xml->getDoubleAttribute("delaysevenfeedback");
        
        mDelayEightTimeParameter = xml->getDoubleAttribute("delayeighttime");
        mDelayEightGainParameter = xml->getDoubleAttribute("delayeightgain");
        mDelayEightModDepthParameter = xml->getDoubleAttribute("delayeightdepth");
        mDelayEightModRateParameter = xml->getDoubleAttribute("delayeightrate");
        mDelayEightFeedbackParameter = xml->getDoubleAttribute("delayeightfeedback");
        
        *mWetGainParameter = xml->getDoubleAttribute("wetgain");
        mRateParameter = xml->getDoubleAttribute("rate");
        mDepthParameter = xml->getDoubleAttribute("depth");
        mDelayOnePanParameter = xml->getDoubleAttribute("panOne");
        mDelayTwoPanParameter = xml->getDoubleAttribute("panTwo");
        mDelayThreePanParameter = xml->getDoubleAttribute("panThree");
        mDelayFourPanParameter = xml->getDoubleAttribute("panFour");
        mDelayFivePanParameter = xml->getDoubleAttribute("panFive");
        mDelaySixPanParameter = xml->getDoubleAttribute("panSix");
        mDelaySevenPanParameter = xml->getDoubleAttribute("panSeven");
        mDelayEightPanParameter = xml->getDoubleAttribute("panEight");
        
        
    }
}

// Note: no createPluginFilter() here - WayloPiano's own PluginProcessor.cpp
// provides the one and only definition for this whole plugin binary. This
// engine is embedded as a headless internal effect, never its own plugin.
