#include "ep_dsp.h"

// Factory presets, taken verbatim from mdaEPianoProcessor::programParams,
// except the LFO rate column (index kLfoRate) which is shifted from the
// original 0.650 to 0.568 so tremolo runs at ~60% of the original speed
// (dlfo is exponential in this param, so the shift is additive in log space).
static const float kProgramParams[MdaEPiano::kNumPrograms][MdaEPiano::kNumParams] = {
    {0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.568f, 0.250f, 0.500f, 0.50f, 0.500f, 0.146f, 0.000f},
    {0.500f, 0.500f, 1.000f, 0.800f, 0.500f, 0.568f, 0.250f, 0.500f, 0.50f, 0.500f, 0.146f, 0.500f},
    {0.500f, 0.500f, 0.000f, 0.000f, 0.500f, 0.568f, 0.250f, 0.500f, 0.50f, 0.500f, 0.246f, 0.000f},
    {0.500f, 0.500f, 0.500f, 0.500f, 0.250f, 0.568f, 0.250f, 0.500f, 0.50f, 0.500f, 0.246f, 0.000f},
    {0.500f, 0.500f, 0.500f, 0.500f, 0.750f, 0.568f, 0.250f, 0.500f, 0.50f, 0.500f, 0.246f, 0.000f},
};

void MdaEPiano::Init(float sampleRate)
{
    Fs_  = sampleRate;
    iFs_ = 1.0f / Fs_;

    for(int i = 0; i < kNumParams; i++)
        params_[i] = kProgramParams[0][i];
    tremoloDepthSource_ = params_[kMuffle];

    kgrp_[0].root = 36;
    kgrp_[0].high = 39;
    kgrp_[3].root = 43;
    kgrp_[3].high = 45;
    kgrp_[6].root = 48;
    kgrp_[6].high = 51;
    kgrp_[9].root = 55;
    kgrp_[9].high = 57;
    kgrp_[12].root = 60;
    kgrp_[12].high = 63;
    kgrp_[15].root = 67;
    kgrp_[15].high = 69;
    kgrp_[18].root = 72;
    kgrp_[18].high = 75;
    kgrp_[21].root = 79;
    kgrp_[21].high = 81;
    kgrp_[24].root = 84;
    kgrp_[24].high = 87;
    kgrp_[27].root = 91;
    kgrp_[27].high = 93;
    kgrp_[30].root = 96;
    kgrp_[30].high = 999;

    kgrp_[0].pos  = 0;
    kgrp_[0].end  = 8476;
    kgrp_[0].loop = 4400;
    kgrp_[1].pos  = 8477;
    kgrp_[1].end  = 16248;
    kgrp_[1].loop = 4903;
    kgrp_[2].pos  = 16249;
    kgrp_[2].end  = 34565;
    kgrp_[2].loop = 6398;
    kgrp_[3].pos  = 34566;
    kgrp_[3].end  = 41384;
    kgrp_[3].loop = 3938;
    kgrp_[4].pos  = 41385;
    kgrp_[4].end  = 45760;
    kgrp_[4].loop = 1633;
    kgrp_[5].pos  = 45761;
    kgrp_[5].end  = 65211;
    kgrp_[5].loop = 5245;
    kgrp_[6].pos  = 65212;
    kgrp_[6].end  = 72897;
    kgrp_[6].loop = 2937;
    kgrp_[7].pos  = 72898;
    kgrp_[7].end  = 78626;
    kgrp_[7].loop = 2203;
    kgrp_[8].pos  = 78627;
    kgrp_[8].end  = 100387;
    kgrp_[8].loop = 6368;
    kgrp_[9].pos  = 100388;
    kgrp_[9].end  = 116297;
    kgrp_[9].loop = 10452;
    kgrp_[10].pos  = 116298;
    kgrp_[10].end  = 127661;
    kgrp_[10].loop = 5217;
    kgrp_[11].pos  = 127662;
    kgrp_[11].end  = 144113;
    kgrp_[11].loop = 3099;
    kgrp_[12].pos  = 144114;
    kgrp_[12].end  = 152863;
    kgrp_[12].loop = 4284;
    kgrp_[13].pos  = 152864;
    kgrp_[13].end  = 173107;
    kgrp_[13].loop = 3916;
    kgrp_[14].pos  = 173108;
    kgrp_[14].end  = 192734;
    kgrp_[14].loop = 2937;
    kgrp_[15].pos  = 192735;
    kgrp_[15].end  = 204598;
    kgrp_[15].loop = 4732;
    kgrp_[16].pos  = 204599;
    kgrp_[16].end  = 218995;
    kgrp_[16].loop = 4733;
    kgrp_[17].pos  = 218996;
    kgrp_[17].end  = 233801;
    kgrp_[17].loop = 2285;
    kgrp_[18].pos  = 233802;
    kgrp_[18].end  = 248011;
    kgrp_[18].loop = 4098;
    kgrp_[19].pos  = 248012;
    kgrp_[19].end  = 265287;
    kgrp_[19].loop = 4099;
    kgrp_[20].pos  = 265288;
    kgrp_[20].end  = 282255;
    kgrp_[20].loop = 3609;
    kgrp_[21].pos  = 282256;
    kgrp_[21].end  = 293776;
    kgrp_[21].loop = 2446;
    kgrp_[22].pos  = 293777;
    kgrp_[22].end  = 312566;
    kgrp_[22].loop = 6278;
    kgrp_[23].pos  = 312567;
    kgrp_[23].end  = 330200;
    kgrp_[23].loop = 2283;
    kgrp_[24].pos  = 330201;
    kgrp_[24].end  = 348889;
    kgrp_[24].loop = 2689;
    kgrp_[25].pos  = 348890;
    kgrp_[25].end  = 365675;
    kgrp_[25].loop = 4370;
    kgrp_[26].pos  = 365676;
    kgrp_[26].end  = 383661;
    kgrp_[26].loop = 5225;
    kgrp_[27].pos  = 383662;
    kgrp_[27].end  = 393372;
    kgrp_[27].loop = 2811;
    kgrp_[28].pos  = 383662;
    kgrp_[28].end  = 393372;
    kgrp_[28].loop = 2811; // ghost
    kgrp_[29].pos  = 393373;
    kgrp_[29].end  = 406045;
    kgrp_[29].loop = 4522;
    kgrp_[30].pos  = 406046;
    kgrp_[30].end  = 414486;
    kgrp_[30].loop = 2306;
    kgrp_[31].pos  = 406046;
    kgrp_[31].end  = 414486;
    kgrp_[31].loop = 2306; // ghost
    kgrp_[32].pos  = 414487;
    kgrp_[32].end  = 422408;
    kgrp_[32].loop = 2169;

    // NOTE: the original constructor ran an "extra crossfade looping" pass
    // that mutated the wave data at the loop points. Here that pass has
    // already been baked into epianoData at build time (see tools/gen_wavedata.py)
    // since the table lives in read-only QSPI flash.

    for(int v = 0; v < kNumVoices; v++)
    {
        std::memset(&voice_[v], 0, sizeof(Voice));
        voice_[v].env = 0.0f;
        voice_[v].dec = 0.99f;
    }

    activeVoices_ = 0;
    sustainOn_    = false;
    volume_       = 0.2f;
    muff_         = 160.0f;
    tl_ = tr_ = lfo0_ = dlfo_ = 0.0f;
    lfo1_   = 1.0f;
    modwhl_ = 0.0f;

    Recalculate();
}

void MdaEPiano::LoadProgram(int program)
{
    if(program < 0)
        program = 0;
    if(program >= kNumPrograms)
        program = kNumPrograms - 1;
    for(int i = 0; i < kNumParams; i++)
        params_[i] = kProgramParams[program][i];
    tremoloDepthSource_ = params_[kMuffle];
    Recalculate();
}

void MdaEPiano::SetParam(int index, float value01)
{
    if(index < 0 || index >= kNumParams)
        return;
    if(value01 < 0.0f)
        value01 = 0.0f;
    if(value01 > 1.0f)
        value01 = 1.0f;
    params_[index] = value01;
    Recalculate();
}

void MdaEPiano::SetSustain(bool on)
{
    sustainOn_ = on;
    if(!on)
    {
        // end all sustained (held-by-pedal) notes
        for(int v = 0; v < kNumVoices; v++)
        {
            if(voice_[v].note == 128 /* SUSTAIN sentinel */)
            {
                int note        = -1; // unknown original note, just release by decay
                voice_[v].dec   = (float)std::exp(-iFs_ * std::exp(6.0 + 0.01 * 60.0 - 5.0 * params_[kRelease]));
                voice_[v].note  = note;
            }
        }
    }
}

void MdaEPiano::NoteOn(int note, int velocity)
{
    if(velocity <= 0)
    {
        NoteOff(note);
        return;
    }

    float   l = 99.0f;
    int32_t vl = 0, k, s;

    if(activeVoices_ < poly_)
    {
        vl = activeVoices_;
        activeVoices_++;
    }
    else
    {
        for(int v = 0; v < poly_; v++)
        {
            if(voice_[v].env < l)
            {
                l  = voice_[v].env;
                vl = v;
            }
        }
    }

    k = (note - 60) * (note - 60);
    l = fine_ + random_ * ((float)(k % 13) - 6.5f);

    s = size_;
    if(velocity > 40)
        s += (int32_t)(sizevel_ * (float)(velocity - 40));

    k = 0;
    while(note > (kgrp_[k].high + s))
        k += 3;
    l += (float)(note - kgrp_[k].root);
    l = 32000.0f * iFs_ * (float)std::exp(0.05776226505 * l);
    voice_[vl].delta = (int32_t)(65536.0f * l);
    voice_[vl].frac  = 0;

    if(velocity > 48)
        k++;
    if(velocity > 80)
        k++;
    voice_[vl].pos  = kgrp_[k].pos;
    voice_[vl].end  = kgrp_[k].end - 1;
    voice_[vl].loop = kgrp_[k].loop;

    voice_[vl].env = (3.0f + 2.0f * velsens_) * (float)std::pow(0.0078f * velocity, velsens_);
    if(note > 60)
        voice_[vl].env *= (float)std::exp(0.01f * (float)(60 - note));

    // Slight extra brightness for lower notes (descending the keyboard)
    // and for harder velocity, layered on top of the Muffle-driven cutoff.
    float pitchBrighten = (60.0f - (float)note) * 0.9f;
    l = 50.0f + params_[kMuffle] * params_[kMuffle] * muff_ + muffvel_ * (float)(velocity - 64)
        + pitchBrighten;
    if(l < (55.0f + 0.4f * (float)note))
        l = 55.0f + 0.4f * (float)note;
    if(l > 210.0f)
        l = 210.0f;

    voice_[vl].note = note;
    int panNote = note;
    if(panNote < 12)
        panNote = 12;
    if(panNote > 108)
        panNote = 108;
    l               = volume_;
    voice_[vl].outr = l + l * width_ * (float)(panNote - 60);
    voice_[vl].outl = l + l - voice_[vl].outr;

    int decNote = note;
    if(decNote < 44)
        decNote = 44;
    voice_[vl].dec = (float)std::exp(-iFs_ * std::exp(-1.0 + 0.03 * (double)decNote - 2.0f * params_[kDecay]));
}

void MdaEPiano::NoteOff(int note)
{
    for(int v = 0; v < kNumVoices; v++)
    {
        if(voice_[v].note == note)
        {
            if(!sustainOn_)
            {
                voice_[v].dec = (float)std::exp(-iFs_ * std::exp(6.0 + 0.01 * (double)note - 5.0 * params_[kRelease]));
            }
            else
            {
                voice_[v].note = 128; // SUSTAIN sentinel, held until pedal up
            }
        }
    }
}

void MdaEPiano::Recalculate()
{
    size_ = (int32_t)(12.0f * params_[kHardness] - 6.0f);

    treb_ = 4.0f * params_[kTreble] * params_[kTreble] - 1.0f;
    tfrq_ = (params_[kTreble] > 0.5f) ? 14000.0f : 5000.0f;
    tfrq_ = 1.0f - (float)std::exp(-iFs_ * tfrq_);

    rmod_ = lmod_ = tremoloDepthSource_ + tremoloDepthSource_ - 1.0f;
    if(tremoloDepthSource_ < 0.5f)
        rmod_ = -rmod_;

    dlfo_ = 6.283f * iFs_ * (float)std::exp(6.22f * params_[kLfoRate] - 2.61f);

    velsens_ = 1.0f + params_[kVelSense] + params_[kVelSense];
    if(params_[kVelSense] < 0.25f)
        velsens_ -= 0.75f - 3.0f * params_[kVelSense];

    width_ = 0.03f * params_[kWidth];
    poly_  = 1 + (int32_t)(31.9f * params_[kPoly]);
    if(poly_ > kNumVoices)
        poly_ = kNumVoices;
    fine_      = params_[kTune] - 0.5f;
    random_    = 0.077f * params_[kRandomTune] * params_[kRandomTune];
    overdrive_ = 1.8f * params_[kOverdrive];

    if(modwhl_ > 0.05f)
    {
        rmod_ = lmod_ = modwhl_;
        if(params_[kMuffle] < 0.5f)
            rmod_ = -rmod_;
    }
}
