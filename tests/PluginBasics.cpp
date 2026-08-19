#include "helpers/test_helpers.h"
#include <PluginProcessor.h>
#include <memory>
#include <cmath>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

TEST_CASE ("one is equal to one", "[dummy]")
{
    REQUIRE (1 == 1);
}

TEST_CASE ("Plugin instance", "[instance]")
{
    auto testPlugin = std::make_unique<WayloPianoAudioProcessor>();

    SECTION ("name")
    {
        CHECK_THAT (testPlugin->getName().toStdString(),
            Catch::Matchers::Equals ("WayloPiano"));
    }
}

static bool renderIsFiniteAndAudible (WayloPianoAudioProcessor& plugin, int note)
{
    juce::AudioBuffer<float> buffer (2, 512);
    juce::MidiBuffer midi;
    midi.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) 100), 0);
    plugin.processBlock (buffer, midi);
    midi.clear();

    bool audible = false;
    for (int block = 0; block < 10; ++block)
    {
        plugin.processBlock (buffer, midi);
        auto* data = buffer.getReadPointer (0);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            if (! std::isfinite (data[i]))
                return false;
            if (std::abs (data[i]) > 0.001f)
                audible = true;
        }
    }
    return audible;
}

TEST_CASE ("mda ePiano (FM Layer patch) renders audible, finite audio on note-on", "[dsp]")
{
    auto testPlugin = std::make_unique<WayloPianoAudioProcessor>();
    testPlugin->prepareToPlay (48000.0, 512);
    CHECK (renderIsFiniteAndAudible (*testPlugin, 60));
}

TEST_CASE ("Every patch renders audible, finite audio on note-on", "[dsp]")
{
    auto testPlugin = std::make_unique<WayloPianoAudioProcessor>();
    testPlugin->prepareToPlay (48000.0, 512);

    for (int p = 0; p < (int) WayloPianoAudioProcessor::Patch::kNumPatches; ++p)
    {
        *testPlugin->patchParam = p;
        CHECK (renderIsFiniteAndAudible (*testPlugin, 60));
    }
}

TEST_CASE ("CC1 (mod wheel) audibly brightens the tone", "[dsp]")
{
    // Two fresh instances compared at the same block index, avoiding an
    // envelope-decay confound from comparing across time on one instance.
    auto renderHfEnergyAtBlock10 = [] (int modWheelValue) -> double
    {
        auto plugin = std::make_unique<WayloPianoAudioProcessor>();
        plugin->prepareToPlay (48000.0, 512);
        // Default patch is FM Layer, which hard-pans the Rhodes engine
        // (which Treble actually affects) entirely to the right channel and
        // the FM voice (unaffected by Treble) to the left - use a plain
        // Rhodes patch instead so the effect shows up on the channel this
        // test measures. Also isolate from Chorus/Delay2's default wet mix.
        *plugin->patchParam = (int) WayloPianoAudioProcessor::Patch::Rhodes1;
        *plugin->chorusWetParam = 0.0f;
        *plugin->extraDelayWetParam = 0.0f;

        juce::AudioBuffer<float> buffer (2, 512);
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::controllerEvent (1, 1, modWheelValue), 0);
        midi.addEvent (juce::MidiMessage::noteOn (1, 48, (juce::uint8) 100), 0);
        plugin->processBlock (buffer, midi);
        midi.clear();

        for (int i = 0; i < 9; ++i)
            plugin->processBlock (buffer, midi);

        double hf = 0.0;
        auto* data = buffer.getReadPointer (0);
        for (int i = 1; i < buffer.getNumSamples(); ++i)
            hf += std::abs (data[i] - data[i - 1]);
        return hf;
    };

    const double hfNoModWheel = renderHfEnergyAtBlock10 (0);
    const double hfFullModWheel = renderHfEnergyAtBlock10 (127);

    CHECK (hfFullModWheel > hfNoModWheel);
}

TEST_CASE ("Tremolo Pan knob introduces left/right amplitude modulation", "[dsp]")
{
    auto withTremolo = [] (float amount) -> float
    {
        auto plugin = std::make_unique<WayloPianoAudioProcessor>();
        plugin->prepareToPlay (48000.0, 512);
        *plugin->tremoloPanParam = amount;
        // Default patch (FM Layer) hard-pans the Rhodes engine, which
        // tremolo pan actually modulates, entirely to one channel - use a
        // plain Rhodes patch so both channels carry the modulated signal.
        // Also isolate from the extra Chorus/Delay2 effects, which are
        // their own stereo processes and would otherwise swamp the
        // tremolo-specific L/R divergence this test measures.
        *plugin->patchParam = (int) WayloPianoAudioProcessor::Patch::Rhodes1;
        *plugin->chorusWetParam = 0.0f;
        *plugin->extraDelayWetParam = 0.0f;

        juce::AudioBuffer<float> buffer (2, 512);
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);
        plugin->processBlock (buffer, midi);
        midi.clear();

        // Render several blocks and track how much L and R diverge from each
        // other - tremolo pan pushes them in opposite directions, silence or
        // a static pan would not.
        float maxDivergence = 0.0f;
        for (int b = 0; b < 20; ++b)
        {
            plugin->processBlock (buffer, midi);
            auto* l = buffer.getReadPointer (0);
            auto* r = buffer.getReadPointer (1);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                maxDivergence = std::max (maxDivergence, std::abs (l[i] - r[i]));
        }
        return maxDivergence;
    };

    CHECK (withTremolo (1.0f) > withTremolo (0.0f));
}

TEST_CASE ("Extra Chorus and Delay2 effects are in the signal path", "[dsp]")
{
    auto renderTail = [] (float chorusWet, float delayWet) -> float
    {
        auto plugin = std::make_unique<WayloPianoAudioProcessor>();
        plugin->prepareToPlay (48000.0, 512);
        *plugin->chorusWetParam = chorusWet;
        *plugin->extraDelayWetParam = delayWet;
        *plugin->extraDelayFeedbackParam = 0.6f;

        juce::AudioBuffer<float> buffer (2, 512);
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);
        plugin->processBlock (buffer, midi);
        midi.clear();
        // Note off immediately, then render well past the dry voice's tail -
        // any remaining energy this far out has to be delay/chorus feedback.
        juce::MidiBuffer noteOff;
        noteOff.addEvent (juce::MidiMessage::noteOff (1, 60), 0);
        plugin->processBlock (buffer, noteOff);
        noteOff.clear();

        float tailEnergy = 0.0f;
        for (int b = 0; b < 100; ++b)
        {
            plugin->processBlock (buffer, noteOff);
            auto* data = buffer.getReadPointer (0);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                tailEnergy += std::abs (data[i]);
        }
        return tailEnergy;
    };

    const float tailWithEffects = renderTail (0.6f, 0.6f);
    const float tailBypassed = renderTail (0.0f, 0.0f);

    CHECK (tailWithEffects > tailBypassed);
}

TEST_CASE ("Output never exceeds unity, even with dense sustained playing and effects maxed", "[dsp]")
{
    // Up to 32 mda ePiano voices sum with no per-voice gain compensation
    // (matches the firmware), then feed two effects that each add wet on
    // top of undiminished dry - worst case (a full chord held with the
    // sustain pedal down and both extra effects at max) can genuinely push
    // well past 0dBFS without an output safety limiter.
    auto plugin = std::make_unique<WayloPianoAudioProcessor>();
    plugin->prepareToPlay (48000.0, 512);
    *plugin->chorusWetParam = 1.0f;
    *plugin->extraDelayWetParam = 1.0f;
    *plugin->extraDelayFeedbackParam = 0.95f;
    *plugin->nativeDelayParam = 1.0f;

    juce::AudioBuffer<float> buffer (2, 512);
    juce::MidiBuffer midi;
    midi.addEvent (juce::MidiMessage::controllerEvent (1, 64, 127), 0); // sustain pedal down
    for (int note = 36; note <= 96; note += 3) // a wide, dense chord
        midi.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) 127), 0);
    plugin->processBlock (buffer, midi);
    midi.clear();

    float peak = 0.0f;
    for (int b = 0; b < 30; ++b)
    {
        plugin->processBlock (buffer, midi);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getReadPointer (ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                REQUIRE (std::isfinite (data[i]));
                peak = std::max (peak, std::abs (data[i]));
            }
        }
    }

    CHECK (peak <= 1.0f);
}
