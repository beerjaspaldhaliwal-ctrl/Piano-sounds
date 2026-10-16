// Synth keys: polysynth keyboards (Juno / Prophet / OB / Jupiter / CS style),
// digital FM keys, house & stab keys, vocal, chip and lo-fi keys.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

static SoundData polyKeys (int w1, int w2, float det, float cut, float res, float env, float dec, float sus, float rel)
{
    auto s = analogPatch (w1, w2, det, cut, res, env, 0.003f, dec, sus, rel, 0.001f, dec * 0.8f, sus * 0.5f, rel);
    s.layer[0].common.poly = 12;
    s.layer[0].analog.velAmp = 0.6f;
    s.layer[0].analog.fVel = 1.5f;
    space (s, Space::Room, 0.14f);
    return s;
}

void addSynthKeys (Lib& L)
{
    const char* K = kSynthKeys;

    {
        auto s = polyKeys (1, -1, 0, 1400, 0.25f, 2.5f, 1.2f, 0.55f, 0.4f);
        auto& a = s.layer[0].analog; a.subLevel = 0.45f; a.l1Pw = 0.18f; a.l1Rate = 0.7f; a.o1PW = 0.45f;
        mod (s.layer[0], 1, ModFx::Chorus, 0.5f, 0.5f, 0.5f);
        L.add (K, "Analog", "Juno Keys", "juno 106 pwm chorus 80s", s);
    }
    {
        auto s = polyKeys (0, 0, 9, 1800, 0.3f, 2.8f, 1.4f, 0.5f, 0.45f);
        L.add (K, "Analog", "Prophet Keys", "prophet 5 sequential warm poly", s);
    }
    {
        auto s = polyKeys (0, 1, 6, 2200, 0.2f, 2.0f, 1.0f, 0.65f, 0.4f);
        auto& a = s.layer[0].analog; a.fType = 1; a.o2PW = 0.35f;
        mod (s.layer[0], 1, ModFx::Dimension, 0.4f, 0.5f, 0.5f);
        L.add (K, "Analog", "OB Poly Keys", "oberheim ob-x ob-8 sem", s);
    }
    {
        auto s = polyKeys (0, 1, 5, 2600, 0.15f, 2.2f, 1.5f, 0.6f, 0.5f);
        auto& a = s.layer[0].analog; a.subLevel = 0.3f; a.fType = 1;
        mod (s.layer[0], 1, ModFx::Ensemble, 0.6f, 0.5f, 0.45f);
        L.add (K, "Analog", "Jupiter Keys", "jupiter 8 roland ensemble", s);
    }
    {
        auto s = polyKeys (0, -1, 0, 1600, 0.35f, 3.0f, 0.9f, 0.4f, 0.4f);
        mod (s.layer[0], 1, ModFx::Ensemble, 0.5f, 0.6f, 0.5f);
        L.add (K, "Analog", "Polysix Keys", "korg polysix ensemble", s);
    }
    {
        auto s = polyKeys (0, 0, 12, 900, 0.4f, 3.5f, 0.6f, 0.3f, 0.6f);
        auto& a = s.layer[0].analog; a.fVel = 3.0f; a.atCut = 1.5f;
        space (s, Space::Hall, 0.22f);
        L.add (K, "Analog", "CS Keys", "cs80 yamaha expressive vangelis", s);
    }
    {
        auto s = polyKeys (0, 0, 7, 600, 0.2f, 4.5f, 0.9f, 0.0f, 0.5f);
        L.add (K, "Analog", "Synth Piano", "saw piano percussive analog", s);
    }
    {
        auto s = polyKeys (1, 1, 0, 5000, 0.0f, 0.0f, 0.8f, 0.2f, 0.25f);
        auto& a = s.layer[0].analog; a.o2Oct = 1; a.o2Level = 0.35f; a.fType = 1;
        L.add (K, "Analog", "Square Keys", "square clean simple", s);
    }
    {
        auto s = polyKeys (1, -1, 0, 2500, 0.3f, 1.5f, 1.2f, 0.5f, 0.4f);
        s.layer[0].analog.o1PW = 0.16f; s.layer[0].analog.fType = 1;
        L.add (K, "Analog", "Hollow Keys", "pulse narrow hollow", s);
    }
    {
        auto s = polyKeys (0, 0, 14, 700, 0.35f, 4.0f, 0.35f, 0.0f, 0.3f);
        space (s, Space::Hall, 0.3f);
        L.add (K, "Stab", "90s Stab Keys", "rave stab house 90s", s);
    }
    {
        auto s = polyKeys (5, -1, 0, 1200, 0.2f, 3.5f, 0.8f, 0.25f, 0.4f);
        s.layer[0].analog.uniDetune = 22; s.layer[0].analog.uniSpread = 0.9f;
        space (s, Space::Hall, 0.25f); echo (s, 6, 0.3f, 0.15f);
        L.add (K, "Stab", "Supersaw Keys", "supersaw trance jp8000 edm", s);
    }
    {
        auto s = polyKeys (5, -1, 0, 900, 0.3f, 4.0f, 0.25f, 0.0f, 0.35f);
        s.layer[0].analog.uniDetune = 28; space (s, Space::Hall, 0.25f); echo (s, 6, 0.35f, 0.2f);
        L.add (K, "Stab", "Trance Stab", "trance stab supersaw edm", s);
    }
    {
        auto s = polyKeys (0, 1, 7, 1500, 0.45f, 1.0f, 2.0f, 0.7f, 0.6f);
        auto& a = s.layer[0].analog; a.l2Cut = 1.2f; a.l2Rate = 0.35f;
        L.add (K, "Analog", "Wavy Keys", "lfo filter sweep moving", s);
    }
    {
        auto s = polyKeys (0, 0, 0, 1200, 0.5f, 3.0f, 1.0f, 0.4f, 0.4f);
        auto& a = s.layer[0].analog; a.sync = 1; a.o2Semi = 7; a.o2Level = 0.8f; a.o1Level = 0.3f; a.pEnv = 0;
        L.add (K, "Analog", "Sync Keys", "hard sync bright", s);
    }
    {
        auto s = polyKeys (2, 3, 0, 4000, 0.0f, 0.5f, 1.6f, 0.0f, 0.5f);
        auto& a = s.layer[0].analog; a.o2Oct = 1; a.o2Level = 0.3f;
        mod (s.layer[0], 1, ModFx::AutoPan, 3.0f, 0.3f, 1.0f); mod (s.layer[0], 2, ModFx::Chorus, 0.5f, 0.3f, 0.3f);
        L.add (K, "Analog", "Analog EP", "analog electric piano soft sine triangle", s);
    }
    {
        auto s = polyKeys (1, -1, 0, 800, 0.6f, 4.0f, 0.3f, 0.0f, 0.15f);
        s.layer[0].analog.o1PW = 0.12f;
        L.add (K, "Funk", "Synth Clavi", "clav synth funk pulse", s);
    }
    {
        auto s = polyKeys (0, 1, 3, 500, 0.7f, 4.5f, 0.4f, 0.1f, 0.2f);
        s.layer[0].analog.fDrive = 0.6f;
        L.add (K, "Funk", "Funk Synth Keys", "funk resonant envelope", s);
    }
    {
        auto s = polyKeys (0, 1, 8, 1600, 0.2f, 1.8f, 1.2f, 0.5f, 0.5f);
        lofi (s, 10, 18, 0.3f, 0.35f, 5000);
        L.add (K, "Lo-Fi", "Lo-Fi Synth Keys", "lofi tape detuned", s);
    }
    {
        auto s = polyKeys (0, 0, 9, 1200, 0.4f, 1.0f, 1.0f, 0.7f, 0.5f);
        auto& a = s.layer[0].analog; a.fType = 6; a.vowel = 0.5f; a.l2Cut = 0; a.cutoff = 1500;
        mod (s.layer[0], 1, ModFx::Chorus, 0.4f, 0.4f, 0.4f);
        L.add (K, "Vocal", "Vocal Keys", "formant vowel voice keys", s);
    }
    {
        auto s = polyKeys (0, 1, 6, 2000, 0.2f, 2.0f, 1.0f, 0.5f, 0.4f);
        lofi (s, 6, 11, 0.0f, 0.0f, 9000);
        L.add (K, "Lo-Fi", "Bitcrushed Keys", "bitcrush 8bit digital", s);
    }
    {
        auto s = polyKeys (0, 3, 5, 1800, 0.15f, 1.5f, 2.5f, 0.4f, 1.2f);
        mod (s.layer[0], 1, ModFx::Chorus, 0.4f, 0.5f, 0.45f); echo (s, 3, 0.45f, 0.25f, 3000); space (s, Space::Ambient, 0.35f);
        L.add (K, "Ambient", "Dream Keys", "dreamy ambient delay", s);
    }
    {
        auto s = polyKeys (1, 1, 0, 6000, 0.0f, 0.0f, 0.4f, 0.4f, 0.1f);
        auto& a = s.layer[0].analog; a.o2Oct = 1; a.o2Level = 0.25f; a.fType = 1; a.o1PW = 0.25f;
        L.add (K, "Chip", "Retro Computer Keys", "chiptune 8bit game square", s);
    }
    {
        auto s = polyKeys (2, 2, 0, 3000, 0.0f, 0.5f, 0.05f, 1.0f, 0.08f);
        auto& a = s.layer[0].analog; a.o2Oct = -1; a.o2Level = 0.5f; a.subLevel = 0.2f; a.aA = 0.004f;
        mod (s.layer[0], 1, ModFx::Rotary, 0.8f, 0.5f, 0.8f);
        L.add (K, "Analog", "Organ Synth Keys", "synth organ triangle", s);
    }
    {
        auto s = polyKeys (3, 3, 0, 8000, 0.0f, 0.0f, 2.2f, 0.0f, 0.6f);
        auto& a = s.layer[0].analog; a.o2Oct = 1; a.o2Level = 0.18f; space (s, Space::Hall, 0.25f);
        L.add (K, "Analog", "Sine Keys", "sine pure soft", s);
    }
    {
        auto s = polyKeys (0, 0, 15, 1400, 0.2f, 2.0f, 1.0f, 0.6f, 0.8f);
        lofi (s, 24, 48, 0.1f, 0.45f, 7000); space (s, Space::Plate, 0.25f);
        L.add (K, "Lo-Fi", "Vapor Keys", "vaporwave detuned tape wobble", s);
    }
    {
        auto s = polyKeys (0, 1, 7, 900, 0.35f, 3.2f, 0.7f, 0.2f, 0.5f);
        mod (s.layer[0], 1, ModFx::Chorus, 0.3f, 0.4f, 0.4f); echo (s, 6, 0.4f, 0.22f); space (s, Space::Hall, 0.25f);
        L.add (K, "Analog", "Synthwave Keys", "synthwave retrowave 80s outrun", s);
    }

    //--------------------------------------------------------------- FM keys
    {
        auto s = fmPair (1.0f, 3.5f, 0.68f, 2.2f, 0.9f, 0.0f, 0.0f, 0.1f, 0.6f);
        space (s, Space::Hall, 0.22f);
        L.add (K, "FM", "Digital Bell Keys", "fm bell digital 80s", s);
    }
    {
        auto s = fmPair (1.0f, 1.0f, 0.82f, 1.2f, 0.4f, 0.3f, 0.2f, 0.6f, 0.3f);
        L.add (K, "FM", "FM Clav Keys", "fm clav dx funk", s);
    }
    {
        auto s = fmPair (1.0f, 2.0f, 0.72f, 3.0f, 1.2f, 0.0f, 0.0f, 0.2f, 0.6f);
        auto& f = s.layer[0].fm; op (f, 2, 2.0f, 0.6f, 0.002f, 1.5f, 0.0f, 0.5f, 0.5f); op (f, 3, 5.0f, 0.5f, 0.001f, 0.4f, 0.0f, 0.3f, 0.8f);
        mod (s.layer[0], 1, ModFx::Chorus, 0.5f, 0.4f, 0.35f); space (s, Space::Hall, 0.25f);
        L.add (K, "FM", "Glass Keys", "fm glass crystal", s);
    }
    {
        auto s = fmPair (1.0f, 1.0f, 0.6f, 0.15f, 0.12f, 0.6f, 0.4f, 0.75f, 0.12f);
        auto& f = s.layer[0].fm; op (f, 2, 2.0f, 0.75f, 0.002f, 0.3f, 0.5f, 0.12f, 0.2f); op (f, 4, 3.0f, 0.55f, 0.002f, 0.2f, 0.3f, 0.1f, 0.2f);
        L.add (K, "House", "House Organ Keys", "m1 organ house 90s", s);
    }
    {
        auto s = dxEP (0.7f, 0.85f, 0.72f, 4.0f); auto& l = s.layer[0]; l.fx.compAmt = 0.4f; eq (l, 2.0f, 0.0f, 1000.0f, 3.0f);
        space (s, Space::Hall, 0.18f);
        L.add (K, "House", "House Piano Keys", "house piano 90s m1 korg", s);
    }
    {
        auto s = fmPair (1.0f, 7.0f, 0.62f, 1.6f, 0.5f, 0.0f, 0.0f, 0.15f, 0.5f);
        auto& f = s.layer[0].fm; op (f, 2, 1.0f, 0.8f, 0.002f, 2.4f, 0.1f, 0.6f, 0.3f, 0.0f, 6.0f); op (f, 3, 1.0f, 0.62f, 0.002f, 1.2f, 0.0f, 0.3f, 0.6f);
        mod (s.layer[0], 1, ModFx::Chorus, 0.5f, 0.3f, 0.3f);
        L.add (K, "FM", "Tine Synth", "fm tine synth bright", s);
    }
    {
        auto s = fmPair (2.0f, 1.0f, 0.7f, 1.2f, 0.6f, 0.2f, 0.1f, 0.4f, 0.3f);
        lofi (s, 8, 16, 0.0f, 0.0f, 8000);
        L.add (K, "FM", "Lo-Fi FM Keys", "fm digital crunchy 8bit", s);
    }
    {
        auto s = fmPair (1.0f, 0.5f, 0.75f, 1.8f, 1.2f, 0.3f, 0.2f, 0.3f, 0.5f);
        mod (s.layer[0], 1, ModFx::Ensemble, 0.5f, 0.4f, 0.4f); space (s, Space::Hall, 0.25f);
        L.add (K, "FM", "Noir Keys", "dark fm moody", s);
    }
}
} // namespace kv::banks
