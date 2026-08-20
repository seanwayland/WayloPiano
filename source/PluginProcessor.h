#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "daisysp.h"
#include "ep_dsp.h"
#include "fm_piano.h"
#include "ChorusEngine.h"
#include "DelayEngine.h"

//==============================================================================
/** WayloPiano: a JUCE port of the electric-piano firmware from the
    Waylinator Daisy Pod hardware (daisy_ep/src/main.cpp) - a sample-based
    mda ePiano (Paul Kellett) engine plus a simple 2-op FM piano voice,
    reusing the exact same DSP (ep_dsp.h/.cpp, fm_piano.h, and the baked
    wavetable in ep_wavedata.cpp) so it should sound identical at the
    default settings.

    Patch select cycles 6 patches: "FM Layer" and "Dual Rhodes" (firmware
    order, hard-panned Rhodes+FM and two distinct Rhodes presets panned
    wide), "Dual FM" (new - two FM piano voices hard-panned left/right,
    same modulator:carrier ratio but a higher modulation-index scale and a
    snappier attack on the right voice, for a related but distinct
    timbre), then 3 of the firmware's 5 mda ePiano factory presets. The mod
    wheel drives the same big timbral sweep the firmware's mod wheel does
    (Hardness/Muffle/Treble on the Rhodes engine, Brightness on the FM
    voice) - no vibrato. A dedicated Tremolo Pan knob drives the firmware's
    pan-tremolo LFO directly (0 = off, turning it up makes it both faster
    and deeper at once).

    On top of the firmware's own built-in triple-tap delay (the "Delay"
    knob, matching hardware exactly), this also chains in the WayloUD-Chorus
    and WayloDelay effects verbatim (reused as headless internal
    AudioProcessor instances, see modules/fx/) as extra, fully knobbed
    effects the hardware never had.
 */
class WayloPianoAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    WayloPianoAudioProcessor();
    ~WayloPianoAudioProcessor() override;

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
    // 0 = FM Layer, 1 = Dual Rhodes (firmware order), 2 = Dual FM (new -
    // not in the firmware), 3..5 = 3 of the firmware's 5 mda ePiano factory
    // presets (Rhodes 4/5 dropped).
    enum class Patch
    {
        FmLayer = 0,
        DualRhodes,
        DualFm,
        Rhodes1,
        Rhodes2,
        Rhodes3,
        kNumPatches
    };
    juce::AudioParameterChoice* patchParam = nullptr;

    juce::AudioParameterFloat* outputGainParam = nullptr;

    // Firmware's Knob 1: wet level + feedback on the built-in triple-tap
    // delay (0 = fully dry). Applied after everything else, same as hardware.
    juce::AudioParameterFloat* nativeDelayParam = nullptr;

    // Firmware's Knob 2: Muffle (tone) base for the Rhodes engine, and FM
    // decay time. Also feeds the mod-wheel-driven brightness sweep below.
    juce::AudioParameterFloat* toneParam = nullptr;

    // Dedicated knob (not tied to Tone, unlike the firmware's Knob 2): pan
    // tremolo rate+depth together, 0 = completely off.
    juce::AudioParameterFloat* tremoloPanParam = nullptr;

    // Firmware's Button 1.
    juce::AudioParameterBool* overdriveParam = nullptr;

    // WayloUD-Chorus, chained in as an extra effect after the piano engine.
    // No Dry control: unlike a guitar-pedal effect blending an external
    // input, this is in series after the piano with nothing else to
    // preserve - dry is always full, so there's only a Wet/amount to set.
    juce::AudioParameterFloat* chorusWetParam = nullptr;

    // WayloDelay (three independent Left/Mid/Right taps), chained in last.
    juce::AudioParameterFloat* extraDelayTimeLeftParam = nullptr;
    juce::AudioParameterFloat* extraDelayTimeMidParam = nullptr;
    juce::AudioParameterFloat* extraDelayTimeRightParam = nullptr;
    juce::AudioParameterFloat* extraDelayFeedbackParam = nullptr;
    juce::AudioParameterFloat* extraDelayWetParam = nullptr;

    juce::MidiKeyboardState keyboardState;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WayloPianoAudioProcessor)

    static constexpr float kPitchBendRangeSemitones = 7.0f; // fixed, matches firmware

    // Hard pan, matches firmware exactly.
    static constexpr float kRhodesPanL = 0.0f, kRhodesPanR = 1.0f;
    static constexpr float kFmPanL = 1.0f, kFmPanR = 0.0f;
    static constexpr float kDual1PanL = 1.0f, kDual1PanR = 0.0f;
    static constexpr float kDual2PanL = 0.0f, kDual2PanR = 1.0f;
    static constexpr int kFmLayerRhodesPreset = 2;
    // Two distinct mda ePiano presets, neither using overdrive (standard
    // vs. soft/mellow with no treble), hard-panned left/right - no detune,
    // no overdrive, just two genuinely different Rhodes timbres.
    static constexpr int kDualRhodesLeftPreset = 0;
    static constexpr int kDualRhodesRightPreset = 2;
    static constexpr float kMasterVolume = 0.35f; // firmware's fixed internal level; outputGainParam multiplies on top

    // Dual FM patch: same modulator:carrier ratio on both voices (no
    // beating risk at all, since fundamental and overtone spacing are
    // identical) - the right voice just runs with more modulation index,
    // giving it a brighter, more complex-but-related version of the same
    // FM piano character as the left voice.
    static constexpr float kDualFmRightModIndexScale = 1.6f;

    // Dual FM's attack is snappier/more percussive than FM Layer's FM
    // voice, which keeps the original 8ms default.
    static constexpr float kFmLayerAttackSeconds = 0.008f;
    static constexpr float kDualFmAttackSeconds = 0.002f;

    MdaEPiano piano, piano2;
    FmPiano fmPiano, fmPiano2;
    float modWheel = 0.0f; // 0..1, CC1

    void setPatch (Patch newPatch);
    void handleNoteOn (int note, int velocity);
    void handleNoteOff (int note);

    // Ported from delay_fx.h's TripleDelay, minus the SDRAM/hardware
    // placement bits (plain DaisySP DelayLine members here instead).
    struct NativeTripleDelay
    {
        void init (float sampleRate);
        void setAmount (float value01);
        void process (float inL, float inR, float& outL, float& outR);

        daisysp::DelayLine<float, 48000>* left = nullptr;
        daisysp::DelayLine<float, 48000>* centre = nullptr;
        daisysp::DelayLine<float, 48000>* right = nullptr;
        float wet = 0.0f;
        float feedback = 0.0f;
    };
    NativeTripleDelay nativeDelay;

    // Headless internal effects, reused verbatim from their own repos - see
    // modules/fx/. Never given an editor or registered with any host; this
    // processor owns them and drives their two-or-so public parameters
    // directly from its own knobs each block.
    WayloChorusEngine chorusEngine;
    WayloDelayEngine delayEngine;

    double currentSampleRate = 44100.0;
};
