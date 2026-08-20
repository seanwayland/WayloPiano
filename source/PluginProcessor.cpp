#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
void WayloPianoAudioProcessor::NativeTripleDelay::init (float sampleRate)
{
    left = new daisysp::DelayLine<float, 48000>();
    centre = new daisysp::DelayLine<float, 48000>();
    right = new daisysp::DelayLine<float, 48000>();
    left->Init();
    centre->Init();
    right->Init();
    left->SetDelay (0.300f * sampleRate);
    centre->SetDelay (0.800f * sampleRate);
    right->SetDelay (0.450f * sampleRate);
}

void WayloPianoAudioProcessor::NativeTripleDelay::setAmount (float value01)
{
    value01 = juce::jlimit (0.0f, 1.0f, value01);
    wet = std::pow (value01, 0.3f);
    feedback = 0.75f * std::pow (value01, 2.5f);
}

void WayloPianoAudioProcessor::NativeTripleDelay::process (float inL, float inR, float& outL, float& outR)
{
    const float mono = 0.5f * (inL + inR);

    const float readL = left->Read();
    const float readC = centre->Read();
    const float readR = right->Read();

    left->Write (mono + readL * feedback);
    centre->Write (mono + readC * feedback);
    right->Write (mono + readR * feedback);

    const float wetL = readL * 0.85f + readC * 0.5f + readR * 0.15f;
    const float wetR = readL * 0.15f + readC * 0.5f + readR * 0.85f;

    outL = inL + wetL * wet;
    outR = inR + wetR * wet;
}

//==============================================================================
WayloPianoAudioProcessor::WayloPianoAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    auto patchChoices = juce::StringArray { "FM Layer", "Dual Rhodes", "Dual FM", "Rhodes 1", "Rhodes 2", "Rhodes 3" };
    addParameter (patchParam = new juce::AudioParameterChoice ("patch", "Patch", patchChoices, 0));

    addParameter (outputGainParam = new juce::AudioParameterFloat (
        "outputGain", "Output Gain", juce::NormalisableRange<float> (0.0f, 2.0f), 1.0f));

    addParameter (nativeDelayParam = new juce::AudioParameterFloat (
        "nativeDelay", "Delay", juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));

    addParameter (toneParam = new juce::AudioParameterFloat (
        "tone", "Tone", juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));

    addParameter (tremoloPanParam = new juce::AudioParameterFloat (
        "tremoloPan", "Tremolo Pan", juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));

    addParameter (overdriveParam = new juce::AudioParameterBool ("overdrive", "Overdrive", false));

    addParameter (chorusWetParam = new juce::AudioParameterFloat (
        "chorusWet", "Chorus Wet", juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));

    // WayloDelay's own out-of-the-box defaults.
    addParameter (extraDelayTimeLeftParam = new juce::AudioParameterFloat (
        "extraDelayTimeLeft", "Delay 2 Time L", juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.4f), 300.0f));
    addParameter (extraDelayTimeMidParam = new juce::AudioParameterFloat (
        "extraDelayTimeMid", "Delay 2 Time M", juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.4f), 800.0f));
    addParameter (extraDelayTimeRightParam = new juce::AudioParameterFloat (
        "extraDelayTimeRight", "Delay 2 Time R", juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.4f), 470.0f));
    addParameter (extraDelayFeedbackParam = new juce::AudioParameterFloat (
        "extraDelayFeedback", "Delay 2 Feedback", juce::NormalisableRange<float> (0.0f, 0.95f), 0.3f));
    addParameter (extraDelayWetParam = new juce::AudioParameterFloat (
        "extraDelayWet", "Delay 2 Wet", juce::NormalisableRange<float> (0.0f, 1.0f), 0.35f));
}

WayloPianoAudioProcessor::~WayloPianoAudioProcessor()
{
    delete nativeDelay.left;
    delete nativeDelay.centre;
    delete nativeDelay.right;
}

//==============================================================================
void WayloPianoAudioProcessor::setPatch (Patch newPatch)
{
    const int total = (int) Patch::kNumPatches;
    int p = ((int) newPatch % total + total) % total;
    *patchParam = p;

    if ((Patch) p == Patch::FmLayer)
    {
        piano.LoadProgram (kFmLayerRhodesPreset);
    }
    else if ((Patch) p == Patch::DualRhodes)
    {
        piano.LoadProgram (kDualRhodesLeftPreset);
        piano2.LoadProgram (kDualRhodesRightPreset);
    }
    else if ((Patch) p == Patch::DualFm)
    {
        // No Rhodes engine involved in this patch at all - purely two FM
        // voices, hard-panned, see processBlock().
    }
    else
    {
        piano.LoadProgram (p - (int) Patch::Rhodes1);
    }
}

void WayloPianoAudioProcessor::handleNoteOn (int note, int velocity)
{
    const auto patch = (Patch) patchParam->getIndex();
    if (velocity <= 0)
    {
        handleNoteOff (note);
        return;
    }
    if (patch == Patch::DualFm)
    {
        fmPiano.NoteOn (note, velocity);
        fmPiano2.NoteOn (note, velocity);
        return;
    }
    piano.NoteOn (note, velocity);
    if (patch == Patch::FmLayer)
        fmPiano.NoteOn (note, velocity);
    if (patch == Patch::DualRhodes)
        piano2.NoteOn (note, velocity);
}

void WayloPianoAudioProcessor::handleNoteOff (int note)
{
    const auto patch = (Patch) patchParam->getIndex();
    if (patch == Patch::DualFm)
    {
        fmPiano.NoteOff (note);
        fmPiano2.NoteOff (note);
        return;
    }
    piano.NoteOff (note);
    if (patch == Patch::FmLayer)
        fmPiano.NoteOff (note);
    if (patch == Patch::DualRhodes)
        piano2.NoteOff (note);
}

//==============================================================================
void WayloPianoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    piano.Init ((float) sampleRate);
    piano2.Init ((float) sampleRate);
    fmPiano.Init ((float) sampleRate);
    fmPiano2.Init ((float) sampleRate);
    fmPiano2.SetModIndexScale (kDualFmRightModIndexScale);
    nativeDelay.init ((float) sampleRate);
    setPatch (Patch::FmLayer);

    // These engines are driven directly (never hosted), so nothing else
    // ever calls setRateAndBufferSizeDetails() on them - without it,
    // AudioProcessor::getSampleRate() returns 0 inside their processBlock()
    // (ChorusEngine divides by it every sample), producing NaN throughout.
    chorusEngine.setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
    chorusEngine.prepareToPlay (sampleRate, samplesPerBlock);
    chorusEngine.setDryGain (1.0f); // always full dry - see chorusWetParam's declaration comment
    delayEngine.setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
    delayEngine.prepareToPlay (sampleRate, samplesPerBlock);
    *delayEngine.dryParam = 1.0f; // always full dry - see extraDelayWetParam's declaration comment

    keyboardState.reset();
}

void WayloPianoAudioProcessor::releaseResources()
{
}

bool WayloPianoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void WayloPianoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    // Firmware's Controls(): re-derive every knob-driven parameter once per
    // block (matches the pod's ~1kHz control-rate poll closely enough).
    nativeDelay.setAmount (nativeDelayParam->get());
    fmPiano.SetDecay (toneParam->get());
    fmPiano2.SetDecay (toneParam->get());

    // Dual FM gets a snappier, more percussive attack than FM Layer's FM
    // voice - both fmPiano/fmPiano2 share this since only one of the two
    // patches is ever actually audible at a time.
    const bool isDualFmPatch = (patchParam->getIndex() == (int) Patch::DualFm);
    fmPiano.SetAttackTime (isDualFmPatch ? kDualFmAttackSeconds : kFmLayerAttackSeconds);
    fmPiano2.SetAttackTime (isDualFmPatch ? kDualFmAttackSeconds : kFmLayerAttackSeconds);

    const float hardness = std::pow (modWheel, 0.2f);
    piano.SetParam (MdaEPiano::kHardness, hardness);
    const float hardness2 = std::pow (modWheel, 0.5f);
    piano2.SetParam (MdaEPiano::kHardness, hardness2);

    float muffle = toneParam->get() + modWheel * 0.9f;
    if (muffle > 1.0f)
        muffle = 1.0f;
    piano.SetParam (MdaEPiano::kMuffle, muffle);
    piano2.SetParam (MdaEPiano::kMuffle, muffle);

    piano.SetParam (MdaEPiano::kTreble, modWheel);
    piano2.SetParam (MdaEPiano::kTreble, modWheel);

    fmPiano.SetBrightness (modWheel * modWheel);
    fmPiano2.SetBrightness (modWheel * modWheel);

    piano.SetParam (MdaEPiano::kOverdrive, overdriveParam->get() ? 0.5f : 0.0f);

    // Must run after every SetParam() call above (each one internally calls
    // Recalculate(), which would otherwise clobber this).
    piano.SetPanTremolo (tremoloPanParam->get());
    piano2.SetPanTremolo (tremoloPanParam->get());

    auto* outL = buffer.getWritePointer (0);
    auto* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : outL;

    const auto patch = (Patch) patchParam->getIndex();
    const bool fmLayerActive = (patch == Patch::FmLayer);
    const bool dualRhodesActive = (patch == Patch::DualRhodes);
    const bool dualFmActive = (patch == Patch::DualFm);

    int samplePos = 0;
    for (const auto metadata : midiMessages)
    {
        const int eventSample = metadata.samplePosition;
        for (; samplePos < eventSample && samplePos < buffer.getNumSamples(); ++samplePos)
        {
            float l, r;
            piano.Process (l, r);

            if (fmLayerActive)
            {
                const float fm = fmPiano.Process();
                l = l * kRhodesPanL + fm * kFmPanL;
                r = r * kRhodesPanR + fm * kFmPanR;
            }
            else if (dualRhodesActive)
            {
                float l2, r2;
                piano2.Process (l2, r2);
                const float mono1 = 0.5f * (l + r);
                const float mono2 = 0.5f * (l2 + r2);
                l = mono1 * kDual1PanL + mono2 * kDual2PanL;
                r = mono1 * kDual1PanR + mono2 * kDual2PanR;
            }
            else if (dualFmActive)
            {
                l = fmPiano.Process();
                r = fmPiano2.Process();
            }

            float wetL, wetR;
            nativeDelay.process (l, r, wetL, wetR);

            outL[samplePos] = wetL * kMasterVolume;
            outR[samplePos] = wetR * kMasterVolume;
        }

        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
            handleNoteOn (msg.getNoteNumber(), (int) (msg.getFloatVelocity() * 127.0f));
        else if (msg.isNoteOff())
            handleNoteOff (msg.getNoteNumber());
        else if (msg.isControllerOfType (1)) // CC1: mod wheel
        {
            modWheel = (float) msg.getControllerValue() / 127.0f;
        }
        else if (msg.isControllerOfType (64)) // sustain pedal
        {
            const bool on = msg.getControllerValue() >= 64;
            piano.SetSustain (on);
            piano2.SetSustain (on);
        }
        else if (msg.isPitchWheel())
        {
            const float norm = ((float) msg.getPitchWheelValue() - 8192.0f) / 8192.0f;
            const float semitones = norm * kPitchBendRangeSemitones;
            piano.SetPitchBend (semitones);
            piano2.SetPitchBend (semitones);
            fmPiano.SetPitchBend (semitones);
            fmPiano2.SetPitchBend (semitones);
        }
    }

    for (; samplePos < buffer.getNumSamples(); ++samplePos)
    {
        float l, r;
        piano.Process (l, r);

        if (fmLayerActive)
        {
            const float fm = fmPiano.Process();
            l = l * kRhodesPanL + fm * kFmPanL;
            r = r * kRhodesPanR + fm * kFmPanR;
        }
        else if (dualRhodesActive)
        {
            float l2, r2;
            piano2.Process (l2, r2);
            const float mono1 = 0.5f * (l + r);
            const float mono2 = 0.5f * (l2 + r2);
            l = mono1 * kDual1PanL + mono2 * kDual2PanL;
            r = mono1 * kDual1PanR + mono2 * kDual2PanR;
        }
        else if (dualFmActive)
        {
            l = fmPiano.Process();
            r = fmPiano2.Process();
        }

        float wetL, wetR;
        nativeDelay.process (l, r, wetL, wetR);

        outL[samplePos] = wetL * kMasterVolume;
        outR[samplePos] = wetR * kMasterVolume;
    }

    // Extra effects chain: WayloUD-Chorus, then WayloDelay, both reused
    // verbatim as headless internal processors - drive their own public
    // parameters directly from our knobs, then run their processBlock().
    chorusEngine.setWetGain (chorusWetParam->get());
    juce::MidiBuffer emptyMidi;
    chorusEngine.processBlock (buffer, emptyMidi);

    *delayEngine.delayTimeLeftParam = extraDelayTimeLeftParam->get();
    *delayEngine.delayTimeMidParam = extraDelayTimeMidParam->get();
    *delayEngine.delayTimeRightParam = extraDelayTimeRightParam->get();
    *delayEngine.feedbackParam = extraDelayFeedbackParam->get();
    *delayEngine.wetParam = extraDelayWetParam->get();
    emptyMidi.clear();
    delayEngine.processBlock (buffer, emptyMidi);

    const float outputGain = outputGainParam->get();
    buffer.applyGain (outputGain);

    // Safety limiter: up to 32 stacked mda ePiano voices (no per-voice gain
    // compensation, matching the firmware) feeding two effects that each
    // add wet on top of undiminished dry (Chorus, Delay2 - matching their
    // own source plugins' mixing) can genuinely sum well past 0dBFS with
    // dense/sustained playing. A soft tanh ceiling keeps that from ever
    // becoming raw, uncontrolled digital overflow.
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
            data[n] = std::tanh (data[n]);
    }
}

//==============================================================================
juce::AudioProcessorEditor* WayloPianoAudioProcessor::createEditor()
{
    return new WayloPianoAudioProcessorEditor (*this);
}

bool WayloPianoAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String WayloPianoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WayloPianoAudioProcessor::acceptsMidi() const { return true; }
bool WayloPianoAudioProcessor::producesMidi() const { return false; }
bool WayloPianoAudioProcessor::isMidiEffect() const { return false; }
double WayloPianoAudioProcessor::getTailLengthSeconds() const { return 2.0; }

int WayloPianoAudioProcessor::getNumPrograms() { return 1; }
int WayloPianoAudioProcessor::getCurrentProgram() { return 0; }
void WayloPianoAudioProcessor::setCurrentProgram (int) {}
const juce::String WayloPianoAudioProcessor::getProgramName (int) { return {}; }
void WayloPianoAudioProcessor::changeProgramName (int, const juce::String&) {}

void WayloPianoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml ("WayloPianoState");
    xml.setAttribute ("patch", patchParam->getIndex());
    xml.setAttribute ("outputGain", (double) outputGainParam->get());
    xml.setAttribute ("nativeDelay", (double) nativeDelayParam->get());
    xml.setAttribute ("tone", (double) toneParam->get());
    xml.setAttribute ("tremoloPan", (double) tremoloPanParam->get());
    xml.setAttribute ("overdrive", overdriveParam->get());
    xml.setAttribute ("chorusWet", (double) chorusWetParam->get());
    xml.setAttribute ("extraDelayTimeLeft", (double) extraDelayTimeLeftParam->get());
    xml.setAttribute ("extraDelayTimeMid", (double) extraDelayTimeMidParam->get());
    xml.setAttribute ("extraDelayTimeRight", (double) extraDelayTimeRightParam->get());
    xml.setAttribute ("extraDelayFeedback", (double) extraDelayFeedbackParam->get());
    xml.setAttribute ("extraDelayWet", (double) extraDelayWetParam->get());
    copyXmlToBinary (xml, destData);
}

void WayloPianoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml == nullptr || ! xml->hasTagName ("WayloPianoState"))
        return;

    setPatch ((Patch) xml->getIntAttribute ("patch", 0));
    *outputGainParam = (float) xml->getDoubleAttribute ("outputGain", outputGainParam->get());
    *nativeDelayParam = (float) xml->getDoubleAttribute ("nativeDelay", nativeDelayParam->get());
    *toneParam = (float) xml->getDoubleAttribute ("tone", toneParam->get());
    *tremoloPanParam = (float) xml->getDoubleAttribute ("tremoloPan", tremoloPanParam->get());
    *overdriveParam = xml->getBoolAttribute ("overdrive", overdriveParam->get());
    *chorusWetParam = (float) xml->getDoubleAttribute ("chorusWet", chorusWetParam->get());
    *extraDelayTimeLeftParam = (float) xml->getDoubleAttribute ("extraDelayTimeLeft", extraDelayTimeLeftParam->get());
    *extraDelayTimeMidParam = (float) xml->getDoubleAttribute ("extraDelayTimeMid", extraDelayTimeMidParam->get());
    *extraDelayTimeRightParam = (float) xml->getDoubleAttribute ("extraDelayTimeRight", extraDelayTimeRightParam->get());
    *extraDelayFeedbackParam = (float) xml->getDoubleAttribute ("extraDelayFeedback", extraDelayFeedbackParam->get());
    *extraDelayWetParam = (float) xml->getDoubleAttribute ("extraDelayWet", extraDelayWetParam->get());
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WayloPianoAudioProcessor();
}
