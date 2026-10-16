// Synth pads: analog, PWM, supersaw, choir/vocal, FM glass & bell pads,
// evolving, ambient and lo-fi pads.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

static SoundData pad (int w1, int w2, float det, int uni, float cut, float res, float env, float atk, float rel)
{
    auto s = analogPatch (w1, w2, det, cut, res, env, atk, 1.5f, 0.85f, rel, atk * 1.5f, 2.5f, 0.5f, rel);
    auto& a = s.layer[0].analog;
    a.unison = (float) uni; a.uniDetune = 14.0f; a.uniSpread = 0.8f; a.velAmp = 0.3f; a.drift = 0.4f; a.fVel = 0.5f;
    s.layer[0].common.poly = 12;
    space (s, Space::Hall, 0.35f);
    return s;
}

static SoundData fmPad (float modRatio, float modLevel, float atk, float rel, float bellDecay)
{
    auto s = fmSynth();
    auto& l = s.layer[0];
    l.common.poly = 12;
    auto& f = l.fm;
    f.alg = 2; f.feedback = 0.1f;
    op (f, 0, 1.0f, 1.0f, atk, 2.0f, 0.85f, rel, 0.2f);
    op (f, 1, modRatio, modLevel, atk * 0.7f, bellDecay, 0.45f, rel, 0.5f);
    op (f, 2, 1.0f, 0.85f, atk * 1.4f, 2.0f, 0.9f, rel, 0.1f, 0.0f, 7.0f);
    op (f, 3, 1.0f, 0.55f, atk, 3.0f, 0.6f, rel, 0.3f);
    op (f, 4, 2.0f, 0.6f, atk * 2.0f, 2.0f, 0.7f, rel, 0.1f, 0.0f, -6.0f);
    op (f, 5, 1.0f, 0.4f, atk, 2.0f, 0.5f, rel, 0.3f);
    f.lfoRate = 0.3f; f.lfoAmp = 0.1f;
    mod (l, 1, ModFx::Chorus, 0.35f, 0.5f, 0.45f);
    space (s, Space::Hall, 0.4f);
    return s;
}

void addPads (Lib& L)
{
    const char* P = kPads;

    {
        auto s = pad (0, 0, 8, 2, 1600, 0.15f, 1.2f, 0.8f, 1.8f); auto& a = s.layer[0].analog; a.l2Cut = 0.4f; a.l2Rate = 0.2f;
        mod (s.layer[0], 1, ModFx::Chorus, 0.4f, 0.4f, 0.35f);
        L.add (P, "Analog", "Warm Analog Pad", "warm analog lush", s);
    }
    {
        auto s = pad (0, 0, 6, 1, 2500, 0.1f, 1.0f, 0.5f, 1.5f); s.layer[0].analog.fType = 1;
        mod (s.layer[0], 1, ModFx::Ensemble, 0.5f, 0.6f, 0.5f);
        L.add (P, "Analog", "OB Pad", "oberheim ob-x pad 80s", s);
    }
    {
        auto s = pad (0, 1, 5, 2, 2200, 0.15f, 1.0f, 0.6f, 1.6f); s.layer[0].analog.fType = 1;
        mod (s.layer[0], 1, ModFx::Ensemble, 0.6f, 0.5f, 0.45f);
        L.add (P, "Analog", "Jupiter Pad", "jupiter 8 pad", s);
    }
    {
        auto s = pad (1, -1, 0, 1, 1800, 0.2f, 1.0f, 0.5f, 1.5f); auto& a = s.layer[0].analog;
        a.l1Pw = 0.25f; a.l1Rate = 0.5f; a.subLevel = 0.3f; mod (s.layer[0], 1, ModFx::Chorus, 0.5f, 0.55f, 0.5f);
        L.add (P, "Analog", "Juno Pad", "juno pwm chorus pad", s);
    }
    {
        auto s = pad (0, 0, 10, 1, 1400, 0.25f, 2.0f, 0.6f, 1.6f);
        L.add (P, "Analog", "Prophet Pad", "prophet 5 pad warm", s);
    }
    {
        auto s = pad (0, 0, 7, 2, 450, 0.5f, 0.5f, 0.8f, 2.0f); auto& a = s.layer[0].analog; a.l2Cut = 2.6f; a.l2Rate = 0.07f;
        L.add (P, "Analog", "Filter Sweep Pad", "sweep filter lfo slow", s);
    }
    {
        auto s = pad (1, 1, 6, 1, 2000, 0.15f, 0.8f, 0.6f, 1.6f); auto& a = s.layer[0].analog;
        a.l1Pw = 0.3f; a.l1Rate = 0.4f; a.l2Pw = 0.2f; a.l2Rate = 0.13f;
        L.add (P, "Analog", "PWM Pad", "pwm pulse width modulation", s);
    }
    {
        auto s = pad (5, -1, 0, 7, 3000, 0.1f, 0.5f, 0.7f, 2.0f); s.layer[0].analog.uniDetune = 26;
        mod (s.layer[0], 1, ModFx::Chorus, 0.3f, 0.3f, 0.3f);
        L.add (P, "Digital", "Supersaw Pad", "supersaw trance pad wide", s);
    }
    {
        auto s = pad (0, 0, 6, 2, 420, 0.2f, 0.6f, 1.0f, 2.5f); auto& a = s.layer[0].analog; a.o2Oct = -1;
        eq (s.layer[0], 3.0f, 0.0f, 1000.0f, -4.0f);
        L.add (P, "Dark", "Dark Pad", "dark low moody", s);
    }
    {
        auto s = pad (0, 0, 9, 3, 600, 0.25f, 0.8f, 3.0f, 6.0f); s.layer[0].analog.l2Cut = 1.2f; s.layer[0].analog.l2Rate = 0.05f;
        echo (s, 9, 0.6f, 0.3f, 2500); space (s, Space::Ambient, 0.6f);
        L.add (P, "Ambient", "Ambient Drone", "drone ambient cinematic", s);
    }
    {
        auto s = pad (0, 0, 6, 3, 1500, 0.3f, 0.0f, 0.6f, 2.0f); auto& a = s.layer[0].analog; a.fType = 6; a.vowel = 0.0f;
        mod (s.layer[0], 1, ModFx::Ensemble, 0.4f, 0.5f, 0.45f);
        L.add (P, "Choir", "Choir Pad Aah", "choir voices aah vocal", s);
    }
    {
        auto s = pad (0, 0, 6, 3, 1500, 0.3f, 0.0f, 0.8f, 2.2f); auto& a = s.layer[0].analog; a.fType = 6; a.vowel = 4.0f;
        mod (s.layer[0], 1, ModFx::Ensemble, 0.4f, 0.5f, 0.45f);
        L.add (P, "Choir", "Choir Pad Ooh", "choir ooh soft vocal", s);
    }
    {
        auto s = pad (1, 0, 8, 2, 1500, 0.35f, 0.0f, 0.4f, 1.6f); auto& a = s.layer[0].analog; a.fType = 6; a.vowel = 1.6f;
        a.noiseLevel = 0.08f;
        L.add (P, "Choir", "Vox Pad", "vox vocal synth pad", s);
    }
    L.add (P, "FM", "Glass Pad", "fm glass crystal pad", fmPad (3.0f, 0.55f, 0.8f, 2.0f, 3.0f));
    L.add (P, "FM", "Bell Pad", "fm bell pad shimmer", fmPad (3.5f, 0.75f, 0.01f, 2.5f, 1.2f));
    L.add (P, "FM", "Digital Pad", "fm digital 80s dx pad", fmPad (2.0f, 0.62f, 0.4f, 1.6f, 2.0f));
    {
        auto s = pad (5, 0, 0, 7, 3500, 0.1f, 0.3f, 1.2f, 3.0f); auto& a = s.layer[0].analog; a.o2Oct = 1; a.o2Level = 0.35f; a.uniDetune = 20;
        echo (s, 3, 0.55f, 0.3f, 6000); space (s, Space::Ambient, 0.55f);
        L.add (P, "Ambient", "Shimmer Pad", "shimmer bright ambient octave", s);
    }
    {
        auto s = pad (0, 1, 7, 2, 900, 0.45f, 0.5f, 0.8f, 2.0f); auto& a = s.layer[0].analog;
        a.l2Shape = 5; a.l2Rate = 2.0f; a.l2Cut = 1.0f;
        L.add (P, "Ambient", "Evolving Pad", "sample hold evolving movement", s);
    }
    {
        auto s = pad (0, -1, 0, 1, 1800, 0.3f, 0.0f, 1.0f, 2.0f); auto& a = s.layer[0].analog;
        a.noiseLevel = 0.45f; a.o1Level = 0.35f; a.fType = 6; a.vowel = 2.0f;
        L.add (P, "Ambient", "Breath Pad", "breath air noise", s);
    }
    {
        auto s = pad (3, 3, 0, 1, 8000, 0.0f, 0.0f, 1.0f, 2.0f); auto& a = s.layer[0].analog; a.o2Oct = 1; a.o2Level = 0.25f;
        mod (s.layer[0], 1, ModFx::Chorus, 0.3f, 0.5f, 0.4f);
        L.add (P, "Analog", "Soft Sine Pad", "sine soft gentle", s);
    }
    {
        auto s = pad (0, 0, 7, 2, 1800, 0.15f, 0.8f, 0.6f, 2.2f); s.layer[0].analog.fType = 1;
        mod (s.layer[0], 1, ModFx::Ensemble, 0.4f, 0.6f, 0.5f); echo (s, 3, 0.4f, 0.18f); space (s, Space::Hall, 0.45f);
        L.add (P, "Analog", "80s Movie Pad", "80s film score pad vangelis", s);
    }
    {
        auto s = pad (0, 0, 9, 2, 1200, 0.25f, 1.5f, 0.4f, 1.6f);
        mod (s.layer[0], 1, ModFx::Chorus, 0.4f, 0.5f, 0.45f); echo (s, 3, 0.45f, 0.22f);
        L.add (P, "Analog", "Synthwave Pad", "synthwave outrun retro 80s", s);
    }
    {
        auto s = pad (0, 1, 10, 2, 1500, 0.2f, 0.5f, 0.6f, 1.8f); lofi (s, 12, 20, 0.3f, 0.4f, 5000);
        L.add (P, "Lo-Fi", "Lo-Fi Pad", "lofi tape wobble chill", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.32f, 4.25f, 6.63f }, { 0, -8, -14, -20 }, { 1.0f, 0.8f, 0.6f, 0.4f }, 10.0f, 0.5f);
        auto& m = s.layer[0].mallet; m.excite = 2; m.bowAttack = 1.0f; m.beat = 0.5f; m.beatMix = 0.4f;
        space (s, Space::Ambient, 0.5f);
        L.add (P, "Ambient", "Bowed Glass Pad", "bowed glass harmonica pad", s);
    }
    {
        auto s = pad (1, 2, 0, 1, 2400, 0.2f, 0.4f, 0.6f, 1.6f); s.layer[0].analog.o1PW = 0.15f;
        L.add (P, "Analog", "Hollow Pad", "hollow pulse thin", s);
    }
    {
        auto s = pad (0, 0, 0, 8, 1600, 0.15f, 0.4f, 1.5f, 3.0f); auto& a = s.layer[0].analog; a.uniDetune = 42; a.uniSpread = 1.0f;
        a.l2Cut = 0.6f; a.l2Rate = 0.1f; space (s, Space::Ambient, 0.5f);
        L.add (P, "Ambient", "Detuned Cloud", "detuned unison wide cloud", s);
    }
    {
        auto s = pad (0, 1, 6, 1, 2000, 0.2f, 1.0f, 0.3f, 1.2f);
        L.add (P, "Analog", "Polysynth Pad", "polysynth classic", s);
    }
    {
        auto s = pad (3, 0, 7, 2, 6000, 0.1f, 0.0f, 1.0f, 2.5f); auto& a = s.layer[0].analog; a.hpCut = 300; a.noiseLevel = 0.1f; a.o2Level = 0.35f;
        space (s, Space::Hall, 0.5f);
        L.add (P, "Ambient", "Arctic Pad", "cold icy airy thin", s);
    }
    {
        auto s = pad (0, 0, 8, 2, 500, 0.7f, 1.0f, 0.8f, 2.0f); auto& a = s.layer[0].analog; a.l2Cut = 1.8f; a.l2Rate = 0.06f;
        L.add (P, "Analog", "Resonant Sweep Pad", "resonant filter sweep", s);
    }
    {
        auto s = pad (2, 3, 0, 1, 3000, 0.0f, 0.0f, 0.3f, 1.2f); s.layer[0].analog.subLevel = 0.3f;
        mod (s.layer[0], 1, ModFx::Rotary, 0.8f, 0.4f, 0.6f);
        L.add (P, "Analog", "Warm Organ Pad", "organ pad soft", s);
    }
    {
        auto s = pad (0, 0, 8, 2, 300, 0.3f, 0.8f, 1.5f, 3.0f); auto& a = s.layer[0].analog; a.o1Oct = -1; a.o2Oct = -1;
        a.l2Cut = 1.5f; a.l2Rate = 0.04f; space (s, Space::Ambient, 0.5f);
        L.add (P, "Dark", "Dark Matter", "dark deep cinematic drone", s);
    }
}
} // namespace kv::banks
