// Organs: tonewheel (drawbar registrations with percussion, click, scanner
// vibrato & rotary speaker), transistor/Vox combo organs, pipe organs,
// harmonium, accordion and other reed organs.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

static SoundData tw (const char* reg, bool fastLeslie, float drive, int vib = 6)
{
    auto s = tonewheel (reg);
    auto& l = s.layer[0];
    l.fx.m1Rate = fastLeslie ? 6.7f : 0.8f;
    l.fx.ampGain = drive;
    l.organ.vibrato = (float) vib;
    return s;
}

static SoundData combo (int model, const char* reg, float vibRate, float vibDepth)
{
    auto s = make (Engine::Organ);
    auto& l = s.layer[0];
    l.common.poly = 40;
    auto& o = l.organ;
    o.model = (float) model;
    drawbars (o, reg);
    o.click = 0.1f; o.attack = 0.003f; o.release = 0.015f; o.tone = 12000.0f;
    if (vibDepth > 0.0f) mod (l, 1, ModFx::Vibrato, vibRate, vibDepth, 1.0f);
    space (s, Space::Room, 0.14f);
    return s;
}

static SoundData pipe (int model, const char* reg, float chiff, float wind)
{
    auto s = make (Engine::Organ);
    auto& l = s.layer[0];
    l.common.poly = 48;
    auto& o = l.organ;
    o.model = (float) model;
    drawbars (o, reg);
    o.click = 0.0f; o.attack = 0.06f; o.release = 0.25f; o.chiff = chiff; o.wind = wind; o.tone = 14000.0f;
    space (s, Space::Cathedral, 0.38f);
    return s;
}

static SoundData reed (int model, float musette, int voices, float tone)
{
    auto s = make (Engine::Organ);
    auto& l = s.layer[0];
    l.common.poly = 24;
    auto& o = l.organ;
    o.model = (float) model;
    drawbars (o, "008000000");
    o.click = 0.0f; o.attack = 0.04f; o.release = 0.08f; o.wind = 0.4f; o.musette = musette; o.musetteVoices = (float) voices; o.tone = tone;
    space (s, Space::Room, 0.14f);
    return s;
}

void addOrgans (Lib& L)
{
    const char* O = kOrgans;

    //--------------------------------------------------------------- tonewheel
    {
        auto s = tw ("888000000", false, 0.25f); auto& o = s.layer[0].organ; o.percOn = 1; o.percHarm = 1; o.percFast = 1;
        L.add (O, "Tonewheel", "Jazz Organ 888", "b3 hammond jazz jimmy smith percussion", s);
    }
    {
        auto s = tw ("888888888", true, 0.55f);
        L.add (O, "Tonewheel", "Full Organ Fast", "b3 full drawbars leslie fast", s);
    }
    {
        auto s = tw ("888808008", false, 0.4f);
        L.add (O, "Tonewheel", "Gospel Organ", "b3 gospel church soul", s);
    }
    {
        auto s = tw ("888800000", true, 0.78f); amp (s.layer[0], OrganAmp, 0.78f, 0.15f);
        L.add (O, "Tonewheel", "Rock Organ Overdrive", "b3 deep purple rock distortion", s);
    }
    {
        auto s = tw ("886000000", false, 0.35f, 6);
        L.add (O, "Tonewheel", "Blues Organ", "b3 blues soul", s);
    }
    {
        auto s = tw ("006600000", false, 0.15f, 4); space (s, Space::Hall, 0.2f);
        L.add (O, "Tonewheel", "Ballad Organ", "soft ballad organ", s);
    }
    {
        auto s = tw ("886000000", false, 0.3f, 0); auto& o = s.layer[0].organ; o.percOn = 1; o.percHarm = 1; mod (s.layer[0], 1, ModFx::Off, 0, 0, 0);
        L.add (O, "Tonewheel", "Green Onions", "booker t mg soul 60s", s);
    }
    {
        auto s = tw ("888800000", true, 0.6f);
        L.add (O, "Tonewheel", "Steppin Rock", "b3 rock 60s steppenwolf", s);
    }
    {
        auto s = tw ("800000000", false, 0.2f); auto& o = s.layer[0].organ; o.percOn = 1; o.percHarm = 0; o.percFast = 1;
        L.add (O, "Tonewheel", "Perc Jazz 2nd", "percussion second harmonic jazz", s);
    }
    {
        auto s = tw ("688600000", false, 0.25f, 3); space (s, Space::Hall, 0.25f);
        L.add (O, "Tonewheel", "Whiter Shade", "procol harum ballad 60s", s);
    }
    {
        auto s = tw ("008000000", false, 0.1f, 4);
        L.add (O, "Tonewheel", "Soft Flute Organ", "flute drawbar soft", s);
    }
    {
        auto s = tw ("008800000", false, 0.2f, 0); auto& o = s.layer[0].organ; o.percOn = 1; o.percHarm = 1; o.release = 0.005f;
        mod (s.layer[0], 1, ModFx::Off, 0, 0, 0); space (s, Space::Spring, 0.15f);
        L.add (O, "Tonewheel", "Reggae Bubble", "reggae bubble skank organ", s);
    }
    {
        auto s = tw ("888000000", false, 0.15f, 0); auto& o = s.layer[0].organ; o.percOn = 1; o.percHarm = 1; o.release = 0.005f;
        mod (s.layer[0], 1, ModFx::Off, 0, 0, 0);
        L.add (O, "Tonewheel", "Ska Organ", "ska skank short", s);
    }
    {
        auto s = tw ("888888000", true, 0.85f); amp (s.layer[0], OrganAmp, 0.85f, 0.3f);
        L.add (O, "Tonewheel", "Dirty Leslie", "b3 dirty overdrive fast leslie", s);
    }
    {
        auto s = tw ("888600000", false, 0.0f, 6); mod (s.layer[0], 1, ModFx::Off, 0, 0, 0); amp (s.layer[0], AmpOff, 0.0f);
        L.add (O, "Tonewheel", "Clean Tonewheel", "b3 direct no leslie", s);
    }
    {
        auto s = tw ("838000000", false, 0.3f, 6); auto& o = s.layer[0].organ; o.percOn = 1; o.percHarm = 1; o.percSoft = 1;
        L.add (O, "Tonewheel", "Smoky Jazz", "jazz soft percussion", s);
    }
    {
        auto s = tw ("008808008", true, 0.45f);
        L.add (O, "Tonewheel", "Church Upper", "gospel upper bright", s);
    }
    {
        auto s = tw ("800000008", false, 0.2f, 6);
        L.add (O, "Tonewheel", "Hollow Whistle", "16 and 1 foot hollow", s);
    }
    {
        auto s = tw ("888000000", false, 0.25f); auto& l = s.layer[0]; l.common.keyHi = 47; l.organ.click = 0.2f;
        auto upper = tw ("888800000", false, 0.35f); upper.layer[0].common.keyLo = 48;
        auto sp = stack (s, upper, -1.0f, 0.0f);
        L.add (O, "Tonewheel", "Split Bass & Upper", "split organ bass lower manual", sp);
    }

    //--------------------------------------------------------------- theatre / combo
    {
        auto s = pipe (4, "808808000", 0.0f, 0.1f); auto& l = s.layer[0]; l.organ.vibrato = 3; l.organ.attack = 0.02f;
        mod (l, 1, ModFx::Vibrato, 6.5f, 0.25f, 1.0f); mod (l, 2, ModFx::Tremolo, 6.5f, 0.2f, 0.0f); space (s, Space::Hall, 0.3f);
        L.add (O, "Theatre", "Theatre Organ", "wurlitzer theatre tibia tremulant cinema", s);
    }
    L.add (O, "Combo", "Transistor Combo", "farfisa combo 60s", combo (1, "008880000", 6.0f, 0.12f));
    L.add (O, "Combo", "Vox Combo", "vox continental 60s doors", combo (2, "888000000", 6.5f, 0.1f));
    L.add (O, "Combo", "96 Tears Combo", "farfisa bright garage", combo (1, "088888000", 6.5f, 0.15f));
    {
        auto s = combo (1, "888800000", 6.0f, 0.1f); amp (s.layer[0], Fuzz, 0.35f, 0.0f, Combo2x12, -1.0f);
        L.add (O, "Combo", "Garage Fuzz Combo", "garage fuzz psych 60s", s);
    }
    {
        auto s = combo (2, "808000000", 6.5f, 0.0f); mod (s.layer[0], 1, ModFx::Rotary, 0.8f, 0.5f, 1.0f);
        L.add (O, "Combo", "Light My Combo", "vox doors psychedelic", s);
    }

    //--------------------------------------------------------------- pipe
    L.add (O, "Pipe", "Church Principal 8 4", "church pipe organ principal", pipe (3, "008600400", 0.35f, 0.15f));
    L.add (O, "Pipe", "Full Plenum", "full organ mixture tutti bach", pipe (3, "808876556", 0.4f, 0.1f));
    L.add (O, "Pipe", "Flute 8", "pipe flute gedackt soft", pipe (4, "008000000", 0.5f, 0.25f));
    L.add (O, "Pipe", "Trumpet Stop", "pipe reed trumpet", pipe (5, "008400000", 0.2f, 0.1f));
    {
        auto s = pipe (3, "888878668", 0.4f, 0.1f); s.gfx.revDecay = 8.0f;
        L.add (O, "Pipe", "Toccata Organ", "bach toccata d minor full", s);
    }
    {
        auto s = pipe (3, "008000000", 0.2f, 0.15f); auto& o = s.layer[0].organ; o.musette = 6.0f; o.musetteVoices = 2;
        L.add (O, "Pipe", "Voix Celeste", "celeste soft strings pipe", s);
    }
    {
        auto s = pipe (4, "008800400", 0.6f, 0.3f); s.gfx.revMix = 0.25f; space (s, Space::Chamber, 0.25f);
        L.add (O, "Pipe", "Chamber Organ", "baroque chamber positive", s);
    }
    {
        auto s = pipe (5, "808800000", 0.25f, 0.1f); s.layer[0].organ.attack = 0.04f;
        L.add (O, "Pipe", "Reed Chorus", "pipe reed bombarde", s);
    }

    //--------------------------------------------------------------- reed organs
    L.add (O, "Reed", "Harmonium", "harmonium pump reed", reed (6, 0.0f, 1, 6000.0f));
    {
        auto s = reed (6, 3.0f, 2, 5000.0f);
        L.add (O, "Reed", "Indian Harmonium", "harmonium india bhajan", s);
    }
    {
        auto s = reed (7, 16.0f, 3, 9000.0f); mod (s.layer[0], 1, ModFx::Tremolo, 5.5f, 0.12f, 1.0f);
        L.add (O, "Accordion", "Accordion Musette", "accordion musette french paris", s);
    }
    {
        auto s = reed (7, 3.0f, 2, 6000.0f);
        L.add (O, "Accordion", "Bandoneon", "bandoneon tango argentina", s);
    }
    {
        auto s = reed (7, 0.0f, 1, 7000.0f); s.layer[0].organ.db[2] = 8; s.layer[0].organ.db[0] = 6;
        L.add (O, "Accordion", "Accordion Bassoon", "accordion dry reed", s);
    }
    {
        auto s = reed (6, 0.0f, 1, 3500.0f); auto& o = s.layer[0].organ; o.wind = 0.6f; o.attack = 0.025f;
        s.layer[0].common.mwVib = 0.4f;
        L.add (O, "Reed", "Melodica", "melodica pianica breath", s);
    }
    {
        auto s = reed (6, 8.0f, 2, 4500.0f); auto& o = s.layer[0].organ; drawbars (o, "008800000"); o.wind = 0.2f;
        L.add (O, "Reed", "Harmochord", "harmochord vintage reed", s);
    }
}
} // namespace kv::banks
