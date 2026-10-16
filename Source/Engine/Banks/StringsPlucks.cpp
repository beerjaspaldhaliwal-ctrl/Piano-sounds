// Strings & brass (string machines, tape-replay strings/flutes/choir, synth &
// FM brass, orchestra hit) and plucks & arpeggiated sounds.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

static SoundData strings (int fType, float cut, float atk, float rel, ModFx::Type fx, float fxMix)
{
    auto s = analogPatch (0, 0, 7, cut, 0.1f, 0.6f, atk, 1.0f, 0.9f, rel, atk, 1.5f, 0.6f, rel);
    auto& a = s.layer[0].analog;
    a.fType = (float) fType; a.unison = 2; a.uniDetune = 10; a.uniSpread = 0.7f; a.velAmp = 0.35f; a.l1Pitch = 0.04f; a.l1Rate = 5.2f; a.l1Delay = 0.4f;
    s.layer[0].common.poly = 16;
    mod (s.layer[0], 1, fx, 0.6f, 0.6f, fxMix);
    space (s, Space::Hall, 0.3f);
    return s;
}

static SoundData brass (int w2, float cut, float env, float fA, float atk)
{
    auto s = analogPatch (0, w2, 6, cut, 0.15f, env, atk, 0.6f, 0.85f, 0.3f, fA, 0.6f, 0.45f, 0.3f);
    auto& a = s.layer[0].analog;
    a.fVel = 1.5f; a.velAmp = 0.5f; a.drift = 0.35f;
    s.layer[0].common.poly = 10;
    s.layer[0].common.mwVib = 0.35f;
    space (s, Space::Hall, 0.2f);
    return s;
}

static SoundData pluckSynth (int w1, int w2, float cut, float res, float env, float dec)
{
    auto s = analogPatch (w1, w2, 7, cut, res, env, 0.001f, dec, 0.0f, dec * 0.6f, 0.001f, dec * 0.4f, 0.0f, dec * 0.4f);
    s.layer[0].common.poly = 16;
    s.layer[0].common.level = 4.0f;
    s.layer[0].analog.velAmp = 0.5f;
    space (s, Space::Hall, 0.22f);
    return s;
}

static void arp (SoundData& s, int mode, int rate, int oct, float gate, float swing = 0.0f)
{
    auto& c = s.layer[0].common;
    c.arpMode = (float) mode; c.arpRate = (float) rate; c.arpOct = (float) oct; c.arpGate = gate; c.arpSwing = swing;
}

void addStringsBrass (Lib& L)
{
    const char* S = kStrBrass;

    {
        auto s = strings (1, 6000, 0.15f, 0.8f, ModFx::Ensemble, 0.75f); auto& a = s.layer[0].analog; a.o2Oct = 1; a.o2Level = 0.5f; a.unison = 1;
        L.add (S, "String Machine", "String Machine", "solina arp string ensemble 70s", s);
    }
    L.add (S, "Analog Strings", "Analog Strings", "analog strings warm", strings (0, 2800, 0.4f, 1.2f, ModFx::Chorus, 0.4f));
    {
        auto s = strings (1, 3200, 0.3f, 1.0f, ModFx::Ensemble, 0.5f); s.layer[0].analog.o2Wave = 1; s.layer[0].analog.o2PW = 0.3f;
        L.add (S, "Analog Strings", "OB Strings", "oberheim strings 80s", s);
    }
    {
        auto s = strings (1, 3500, 0.35f, 1.4f, ModFx::Ensemble, 0.6f);
        L.add (S, "Analog Strings", "Jupiter Strings", "jupiter roland strings", s);
    }
    {
        auto s = strings (1, 5000, 0.12f, 1.0f, ModFx::Ensemble, 0.8f); auto& a = s.layer[0].analog; a.o2Oct = 1; a.o2Level = 0.4f;
        echo (s, 3, 0.3f, 0.15f);
        L.add (S, "String Machine", "80s String Machine", "string machine bright 80s", s);
    }
    {
        auto s = strings (0, 3000, 0.7f, 2.0f, ModFx::Chorus, 0.3f); s.layer[0].analog.o1Wave = 5; s.layer[0].analog.uniDetune = 18;
        space (s, Space::Hall, 0.4f);
        L.add (S, "Analog Strings", "Lush Strings", "lush wide strings supersaw", s);
    }
    {
        auto s = strings (0, 2000, 0.6f, 1.5f, ModFx::Ensemble, 0.5f); s.layer[0].analog.cutoff = 2000;
        L.add (S, "Analog Strings", "Warm Strings Section", "warm slow strings", s);
    }
    {
        auto s = pluckSynth (0, 0, 900, 0.1f, 2.5f, 0.35f); space (s, Space::Hall, 0.3f);
        L.add (S, "Analog Strings", "Synth Pizzicato", "pizzicato plucked strings", s);
    }
    {
        auto s = strings (1, 3000, 0.08f, 0.35f, ModFx::Off, 0.0f); auto& a = s.layer[0].analog; a.o2Oct = 1; a.o2Level = 0.3f; a.unison = 1; a.uniDetune = 6;
        lofi (s, 24, 48, 0.12f, 0.35f, 7000); space (s, Space::Room, 0.18f);
        L.add (S, "Tape", "Tape Strings", "mellotron strings tape strawberry fields", s);
    }
    {
        auto s = analogPatch (3, 2, 0, 3500, 0.0f, 0.0f, 0.06f, 1.0f, 0.9f, 0.3f, 0.06f, 1.0f, 0.9f, 0.3f);
        auto& a = s.layer[0].analog; a.noiseLevel = 0.12f; a.o2Level = 0.3f; s.layer[0].common.poly = 12;
        lofi (s, 24, 48, 0.12f, 0.35f, 6500); space (s, Space::Room, 0.2f);
        L.add (S, "Tape", "Tape Flutes", "mellotron flute tape", s);
    }
    {
        auto s = strings (6, 1500, 0.15f, 0.5f, ModFx::Off, 0.0f); auto& a = s.layer[0].analog; a.vowel = 0.2f; a.res = 0.35f;
        lofi (s, 24, 48, 0.12f, 0.35f, 7000);
        L.add (S, "Tape", "Tape Choir", "mellotron choir tape vocal", s);
    }
    {
        auto s = strings (6, 1500, 0.4f, 1.5f, ModFx::Ensemble, 0.6f); s.layer[0].analog.vowel = 3.0f;
        L.add (S, "Choir", "Synth Choir Strings", "choir strings ensemble", s);
    }
    L.add (S, "Synth Brass", "Synth Brass", "synth brass classic", brass (0, 900, 2.5f, 0.08f, 0.02f));
    {
        auto s = brass (0, 1200, 3.0f, 0.02f, 0.005f); s.layer[0].analog.fType = 1; s.layer[0].analog.fD = 0.35f;
        L.add (S, "Synth Brass", "Jump Brass", "jump van halen ob-xa brass stab", s);
    }
    {
        auto s = brass (0, 700, 3.0f, 0.15f, 0.08f); auto& a = s.layer[0].analog; a.fVel = 3.0f; a.atCut = 2.0f;
        space (s, Space::Hall, 0.35f);
        L.add (S, "Synth Brass", "CS Brass", "cs80 vangelis blade runner brass", s);
    }
    {
        auto s = brass (0, 1000, 3.0f, 0.01f, 0.004f); s.layer[0].analog.aD = 0.3f; s.layer[0].analog.aS = 0.0f; s.layer[0].analog.aR = 0.2f;
        s.layer[0].common.level = 4.0f;
        L.add (S, "Synth Brass", "Poly Brass Stab", "brass stab short", s);
    }
    {
        auto s = brass (0, 700, 2.0f, 0.2f, 0.15f);
        L.add (S, "Synth Brass", "Soft Brass Section", "soft brass section pad", s);
    }
    {
        auto s = fmSynth(); auto& f = s.layer[0].fm; s.layer[0].common.poly = 10; s.layer[0].common.mwVib = 0.3f;
        f.alg = 3; f.feedback = 0.45f;
        op (f, 0, 1.0f, 1.0f, 0.03f, 1.0f, 0.85f, 0.25f, 0.3f);
        op (f, 1, 1.0f, 0.78f, 0.06f, 0.8f, 0.6f, 0.25f, 0.6f);
        op (f, 2, 1.0f, 0.85f, 0.04f, 1.0f, 0.85f, 0.25f, 0.3f, 0.0f, 5.0f);
        op (f, 3, 1.0f, 0.72f, 0.08f, 1.0f, 0.5f, 0.25f, 0.6f);
        op (f, 4, 1.0f, 0.5f, 0.05f, 0.5f, 0.3f, 0.25f, 0.5f);
        op (f, 5, 1.0f, 0.4f, 0.05f, 0.5f, 0.3f, 0.25f, 0.5f);
        space (s, Space::Hall, 0.22f);
        L.add (S, "FM Brass", "FM Brass", "dx7 brass fm 80s", s);
    }
    {
        auto s = brass (0, 600, 2.5f, 1.0f, 0.8f);
        L.add (S, "Synth Brass", "Brass Swell", "brass swell slow crescendo", s);
    }
    {
        auto s = brass (0, 900, 2.0f, 0.05f, 0.03f); mono (s, 0.04f); auto& a = s.layer[0].analog; a.l1Pitch = 0.15f; a.l1Delay = 0.35f;
        L.add (S, "Synth Brass", "Trumpet Lead Synth", "trumpet synth lead mono", s);
    }
    {
        auto s = analogPatch (1, 0, 4, 1500, 0.35f, 1.0f, 0.03f, 0.5f, 0.9f, 0.2f, 0.04f, 0.6f, 0.6f, 0.2f);
        mono (s, 0.05f); auto& a = s.layer[0].analog; a.fType = 6; a.vowel = 1.0f; a.noiseLevel = 0.08f; a.l1Pitch = 0.2f; a.l1Delay = 0.3f;
        s.layer[0].common.level = -6.0f;
        space (s, Space::Room, 0.18f);
        L.add (S, "Woodwind", "Sax Synth", "saxophone synth lead breathy", s);
    }
    {
        auto s = analogPatch (0, 0, 3, 1200, 0.15f, 0.5f, 0.2f, 1.0f, 0.9f, 0.5f, 0.2f, 1.0f, 0.7f, 0.5f);
        auto& a = s.layer[0].analog; a.o1Oct = -1; a.o2Oct = -1; a.l1Pitch = 0.12f; a.l1Delay = 0.5f; a.l1Rate = 5.0f;
        s.layer[0].common.poly = 8; space (s, Space::Hall, 0.3f);
        L.add (S, "Analog Strings", "Bowed Cello Synth", "cello bowed solo", s);
    }
    {
        auto s = analogPatch (0, 0, 0, 3000, 0.1f, 2.0f, 0.001f, 0.45f, 0.0f, 0.3f, 0.001f, 0.3f, 0.0f, 0.3f);
        auto& a = s.layer[0].analog; a.o2Oct = 1; a.o3Level = 0.7f; a.o3Oct = -1; a.noiseLevel = 0.15f;
        s.layer[0].common.poly = 8; s.layer[0].common.level = 4.0f; space (s, Space::Hall, 0.35f);
        L.add (S, "Hits", "Orchestra Hit", "orchestra hit fairlight 80s stab", s);
    }
    {
        auto s = brass (1, 1000, 2.0f, 0.05f, 0.03f); lofi (s, 24, 48, 0.12f, 0.3f, 7000);
        L.add (S, "Tape", "Tape Brass", "mellotron brass tape", s);
    }
}

void addPlucks (Lib& L)
{
    const char* P = kPlucks;

    {
        auto s = pluckSynth (1, -1, 300, 0.3f, 5.0f, 0.5f); s.layer[0].analog.o1PW = 0.3f; arp (s, 1, 3, 2, 0.6f);
        echo (s, 3, 0.35f, 0.25f);
        L.add (P, "Arp", "Pluck Arp", "arpeggio 16th pluck", s);
    }
    {
        auto s = fmPair (1.0f, 3.5f, 0.7f, 0.8f, 0.3f, 0.0f, 0.0f, 0.1f, 0.4f); space (s, Space::Hall, 0.25f); echo (s, 3, 0.3f, 0.2f);
        L.add (P, "Pluck", "Bell Pluck", "fm bell pluck", s);
    }
    L.add (P, "Pluck", "Synth Pluck", "synth pluck short", pluckSynth (0, 0, 500, 0.25f, 4.0f, 0.4f));
    {
        auto s = pluckSynth (0, 0, 800, 0.2f, 3.5f, 0.5f); s.layer[0].analog.unison = 3; s.layer[0].analog.uniDetune = 18;
        mod (s.layer[0], 1, ModFx::Chorus, 0.5f, 0.5f, 0.4f);
        L.add (P, "Pluck", "Saw Pluck Chords", "saw pluck chords future bass", s);
    }
    {
        auto s = pluckSynth (3, 2, 4000, 0.0f, 0.0f, 0.5f); s.layer[0].analog.o2Oct = 2; s.layer[0].analog.o2Level = 0.2f;
        L.add (P, "Pluck", "Synth Marimba", "synth marimba sine pluck", s);
    }
    {
        auto s = fmPair (1.0f, 7.0f, 0.55f, 1.0f, 0.15f, 0.0f, 0.0f, 0.2f, 0.4f); space (s, Space::Hall, 0.3f);
        L.add (P, "Pluck", "Glass Pluck", "fm glass pluck crystal", s);
    }
    {
        auto s = make (Engine::Pluck); auto& p = s.layer[0].pluck; s.layer[0].common.poly = 24;
        p.excite = 1; p.bright = 0.55f; p.pluckPos = 0.3f; p.decay = 1.5f; p.damping = 0.3f; p.body = 5; p.bodyMix = 0.5f; p.sustainHold = 1;
        arp (s, 3, 3, 3, 0.9f); space (s, Space::Hall, 0.35f);
        L.add (P, "Arp", "Harp Arpeggio", "harp arpeggio glissando", s);
    }
    {
        auto s = pluckSynth (0, -1, 700, 0.35f, 3.0f, 0.35f); arp (s, 1, 3, 1, 0.5f); echo (s, 6, 0.35f, 0.2f);
        L.add (P, "Arp", "Arp 16th Up", "arpeggiator 16th up", s);
    }
    {
        auto s = pluckSynth (1, -1, 1500, 0.2f, 2.0f, 0.4f); arp (s, 2, 1, 3, 0.6f);
        L.add (P, "Arp", "Arp Down Octaves", "arpeggiator down octaves", s);
    }
    {
        auto s = fmPair (1.0f, 3.5f, 0.68f, 1.5f, 0.5f, 0.0f, 0.0f, 0.1f, 0.6f); arp (s, 4, 3, 3, 0.7f); echo (s, 3, 0.45f, 0.3f);
        space (s, Space::Hall, 0.35f);
        L.add (P, "Arp", "Random Bell Arp", "random arpeggio bells", s);
    }
    {
        auto s = pluckSynth (0, 0, 1200, 0.2f, 2.5f, 0.3f); s.layer[0].analog.aS = 0.8f; s.layer[0].analog.aD = 0.2f;
        arp (s, 6, 3, 1, 0.35f); space (s, Space::Hall, 0.25f);
        L.add (P, "Arp", "Chord Gate", "trance gate chords stutter", s);
    }
    {
        auto s = pluckSynth (0, -1, 500, 0.4f, 3.5f, 0.3f); s.layer[0].common.poly = 4; arp (s, 1, 3, 2, 0.6f);
        echo (s, 3, 0.45f, 0.3f, 3500);
        L.add (P, "Arp", "Berlin Sequence", "berlin school sequence 70s", s);
    }
    {
        auto s = pluckSynth (0, 1, 900, 0.3f, 3.0f, 0.4f); arp (s, 1, 3, 2, 0.55f); echo (s, 3, 0.4f, 0.3f);
        mod (s.layer[0], 1, ModFx::Chorus, 0.4f, 0.4f, 0.4f);
        L.add (P, "Arp", "Synthwave Arp", "synthwave stranger things arp", s);
    }
    {
        auto s = mallet ({ 1.0f, 5.4f, 14.1f }, { 0, -14, -26 }, { 1.0f, 0.25f, 0.1f }, 1.6f, 0.5f, 0.15f, 2000.0f);
        s.layer[0].mallet.excite = 1; arp (s, 3, 1, 2, 0.8f); echo (s, 2, 0.3f, 0.2f);
        L.add (P, "Arp", "Kalimba Arp", "kalimba arpeggio", s);
    }
    {
        auto s = nylonGuitar(); arp (s, 5, 1, 1, 0.95f); s.layer[0].pluck.sustainHold = 1;
        L.add (P, "Arp", "Guitar Picking Arp", "fingerpicking arpeggio guitar", s);
    }
    {
        auto s = grand(); arp (s, 3, 1, 2, 0.9f); space (s, Space::Hall, 0.25f);
        L.add (P, "Arp", "Piano Arpeggio", "piano arpeggio broken chords", s);
    }
    {
        auto s = tineEP(); arp (s, 1, 3, 2, 0.7f); echo (s, 6, 0.35f, 0.2f);
        L.add (P, "Arp", "Tine Arp", "rhodes arpeggio", s);
    }
    {
        auto s = pluckSynth (3, 3, 4000, 0.0f, 0.0f, 0.8f); s.layer[0].analog.o2Oct = 1; s.layer[0].analog.o2Level = 0.3f;
        arp (s, 3, 3, 3, 0.4f); echo (s, 3, 0.6f, 0.35f, 3000); space (s, Space::Ambient, 0.45f);
        L.add (P, "Arp", "Ambient Arp", "ambient arpeggio delay", s);
    }
    {
        auto s = pluckSynth (1, -1, 1200, 0.25f, 2.0f, 0.3f); arp (s, 1, 3, 2, 0.5f, 0.28f);
        L.add (P, "Arp", "Swing Arp", "swing shuffle arpeggio", s);
    }
    {
        auto s = strings (1, 4000, 0.005f, 0.15f, ModFx::Ensemble, 0.4f); arp (s, 3, 3, 1, 0.4f);
        L.add (P, "Arp", "Staccato Strings Arp", "staccato strings ostinato", s);
    }
    {
        auto s = pluckSynth (5, -1, 1500, 0.15f, 3.0f, 0.35f); s.layer[0].analog.uniDetune = 25; echo (s, 3, 0.45f, 0.3f, 5000);
        L.add (P, "Pluck", "Trance Pluck", "trance pluck supersaw", s);
    }
    {
        auto s = pluckSynth (1, 1, 2000, 0.15f, 2.0f, 0.3f); s.layer[0].analog.o1PW = 0.25f; s.layer[0].analog.o2Oct = 1;
        L.add (P, "Pluck", "Square Pluck", "square pluck chip", s);
    }
    {
        auto s = musicBox(); arp (s, 1, 1, 2, 0.8f);
        L.add (P, "Arp", "Music Box Arp", "music box arpeggio", s);
    }
    {
        auto s = pluckSynth (0, 0, 600, 0.6f, 4.0f, 0.25f); s.layer[0].analog.fDrive = 0.5f; arp (s, 5, 3, 1, 0.5f);
        L.add (P, "Arp", "Acid Sequence", "acid 303 sequence", s);
    }
}
} // namespace kv::banks
