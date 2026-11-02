// Wavetable synths: morphing pads, keys, leads, basses, plucks, vocal and
// evolving textures built on the wavetable engine.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;

namespace
{
enum Table { Basic = 0, Pulse, SawSync, Harmonic, Vocal, Digital, Drawbar, FMSweep, Bell, Fold, ResSweep, Hollow, Noise, LPSweep, Choir, Metallic };
enum Warp { WOff = 0, BendUp, BendDown, Sync, Mirror, Squeeze, FMWarp, Crush };
enum Filter { Ladder24 = 0, SvfLP, SvfBP, SvfHP, SvfNotch, LPHP, Formant };

SoundData wt (int t1, float pos1, int t2 = Basic, float pos2 = 0.0f, float level2 = 0.0f)
{
    auto s = make (Engine::Wavetable);
    auto& l = s.layer[0];
    l.common.poly = 16;
    auto& w = l.wt;
    w.t1Table = (float) t1; w.t1Pos = pos1; w.t1Level = 1.0f;
    w.t2Table = (float) t2; w.t2Pos = pos2; w.t2Level = level2;
    return s;
}

WavetableParams& W (SoundData& s) { return s.layer[0].wt; }
void amp (WavetableParams& w, float A, float D, float S, float R) { w.aA = A; w.aD = D; w.aS = S; w.aR = R; }
void fenv (WavetableParams& w, float A, float D, float S, float R) { w.fA = A; w.fD = D; w.fS = S; w.fR = R; }
void menv (WavetableParams& w, float A, float D, float S, float R, float pos1, float pos2 = 0.0f)
{
    w.mA = A; w.mD = D; w.mS = S; w.mR = R; w.mPos1 = pos1; w.mPos2 = pos2;
}
void filter (WavetableParams& w, int type, float cutoff, float res, float env, float keyTrack = 0.5f, float drive = 0.2f)
{
    w.fType = (float) type; w.cutoff = cutoff; w.res = res; w.fEnv = env; w.keyTrack = keyTrack; w.fDrive = drive;
}
void unison (WavetableParams& w, int voices, float detune, float spread, float tableSpread = 0.0f)
{
    w.unison = (float) voices; w.uniDetune = detune; w.uniSpread = spread; w.uniPos = tableSpread;
}
void warp (WavetableParams& w, int osc, int mode, float amt)
{
    if (osc == 1) { w.t1Warp = (float) mode; w.t1WarpAmt = amt; }
    else          { w.t2Warp = (float) mode; w.t2WarpAmt = amt; }
}
void osc2 (WavetableParams& w, float oct, float semi, float fine)
{
    w.t2Oct = oct; w.t2Semi = semi; w.t2Fine = fine;
}

// Common starting points
SoundData padBase (int t1, float p1, int t2, float p2, float lvl2)
{
    auto s = wt (t1, p1, t2, p2, lvl2);
    auto& w = W (s);
    amp (w, 0.9f, 2.0f, 0.85f, 2.2f);
    fenv (w, 1.2f, 3.0f, 0.6f, 2.0f);
    filter (w, SvfLP, 2500, 0.15f, 1.0f, 0.4f);
    unison (w, 4, 14, 0.8f, 0.06f);
    w.velAmp = 0.25f; w.fVel = 0.4f; w.drift = 0.3f;
    w.l2Rate = 0.12f; w.l2Pos = 0.12f;
    space (s, Space::Hall, 0.38f);
    return s;
}

SoundData bassBase (int t1, float p1)
{
    auto s = wt (t1, p1);
    auto& l = s.layer[0];
    l.common.poly = 1; l.common.legato = 1; l.common.glide = 0.03f;
    auto& w = l.wt;
    amp (w, 0.002f, 0.6f, 0.8f, 0.12f);
    fenv (w, 0.001f, 0.35f, 0.25f, 0.2f);
    filter (w, Ladder24, 500, 0.3f, 3.0f, 0.4f, 0.4f);
    w.subLevel = 0.5f; w.velAmp = 0.35f; w.fVel = 1.0f; w.drift = 0.1f;
    return s;
}

SoundData pluckBase (int t1, float p1, float decay)
{
    auto s = wt (t1, p1);
    auto& w = W (s);
    amp (w, 0.001f, decay, 0.0f, decay * 0.6f);
    fenv (w, 0.001f, decay * 0.5f, 0.0f, decay * 0.4f);
    filter (w, SvfLP, 900, 0.2f, 4.0f, 0.6f);
    menv (w, 0.001f, decay * 0.7f, 0.0f, decay * 0.5f, 0.45f);
    w.velAmp = 0.5f; w.fVel = 1.2f; w.velPos = 0.3f;
    space (s, Space::Plate, 0.25f);
    return s;
}

SoundData leadBase (int t1, float p1)
{
    auto s = wt (t1, p1);
    auto& l = s.layer[0];
    l.common.poly = 1; l.common.legato = 1; l.common.glide = 0.06f; l.common.mwVib = 0.0f;
    auto& w = l.wt;
    amp (w, 0.003f, 0.5f, 0.9f, 0.25f);
    fenv (w, 0.002f, 0.6f, 0.5f, 0.3f);
    filter (w, Ladder24, 3000, 0.25f, 2.0f, 0.6f, 0.3f);
    w.l1Rate = 5.5f; w.l1Delay = 0.35f; w.mwPos = 0.4f; w.atVib = 0.4f; w.velAmp = 0.3f;
    echo (s, 2, 0.35f, 0.2f, 4000.0f);
    space (s, Space::Plate, 0.2f);
    return s;
}
} // namespace

void addWavetable (Lib& L)
{
    const char* C = kWavetable;

    // ---------------------------------------------------------------- Pads
    {
        auto s = padBase (Harmonic, 0.35f, LPSweep, 0.5f, 0.5f);
        auto& w = W (s); osc2 (w, 0, 0, 7); w.l2Pos = 0.2f; w.l2Rate = 0.08f;
        mod (s.layer[0], 1, ModFx::Chorus, 0.3f, 0.5f, 0.4f);
        L.add (C, "Pad", "Spectral Bloom", "wavetable pad evolving lush morph", s);
    }
    {
        auto s = padBase (Choir, 0.2f, Vocal, 0.6f, 0.35f);
        auto& w = W (s); amp (w, 1.4f, 2.0f, 0.9f, 3.0f); w.l2Pos = 0.3f; w.l2Rate = 0.06f; w.cutoff = 3500;
        mod (s.layer[0], 1, ModFx::Ensemble, 0.4f, 0.5f, 0.4f);
        space (s, Space::Cathedral, 0.4f);
        L.add (C, "Pad", "Glass Choir", "choir vocal aah pad wavetable", s);
    }
    {
        auto s = padBase (Digital, 0.1f, Digital, 0.6f, 0.6f);
        auto& w = W (s); osc2 (w, 1, 0, -5); filter (w, Ladder24, 1800, 0.3f, 1.5f, 0.4f);
        menv (w, 2.5f, 6.0f, 0.3f, 3.0f, 0.6f, -0.4f);
        mod (s.layer[0], 1, ModFx::Chorus, 0.25f, 0.6f, 0.45f);
        L.add (C, "Pad", "PPG Waves", "ppg digital 80s wave pad", s);
    }
    {
        auto s = padBase (ResSweep, 0.1f, ResSweep, 0.3f, 0.5f);
        auto& w = W (s); osc2 (w, 0, 0, -9); w.l2Pos = 0.35f; w.l2Rate = 0.05f; w.l2Shape = 6; w.cutoff = 6000;
        mod (s.layer[0], 1, ModFx::Phaser, 0.15f, 0.6f, 0.4f, 0.4f);
        L.add (C, "Pad", "Resonant Drift", "evolving sweep pad slow", s);
    }
    {
        auto s = padBase (Basic, 0.5f, Pulse, 0.4f, 0.6f);
        auto& w = W (s); unison (w, 7, 22, 1.0f, 0.04f); filter (w, Ladder24, 3200, 0.1f, 0.8f, 0.4f);
        mod (s.layer[0], 1, ModFx::Dimension, 0.4f, 0.6f, 0.5f);
        L.add (C, "Pad", "Hyper Saw Pad", "supersaw trance pad wide", s);
    }
    {
        auto s = padBase (Metallic, 0.3f, Bell, 0.5f, 0.4f);
        auto& w = W (s); osc2 (w, 1, 0, 0); amp (w, 2.0f, 3.0f, 0.8f, 4.0f); w.l2Pos = 0.4f; w.l2Rate = 0.04f;
        space (s, Space::Ambient, 0.5f); echo (s, 3, 0.45f, 0.2f, 3000.0f);
        L.add (C, "Pad", "Frozen Metal", "metallic cold ambient pad cinematic", s);
    }
    {
        auto s = padBase (Hollow, 0.0f, Hollow, 1.0f, 0.5f);
        auto& w = W (s); w.l2Pos = 0.45f; w.l2Rate = 0.1f; w.l2Shape = 1; w.l2Pan = 0.3f;
        mod (s.layer[0], 1, ModFx::Flanger, 0.08f, 0.5f, 0.35f, 0.5f);
        L.add (C, "Pad", "Comb Tide", "hollow flanging pad moving", s);
    }
    {
        auto s = padBase (Drawbar, 0.4f, Harmonic, 0.8f, 0.3f);
        auto& w = W (s); amp (w, 0.6f, 1.0f, 0.9f, 1.5f); w.mwPos = 0.5f; w.cutoff = 5000;
        mod (s.layer[0], 1, ModFx::Ensemble, 0.6f, 0.6f, 0.5f);
        L.add (C, "Pad", "Organ Shimmer Pad", "organ drawbar pad warm", s);
    }
    {
        auto s = padBase (Noise, 0.2f, Basic, 0.0f, 0.4f);
        auto& w = W (s); osc2 (w, -1, 0, 0); w.l2Pos = 0.4f; w.l2Rate = 0.03f; filter (w, SvfLP, 1400, 0.25f, 0.6f, 0.3f);
        space (s, Space::Ambient, 0.55f);
        L.add (C, "Pad", "Dust Nebula", "dark texture drone pad ambient", s);
    }
    {
        auto s = padBase (FMSweep, 0.15f, LPSweep, 0.4f, 0.5f);
        auto& w = W (s); menv (w, 3.0f, 5.0f, 0.5f, 3.0f, 0.5f); osc2 (w, 0, 0, 6);
        mod (s.layer[0], 1, ModFx::Chorus, 0.3f, 0.5f, 0.4f);
        L.add (C, "Pad", "Digital Dawn", "fm swell pad bright", s);
    }

    // ---------------------------------------------------------------- Keys
    {
        auto s = wt (FMSweep, 0.25f, Bell, 0.4f, 0.4f);
        auto& w = W (s); osc2 (w, 1, 0, 0);
        amp (w, 0.002f, 2.5f, 0.15f, 0.6f); filter (w, SvfLP, 6000, 0.05f, 0.5f, 0.5f);
        menv (w, 0.001f, 1.2f, 0.0f, 0.5f, 0.35f, 0.4f);
        w.velPos = 0.5f; w.velAmp = 0.6f;
        mod (s.layer[0], 1, ModFx::Chorus, 0.5f, 0.4f, 0.35f);
        space (s, Space::Room, 0.25f);
        L.add (C, "Keys", "Wave E.Piano", "digital electric piano wavetable keys", s);
    }
    {
        auto s = wt (Bell, 0.6f, Metallic, 0.2f, 0.3f);
        auto& w = W (s); osc2 (w, 2, 0, 0);
        amp (w, 0.001f, 3.5f, 0.0f, 2.0f); filter (w, SvfLP, 9000, 0.0f, 0.0f, 0.5f);
        menv (w, 0.001f, 0.8f, 0.0f, 0.5f, -0.5f);
        w.velPos = 0.4f;
        space (s, Space::Hall, 0.35f); echo (s, 2, 0.3f, 0.15f, 5000.0f);
        L.add (C, "Keys", "Crystal Keys", "bell glass keys sparkle", s);
    }
    {
        auto s = wt (Drawbar, 0.75f);
        auto& w = W (s); amp (w, 0.003f, 0.3f, 1.0f, 0.06f); filter (w, SvfLP, 12000, 0.0f, 0.0f, 0.0f);
        w.velAmp = 0.0f; w.mwPos = 0.25f;
        mod (s.layer[0], 1, ModFx::Rotary, 0.8f, 0.6f, 0.8f);
        space (s, Space::Room, 0.2f);
        L.add (C, "Keys", "Morph Organ", "organ drawbar wavetable rotary", s);
    }
    {
        auto s = wt (Pulse, 0.3f, Basic, 0.6f, 0.5f);
        auto& w = W (s); osc2 (w, 0, 0, 8);
        amp (w, 0.004f, 1.5f, 0.5f, 0.5f); fenv (w, 0.002f, 0.8f, 0.3f, 0.4f); filter (w, Ladder24, 1200, 0.2f, 2.5f, 0.5f);
        w.l1Rate = 0.4f; w.l1Pos = 0.12f;
        mod (s.layer[0], 1, ModFx::Chorus, 0.6f, 0.5f, 0.5f);
        space (s, Space::Room, 0.2f);
        L.add (C, "Keys", "Poly Wave Keys", "synth keys 80s poly", s);
    }
    {
        auto s = wt (Fold, 0.2f);
        auto& w = W (s); amp (w, 0.001f, 1.8f, 0.2f, 0.4f); filter (w, SvfLP, 8000, 0.0f, 0.0f, 0.5f);
        menv (w, 0.001f, 0.5f, 0.1f, 0.3f, 0.5f);
        w.velPos = 0.6f; w.velAmp = 0.5f;
        space (s, Space::Plate, 0.2f);
        L.add (C, "Keys", "West Coast Keys", "wavefolder buchla keys", s);
    }
    {
        auto s = wt (Harmonic, 0.15f, Harmonic, 0.5f, 0.4f);
        auto& w = W (s); osc2 (w, 1, 7, 0);
        amp (w, 0.002f, 2.0f, 0.3f, 0.8f); filter (w, SvfLP, 5000, 0.1f, 1.0f, 0.5f);
        menv (w, 0.001f, 1.5f, 0.2f, 0.6f, 0.3f, 0.3f);
        mod (s.layer[0], 1, ModFx::Phaser, 0.3f, 0.5f, 0.3f);
        space (s, Space::Hall, 0.25f);
        L.add (C, "Keys", "Additive Keys", "harmonic additive keys bright", s);
    }

    // ---------------------------------------------------------------- Leads
    {
        auto s = leadBase (SawSync, 0.2f);
        auto& w = W (s); menv (w, 0.001f, 0.8f, 0.2f, 0.3f, 0.6f); w.mwPos = 0.5f;
        L.add (C, "Lead", "Sync Scream", "sync lead aggressive", s);
    }
    {
        auto s = leadBase (Vocal, 0.0f);
        auto& w = W (s); w.l2Pos = 0.5f; w.l2Rate = 0.6f; w.l2Shape = 1; filter (w, SvfLP, 5000, 0.1f, 0.5f, 0.5f);
        L.add (C, "Lead", "Talking Lead", "vowel vocal talkbox lead", s);
    }
    {
        auto s = leadBase (Basic, 0.55f);
        auto& w = W (s); unison (w, 5, 18, 0.7f); w.t2Table = Pulse; w.t2Level = 0.6f; w.t2Pos = 0.3f; osc2 (w, 1, 0, 3);
        L.add (C, "Lead", "Wide Wave Lead", "unison saw lead edm", s);
    }
    {
        auto s = leadBase (FMSweep, 0.3f);
        auto& w = W (s); warp (w, 1, FMWarp, 0.3f); w.t2Table = Basic; w.t2Pos = 0.0f; w.t2Level = 0.0f; osc2 (w, 1, 0, 0);
        menv (w, 0.001f, 0.4f, 0.3f, 0.3f, 0.3f); w.mWarp = 0.3f;
        L.add (C, "Lead", "Digital Brass Lead", "fm brass lead bright", s);
    }
    {
        auto s = leadBase (Hollow, 0.6f);
        auto& w = W (s); warp (w, 1, Squeeze, 0.4f); w.l1Pos = 0.1f; w.l1Rate = 4.0f; filter (w, Ladder24, 2200, 0.35f, 1.5f, 0.6f);
        L.add (C, "Lead", "Hollow Whistle", "soft flute lead hollow", s);
    }
    {
        auto s = leadBase (Fold, 0.4f);
        auto& w = W (s); menv (w, 0.001f, 1.0f, 0.4f, 0.4f, 0.5f); w.atPos = 0.4f; filter (w, Ladder24, 4000, 0.2f, 1.0f, 0.6f, 0.6f);
        L.add (C, "Lead", "Folded Solo", "wavefolder lead expressive aftertouch", s);
    }

    // ---------------------------------------------------------------- Bass
    {
        auto s = bassBase (Basic, 0.5f);
        L.add (C, "Bass", "Wave Bass", "wavetable bass saw", s);
    }
    {
        auto s = bassBase (ResSweep, 0.1f);
        auto& w = W (s); menv (w, 0.001f, 0.25f, 0.0f, 0.2f, 0.5f); filter (w, Ladder24, 900, 0.2f, 2.0f, 0.4f, 0.5f);
        L.add (C, "Bass", "Growl Bass", "growl dubstep bass wobble", s);
    }
    {
        auto s = bassBase (Fold, 0.25f);
        auto& w = W (s); w.l1Rate = 2.0f; w.l1Pos = 0.3f; w.l1Shape = 1; filter (w, Ladder24, 1600, 0.25f, 1.0f, 0.4f, 0.6f);
        L.add (C, "Bass", "Folded Wobble", "wobble bass lfo", s);
    }
    {
        auto s = bassBase (FMSweep, 0.4f);
        auto& w = W (s); menv (w, 0.001f, 0.18f, 0.1f, 0.2f, 0.4f); filter (w, SvfLP, 3000, 0.0f, 0.5f, 0.5f);
        L.add (C, "Bass", "FM Wave Bass", "fm bass punchy", s);
    }
    {
        auto s = bassBase (Basic, 0.0f);
        auto& w = W (s); w.subLevel = 0.6f; amp (w, 0.002f, 1.2f, 0.7f, 0.15f); filter (w, SvfLP, 300, 0.0f, 1.0f, 0.3f);
        menv (w, 0.001f, 0.08f, 0.0f, 0.1f, 0.25f); w.mPitch = 7.0f;
        L.add (C, "Bass", "Sub Drop", "808 sub bass deep", s);
    }
    {
        auto s = bassBase (Digital, 0.3f);
        auto& w = W (s); warp (w, 1, Crush, 0.4f); filter (w, Ladder24, 1200, 0.3f, 2.0f, 0.4f, 0.7f);
        L.add (C, "Bass", "Crushed Bass", "bitcrush digital bass dirty", s);
    }
    {
        auto s = bassBase (Vocal, 0.2f);
        auto& w = W (s); w.mwPos = 0.8f; menv (w, 0.005f, 0.4f, 0.3f, 0.2f, 0.3f);
        L.add (C, "Bass", "Vowel Bass", "talking bass yoy vowel", s);
    }
    {
        auto s = bassBase (SawSync, 0.1f);
        auto& w = W (s); menv (w, 0.001f, 0.3f, 0.0f, 0.2f, 0.5f); s.layer[0].common.glide = 0.08f;
        L.add (C, "Bass", "Sync Bass", "sync bass acid", s);
    }

    // ---------------------------------------------------------------- Plucks
    {
        auto s = pluckBase (Harmonic, 0.2f, 0.6f);
        echo (s, 2, 0.35f, 0.2f, 5000.0f);
        L.add (C, "Pluck", "Harmonic Pluck", "pluck bright additive", s);
    }
    {
        auto s = pluckBase (Bell, 0.3f, 1.2f);
        auto& w = W (s); filter (w, SvfLP, 3000, 0.1f, 2.0f, 0.6f);
        echo (s, 3, 0.4f, 0.2f, 4000.0f);
        L.add (C, "Pluck", "Bell Pluck", "bell pluck mallet", s);
    }
    {
        auto s = pluckBase (Digital, 0.0f, 0.35f);
        auto& w = W (s); unison (w, 3, 10, 0.6f, 0.05f); echo (s, 5, 0.3f, 0.2f, 6000.0f);
        L.add (C, "Pluck", "Digital Stab", "digital pluck house stab", s);
    }
    {
        auto s = pluckBase (Metallic, 0.4f, 0.8f);
        auto& w = W (s); w.mPos1 = -0.3f; filter (w, SvfLP, 2000, 0.2f, 3.0f, 0.6f);
        L.add (C, "Pluck", "Metal Pluck", "metallic pluck kalimba", s);
    }
    {
        auto s = pluckBase (Basic, 0.55f, 0.4f);
        auto& w = W (s); unison (w, 4, 16, 0.8f); filter (w, Ladder24, 600, 0.25f, 4.5f, 0.6f);
        echo (s, 2, 0.3f, 0.18f, 5000.0f);
        L.add (C, "Pluck", "Trance Pluck", "trance pluck saw supersaw", s);
    }
    {
        auto s = pluckBase (Fold, 0.0f, 0.5f);
        auto& w = W (s); w.mPos1 = 0.7f; filter (w, SvfLP, 6000, 0.0f, 1.0f, 0.5f);
        L.add (C, "Pluck", "Folded Pluck", "wavefolder pluck west coast", s);
    }

    // ---------------------------------------------------------------- Textures & Vocal
    {
        auto s = padBase (Vocal, 0.0f, Choir, 0.5f, 0.4f);
        auto& w = W (s); w.l2Pos = 0.5f; w.l2Rate = 0.07f; w.l2Shape = 6; filter (w, SvfLP, 4500, 0.1f, 0.5f, 0.4f);
        space (s, Space::Cathedral, 0.45f);
        L.add (C, "Texture", "Vowel Morph Choir", "vocal vowel morph choir", s);
    }
    {
        auto s = padBase (Noise, 0.0f, Metallic, 0.5f, 0.4f);
        auto& w = W (s); amp (w, 3.0f, 4.0f, 0.8f, 5.0f); w.l2Pos = 0.5f; w.l2Rate = 0.02f; w.l2Cut = 1.0f;
        space (s, Space::Ambient, 0.6f); echo (s, 9, 0.55f, 0.25f, 2500.0f);
        L.add (C, "Texture", "Event Horizon", "cinematic drone texture dark", s);
    }
    {
        auto s = wt (Digital, 0.0f, Noise, 0.5f, 0.4f);
        auto& l = s.layer[0];
        l.common.arpMode = 1; l.common.arpRate = 3; l.common.arpOct = 2; l.common.arpGate = 0.5f;
        auto& w = l.wt; amp (w, 0.001f, 0.25f, 0.0f, 0.2f); filter (w, SvfLP, 3000, 0.3f, 2.0f, 0.5f);
        w.l2Pos = 0.5f; w.l2Rate = 0.1f;
        echo (s, 4, 0.45f, 0.25f, 4000.0f); space (s, Space::Hall, 0.25f);
        L.add (C, "Texture", "Wave Sequence", "arp sequence wavestation evolving", s);
    }
    {
        auto s = padBase (SawSync, 0.0f, Hollow, 0.5f, 0.3f);
        auto& w = W (s); amp (w, 4.0f, 2.0f, 1.0f, 3.0f); menv (w, 6.0f, 2.0f, 1.0f, 2.0f, 0.8f);
        filter (w, SvfLP, 1200, 0.3f, 3.0f, 0.4f); fenv (w, 6.0f, 2.0f, 1.0f, 2.0f);
        space (s, Space::Ambient, 0.4f);
        L.add (C, "Texture", "Rising Tension", "riser swell cinematic build", s);
    }
    {
        auto s = wt (Choir, 0.4f, Vocal, 0.8f, 0.3f);
        auto& w = W (s); amp (w, 0.25f, 1.0f, 0.9f, 1.2f); filter (w, Formant, 3000, 0.4f, 0.0f, 0.0f); w.vowel = 1.5f;
        unison (w, 3, 10, 0.7f, 0.05f); w.mwPos = 0.5f;
        mod (s.layer[0], 1, ModFx::Ensemble, 0.4f, 0.5f, 0.4f);
        space (s, Space::Hall, 0.35f);
        L.add (C, "Texture", "Formant Voices", "voice choir ooh formant", s);
    }
}
} // namespace kv::banks
