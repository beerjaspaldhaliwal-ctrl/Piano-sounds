// Guitars: nylon, steel-string, 12-string, jazz archtop, clean / crunch /
// lead / metal electrics, funk, surf, country, slide and ambient guitars.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

void addGuitars (Lib& L)
{
    const char* G = kGuitars;

    //--------------------------------------------------------------- nylon
    L.add (G, "Nylon", "Nylon Classical", "classical guitar spanish nylon", nylonGuitar());
    {
        auto s = nylonGuitar(); auto& p = s.layer[0].pluck; p.bright = 0.32f; p.pluckPos = 0.22f; space (s, Space::Studio, 0.08f);
        L.add (G, "Nylon", "Nylon Fingerstyle Soft", "nylon soft fingerstyle close", s);
    }
    {
        auto s = nylonGuitar(); auto& p = s.layer[0].pluck; p.bright = 0.4f; p.decay = 0.6f; s.layer[0].common.strum = 22;
        space (s, Space::Room, 0.18f);
        L.add (G, "Nylon", "Bossa Nylon", "bossa nova brazil jobim", s);
    }
    {
        auto s = nylonGuitar(); auto& p = s.layer[0].pluck; p.excite = 0; p.bright = 0.7f; p.pluckPos = 0.11f; p.pickNoise = 0.35f;
        s.layer[0].common.strum = 14; s.layer[0].common.level = 4.0f;
        L.add (G, "Nylon", "Flamenco", "flamenco spanish rasgueado bright", s);
    }
    {
        auto s = nylonGuitar(); s.layer[0].common.strum = 32;
        L.add (G, "Nylon", "Nylon Strummed", "nylon strum chords", s);
    }

    //--------------------------------------------------------------- steel acoustic
    L.add (G, "Steel", "Steel Acoustic", "acoustic guitar dreadnought steel", steelGuitar());
    {
        auto s = steelGuitar(); s.layer[0].common.strum = 26; s.layer[0].fx.compAmt = 0.25f;
        L.add (G, "Steel", "Steel Strummed", "acoustic strum chords pop", s);
    }
    {
        auto s = steelGuitar(); auto& p = s.layer[0].pluck; p.excite = 1; p.bright = 0.5f; p.pluckPos = 0.18f; p.pickNoise = 0.1f;
        L.add (G, "Steel", "Steel Fingerpicked", "fingerpicking folk travis", s);
    }
    {
        auto s = steelGuitar(); auto& l = s.layer[0]; l.pluck.bodyMix = 0.82f; eq (l, 2.5f, -1.5f, 600.0f, 1.0f);
        L.add (G, "Steel", "Jumbo Acoustic", "jumbo big body warm", s);
    }
    {
        auto s = steelGuitar(); auto& p = s.layer[0].pluck; p.course = 2; p.courseOct = 1; p.courseDetune = 4.0f; p.courseDelay = 6.0f;
        s.layer[0].common.strum = 24; mod (s.layer[0], 1, ModFx::Chorus, 0.4f, 0.15f, 0.15f);
        L.add (G, "Steel", "12-String Acoustic", "twelve string jangle folk", s);
    }
    {
        auto s = steelGuitar(); auto& p = s.layer[0].pluck; p.body = 8; p.bright = 0.6f; p.decay = 0.7f;
        L.add (G, "Steel", "Parlor Guitar", "parlor small body blues", s);
    }
    {
        auto s = steelGuitar(); auto& l = s.layer[0]; auto& p = l.pluck;
        p.body = 4; p.bodyMix = 0.55f; p.stiffness = 0.35f; p.bright = 0.75f; p.decay = 1.3f;
        l.common.legato = 1; l.common.glide = 0.08f; l.common.poly = 6;
        L.add (G, "Steel", "Resonator Dobro", "dobro resonator slide bluegrass", s);
    }
    {
        auto s = steelGuitar(); auto& p = s.layer[0].pluck; s.layer[0].common.transpose = 12;
        p.excite = 1; p.bright = 0.9f; p.pluckPos = 0.04f; p.decay = 1.6f; p.damping = 0.15f; space (s, Space::Hall, 0.25f);
        L.add (G, "Steel", "Natural Harmonics", "harmonics bell like chime", s);
    }
    {
        auto s = steelGuitar(); echo (s, 6, 0.45f, 0.25f, 3500.0f); space (s, Space::Ambient, 0.35f);
        L.add (G, "Steel", "Acoustic Ambient", "acoustic delay ambient post rock", s);
    }

    //--------------------------------------------------------------- electric clean
    L.add (G, "Electric Clean", "Clean Strat Neck", "strat clean neck single coil", electricGuitar (0.22f));
    {
        auto s = electricGuitar (0.06f); amp (s.layer[0], CleanAmp, 0.35f, 0.25f, Open1x12);
        L.add (G, "Electric Clean", "Clean Bridge Twang", "telecaster bridge twang clean", s);
    }
    {
        auto s = electricGuitar (0.16f); mod (s.layer[0], 1, ModFx::Chorus, 0.6f, 0.5f, 0.5f); space (s, Space::Hall, 0.18f);
        L.add (G, "Electric Clean", "Chorus Clean", "chorus clean 80s police", s);
    }
    {
        auto s = electricGuitar (0.26f); auto& p = s.layer[0].pluck; p.bright = 0.3f; p.damping = 0.62f; p.decay = 1.1f; p.excite = 1;
        amp (s.layer[0], CleanAmp, 0.25f, -0.4f, Combo2x12); space (s, Space::Room, 0.14f);
        L.add (G, "Jazz", "Jazz Archtop", "jazz archtop hollow body warm", s);
    }
    {
        auto s = electricGuitar (0.26f); auto& p = s.layer[0].pluck; p.bright = 0.35f; p.damping = 0.6f; p.excite = 1;
        p.course = 2; p.courseOct = 1; p.courseDetune = 0.0f; p.courseDelay = 0.0f;
        amp (s.layer[0], CleanAmp, 0.25f, -0.4f, Combo2x12);
        L.add (G, "Jazz", "Jazz Octaves", "wes montgomery octaves jazz", s);
    }
    {
        auto s = electricGuitar (0.1f); auto& l = s.layer[0]; l.pluck.mute = 0.5f; l.fx.compAmt = 0.45f;
        L.add (G, "Funk", "Funk Rhythm Muted", "funk muted chicken scratch nile rodgers", s);
    }
    {
        auto s = electricGuitar (0.1f); auto& l = s.layer[0]; wah (l, 1, 700.0f, 7.0f, 0.85f); l.pluck.mute = 0.25f;
        L.add (G, "Funk", "Funk Wah", "wah shaft funk 70s", s);
    }
    {
        auto s = electricGuitar (0.06f); auto& l = s.layer[0]; mod (l, 1, ModFx::Tremolo, 6.5f, 0.6f, 1.0f, 0.3f);
        space (s, Space::Spring, 0.35f);
        L.add (G, "Electric Clean", "Surf Guitar", "surf spring reverb tremolo dick dale", s);
    }
    {
        auto s = electricGuitar (0.18f); mod (s.layer[0], 1, ModFx::Tremolo, 5.0f, 0.55f, 1.0f); space (s, Space::Room, 0.18f);
        L.add (G, "Electric Clean", "Tremolo Clean", "tremolo vintage amp", s);
    }
    {
        auto s = electricGuitar (0.18f); mod (s.layer[0], 1, ModFx::Rotary, 6.7f, 0.5f, 1.0f);
        L.add (G, "Electric Clean", "Rotary Guitar", "leslie guitar", s);
    }
    {
        auto s = electricGuitar (0.1f); auto& p = s.layer[0].pluck; p.course = 2; p.courseOct = 1; p.courseDetune = 3.0f; p.courseDelay = 5.0f;
        s.layer[0].common.strum = 18; mod (s.layer[0], 1, ModFx::Chorus, 0.5f, 0.3f, 0.3f); s.layer[0].fx.compAmt = 0.35f;
        L.add (G, "Electric Clean", "Electric 12-String Jangle", "rickenbacker 12 string byrds jangle", s);
    }
    {
        auto s = electricGuitar (0.06f); auto& l = s.layer[0]; l.pluck.twang = 10.0f; l.fx.compAmt = 0.4f;
        echo (s, 0, 0.15f, 0.25f, 4500.0f, false, 110.0f);
        L.add (G, "Electric Clean", "Country Twang", "country telecaster chicken pickin slapback", s);
    }
    {
        auto s = electricGuitar (0.2f); auto& l = s.layer[0]; l.pluck.excite = 1; l.pluck.bright = 0.4f;
        L.add (G, "Electric Clean", "Fingerstyle Electric", "fingerstyle clean neck soft", s);
    }
    {
        auto s = electricGuitar (0.14f); auto& l = s.layer[0]; l.pluck.release = 1.2f; l.pluck.decay = 3.0f; l.pluck.excite = 1;
        l.common.poly = 6; l.common.legato = 1; l.common.glide = 0.12f; l.common.mwVib = 0.4f;
        space (s, Space::Hall, 0.22f);
        L.add (G, "Slide", "Lap Steel", "lap steel hawaiian slide glide", s);
    }
    {
        auto s = electricGuitar (0.2f); auto& l = s.layer[0]; l.pluck.excite = 1; l.pluck.decay = 3.0f; l.pluck.bright = 0.4f;
        mod (l, 1, ModFx::Chorus, 0.3f, 0.3f, 0.3f); echo (s, 9, 0.55f, 0.3f, 3000.0f); space (s, Space::Ambient, 0.45f);
        L.add (G, "Ambient", "Ambient Swell Guitar", "ambient post rock delay wash", s);
    }

    //--------------------------------------------------------------- driven
    {
        auto s = electricGuitar (0.08f); amp (s.layer[0], CrunchAmp, 0.5f, 0.1f, Closed4x12);
        L.add (G, "Electric Drive", "Crunch Rhythm", "crunch marshall rock rhythm acdc", s);
    }
    {
        auto s = electricGuitar (0.14f); amp (s.layer[0], Overdrive, 0.5f, 0.0f, Combo2x12); space (s, Space::Room, 0.16f);
        L.add (G, "Electric Drive", "Blues Overdrive", "blues overdrive tube screamer srv", s);
    }
    {
        auto s = electricGuitar (0.08f); amp (s.layer[0], CrunchAmp, 0.78f, 0.15f, Closed4x12);
        echo (s, 2, 0.3f, 0.18f, 3500.0f); space (s, Space::Hall, 0.15f);
        L.add (G, "Electric Drive", "Classic Rock Lead", "rock lead 70s 80s solo", s);
    }
    {
        auto s = electricGuitar (0.07f); auto& l = s.layer[0]; amp (l, LeadAmp, 0.85f, 0.1f, Closed4x12);
        l.common.poly = 4; l.common.mwVib = 0.5f; echo (s, 3, 0.35f, 0.2f, 3000.0f); space (s, Space::Hall, 0.18f);
        L.add (G, "Electric Drive", "High Gain Lead", "high gain lead shred metal solo", s);
    }
    {
        auto s = electricGuitar (0.07f); auto& l = s.layer[0]; amp (l, LeadAmp, 0.9f, 0.0f, Closed4x12); l.pluck.mute = 0.55f;
        l.common.poly = 6; eq (l, 2.0f, -3.0f, 600.0f, 0.0f, 70.0f);
        L.add (G, "Electric Drive", "Metal Palm Mute", "metal chug palm mute djent", s);
    }
    {
        auto s = electricGuitar (0.08f); auto& l = s.layer[0]; amp (l, LeadAmp, 0.7f, 0.0f, Closed4x12); l.common.strum = 8; l.common.poly = 6;
        L.add (G, "Electric Drive", "Power Chords", "power chords punk rock distortion", s);
    }
    {
        auto s = electricGuitar (0.12f); amp (s.layer[0], Fuzz, 0.7f, 0.1f, Combo2x12);
        L.add (G, "Electric Drive", "Fuzz Guitar", "fuzz face hendrix 60s", s);
    }
    {
        auto s = electricGuitar (0.14f); auto& l = s.layer[0]; amp (l, Fuzz, 0.8f, -0.2f, Closed4x12); mod (l, 1, ModFx::Chorus, 0.3f, 0.5f, 0.4f);
        l.pluck.decay = 3.0f; space (s, Space::Ambient, 0.4f);
        L.add (G, "Electric Drive", "Shoegaze Wall", "shoegaze fuzz wall ambient mbv", s);
    }
    {
        auto s = electricGuitar (0.08f); auto& l = s.layer[0]; amp (l, CrunchAmp, 0.65f, 0.1f, Closed4x12);
        l.common.poly = 1; l.common.legato = 1; l.common.glide = 0.06f; l.common.bendRange = 2; l.pluck.decay = 2.5f; l.common.mwVib = 0.5f;
        echo (s, 3, 0.3f, 0.2f); space (s, Space::Hall, 0.15f);
        L.add (G, "Electric Drive", "Mono Bend Lead", "mono lead legato slide bend", s);
    }
    {
        auto s = electricGuitar (0.1f); auto& l = s.layer[0]; amp (l, Overdrive, 0.4f, 0.0f, Combo2x12); wah (l, 3, 900.0f, 6.0f, 0.9f);
        L.add (G, "Electric Drive", "Wah Pedal Lead", "wah pedal modwheel lead", s);
    }
}

void addBasses (Lib& L)
{
    const char* B = kBasses;

    L.add (B, "Electric", "Finger Bass", "jazz bass precision fingerstyle", bassGuitar());
    {
        auto s = bassGuitar(); auto& p = s.layer[0].pluck; p.excite = 0; p.bright = 0.55f; p.pickNoise = 0.35f;
        L.add (B, "Electric", "Pick Bass", "pick bass rock punk", s);
    }
    {
        auto s = bassGuitar(); auto& l = s.layer[0]; auto& p = l.pluck; p.excite = 2; p.bright = 0.8f; p.pickupPos = 0.1f; p.buzz = 0.15f;
        l.fx.compAmt = 0.5f; eq (l, 3.0f, -4.0f, 500.0f, 4.0f);
        L.add (B, "Electric", "Slap Bass", "slap pop funk", s);
    }
    {
        auto s = bassGuitar(); auto& l = s.layer[0]; auto& p = l.pluck; p.bright = 0.28f; p.damping = 0.4f; p.decay = 1.8f; p.twang = 0.0f;
        l.common.poly = 1; l.common.legato = 1; l.common.glide = 0.09f; l.common.mwVib = 0.4f;
        mod (l, 1, ModFx::Chorus, 0.5f, 0.3f, 0.3f);
        L.add (B, "Electric", "Fretless Bass", "fretless jaco pastorius mwah", s);
    }
    {
        auto s = bassGuitar(); auto& p = s.layer[0].pluck; p.mute = 0.5f; p.bright = 0.3f; p.pickupPos = 0.24f;
        L.add (B, "Electric", "Motown Muted Bass", "motown flatwound muted jamerson", s);
    }
    {
        auto s = bassGuitar(); auto& p = s.layer[0].pluck; p.pickupPos = 0.24f; p.damping = 0.7f; p.bright = 0.3f;
        L.add (B, "Electric", "Vintage P-Bass", "precision vintage flatwound warm", s);
    }
    {
        auto s = bassGuitar(); auto& p = s.layer[0].pluck; p.pickupPos = 0.08f; p.bright = 0.5f;
        L.add (B, "Electric", "Bridge Pickup Bass", "jazz bass bridge growl", s);
    }
    {
        auto s = bassGuitar(); amp (s.layer[0], BassAmp, 0.65f, 0.1f, Bass1x15);
        L.add (B, "Electric", "Rock Bass Drive", "rock bass overdrive amp", s);
    }
    {
        auto s = bassGuitar(); auto& l = s.layer[0]; l.fx.compAmt = 0.6f; l.pluck.bright = 0.5f; eq (l, 2.0f, -2.0f, 700.0f, 2.0f);
        L.add (B, "Electric", "Funk Bass Comp", "funk bass compressed", s);
    }
    {
        auto s = bassGuitar(); auto& p = s.layer[0].pluck; p.bright = 0.6f; p.stiffness = 0.35f; s.layer[0].fx.compAmt = 0.4f;
        L.add (B, "Electric", "Modern 5-String", "modern active 5 string", s);
    }
    {
        auto s = bassGuitar(); amp (s.layer[0], Fuzz, 0.6f, 0.0f, Bass1x15, -2.0f);
        L.add (B, "Electric", "Fuzz Bass", "fuzz bass stoner", s);
    }
    {
        auto s = bassGuitar(); auto& l = s.layer[0]; wah (l, 1, 500.0f, 5.0f, 0.8f);
        L.add (B, "Electric", "Envelope Filter Bass", "mutron envelope filter funk bass", s);
    }

    //--------------------------------------------------------------- acoustic basses
    {
        auto s = make (Engine::Pluck); auto& l = s.layer[0]; auto& p = l.pluck;
        l.common.poly = 6;
        p.excite = 1; p.bright = 0.25f; p.pluckPos = 0.25f; p.decay = 0.9f; p.damping = 0.6f; p.stiffness = 0.2f;
        p.body = 9; p.bodyMix = 0.75f; p.twang = 6.0f; p.release = 0.12f; p.relNoise = 0.2f;
        space (s, Space::Room, 0.14f);
        L.add (B, "Upright", "Upright Bass", "double bass upright jazz pizzicato", s);
    }
    {
        auto s = make (Engine::Pluck); auto& l = s.layer[0]; auto& p = l.pluck;
        l.common.poly = 6;
        p.excite = 1; p.bright = 0.35f; p.pluckPos = 0.2f; p.decay = 1.1f; p.damping = 0.55f; p.stiffness = 0.2f;
        p.body = 9; p.bodyMix = 0.6f; p.twang = 9.0f; p.release = 0.15f; p.relNoise = 0.35f; p.buzz = 0.1f;
        space (s, Space::Chamber, 0.16f);
        L.add (B, "Upright", "Jazz Upright Pizz", "upright slap jazz walking", s);
    }
    {
        auto s = bassGuitar(); auto& l = s.layer[0]; l.pluck.pickupPos = 0.0f; l.pluck.body = 2; l.pluck.bodyMix = 0.6f;
        amp (l, AmpOff, 0.0f);
        L.add (B, "Acoustic", "Acoustic Bass Guitar", "acoustic bass guitar", s);
    }

    //--------------------------------------------------------------- key basses
    {
        auto s = tineEP(); auto& l = s.layer[0]; auto& e = l.ep; l.common.poly = 4; e.decay = 1.4f; e.bell = 0.15f; e.drive = 0.55f;
        e.width = 0.0f; amp (l, Tube, 0.3f);
        L.add (B, "Key Bass", "Tine Key Bass", "rhodes piano bass doors ray manzarek", s);
    }
    {
        auto s = clavinet(); auto& p = s.layer[0].pluck; p.pickupPos = 0.12f; p.pickup2Pos = 0.0f; p.decay = 0.9f;
        s.layer[0].common.poly = 4;
        L.add (B, "Key Bass", "Clav Bass", "clavinet bass funk", s);
    }
    {
        auto s = tonewheel ("880000000"); auto& l = s.layer[0]; l.organ.click = 0.6f; l.organ.vibrato = 0; mod (l, 1, ModFx::Off, 0, 0, 0);
        l.common.poly = 4;
        L.add (B, "Key Bass", "Organ Pedal Bass", "organ pedal bass jazz", s);
    }
}
} // namespace kv::banks
