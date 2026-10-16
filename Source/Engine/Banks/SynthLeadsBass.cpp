// Synth leads and synth basses.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

static SoundData lead (int w1, int w2, float det, float cut, float res, float env, float glide)
{
    auto s = analogPatch (w1, w2, det, cut, res, env, 0.004f, 0.4f, 0.85f, 0.25f, 0.004f, 0.5f, 0.4f, 0.3f);
    mono (s, glide, true);
    auto& a = s.layer[0].analog;
    a.l1Pitch = 0.0f; a.velAmp = 0.3f; a.drift = 0.3f;
    s.layer[0].common.mwVib = 0.5f;
    echo (s, 3, 0.3f, 0.18f, 4000.0f);
    space (s, Space::Plate, 0.16f);
    return s;
}

static SoundData bass (int w1, int w2, float det, float cut, float res, float env, float dec, float sus)
{
    auto s = analogPatch (w1, w2, det, cut, res, env, 0.002f, dec, sus, 0.08f, 0.001f, dec * 0.6f, 0.1f, 0.08f);
    mono (s, 0.0f, false);
    auto& a = s.layer[0].analog;
    a.keyTrack = 0.3f; a.velAmp = 0.4f; a.drift = 0.2f;
    return s;
}

void addLeads (Lib& L)
{
    const char* D = kLeads;

    {
        auto s = lead (0, 1, 6, 2500, 0.35f, 2.0f, 0.06f); s.layer[0].analog.o2Oct = 0;
        L.add (D, "Mono", "Classic Mono Lead", "minimoog lead classic 70s", s);
    }
    L.add (D, "Mono", "Saw Lead", "saw lead bright", lead (0, 0, 10, 4000, 0.2f, 1.0f, 0.03f));
    {
        auto s = lead (1, 1, 0, 3500, 0.15f, 1.0f, 0.03f); s.layer[0].analog.o2Oct = 1; s.layer[0].analog.o2Level = 0.3f;
        L.add (D, "Mono", "Square Lead", "square lead", s);
    }
    {
        auto s = lead (0, 0, 0, 5000, 0.2f, 0.5f, 0.02f); auto& a = s.layer[0].analog; a.sync = 1; a.o2Semi = 12; a.o2Fine = 70; a.o1Level = 0.2f;
        a.l1Pitch = 0.0f; a.mwCut = 1.0f;
        L.add (D, "Mono", "Sync Lead", "hard sync lead cars", s);
    }
    {
        auto s = lead (1, -1, 0, 3000, 0.2f, 1.0f, 0.03f); s.layer[0].analog.o1PW = 0.1f;
        L.add (D, "Mono", "Thin Pulse Lead", "pulse thin nasal", s);
    }
    {
        auto s = lead (0, 0, 14, 3000, 0.25f, 1.5f, 0.05f); auto& a = s.layer[0].analog; a.unison = 3; a.uniDetune = 20;
        L.add (D, "Mono", "Fat Detuned Lead", "fat detuned unison", s);
    }
    {
        auto s = lead (5, -1, 0, 5000, 0.15f, 0.5f, 0.02f); s.layer[0].analog.uniDetune = 30; echo (s, 3, 0.4f, 0.25f, 5000);
        L.add (D, "Digital", "Supersaw Lead", "supersaw trance edm lead", s);
    }
    {
        auto s = lead (0, 1, 7, 2200, 0.3f, 1.5f, 0.22f);
        L.add (D, "Mono", "Portamento Lead", "portamento glide slide", s);
    }
    {
        auto s = lead (3, 0, 0, 2000, 0.0f, 0.0f, 0.12f); auto& a = s.layer[0].analog; a.o2Level = 0.12f;
        a.l1Pitch = 0.25f; a.l1Rate = 5.5f; a.l1Delay = 0.35f; a.hpCut = 200;
        L.add (D, "Mono", "G-Funk Whine", "g funk west coast sine whistle", s);
    }
    {
        auto s = lead (1, -1, 0, 18000, 0.0f, 0.0f, 0.0f); auto& a = s.layer[0].analog; a.o1PW = 0.25f; a.l1Pitch = 0.2f; a.l1Delay = 0.25f;
        s.gfx.dlyMix = 0.0f; space (s, Space::Room, 0.08f);
        L.add (D, "Chip", "Chiptune Lead", "chiptune 8bit gameboy nes", s);
    }
    {
        auto s = fmPair (1.0f, 1.0f, 0.8f, 3.0f, 2.0f, 0.8f, 0.6f, 0.55f, 0.25f); mono (s, 0.03f);
        s.layer[0].common.mwVib = 0.5f; echo (s, 3, 0.3f, 0.18f);
        L.add (D, "FM", "FM Lead", "fm digital lead", s);
    }
    {
        auto s = lead (0, 0, 6, 900, 0.15f, 2.5f, 0.04f); auto& a = s.layer[0].analog; a.fA = 0.06f; a.fD = 0.5f; a.fS = 0.55f;
        L.add (D, "Mono", "Brass Lead", "brassy lead swell", s);
    }
    {
        auto s = lead (0, 1, 6, 1500, 0.3f, 0.0f, 0.05f); auto& a = s.layer[0].analog; a.fType = 6; a.vowel = 0.5f;
        s.layer[0].common.level = -5.0f;
        L.add (D, "Vocal", "Vocal Lead", "vocal formant lead talk", s);
    }
    {
        auto s = lead (3, -1, 0, 6000, 0.0f, 0.0f, 0.06f); auto& a = s.layer[0].analog; a.noiseLevel = 0.05f; a.l1Pitch = 0.2f; a.l1Rate = 5.8f; a.l1Delay = 0.3f;
        L.add (D, "Mono", "Whistle Lead", "whistle sine pure", s);
    }
    {
        auto s = lead (0, 0, 8, 3000, 0.3f, 1.0f, 0.04f); amp (s.layer[0], LeadAmp, 0.6f, 0.0f, Closed4x12);
        L.add (D, "Mono", "Screamer Lead", "distorted screaming lead", s);
    }
    {
        auto s = lead (1, 1, 5, 2500, 0.2f, 1.0f, 0.03f); amp (s.layer[0], Fuzz, 0.5f, 0.1f, Combo2x12);
        L.add (D, "Mono", "Fuzz Square Lead", "fuzz square distorted", s);
    }
    {
        auto s = lead (3, -1, 0, 2500, 0.1f, 0.5f, 0.05f); auto& a = s.layer[0].analog;
        a.noiseLevel = 0.18f; a.aA = 0.05f; a.l1Pitch = 0.18f; a.l1Delay = 0.4f; a.l1Rate = 5.0f; a.velAmp = 0.6f;
        L.add (D, "Mono", "Flute Lead", "flute breathy synth", s);
    }
    {
        auto s = lead (0, 1, 5, 1100, 0.6f, 3.0f, 0.05f);
        L.add (D, "Mono", "Prog Lead", "prog rock emerson resonant", s);
    }
    {
        auto s = lead (0, 1, 0, 6000, 0.1f, 0.0f, 0.03f); auto& a = s.layer[0].analog; a.ring = 0.8f; a.o2Semi = 7; a.o1Level = 0.3f; a.o2Level = 0.2f;
        L.add (D, "Mono", "Ring Mod Lead", "ring modulator metallic", s);
    }
    {
        auto s = lead (0, 1, 6, 4000, 0.2f, 0.0f, 0.04f); wah (s.layer[0], 1, 800.0f, 7.0f, 0.8f);
        L.add (D, "Mono", "Wah Lead", "auto wah lead funk", s);
    }
    {
        auto s = lead (3, 2, 0, 5000, 0.0f, 0.0f, 0.08f); s.layer[0].analog.o2Level = 0.3f; s.layer[0].common.level = -4.0f;
        L.add (D, "Mono", "Soft Sine Lead", "soft sine smooth", s);
    }
    {
        auto s = lead (3, -1, 0, 8000, 0.0f, 0.0f, 0.25f); auto& a = s.layer[0].analog;
        a.l1Pitch = 0.35f; a.l1Rate = 6.0f; a.l1Delay = 0.15f; a.aA = 0.15f; a.aR = 0.5f;
        space (s, Space::Hall, 0.3f);
        L.add (D, "Mono", "Theremin", "theremin sci-fi 50s", s);
    }
    {
        auto s = lead (5, 0, 7, 6000, 0.1f, 0.5f, 0.02f); auto& a = s.layer[0].analog; a.uniDetune = 24;
        echo (s, 3, 0.5f, 0.3f, 6000); space (s, Space::Hall, 0.3f);
        L.add (D, "Digital", "Trance Lead", "trance anthem lead", s);
    }
    {
        auto s = lead (0, -1, 0, 450, 0.82f, 3.5f, 0.05f); auto& a = s.layer[0].analog; a.fD = 0.2f; a.fS = 0.0f; a.fDrive = 0.6f;
        a.fVel = 2.0f;
        L.add (D, "Acid", "Acid Lead", "acid 303 squelch resonant", s);
    }
    {
        auto s = lead (1, 1, 20, 2000, 0.2f, 1.0f, 0.04f); auto& a = s.layer[0].analog; a.unison = 4; a.uniDetune = 35; a.pEnv = -10; a.pDecay = 0.18f;
        a.o1PW = 0.3f; mod (s.layer[0], 1, ModFx::Chorus, 0.6f, 0.5f, 0.5f);
        L.add (D, "Digital", "Hoover Lead", "hoover rave mentasm 90s", s);
    }
    {
        auto s = lead (1, 1, 6, 4000, 0.15f, 0.5f, 0.02f); echo (s, 3, 0.45f, 0.25f);
        L.add (D, "Mono", "80s Square Lead", "80s square lead delay", s);
    }
    {
        auto s = lead (2, 1, 0, 3500, 0.1f, 0.3f, 0.0f); s.layer[0].analog.o2Level = 0.25f; mod (s.layer[0], 1, ModFx::Rotary, 6.7f, 0.5f, 0.8f);
        L.add (D, "Mono", "Lead Organ", "organ lead rotary", s);
    }
    {
        auto s = lead (0, 1, 4, 1800, 0.35f, 1.8f, 0.09f); s.layer[0].analog.o2Oct = -1;
        L.add (D, "Mono", "Lucky Lead", "emerson lucky man portamento prog", s);
    }
    {
        auto s = lead (1, 0, 0, 2800, 0.3f, 1.5f, 0.02f); auto& a = s.layer[0].analog; a.unison = 2; a.uniDetune = 12; a.l1Pitch = 0.0f;
        s.layer[0].common.poly = 8; s.layer[0].common.legato = 0;
        L.add (D, "Poly", "Poly Lead", "polyphonic lead chords", s);
    }
}

void addSynthBass (Lib& L)
{
    const char* S = kSynthBass;

    {
        auto s = bass (0, 1, 0, 400, 0.3f, 3.0f, 0.4f, 0.8f); s.layer[0].analog.o2Oct = -1; s.layer[0].analog.fDrive = 0.4f;
        L.add (S, "Analog", "Moog Bass", "minimoog bass classic fat", s);
    }
    {
        auto s = bass (3, -1, 0, 20000, 0.0f, 0.0f, 1.0f, 1.0f); s.layer[0].analog.aR = 0.12f; s.layer[0].common.level = -5.0f;
        L.add (S, "Sub", "Sub Bass", "sub sine deep 808", s);
    }
    {
        auto s = bass (0, -1, 0, 280, 0.85f, 4.0f, 0.4f, 0.6f); mono (s, 0.05f, true); auto& a = s.layer[0].analog;
        a.fD = 0.18f; a.fS = 0.0f; a.fDrive = 0.5f; a.fVel = 2.5f;
        L.add (S, "Acid", "Acid Bass", "303 acid squelch techno", s);
    }
    {
        auto s = bass (0, 0, 25, 700, 0.1f, 0.5f, 1.0f, 1.0f); auto& a = s.layer[0].analog; a.unison = 2; a.uniDetune = 18; a.uniSpread = 0.3f;
        L.add (S, "Analog", "Reese Bass", "reese dnb jungle detuned", s);
    }
    {
        auto s = fmPair (1.0f, 1.0f, 0.82f, 1.2f, 0.35f, 0.4f, 0.25f, 0.35f, 0.08f); mono (s, 0.0f, false);
        L.add (S, "FM", "Lately FM Bass", "fm bass dx lately 80s", s);
    }
    {
        auto s = fmPair (1.0f, 2.0f, 0.72f, 0.8f, 0.25f, 0.5f, 0.15f, 0.2f, 0.08f); mono (s, 0.0f, false);
        auto& f = s.layer[0].fm; op (f, 2, 0.5f, 0.75f, 0.002f, 1.0f, 0.6f, 0.08f, 0.2f);
        L.add (S, "FM", "Solid FM Bass", "fm solid bass dx", s);
    }
    {
        auto s = bass (0, -1, 0, 250, 0.3f, 4.5f, 0.6f, 0.0f); s.layer[0].analog.fD = 0.25f;
        L.add (S, "Analog", "Pluck Bass", "pluck bass short", s);
    }
    {
        auto s = bass (1, -1, 0, 900, 0.2f, 1.5f, 0.6f, 0.7f); s.layer[0].analog.subLevel = 0.6f;
        L.add (S, "Analog", "Square Bass", "square sub bass", s);
    }
    {
        auto s = bass (0, 1, 0, 300, 0.45f, 0.5f, 1.0f, 1.0f); auto& a = s.layer[0].analog; a.l1Cut = 3.0f; a.l1Rate = 3.0f; a.l1Shape = 1;
        L.add (S, "Dubstep", "Wobble Bass", "wobble dubstep lfo", s);
    }
    {
        auto s = bass (1, -1, 0, 450, 0.5f, 3.0f, 0.5f, 0.5f); auto& a = s.layer[0].analog; a.o1PW = 0.3f; a.fD = 0.2f;
        L.add (S, "Analog", "Funk Synth Bass", "funk synth bass envelope", s);
    }
    {
        auto s = bass (1, -1, 0, 800, 0.2f, 2.0f, 0.5f, 0.7f); auto& a = s.layer[0].analog; a.subLevel = 0.6f; a.o1PW = 0.4f;
        L.add (S, "Analog", "Juno Bass", "juno 106 bass sub pulse", s);
    }
    {
        auto s = bass (3, -1, 0, 20000, 0.0f, 0.0f, 2.2f, 0.0f); auto& a = s.layer[0].analog; a.pEnv = 7; a.pDecay = 0.05f; a.aR = 0.4f;
        amp (s.layer[0], Tube, 0.3f);
        L.add (S, "Sub", "808 Sub Boom", "808 trap hip hop boom", s);
    }
    {
        auto s = bass (1, 1, 30, 1500, 0.2f, 1.0f, 1.0f, 1.0f); auto& a = s.layer[0].analog; a.unison = 3; a.uniDetune = 40; a.o1Oct = -1; a.o2Oct = -1;
        L.add (S, "Digital", "Hoover Bass", "hoover rave bass", s);
    }
    {
        auto s = bass (0, -1, 0, 600, 0.25f, 2.5f, 0.3f, 0.4f);
        s.layer[0].common.arpMode = 1; s.layer[0].common.arpRate = 3; s.layer[0].common.arpOct = 1; s.layer[0].common.arpGate = 0.5f;
        echo (s, 3, 0.25f, 0.1f);
        L.add (S, "Sequence", "Synthwave Bass Arp", "synthwave outrun 16th bass arp", s);
    }
    {
        auto s = fmPair (1.0f, 1.0f, 0.6f, 0.2f, 0.12f, 0.8f, 0.5f, 0.75f, 0.06f); mono (s, 0.0f, false);
        auto& f = s.layer[0].fm; op (f, 2, 2.0f, 0.7f, 0.002f, 0.25f, 0.6f, 0.06f, 0.2f);
        L.add (S, "FM", "House Organ Bass", "m1 organ bass house 90s", s);
    }
    {
        auto s = bass (1, 1, 0, 700, 0.4f, 3.0f, 0.25f, 0.3f); s.layer[0].analog.fDrive = 0.8f; amp (s.layer[0], Tube, 0.5f);
        L.add (S, "Analog", "Electro Bass", "electro funk 80s", s);
    }
    {
        auto s = bass (3, 2, 0, 3000, 0.0f, 0.0f, 0.5f, 0.6f); auto& a = s.layer[0].analog; a.pEnv = 3; a.pDecay = 0.04f;
        L.add (S, "Analog", "Rubber Bass", "rubber boing bass", s);
    }
    {
        auto s = bass (0, 0, 7, 600, 0.2f, 1.0f, 2.0f, 1.0f); s.layer[0].analog.aR = 0.3f;
        L.add (S, "Analog", "Analog Long Bass", "sustained bass legato", s);
    }
    {
        auto s = bass (1, 0, 0, 500, 0.3f, 2.0f, 0.4f, 0.5f); auto& a = s.layer[0].analog; a.sync = 1; a.o2Semi = 9;
        L.add (S, "Analog", "Detroit Sync Bass", "detroit techno sync bass", s);
    }
    {
        auto s = bass (0, 0, 10, 800, 0.4f, 0.5f, 1.0f, 1.0f); auto& a = s.layer[0].analog; a.fType = 6; a.vowel = 1.0f; a.l2Cut = 1.0f; a.l2Rate = 1.5f;
        s.layer[0].common.level = -5.0f;
        L.add (S, "Dubstep", "Growl Bass", "growl formant bass", s);
    }
    {
        auto s = bass (0, 0, 5, 500, 0.3f, 2.0f, 0.8f, 0.8f); auto& a = s.layer[0].analog; a.o3Level = 0.7f; a.o3Wave = 1; a.o3Oct = -1;
        L.add (S, "Analog", "Fat Three-Osc Bass", "three oscillator fat moog", s);
    }
    {
        auto s = bass (0, -1, 0, 900, 0.2f, 3.0f, 0.2f, 0.0f);
        L.add (S, "Analog", "Short Saw Bass", "short staccato saw", s);
    }
    {
        auto s = fmPair (1.0f, 3.0f, 0.65f, 0.6f, 0.08f, 0.0f, 0.0f, 0.2f, 0.08f); mono (s, 0.0f, false);
        auto& f = s.layer[0].fm; op (f, 2, 1.0f, 0.8f, 0.002f, 0.9f, 0.0f, 0.1f, 0.3f); op (f, 3, 1.0f, 0.5f, 0.002f, 0.4f, 0.0f, 0.1f, 0.5f);
        L.add (S, "FM", "Wood FM Bass", "fm wood bass plucky", s);
    }
    {
        auto s = fmPair (1.0f, 5.0f, 0.75f, 0.9f, 0.06f, 0.2f, 0.0f, 0.4f, 0.08f); mono (s, 0.0f, false);
        L.add (S, "FM", "Slap FM Bass", "fm slap bass dx", s);
    }
    {
        auto s = bass (3, 0, 0, 350, 0.15f, 1.0f, 0.6f, 0.5f); s.layer[0].analog.o2Level = 0.25f;
        L.add (S, "Analog", "Deep House Bass", "deep house warm round", s);
    }
    {
        auto s = bass (0, 0, 22, 650, 0.15f, 0.8f, 1.0f, 1.0f); auto& a = s.layer[0].analog; a.unison = 2; a.uniDetune = 22;
        amp (s.layer[0], Overdrive, 0.4f, 0.0f, CabOff, -2.0f); a.l2Cut = 0.8f; a.l2Rate = 0.25f;
        L.add (S, "Analog", "DnB Reese Drive", "drum and bass reese distorted", s);
    }
    {
        auto s = bass (0, -1, 0, 350, 0.7f, 4.0f, 0.3f, 0.3f); s.layer[0].analog.fDrive = 0.7f;
        L.add (S, "Acid", "Ladder Squelch", "squelch resonant ladder", s);
    }
    {
        auto s = bass (0, 0, 4, 300, 0.25f, 1.5f, 2.5f, 0.0f); auto& a = s.layer[0].analog; a.fD = 2.0f; a.o2Oct = -1;
        L.add (S, "Analog", "Taurus Pedal Bass", "taurus pedals rush genesis huge", s);
    }
    {
        auto s = bass (1, -1, 0, 700, 0.3f, 2.5f, 0.25f, 0.3f);
        s.layer[0].common.arpMode = 1; s.layer[0].common.arpRate = 3; s.layer[0].common.arpOct = 2; s.layer[0].common.arpGate = 0.45f;
        L.add (S, "Sequence", "Berlin Bass Sequence", "berlin school tangerine dream sequencer", s);
    }
}
} // namespace kv::banks
