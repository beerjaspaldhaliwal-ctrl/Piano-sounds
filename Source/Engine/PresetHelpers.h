// Keyverse - helpers used by the factory preset definitions.
#pragma once

#include "Effects.h"
#include "Presets.h"

namespace kv::pb
{
struct Lib
{
    std::vector<Preset> v;
    void add (const char* cat, const char* sub, const char* name, const char* tags, const SoundData& d)
    {
        v.push_back ({ name, cat, sub, tags, d });
    }
};

// Category names (browser order).
inline const char* const kAcoustic = "Acoustic Pianos";
inline const char* const kElectric = "Electric Pianos";
inline const char* const kSynthKeys = "Synth Keys";
inline const char* const kOrgans = "Organs";
inline const char* const kClavs = "Clavs & Harpsichords";
inline const char* const kMallets = "Mallets & Bells";
inline const char* const kGuitars = "Guitars";
inline const char* const kBasses = "Basses";
inline const char* const kPads = "Synth Pads";
inline const char* const kLeads = "Synth Leads";
inline const char* const kSynthBass = "Synth Bass";
inline const char* const kStrBrass = "Strings & Brass";
inline const char* const kPlucks = "Plucks & Arps";
inline const char* const kWorld = "World";
inline const char* const kLayers = "Layers & Splits";
inline const char* const kFx = "FX & Atmospheres";

inline SoundData make (Engine e)
{
    SoundData s;
    s.numLayers = 1;
    s.layer[0].common.engine = (float) (int) e;
    s.gfx.revMix = 0.0f;
    return s;
}

enum class Space { Dry, Studio, Room, Chamber, Hall, Plate, Cathedral, Ambient, Stage, Spring };

inline void space (SoundData& s, Space sp, float mix)
{
    auto& g = s.gfx;
    g.revMix = mix;
    switch (sp)
    {
        case Space::Dry:       g.revMix = 0.0f; break;
        case Space::Studio:    g.revSize = 0.22f; g.revDecay = 0.6f;  g.revDamp = 0.45f; g.revPre = 4;  g.revLowCut = 150; g.revMod = 0.2f; break;
        case Space::Room:      g.revSize = 0.35f; g.revDecay = 0.9f;  g.revDamp = 0.5f;  g.revPre = 8;  g.revLowCut = 130; g.revMod = 0.25f; break;
        case Space::Chamber:   g.revSize = 0.5f;  g.revDecay = 1.4f;  g.revDamp = 0.45f; g.revPre = 12; g.revLowCut = 120; g.revMod = 0.3f; break;
        case Space::Stage:     g.revSize = 0.6f;  g.revDecay = 1.7f;  g.revDamp = 0.5f;  g.revPre = 18; g.revLowCut = 140; g.revMod = 0.3f; break;
        case Space::Hall:      g.revSize = 0.78f; g.revDecay = 2.4f;  g.revDamp = 0.45f; g.revPre = 24; g.revLowCut = 110; g.revMod = 0.35f; break;
        case Space::Plate:     g.revSize = 0.42f; g.revDecay = 2.0f;  g.revDamp = 0.2f;  g.revPre = 6;  g.revLowCut = 200; g.revMod = 0.5f; break;
        case Space::Cathedral: g.revSize = 1.0f;  g.revDecay = 6.5f;  g.revDamp = 0.5f;  g.revPre = 35; g.revLowCut = 90;  g.revMod = 0.4f; break;
        case Space::Ambient:   g.revSize = 0.95f; g.revDecay = 11.0f; g.revDamp = 0.35f; g.revPre = 45; g.revLowCut = 160; g.revMod = 0.8f; break;
        case Space::Spring:    g.revSize = 0.2f;  g.revDecay = 1.6f;  g.revDamp = 0.15f; g.revPre = 2;  g.revLowCut = 300; g.revMod = 0.9f; break;
    }
}

// sync: 0 = free (ms), else StereoDelay division index
inline void echo (SoundData& s, int sync, float fb, float mix, float tone = 5000.0f, bool ping = true, float ms = 375.0f)
{
    auto& g = s.gfx;
    g.dlySync = (float) sync; g.dlyFb = fb; g.dlyMix = mix; g.dlyTone = tone; g.dlyPing = ping ? 1.0f : 0.0f; g.dlyTime = ms;
}

inline void mod (LayerParams& l, int slot, ModFx::Type t, float rate, float depth, float mix, float fb = 0.0f)
{
    auto& f = l.fx;
    if (slot == 1) { f.m1Type = (float) t; f.m1Rate = rate; f.m1Depth = depth; f.m1Mix = mix; f.m1Fb = fb; }
    else           { f.m2Type = (float) t; f.m2Rate = rate; f.m2Depth = depth; f.m2Mix = mix; f.m2Fb = fb; }
}

enum AmpType { AmpOff = 0, Tube, Overdrive, Fuzz, CleanAmp, CrunchAmp, LeadAmp, BassAmp, OrganAmp, Tape };
enum Cab { CabOff = 0, Open1x12, Combo2x12, Closed4x12, Bass1x15, EPSpeaker };

inline void amp (LayerParams& l, AmpType t, float gain, float tone = 0.0f, Cab cab = CabOff, float levelDb = 0.0f)
{
    auto& f = l.fx;
    f.ampType = (float) t; f.ampGain = gain; f.ampTone = tone; f.ampCab = (float) cab; f.ampLevel = levelDb;
}

inline void eq (LayerParams& l, float low, float mid, float midF, float high, float lowCut = 20.0f, float highCut = 20000.0f)
{
    auto& f = l.fx;
    f.eqLow = low; f.eqMid = mid; f.eqMidF = midF; f.eqHigh = high; f.lowCut = lowCut; f.highCut = highCut;
}

inline void wah (LayerParams& l, int mode, float freq, float q, float depth, float rate = 1.5f)
{
    auto& f = l.fx;
    f.wahMode = (float) mode; f.wahFreq = freq; f.wahQ = q; f.wahDepth = depth; f.wahRate = rate;
}

inline void lofi (SoundData& s, float bits, float rateK, float noise, float wow, float tone)
{
    auto& g = s.gfx;
    g.lofiBits = bits; g.lofiRate = rateK; g.lofiNoise = noise; g.lofiWow = wow; g.lofiTone = tone;
}

inline void ampEnv (AnalogParams& a, float A, float D, float S, float R) { a.aA = A; a.aD = D; a.aS = S; a.aR = R; }
inline void fltEnv (AnalogParams& a, float A, float D, float S, float R) { a.fA = A; a.fD = D; a.fS = S; a.fR = R; }
inline void osc (AnalogParams& a, int n, int wave, float oct, float semi, float fine, float level, float pw = 0.5f)
{
    auto set = [&] (float& w, float& o, float& s, float& f, float& l, float& p)
    {
        w = (float) wave; o = oct; s = semi; f = fine; l = level; p = pw;
    };
    if (n == 1)      set (a.o1Wave, a.o1Oct, a.o1Semi, a.o1Fine, a.o1Level, a.o1PW);
    else if (n == 2) set (a.o2Wave, a.o2Oct, a.o2Semi, a.o2Fine, a.o2Level, a.o2PW);
    else             set (a.o3Wave, a.o3Oct, a.o3Semi, a.o3Fine, a.o3Level, a.o3PW);
}
inline void filt (AnalogParams& a, int type, float cutoff, float res, float env, float keyTrack = 0.5f, float drive = 0.2f)
{
    a.fType = (float) type; a.cutoff = cutoff; a.res = res; a.fEnv = env; a.keyTrack = keyTrack; a.fDrive = drive;
}

inline void op (FMParams& f, int i, float ratio, float level, float A, float D, float S, float R, float vel = 0.5f, float key = 0.0f, float detune = 0.0f)
{
    auto& o = f.op[i];
    o.ratio = ratio; o.level = level; o.A = A; o.D = D; o.S = S; o.R = R; o.velSens = vel; o.keyScale = key; o.detune = detune; o.fixed = 0.0f;
}

inline void modes (MalletParams& m, std::initializer_list<float> ratios, std::initializer_list<float> gainsDb, std::initializer_list<float> decays)
{
    int i = 0;
    for (float r : ratios) if (i < 6) m.r[i++] = r;
    for (; i < 6; ++i) { m.r[i] = 1.0f; m.g[i] = -60.0f; }
    i = 0;
    for (float g : gainsDb) if (i < 6) m.g[i++] = g;
    i = 0;
    for (float d : decays) if (i < 6) m.d[i++] = d;
}

inline void drawbars (OrganParams& o, const char* reg)
{
    for (int i = 0; i < 9 && reg[i]; ++i) o.db[i] = (float) (reg[i] - '0');
}

inline void split (SoundData& s, int splitKey)
{
    s.layer[0].common.keyHi = (float) (splitKey - 1);
    s.layer[1].common.keyLo = (float) splitKey;
}

inline SoundData stack (SoundData a, const SoundData& b, float levelA = 0.0f, float levelB = -3.0f)
{
    a.layer[1] = b.layer[0];
    a.numLayers = 2;
    a.layer[0].common.level = levelA;
    a.layer[1].common.level = levelB;
    return a;
}

} // namespace kv::pb
