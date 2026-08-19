#include "DelayEngine.h"

//==============================================================================
WayloDelayEngine::WayloDelayEngine()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                        // Always a stereo-in/stereo-out effect regardless of
                        // the outer plugin's JucePlugin_IsSynth/IsMidiEffect
                        // (this is embedded as a headless internal effect in
                        // WayloPiano, whose own plugin type those macros
                        // actually describe) - so this can't be macro-gated.
                         .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                         )
#endif
{
    addParameter (delayTimeLeftParam = new juce::AudioParameterFloat (
        "delayTimeLeft", "Delay Time Left",
        juce::NormalisableRange<float> (minDelayMs, maxDelayMs, 0.1f, 0.4f), 300.0f));

    addParameter (delayTimeMidParam = new juce::AudioParameterFloat (
        "delayTimeMid", "Delay Time Mid",
        juce::NormalisableRange<float> (minDelayMs, maxDelayMs, 0.1f, 0.4f), 800.0f));

    addParameter (delayTimeRightParam = new juce::AudioParameterFloat (
        "delayTimeRight", "Delay Time Right",
        juce::NormalisableRange<float> (minDelayMs, maxDelayMs, 0.1f, 0.4f), 470.0f));

    addParameter (feedbackParam = new juce::AudioParameterFloat (
        "feedback", "Feedback",
        juce::NormalisableRange<float> (0.0f, 0.95f), 0.3f));

    addParameter (wetParam = new juce::AudioParameterFloat (
        "wet", "Wet",
        juce::NormalisableRange<float> (0.0f, 1.0f), 0.35f));

    addParameter (dryParam = new juce::AudioParameterFloat (
        "dry", "Dry",
        juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
}

WayloDelayEngine::~WayloDelayEngine()
{
}

//==============================================================================
const juce::String WayloDelayEngine::getName() const
{
    return JucePlugin_Name;
}

bool WayloDelayEngine::acceptsMidi() const
{
    return false;
}

bool WayloDelayEngine::producesMidi() const
{
    return false;
}

bool WayloDelayEngine::isMidiEffect() const
{
    return false;
}

double WayloDelayEngine::getTailLengthSeconds() const
{
    return static_cast<double> (maxDelayMs) / 1000.0;
}

int WayloDelayEngine::getNumPrograms()
{
    return 1;
}

int WayloDelayEngine::getCurrentProgram()
{
    return 0;
}

void WayloDelayEngine::setCurrentProgram (int)
{
}

const juce::String WayloDelayEngine::getProgramName (int)
{
    return {};
}

void WayloDelayEngine::changeProgramName (int, const juce::String&)
{
}

//==============================================================================
void WayloDelayEngine::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    const int maxDelaySamples = static_cast<int> (std::ceil (sampleRate * maxDelayMs / 1000.0)) + 4;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 1;

    delayLineLeft.setMaximumDelayInSamples (maxDelaySamples);
    delayLineMid.setMaximumDelayInSamples (maxDelaySamples);
    delayLineRight.setMaximumDelayInSamples (maxDelaySamples);

    delayLineLeft.prepare (spec);
    delayLineMid.prepare (spec);
    delayLineRight.prepare (spec);

    delayLineLeft.reset();
    delayLineMid.reset();
    delayLineRight.reset();

    feedbackStateLeft = feedbackStateMid = feedbackStateRight = 0.0f;
}

void WayloDelayEngine::releaseResources()
{
}

bool WayloDelayEngine::isBusesLayoutSupported (const BusesLayout& layouts) const
{
   #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
   #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
   #endif
}

void WayloDelayEngine::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int numSamples = buffer.getNumSamples();
    const bool isStereo = buffer.getNumChannels() >= 2;

    // Keep an untouched copy of the input for the dry signal and for the
    // mono sum fed into each delay tap, since we overwrite the buffer below.
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf (buffer);

    const float feedback = feedbackParam->get();
    const float wet = wetParam->get();
    const float dry = dryParam->get();

    delayLineLeft.setDelay (msToSamples (delayTimeLeftParam->get()));
    delayLineMid.setDelay (msToSamples (delayTimeMidParam->get()));
    delayLineRight.setDelay (msToSamples (delayTimeRightParam->get()));

    auto* outL = buffer.getWritePointer (0);
    auto* outR = isStereo ? buffer.getWritePointer (1) : outL;

    constexpr float centreTapGain = 0.7071f; // -3dB, so a hard-centre tap doesn't dominate the mix

    for (int n = 0; n < numSamples; ++n)
    {
        const float dryL = dryBuffer.getSample (0, n);
        const float dryR = isStereo ? dryBuffer.getSample (1, n) : dryL;
        const float inputMono = 0.5f * (dryL + dryR);

        const float leftTapIn = inputMono + feedbackStateLeft * feedback;
        delayLineLeft.pushSample (0, leftTapIn);
        const float leftTapOut = delayLineLeft.popSample (0);
        feedbackStateLeft = leftTapOut;

        const float midTapIn = inputMono + feedbackStateMid * feedback;
        delayLineMid.pushSample (0, midTapIn);
        const float midTapOut = delayLineMid.popSample (0);
        feedbackStateMid = midTapOut;

        const float rightTapIn = inputMono + feedbackStateRight * feedback;
        delayLineRight.pushSample (0, rightTapIn);
        const float rightTapOut = delayLineRight.popSample (0);
        feedbackStateRight = rightTapOut;

        const float wetL = leftTapOut + midTapOut * centreTapGain;
        const float wetR = rightTapOut + midTapOut * centreTapGain;

        outL[n] = dryL * dry + wetL * wet;
        outR[n] = dryR * dry + wetR * wet;
    }
}

//==============================================================================
bool WayloDelayEngine::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* WayloDelayEngine::createEditor()
{
    return nullptr; // WayloPiano owns the only editor; this engine is a headless internal effect
}

//==============================================================================
void WayloDelayEngine::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml ("WayloDelayState");
    xml.setAttribute ("delayTimeLeft", (double) delayTimeLeftParam->get());
    xml.setAttribute ("delayTimeMid", (double) delayTimeMidParam->get());
    xml.setAttribute ("delayTimeRight", (double) delayTimeRightParam->get());
    xml.setAttribute ("feedback", (double) feedbackParam->get());
    xml.setAttribute ("wet", (double) wetParam->get());
    xml.setAttribute ("dry", (double) dryParam->get());
    copyXmlToBinary (xml, destData);
}

void WayloDelayEngine::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml.get() != nullptr && xml->hasTagName ("WayloDelayState"))
    {
        *delayTimeLeftParam = (float) xml->getDoubleAttribute ("delayTimeLeft", delayTimeLeftParam->get());
        *delayTimeMidParam = (float) xml->getDoubleAttribute ("delayTimeMid", delayTimeMidParam->get());
        *delayTimeRightParam = (float) xml->getDoubleAttribute ("delayTimeRight", delayTimeRightParam->get());
        *feedbackParam = (float) xml->getDoubleAttribute ("feedback", feedbackParam->get());
        *wetParam = (float) xml->getDoubleAttribute ("wet", wetParam->get());
        *dryParam = (float) xml->getDoubleAttribute ("dry", dryParam->get());
    }
}

// Note: no createPluginFilter() here - WayloPiano's own PluginProcessor.cpp
// provides the one and only definition for this whole plugin binary. This
// engine is embedded as a headless internal effect, never its own plugin.
