// Layers & splits (piano/pad hybrids, keyboard splits, velocity switches)
// and FX & atmospheres.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

static SoundData warmPad()
{
    auto s = analogPatch (0, 0, 8, 1500, 0.15f, 1.0f, 0.9f, 1.5f, 0.85f, 1.8f, 1.2f, 2.5f, 0.5f, 2.0f);
    auto& a = s.layer[0].analog; a.unison = 2; a.uniDetune = 14; a.uniSpread = 0.8f; a.velAmp = 0.2f;
    s.layer[0].common.poly = 12;
    mod (s.layer[0], 1, ModFx::Chorus, 0.4f, 0.4f, 0.35f);
    return s;
}

static SoundData stringMachine()
{
    auto s = analogPatch (0, 0, 7, 5000, 0.1f, 0.3f, 0.35f, 1.0f, 0.9f, 1.2f, 0.3f, 1.5f, 0.6f, 1.2f);
    auto& a = s.layer[0].analog; a.fType = 1; a.o2Oct = 1; a.o2Level = 0.45f;
    s.layer[0].common.poly = 16;
    mod (s.layer[0], 1, ModFx::Ensemble, 0.6f, 0.6f, 0.7f);
    return s;
}

static SoundData fmBell()
{
    auto s = fmPair (1.0f, 3.5f, 0.66f, 3.0f, 1.2f, 0.0f, 0.0f, 0.1f, 1.0f);
    return s;
}

static void velSplit (SoundData& s, int v)
{
    s.layer[0].common.velHi = (float) (v - 1);
    s.layer[1].common.velLo = (float) v;
}

void addLayers (Lib& L)
{
    const char* Y = kLayers;

    {
        auto s = stack (grand(), stringMachine(), 0.0f, -9.0f); space (s, Space::Hall, 0.25f);
        L.add (Y, "Piano Layer", "Grand & Strings", "piano strings ballad layer", s);
    }
    {
        auto s = stack (grand(), warmPad(), 0.0f, -10.0f); space (s, Space::Hall, 0.28f);
        L.add (Y, "Piano Layer", "Grand & Warm Pad", "piano pad layer worship", s);
    }
    {
        auto s = stack (grand(), tineEP(), -1.0f, -5.0f); space (s, Space::Hall, 0.2f);
        L.add (Y, "Hybrid", "Grand & Tine Hybrid", "piano rhodes layer hybrid duo", s);
    }
    {
        auto s = stack (tineEP(), warmPad(), 0.0f, -9.0f); space (s, Space::Hall, 0.22f);
        L.add (Y, "EP Layer", "Tine & Pad", "rhodes pad layer", s);
    }
    {
        auto s = stack (tineEP(), stringMachine(), 0.0f, -9.0f); space (s, Space::Hall, 0.22f);
        L.add (Y, "EP Layer", "Tine & Strings", "rhodes strings 70s soul", s);
    }
    {
        auto s = stack (reedEP(), vibraphone(), 0.0f, -6.0f);
        L.add (Y, "EP Layer", "Reed & Vibes", "wurlitzer vibraphone layer", s);
    }
    {
        auto s = stack (grand(), fmBell(), 0.0f, -10.0f); space (s, Space::Hall, 0.25f);
        L.add (Y, "Piano Layer", "Grand & Bell", "piano bell fm layer 90s", s);
    }
    {
        auto b = grand(); b.layer[0].common.transpose = 12; b.layer[0].piano.hardness = 0.4f;
        auto s = stack (grand(), b, 0.0f, -9.0f);
        L.add (Y, "Piano Layer", "Piano Octaves", "piano octave doubled", s);
    }
    {
        auto s = stack (electricGrand(), stringMachine(), 0.0f, -8.0f); space (s, Space::Hall, 0.25f);
        L.add (Y, "Piano Layer", "CP & Strings Ballad", "cp80 strings 80s ballad", s);
    }
    {
        auto s = stack (dxEP(), warmPad(), 0.0f, -9.0f); space (s, Space::Hall, 0.25f);
        L.add (Y, "EP Layer", "DX EP & Pad 80s", "dx7 pad 80s ballad", s);
    }
    {
        auto org = tonewheel ("888808008"); org.layer[0].fx.ampGain = 0.35f;
        auto s = stack (grand(), org, 0.0f, -8.0f);
        L.add (Y, "Piano Layer", "Gospel Piano & Organ", "gospel church piano organ", s);
    }
    {
        auto s = stack (bassGuitar(), grand(), 0.0f, 0.0f); split (s, 48);
        L.add (Y, "Split", "Bass | Piano", "split bass piano left hand", s);
    }
    {
        auto s = stack (bassGuitar(), tineEP(), 0.0f, 0.0f); split (s, 48);
        L.add (Y, "Split", "Bass | Tine", "split bass rhodes", s);
    }
    {
        auto up = make (Engine::Pluck); auto& p = up.layer[0].pluck;
        p.excite = 1; p.bright = 0.3f; p.pluckPos = 0.22f; p.decay = 1.0f; p.damping = 0.6f; p.stiffness = 0.2f; p.body = 9; p.bodyMix = 0.7f; p.twang = 7.0f;
        up.layer[0].common.poly = 4;
        auto pno = grand(); pno.layer[0].piano.hardness = 0.55f;
        auto s = stack (up, pno, 0.0f, -1.0f); split (s, 48); space (s, Space::Chamber, 0.16f);
        L.add (Y, "Split", "Upright Bass | Jazz Piano", "jazz trio split walking bass", s);
    }
    {
        auto bass = analogPatch (0, 1, 0, 450, 0.3f, 3.0f, 0.002f, 0.4f, 0.8f, 0.08f, 0.001f, 0.25f, 0.1f, 0.08f);
        mono (bass, 0.0f, false); bass.layer[0].analog.o2Oct = -1;
        auto ld = analogPatch (0, 1, 6, 2500, 0.3f, 2.0f, 0.004f, 0.4f, 0.85f, 0.25f, 0.004f, 0.5f, 0.4f, 0.3f);
        mono (ld, 0.05f, true);
        auto s = stack (bass, ld, 0.0f, 0.0f); split (s, 60); echo (s, 3, 0.3f, 0.15f);
        L.add (Y, "Split", "Synth Bass | Lead", "split synth bass lead", s);
    }
    {
        auto s = stack (nylonGuitar(), stringMachine(), 0.0f, -10.0f); space (s, Space::Hall, 0.22f);
        L.add (Y, "Guitar Layer", "Nylon & Strings", "guitar strings layer", s);
    }
    {
        auto s = stack (harpsichord(), stringMachine(), 0.0f, -9.0f);
        L.add (Y, "Baroque", "Harpsichord & Strings", "baroque continuo strings", s);
    }
    {
        auto s = stack (celeste(), warmPad(), 0.0f, -10.0f); space (s, Space::Hall, 0.3f);
        L.add (Y, "Bell Layer", "Celeste & Pad", "celeste pad magical", s);
    }
    {
        auto s = stack (vibraphone(), tineEP(), -2.0f, -3.0f);
        L.add (Y, "EP Layer", "Vibes & Tine", "vibraphone rhodes", s);
    }
    {
        auto s = stack (steelGuitar(), warmPad(), 0.0f, -10.0f); space (s, Space::Hall, 0.25f);
        L.add (Y, "Guitar Layer", "Guitar & Pad", "acoustic guitar pad worship", s);
    }
    {
        auto tack = upright(); tack.layer[0].piano.tack = 0.8f; tack.layer[0].piano.hardness = 0.8f;
        auto s = stack (grand(), tack, 0.0f, -6.0f);
        L.add (Y, "Hybrid", "Tack Grand", "tack grand duo", s);
    }
    {
        auto s = stack (tineEP(), celeste(), 0.0f, -9.0f); space (s, Space::Hall, 0.25f);
        L.add (Y, "Hybrid", "Hollywood Tine", "rhodes celeste film duo", s);
    }
    {
        auto s = stack (feltPiano(), grand(), 0.0f, 0.0f); velSplit (s, 80);
        L.add (Y, "Velocity", "Felt / Grand Velocity", "velocity switch felt grand", s);
    }
    {
        auto soft = tineEP(); soft.layer[0].ep.bark = 0.2f; soft.layer[0].ep.drive = 0.3f;
        auto hard = tineEP(); hard.layer[0].ep.bark = 1.0f; hard.layer[0].ep.drive = 0.85f; amp (hard.layer[0], Tube, 0.5f, 0.0f, EPSpeaker);
        auto s = stack (soft, hard, 0.0f, 0.0f); velSplit (s, 96);
        L.add (Y, "Velocity", "Tine Bark Switch", "velocity bark rhodes", s);
    }
    {
        auto choir = analogPatch (0, 0, 6, 1500, 0.3f, 0.0f, 0.6f, 1.5f, 0.85f, 2.0f, 0.9f, 2.5f, 0.5f, 2.0f);
        choir.layer[0].analog.fType = 6; choir.layer[0].analog.unison = 3; choir.layer[0].common.poly = 12;
        auto s = stack (grand(), choir, 0.0f, -10.0f); space (s, Space::Hall, 0.3f);
        L.add (Y, "Piano Layer", "Piano & Choir", "piano choir epic", s);
    }
    {
        auto pl = analogPatch (1, -1, 0, 500, 0.3f, 4.0f, 0.001f, 0.4f, 0.0f, 0.3f, 0.001f, 0.2f, 0.0f, 0.2f);
        pl.layer[0].common.arpMode = 1; pl.layer[0].common.arpRate = 3; pl.layer[0].common.arpOct = 2; pl.layer[0].common.level = 4.0f;
        auto s = stack (warmPad(), pl, 0.0f, -4.0f); echo (s, 3, 0.35f, 0.2f); space (s, Space::Hall, 0.3f);
        L.add (Y, "Synth Layer", "Pad & Arp", "pad arpeggio layer", s);
    }
    {
        auto br = analogPatch (0, 0, 6, 900, 0.15f, 2.5f, 0.02f, 0.6f, 0.85f, 0.3f, 0.08f, 0.6f, 0.45f, 0.3f);
        auto s = stack (stringMachine(), br, 0.0f, 0.0f); split (s, 60); space (s, Space::Hall, 0.25f);
        L.add (Y, "Split", "Strings | Brass", "split strings brass orchestra", s);
    }
}

void addFx (Lib& L)
{
    const char* F = kFx;

    {
        auto s = mallet ({ 1.0f, 2.32f, 4.25f }, { 0, -6, -12 }, { 1, 0.7f, 0.5f }, 8.0f, 0.5f);
        s.layer[0].mallet.excite = 2; s.layer[0].mallet.bowAttack = 0.6f; space (s, Space::Ambient, 0.4f);
        L.add (F, "Bowed", "Glass Harmonica", "glass harmonica armonica franklin", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.76f, 5.4f, 8.93f }, { 0, -8, -14, -20 }, { 1.0f, 0.7f, 0.5f, 0.3f }, 4.0f, 0.8f, 0.05f, 6000.0f);
        auto& c = s.layer[0].common; c.arpMode = 4; c.arpRate = 4; c.arpOct = 2; c.arpGate = 0.9f; c.transpose = 24;
        space (s, Space::Ambient, 0.45f);
        L.add (F, "Chimes", "Wind Chimes", "wind chimes random garden", s);
    }
    {
        auto s = analogPatch (0, 0, 9, 350, 0.3f, 0.8f, 3.0f, 2.0f, 0.9f, 6.0f, 3.0f, 3.0f, 0.6f, 6.0f);
        auto& a = s.layer[0].analog; a.o1Oct = -1; a.unison = 3; a.uniDetune = 20; a.l2Cut = 1.5f; a.l2Rate = 0.05f;
        echo (s, 9, 0.6f, 0.3f, 2000); space (s, Space::Ambient, 0.65f);
        L.add (F, "Drone", "Dark Drone", "dark drone horror cinematic", s);
    }
    {
        auto s = analogPatch (1, -1, 0, 3000, 0.4f, 0.5f, 0.002f, 0.3f, 0.6f, 0.1f, 0.002f, 0.3f, 0.5f, 0.1f);
        auto& a = s.layer[0].analog; a.l1Shape = 5; a.l1Rate = 9.0f; a.l1Pitch = 12.0f; a.o1PW = 0.3f;
        echo (s, 5, 0.4f, 0.25f);
        L.add (F, "Sci-Fi", "Computer Bleeps", "computer bleeps sample hold sci-fi", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; mod (l, 1, ModFx::Vibrato, 0.6f, 0.4f, 1.0f); mod (l, 2, ModFx::Chorus, 0.3f, 0.9f, 0.6f);
        eq (l, 2.0f, 0.0f, 1000.0f, -6.0f, 20.0f, 1800.0f); space (s, Space::Ambient, 0.45f);
        L.add (F, "Ambient", "Underwater Keys", "underwater muffled wobble", s);
    }
    {
        auto s = fmPair (1.0f, 1.41f, 0.75f, 4.0f, 2.0f, 0.0f, 0.0f, 0.3f, 1.5f);
        auto& f = s.layer[0].fm; op (f, 2, 2.73f, 0.8f, 0.002f, 3.0f, 0.0f, 1.5f, 0.3f); op (f, 3, 3.1f, 0.6f, 0.002f, 1.0f, 0.0f, 1.0f, 0.6f);
        space (s, Space::Hall, 0.4f); echo (s, 9, 0.5f, 0.25f);
        L.add (F, "Sci-Fi", "Alien Bells", "alien metallic inharmonic bells", s);
    }
    {
        auto s = analogPatch (0, 0, 15, 900, 0.4f, 0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f);
        auto& a = s.layer[0].analog; a.l1Shape = 2; a.l1Rate = 0.12f; a.l1Pitch = 12.0f; a.l1Cut = 3.0f; a.noiseLevel = 0.3f;
        s.layer[0].common.level = 8.0f;
        space (s, Space::Hall, 0.35f);
        L.add (F, "Transition", "Riser", "riser build up sweep transition", s);
    }
    {
        auto s = analogPatch (4, -1, 0, 800, 0.5f, 0.0f, 1.5f, 1.0f, 1.0f, 3.0f, 1.0f, 1.0f, 1.0f, 2.0f);
        auto& a = s.layer[0].analog; a.l2Cut = 2.5f; a.l2Rate = 0.07f; a.keyTrack = 1.0f;
        s.layer[0].common.level = 7.0f;
        space (s, Space::Ambient, 0.5f);
        L.add (F, "Ambient", "Noise Wash", "noise wash ocean wind", s);
    }
    {
        auto s = fmPair (1.0f, 4.73f, 0.8f, 6.0f, 4.0f, 0.3f, 0.5f, 0.5f, 3.0f);
        echo (s, 3, 0.65f, 0.35f, 3000); space (s, Space::Ambient, 0.5f);
        L.add (F, "Sci-Fi", "Space Station", "space metallic fm drone", s);
    }
    {
        auto s = analogPatch (0, 0, 8, 1500, 0.3f, 0.0f, 1.0f, 1.0f, 0.8f, 2.0f, 1.0f, 1.0f, 0.8f, 2.0f);
        auto& a = s.layer[0].analog; a.fType = 6; a.vowel = 4.0f; a.unison = 3;
        lofi (s, 24, 48, 0.25f, 0.55f, 5000); space (s, Space::Cathedral, 0.5f);
        L.add (F, "Ambient", "Ghost Choir", "ghost choir haunted horror", s);
    }
    {
        auto s = analogPatch (0, 0, 7, 2000, 0.2f, 1.0f, 2.5f, 0.5f, 1.0f, 0.06f, 2.5f, 0.5f, 1.0f, 0.06f);
        s.layer[0].analog.unison = 2; space (s, Space::Hall, 0.3f);
        L.add (F, "Ambient", "Reverse Swell", "reverse swell backwards", s);
    }
    {
        auto s = analogPatch (0, 1, 7, 1500, 0.45f, 0.0f, 0.5f, 1.0f, 0.9f, 2.0f, 0.5f, 1.0f, 0.9f, 2.0f);
        auto& a = s.layer[0].analog; a.l2Shape = 5; a.l2Rate = 9.0f; a.l2Cut = 2.0f;
        echo (s, 7, 0.5f, 0.3f); space (s, Space::Ambient, 0.5f);
        L.add (F, "Ambient", "Granular Cloud", "granular glitch cloud", s);
    }
    {
        auto s = tineEP(); lofi (s, 24, 48, 0.15f, 0.9f, 6000);
        L.add (F, "Lo-Fi", "Tape Warble Keys", "tape warble wow flutter", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano; p.decay = 4.0f; p.release = 3.0f;
        echo (s, 9, 0.6f, 0.3f, 3000); space (s, Space::Ambient, 0.7f);
        L.add (F, "Ambient", "Infinite Piano Wash", "piano ambient wash infinite", s);
    }
    {
        auto s = analogPatch (4, -1, 0, 180, 0.3f, 1.0f, 0.01f, 3.0f, 0.0f, 2.0f, 0.01f, 2.0f, 0.0f, 2.0f);
        s.layer[0].analog.keyTrack = 0.0f; s.layer[0].common.level = 12.0f; space (s, Space::Cathedral, 0.5f);
        L.add (F, "Nature", "Thunder Rumble", "thunder rumble storm", s);
    }
    {
        auto s = tineEP(); lofi (s, 5, 6, 0.5f, 0.0f, 3500);
        L.add (F, "Lo-Fi", "Radio Static Keys", "radio am static bitcrush", s);
    }
    {
        auto s = mallet ({ 1.0f, 1.47f, 2.09f, 2.56f, 3.4f }, { 0, -4, -6, -8, -10 }, { 1.0f, 0.9f, 0.8f, 0.7f, 0.6f }, 10.0f, 0.5f);
        s.layer[0].mallet.excite = 2; s.layer[0].mallet.bowAttack = 1.5f; s.layer[0].common.transpose = -12;
        space (s, Space::Cathedral, 0.5f);
        L.add (F, "Bowed", "Bowed Metal", "bowed metal cymbal scary", s);
    }
    {
        auto s = analogPatch (1, -1, 0, 6000, 0.2f, 0.0f, 0.001f, 0.25f, 0.0f, 0.1f, 0.001f, 0.2f, 0.0f, 0.1f);
        auto& a = s.layer[0].analog; a.pEnv = 24; a.pDecay = 0.08f; echo (s, 5, 0.3f, 0.2f);
        L.add (F, "Sci-Fi", "Laser Zap", "laser zap pew sci-fi", s);
    }
}

// Clean starting points for sound design on the Edit page.
void addInit (Lib& L)
{
    const char* I = kInit;
    L.add (I, "Init", "Init Acoustic Piano", "init template blank", make (Engine::Piano));
    L.add (I, "Init", "Init Electric Piano", "init template blank", make (Engine::EPiano));
    L.add (I, "Init", "Init Plucked String", "init template blank", make (Engine::Pluck));
    L.add (I, "Init", "Init Mallet & Bell", "init template blank", make (Engine::Mallet));
    L.add (I, "Init", "Init Organ", "init template blank", make (Engine::Organ));
    {
        auto s = make (Engine::Analog); s.layer[0].common.poly = 16;
        L.add (I, "Init", "Init Analog Synth", "init template blank", s);
    }
    {
        auto s = make (Engine::FM); auto& f = s.layer[0].fm; s.layer[0].common.poly = 16;
        f.alg = 2; op (f, 0, 1.0f, 1.0f, 0.002f, 1.0f, 0.8f, 0.3f); op (f, 1, 1.0f, 0.6f, 0.002f, 1.0f, 0.5f, 0.3f);
        L.add (I, "Init", "Init FM Synth", "init template blank", s);
    }
}
} // namespace kv::banks
