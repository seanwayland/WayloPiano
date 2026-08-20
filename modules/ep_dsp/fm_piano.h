#pragma once
// Simple 2-operator phase-modulation "FM piano": one sine carrier, phase
// modulated by a second sine at the same frequency, with a small modulation
// index that decays quickly (bright pluck settling into a plainer sine) on
// top of a slower amplitude decay. Deliberately minimal, as a second, distinct
// voice from the sample-based mda ePiano engine.

#include <cmath>

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

class FmPiano
{
  public:
    enum
    {
        kNumVoices = 8,
    };

    void Init(float sampleRate)
    {
        Fs_  = sampleRate;
        iFs_ = 1.0f / Fs_;
        for(int i = 0; i < kNumVoices; i++)
            voice_[i] = Voice{};
        activeVoices_ = 0;
    }

    // 0..1 knobs: brightness scales the modulation index, decay scales the
    // amplitude decay time (both apply to newly-triggered notes).
    void SetBrightness(float value01) { modIndexBase_ = 0.15f + 2.5f * value01; }
    void SetDecay(float value01) { ampDecaySeconds_ = 0.6f + 3.0f * value01; }

    // Modulator:carrier frequency ratio, default 1.0 (unison - matches the
    // original single-voice behaviour exactly). Applied to newly-triggered
    // notes; lets two instances sound like a related but distinct timbre.
    void SetModRatio(float ratio) { modRatio_ = ratio; }

    // semitones in [-N, +N]; applied live to all voices' pitch.
    void SetPitchBend(float semitones)
    {
        pitchBendRatio_ = std::pow(2.0f, semitones / 12.0f);
    }

    void NoteOn(int note, int velocity)
    {
        int vi;
        if(activeVoices_ < kNumVoices)
        {
            vi = activeVoices_++;
        }
        else
        {
            float lowest = 2.0f;
            vi           = 0;
            for(int i = 0; i < kNumVoices; i++)
            {
                if(voice_[i].ampEnv < lowest)
                {
                    lowest = voice_[i].ampEnv;
                    vi     = i;
                }
            }
        }

        float freq = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);

        Voice &v      = voice_[vi];
        v.note        = note;
        v.carrierInc  = 2.0f * (float)M_PI * freq * iFs_;
        v.modInc      = v.carrierInc * modRatio_;
        v.carrierPhase = 0.0f;
        v.modPhase     = 0.0f;
        v.ampTarget    = velocity / 127.0f;
        v.ampEnv       = 0.0f;
        v.attacking    = true;
        v.attackInc    = v.ampTarget / (kAttackSeconds * Fs_);
        v.ampDecay     = std::exp(-iFs_ / ampDecaySeconds_);

        // Slightly more FM modulation for lower notes and harder velocity,
        // layered on top of the brightness-driven base index.
        float pitchFactor    = 1.0f + 0.012f * (69.0f - (float)note);
        float velocityFactor = 1.0f + 0.006f * ((float)velocity - 64.0f);
        float modIndex        = modIndexBase_ * pitchFactor * velocityFactor;
        if(modIndex < 0.05f)
            modIndex = 0.05f;
        if(modIndex > 6.0f)
            modIndex = 6.0f;
        v.modIndex     = modIndex;
        v.modDecay     = std::exp(-iFs_ / (ampDecaySeconds_ * 0.18f));
        v.releasing    = false;
    }

    void NoteOff(int note)
    {
        for(int i = 0; i < activeVoices_; i++)
        {
            if(voice_[i].note == note && !voice_[i].releasing)
            {
                voice_[i].releasing = true;
                voice_[i].ampDecay  = std::exp(-iFs_ / 0.03f); // ~30ms release
            }
        }
    }

    // Sums all active voices to a single mono sample.
    inline float Process()
    {
        float out = 0.0f;
        for(int i = 0; i < activeVoices_; i++)
        {
            Voice &v = voice_[i];
            v.carrierPhase += v.carrierInc * pitchBendRatio_;
            v.modPhase += v.modInc * pitchBendRatio_;
            if(v.carrierPhase > 2.0f * (float)M_PI)
                v.carrierPhase -= 2.0f * (float)M_PI;
            if(v.modPhase > 2.0f * (float)M_PI)
                v.modPhase -= 2.0f * (float)M_PI;

            float modulator = std::sin(v.modPhase);
            float sample    = std::sin(v.carrierPhase + v.modIndex * modulator);

            out += sample * v.ampEnv * 0.8f;

            if(v.attacking)
            {
                v.ampEnv += v.attackInc;
                if(v.ampEnv >= v.ampTarget)
                {
                    v.ampEnv    = v.ampTarget;
                    v.attacking = false;
                }
            }
            else
            {
                v.ampEnv *= v.ampDecay;
            }
            v.modIndex *= v.modDecay;
        }

        for(int i = 0; i < activeVoices_; i++)
        {
            if(!voice_[i].attacking && voice_[i].ampEnv < 0.0005f)
                voice_[i] = voice_[--activeVoices_];
        }

        return out;
    }

  private:
    struct Voice
    {
        int   note          = -1;
        float carrierPhase  = 0.0f;
        float modPhase      = 0.0f;
        float carrierInc    = 0.0f;
        float modInc        = 0.0f;
        float ampEnv        = 0.0f;
        float ampTarget     = 0.0f;
        float attackInc     = 0.0f;
        bool  attacking     = false;
        float ampDecay      = 0.999f;
        float modIndex      = 0.0f;
        float modDecay      = 0.99f;
        bool  releasing     = false;
    };

    static constexpr float kAttackSeconds = 0.008f; // 8ms attack ramp

    float Fs_  = 48000.0f;
    float iFs_ = 1.0f / 48000.0f;

    Voice voice_[kNumVoices];
    int   activeVoices_ = 0;

    float modIndexBase_    = 0.6f;
    float modRatio_        = 1.0f;
    float ampDecaySeconds_ = 2.0f;
    float pitchBendRatio_  = 1.0f;
};
