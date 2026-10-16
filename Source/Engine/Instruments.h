// Keyverse - base instrument voicings shared by the factory presets.
// Every preset starts from one of these and then varies tone, space and FX.
#pragma once

#include "PresetHelpers.h"

namespace kv::inst
{
using namespace pb;

//==============================================================================
// Acoustic pianos
inline SoundData grand()
{
    auto s = make (Engine::Piano);
    auto& l = s.layer[0];
    l.common.poly = 64;
    auto& p = l.piano;
    p.hardness = 0.5f; p.hammerPos = 0.12f; p.unison = 0.7f; p.decay = 1.0f; p.release = 0.22f;
    p.hfDamp = 0.5f; p.inharm = 1.0f; p.stretch = 1.0f; p.aftersound = 0.4f;
    p.hammerNoise = 0.3f; p.releaseNoise = 0.3f; p.mechNoise = 0.08f;
    p.body = 0.4f; p.bodySize = 0.85f; p.bodyTone = 0.0f; p.pedalRes = 0.4f; p.width = 0.65f; p.velSens = 0.6f;
    space (s, Space::Hall, 0.16f);
    return s;
}

inline SoundData upright()
{
    auto s = grand();
    auto& p = s.layer[0].piano;
    p.inharm = 2.4f; p.decay = 0.62f; p.bodySize = 0.25f; p.hardness = 0.55f; p.hammerPos = 0.115f;
    p.unison = 1.3f; p.hfDamp = 0.75f; p.body = 0.5f; p.bodyTone = -0.15f; p.mechNoise = 0.25f; p.width = 0.45f;
    p.aftersound = 0.3f;
    space (s, Space::Room, 0.14f);
    return s;
}

inline SoundData feltPiano()
{
    auto s = grand();
    auto& p = s.layer[0].piano;
    p.felt = 0.85f; p.hardness = 0.25f; p.mechNoise = 0.75f; p.hammerNoise = 0.1f; p.releaseNoise = 0.6f;
    p.decay = 0.85f; p.unison = 0.4f; p.bodyTone = -0.35f; p.width = 0.75f; p.bodySize = 0.5f; p.velSens = 0.5f;
    space (s, Space::Room, 0.2f);
    return s;
}

inline SoundData electricGrand()
{
    auto s = grand();
    auto& l = s.layer[0];
    auto& p = l.piano;
    p.piezo = 1.0f; p.inharm = 1.9f; p.decay = 0.55f; p.bodySize = 0.3f; p.unison = 2.5f; p.hardness = 0.62f;
    p.body = 0.15f; p.hammerNoise = 0.15f; p.width = 0.5f; p.hfDamp = 0.6f;
    mod (l, 1, ModFx::Chorus, 0.6f, 0.35f, 0.35f);
    space (s, Space::Room, 0.14f);
    return s;
}

//==============================================================================
// Electric pianos
inline SoundData tineEP()
{
    auto s = make (Engine::EPiano);
    auto& l = s.layer[0];
    l.common.poly = 32;
    auto& e = l.ep;
    e.pickup = 0; e.excite = 0; e.hardness = 0.45f; e.bell = 0.45f; e.bellRatio = 6.27f; e.bellDecay = 0.25f;
    e.mode3 = 0.1f; e.toneBar = 0.4f; e.beat = 0.7f; e.offset = 0.45f; e.drive = 0.45f; e.distance = 0.6f;
    e.decay = 1.0f; e.release = 0.15f; e.thump = 0.3f; e.relNoise = 0.25f; e.velSens = 0.6f; e.width = 0.3f; e.bark = 0.55f;
    amp (l, Tube, 0.15f, 0.0f, EPSpeaker);
    space (s, Space::Room, 0.12f);
    return s;
}

inline SoundData reedEP()
{
    auto s = make (Engine::EPiano);
    auto& l = s.layer[0];
    l.common.poly = 32;
    auto& e = l.ep;
    e.pickup = 1; e.excite = 0; e.hardness = 0.6f; e.bell = 0.2f; e.bellRatio = 5.4f; e.bellDecay = 0.06f;
    e.mode3 = 0.05f; e.toneBar = 0.0f; e.beat = 0.0f; e.offset = 0.3f; e.drive = 0.5f; e.distance = 0.55f;
    e.decay = 0.5f; e.release = 0.07f; e.thump = 0.25f; e.relNoise = 0.2f; e.velSens = 0.6f; e.width = 0.25f; e.bark = 0.6f;
    amp (l, Tube, 0.2f, 0.1f, EPSpeaker);
    eq (l, 0.0f, 3.0f, 1200.0f, 0.0f);
    mod (l, 1, ModFx::Tremolo, 5.5f, 0.0f, 1.0f);
    space (s, Space::Room, 0.12f);
    return s;
}

inline SoundData pianet()
{
    auto s = make (Engine::EPiano);
    auto& l = s.layer[0];
    auto& e = l.ep;
    e.pickup = 0; e.excite = 1; e.hardness = 0.5f; e.bell = 0.15f; e.bellRatio = 5.9f; e.bellDecay = 0.05f;
    e.toneBar = 0.0f; e.offset = 0.6f; e.drive = 0.3f; e.distance = 0.7f; e.decay = 0.35f; e.release = 0.05f;
    e.thump = 0.05f; e.relNoise = 0.25f; e.bark = 0.3f; e.width = 0.2f;
    space (s, Space::Room, 0.1f);
    return s;
}

//==============================================================================
// Plucked strings
inline SoundData nylonGuitar()
{
    auto s = make (Engine::Pluck);
    auto& l = s.layer[0];
    l.common.poly = 12;
    auto& p = l.pluck;
    p.excite = 1; p.bright = 0.45f; p.pluckPos = 0.17f; p.decay = 0.75f; p.damping = 0.5f; p.stiffness = 0.05f;
    p.body = 1; p.bodyMix = 0.75f; p.twang = 3.0f; p.release = 0.3f; p.relNoise = 0.12f; p.pickNoise = 0.08f; p.width = 0.4f;
    space (s, Space::Room, 0.16f);
    return s;
}

inline SoundData steelGuitar()
{
    auto s = make (Engine::Pluck);
    auto& l = s.layer[0];
    l.common.poly = 12;
    auto& p = l.pluck;
    p.excite = 0; p.bright = 0.62f; p.pluckPos = 0.13f; p.decay = 1.0f; p.damping = 0.38f; p.stiffness = 0.15f;
    p.body = 2; p.bodyMix = 0.65f; p.twang = 5.0f; p.release = 0.25f; p.relNoise = 0.12f; p.pickNoise = 0.35f; p.width = 0.4f;
    space (s, Space::Room, 0.14f);
    return s;
}

inline SoundData electricGuitar (float pickup = 0.2f)
{
    auto s = make (Engine::Pluck);
    auto& l = s.layer[0];
    l.common.poly = 12;
    auto& p = l.pluck;
    p.excite = 0; p.bright = 0.55f; p.pluckPos = 0.12f; p.decay = 1.5f; p.damping = 0.32f; p.stiffness = 0.1f;
    p.pickupPos = pickup; p.body = 0; p.twang = 6.0f; p.release = 0.15f; p.relNoise = 0.1f; p.pickNoise = 0.25f; p.width = 0.25f;
    amp (l, CleanAmp, 0.3f, 0.1f, Open1x12);
    space (s, Space::Spring, 0.12f);
    return s;
}

inline SoundData bassGuitar()
{
    auto s = make (Engine::Pluck);
    auto& l = s.layer[0];
    l.common.poly = 6;
    auto& p = l.pluck;
    p.excite = 1; p.bright = 0.35f; p.pluckPos = 0.2f; p.decay = 1.2f; p.damping = 0.5f; p.stiffness = 0.25f;
    p.pickupPos = 0.17f; p.twang = 4.0f; p.release = 0.08f; p.relNoise = 0.15f; p.pickNoise = 0.1f; p.width = 0.0f;
    amp (l, BassAmp, 0.25f, 0.0f, Bass1x15);
    return s;
}

inline SoundData harpsichord()
{
    auto s = make (Engine::Pluck);
    auto& l = s.layer[0];
    l.common.poly = 32;
    auto& p = l.pluck;
    p.excite = 4; p.bright = 0.85f; p.pluckPos = 0.08f; p.decay = 0.9f; p.damping = 0.18f; p.stiffness = 0.1f;
    p.body = 7; p.bodyMix = 0.6f; p.course = 2; p.courseOct = 1; p.courseDetune = 2.0f; p.courseDelay = 3.0f;
    p.release = 0.07f; p.relNoise = 0.45f; p.pickNoise = 0.3f; p.twang = 2.0f; p.width = 0.5f; p.velSens = 0.15f;
    space (s, Space::Chamber, 0.18f);
    return s;
}

inline SoundData clavinet()
{
    auto s = make (Engine::Pluck);
    auto& l = s.layer[0];
    l.common.poly = 16;
    auto& p = l.pluck;
    p.excite = 3; p.bright = 0.8f; p.pluckPos = 0.05f; p.decay = 0.6f; p.damping = 0.22f; p.stiffness = 0.2f;
    p.pickupPos = 0.06f; p.pickup2Pos = 0.16f; p.pickup2Mix = 0.5f; p.release = 0.035f; p.relNoise = 0.55f;
    p.pickNoise = 0.15f; p.twang = 3.0f; p.width = 0.15f; p.velSens = 0.7f;
    amp (l, Tube, 0.2f, 0.2f, CabOff);
    space (s, Space::Room, 0.08f);
    return s;
}

//==============================================================================
// Mallets & bells
inline SoundData mallet (std::initializer_list<float> r, std::initializer_list<float> g, std::initializer_list<float> d,
                         float decay, float hardness, float noise = 0.2f, float noiseHz = 2500.0f)
{
    auto s = make (Engine::Mallet);
    auto& m = s.layer[0].mallet;
    s.layer[0].common.poly = 32;
    modes (m, r, g, d);
    m.decay = decay; m.hardness = hardness; m.noise = noise; m.noiseFreq = noiseHz;
    space (s, Space::Hall, 0.16f);
    return s;
}

inline SoundData vibraphone()
{
    auto s = mallet ({ 1.0f, 3.99f, 9.95f }, { 0, -10, -24 }, { 1.0f, 0.35f, 0.15f }, 7.0f, 0.45f, 0.08f, 3000.0f);
    auto& m = s.layer[0].mallet;
    m.resonator = 0.5f; m.damped = 1; m.release = 0.25f; m.decayTrack = 0.5f;
    mod (s.layer[0], 1, ModFx::Tremolo, 5.0f, 0.45f, 1.0f);
    return s;
}

inline SoundData marimba()
{
    auto s = mallet ({ 1.0f, 3.93f, 9.24f }, { 0, -8, -20 }, { 1.0f, 0.3f, 0.12f }, 1.2f, 0.4f, 0.25f, 1400.0f);
    auto& m = s.layer[0].mallet;
    m.resonator = 0.8f; m.decayTrack = 0.7f;
    return s;
}

inline SoundData glockenspiel()
{
    auto s = mallet ({ 1.0f, 2.71f, 5.15f, 8.43f }, { 0, -10, -18, -26 }, { 1.0f, 0.5f, 0.3f, 0.2f }, 4.0f, 0.8f, 0.15f, 6000.0f);
    s.layer[0].common.transpose = 24;
    return s;
}

inline SoundData celeste()
{
    auto s = mallet ({ 1.0f, 2.76f, 4.0f, 5.4f }, { 0, -20, -24, -30 }, { 1.0f, 0.4f, 0.3f, 0.2f }, 2.2f, 0.45f, 0.08f, 2000.0f);
    auto& m = s.layer[0].mallet;
    m.damped = 1; m.release = 0.18f; m.resonator = 0.3f;
    s.layer[0].common.transpose = 12;
    return s;
}

inline SoundData musicBox()
{
    auto s = mallet ({ 1.0f, 6.27f, 17.55f }, { 0, -12, -26 }, { 1.0f, 0.25f, 0.1f }, 2.6f, 0.7f, 0.1f, 5000.0f);
    auto& m = s.layer[0].mallet;
    m.excite = 1; m.beat = 0.6f; m.beatMix = 0.25f;
    s.layer[0].common.transpose = 24;
    return s;
}

//==============================================================================
// Organ
inline SoundData tonewheel (const char* reg)
{
    auto s = make (Engine::Organ);
    auto& l = s.layer[0];
    l.common.poly = 40;
    auto& o = l.organ;
    o.model = 0;
    drawbars (o, reg);
    o.click = 0.45f; o.leakage = 0.2f; o.attack = 0.004f; o.release = 0.012f; o.vibrato = 6; o.tone = 9000.0f;
    amp (l, OrganAmp, 0.25f);
    mod (l, 1, ModFx::Rotary, 0.8f, 0.6f, 1.0f);
    space (s, Space::Room, 0.12f);
    return s;
}

//==============================================================================
// Analog synth starting point (one saw through the ladder)
inline SoundData analog()
{
    auto s = make (Engine::Analog);
    auto& l = s.layer[0];
    l.common.poly = 16;
    return s;
}

inline SoundData fmSynth()
{
    auto s = make (Engine::FM);
    s.layer[0].common.poly = 16;
    return s;
}


//==============================================================================
// FM electric piano (three operator pairs: body, tine, bell)
inline SoundData dxEP (float tine = 0.6f, float body = 0.72f, float bellPair = 0.68f, float spread = 3.0f)
{
    auto s = fmSynth();
    auto& l = s.layer[0];
    l.common.poly = 24;
    auto& f = l.fm;
    f.alg = 2; f.feedback = 0.35f; f.rateScale = 0.45f;
    op (f, 0, 1.0f, 1.0f, 0.001f, 3.2f, 0.0f, 0.45f, 0.35f, 0.0f, 0.0f);
    op (f, 1, 1.0f, body, 0.001f, 1.6f, 0.08f, 0.4f, 0.8f, 0.2f);
    op (f, 2, 1.0f, 0.92f, 0.001f, 2.4f, 0.0f, 0.4f, 0.4f, 0.0f, spread);
    op (f, 3, 14.0f, tine, 0.001f, 0.28f, 0.0f, 0.2f, 0.9f, 0.5f);
    op (f, 4, 1.0f, 0.88f, 0.001f, 2.8f, 0.0f, 0.45f, 0.35f, 0.0f, -spread);
    op (f, 5, 1.0f, bellPair, 0.001f, 1.2f, 0.05f, 0.35f, 0.75f, 0.2f);
    mod (l, 1, ModFx::Chorus, 0.55f, 0.3f, 0.3f);
    space (s, Space::Room, 0.14f);
    return s;
}

// Simple 2-op style FM patch on algorithm "three pairs" using only the first pair.
inline SoundData fmPair (float carRatio, float modRatio, float modLevel, float carDecay, float modDecay, float carSus = 0.0f,
                         float modSus = 0.0f, float fb = 0.0f, float release = 0.4f)
{
    auto s = fmSynth();
    auto& f = s.layer[0].fm;
    f.alg = 2; f.feedback = fb;
    op (f, 0, carRatio, 1.0f, 0.002f, carDecay, carSus, release, 0.4f, 0.0f);
    op (f, 1, modRatio, modLevel, 0.001f, modDecay, modSus, release, 0.8f, 0.2f);
    for (int i = 2; i < 6; ++i) f.op[i].level = 0.0f;
    return s;
}

//==============================================================================
// Analog helpers
inline SoundData analogPatch (int w1, int w2, float detuneCents, float cutoff, float res, float envOct,
                              float A, float D, float S, float R, float fA, float fD, float fS, float fR)
{
    auto s = analog();
    auto& a = s.layer[0].analog;
    osc (a, 1, w1, 0, 0, 0, 0.85f);
    osc (a, 2, w2, 0, 0, detuneCents, w2 >= 0 ? 0.75f : 0.0f);
    filt (a, 0, cutoff, res, envOct);
    ampEnv (a, A, D, S, R);
    fltEnv (a, fA, fD, fS, fR);
    return s;
}

inline void mono (SoundData& s, float glide = 0.0f, bool legato = true)
{
    auto& c = s.layer[0].common;
    c.poly = 1; c.glide = glide; c.legato = legato ? 1.0f : 0.0f;
}

} // namespace kv::inst
