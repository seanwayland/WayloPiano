#pragma once
// Standalone port of mda ePiano (Paul Kellett, mda-vst3 / mdaEPianoProcessor.cpp)
// to run as a bare DSP engine on Electrosmith Daisy hardware.
// Original: Copyright (c) 2008 Paul Kellett, MIT-style license (see mda-vst3 project).
//
// The core per-voice synthesis algorithm (sample playback + one-pole muffle
// filter + envelope + treble shelf + tremolo/autopan LFO + soft overdrive)
// is unchanged from the original. Only the VST3 event-scheduling shell was
// removed in favor of direct NoteOn/NoteOff calls suitable for a MIDI callback.

#include <cstdint>
#include <cstring>
#include <cmath>

extern const short epianoData[];
extern const int   EPIANO_WAVE_LEN;

class MdaEPiano
{
  public:
    enum
    {
        kNumVoices   = 32,
        kNumParams   = 12,
        kNumPrograms = 5,
    };

    // Parameter indices (0..1 normalized, matches original mda ePiano layout)
    enum
    {
        kDecay = 0,     // envelope decay
        kRelease,       // note-off release
        kHardness,      // "Hardness" -> keygroup brightness shift (tine hardness)
        kTreble,        // treble boost amount
        kMuffle,        // lowpass muffle / tremolo depth
        kLfoRate,       // tremolo/autopan LFO rate
        kVelSense,      // velocity sensitivity
        kWidth,         // stereo width
        kPoly,          // polyphony (voice count)
        kTune,          // fine tune
        kRandomTune,    // random detune
        kOverdrive,     // soft overdrive amount
    };

    void Init(float sampleRate);
    void SetParam(int index, float value01);
    float GetParam(int index) const { return params_[index]; }
    void LoadProgram(int program);

    void NoteOn(int note, int velocity);
    void NoteOff(int note);
    void SetSustain(bool on);

    // Dedicated pan-tremolo control, independent of Muffle/Hardness/presets:
    // 0 = completely off, 1 = fastest + deepest. Bypasses Recalculate(), so
    // call this AFTER any SetParam() calls in the same control block (those
    // internally call Recalculate() and would otherwise overwrite this).
    void SetPanTremolo(float amount01)
    {
        if(amount01 < 0.0f)
            amount01 = 0.0f;
        if(amount01 > 1.0f)
            amount01 = 1.0f;
        // Depth ramps up much faster than rate (curved, front-loaded) so it's
        // clearly audible early in the knob's travel, while rate still
        // climbs to something fast by the top of the range.
        float depth = (float)std::pow(amount01, 0.3f);
        lmod_       = depth;
        rmod_       = -depth;
        float rateParam = 0.4f + 0.2f * amount01;
        dlfo_           = 6.283f * iFs_ * (float)std::exp(6.22f * rateParam - 2.61f);
    }

    // semitones in [-N, +N]; applied live to all voices' playback rate.
    void SetPitchBend(float semitones)
    {
        pitchBendRatio_ = std::pow(2.0f, semitones / 12.0f);
    }

    inline void Process(float &outL, float &outR)
    {
        float l = 0.0f, r = 0.0f;
        Voice *v = voice_;
        for(int i = 0; i < activeVoices_; i++)
        {
            v->frac += (int32_t)((float)v->delta * pitchBendRatio_);
            v->pos += v->frac >> 16;
            v->frac &= 0xFFFF;
            if(v->pos > v->end)
                v->pos -= v->loop;

            int32_t raw = epianoData[v->pos]
                          + ((v->frac * (epianoData[v->pos + 1] - epianoData[v->pos])) >> 16);
            float x = v->env * (float)raw / 32768.0f;

            v->env = v->env * v->dec;

            if(x > 0.0f)
            {
                x -= overdrive_ * x * x;
                if(x < -v->env)
                    x = -v->env;
            }

            l += v->outl * x;
            r += v->outr * x;
            v++;
        }

        tl_ += tfrq_ * (l - tl_);
        tr_ += tfrq_ * (r - tr_);
        r += treb_ * (r - tr_);
        l += treb_ * (l - tl_);

        lfo0_ += dlfo_ * lfo1_;
        lfo1_ -= dlfo_ * lfo0_;
        l += l * lmod_ * lfo1_;
        r += r * rmod_ * lfo1_;

        if(std::fabs(tl_) < 1.0e-10f)
            tl_ = 0.0f;
        if(std::fabs(tr_) < 1.0e-10f)
            tr_ = 0.0f;

        outL = l;
        outR = r;

        // reap silent voices (swap-with-last)
        for(int i = 0; i < activeVoices_; i++)
        {
            if(voice_[i].env < kSilence)
                voice_[i] = voice_[--activeVoices_];
        }
    }

  private:
    struct Voice
    {
        int32_t delta, frac, pos, end, loop;
        float   env, dec;
        float   outl, outr;
        int32_t note;
    };
    struct KeyGroup
    {
        int32_t root, high, pos, end, loop;
    };

    static constexpr float kSilence = 0.0001f;

    void Recalculate();

    float Fs_ = 48000.0f, iFs_ = 1.0f / 48000.0f;

    KeyGroup kgrp_[34] = {};
    Voice    voice_[kNumVoices] = {};
    int      activeVoices_ = 0;
    int      poly_ = kNumVoices;
    bool     sustainOn_ = false;

    float params_[kNumParams];
    // Tremolo/autopan depth is derived from the Muffle param in the original
    // algorithm, but callers may drive Muffle live (e.g. a mod wheel) for
    // brightness without wanting vibrato depth to move too. This snapshot
    // is only updated by LoadProgram(), so tremolo depth stays fixed to the
    // preset regardless of live Muffle changes.
    float tremoloDepthSource_ = 0.5f;

    float width_ = 0.0f;
    int32_t size_ = 0;
    float lfo0_ = 0.0f, lfo1_ = 1.0f, dlfo_ = 0.0f, lmod_ = 0.0f, rmod_ = 0.0f;
    float treb_ = 0.0f, tfrq_ = 0.0f, tl_ = 0.0f, tr_ = 0.0f;
    float tune_ = 0.0f, fine_ = 0.0f, random_ = 0.0f, overdrive_ = 0.0f;
    float muff_ = 160.0f, muffvel_ = 0.55f, sizevel_ = 0.05f, velsens_ = 1.0f;
    float volume_ = 0.2f, modwhl_ = 0.0f;
    float pitchBendRatio_ = 1.0f;
};
