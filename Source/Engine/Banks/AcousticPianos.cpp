// Acoustic pianos: grands, uprights, felt, electric grands, vintage & toy pianos.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

static SoundData toyPiano (float ring, float metal)
{
    auto s = mallet ({ 1.0f, 3.91f, 9.3f, 2.02f, 5.85f }, { 0, -7, -14, -16 - 10 * (1 - metal), -22 }, { 1.0f, 0.45f, 0.2f, 0.7f, 0.3f },
                     1.1f * ring, 0.75f, 0.35f, 3500.0f);
    auto& m = s.layer[0].mallet;
    m.beat = 2.5f; m.beatMix = 0.35f; m.damped = 1; m.release = 0.35f; m.decayTrack = 0.8f;
    s.layer[0].common.transpose = 12;
    space (s, Space::Room, 0.14f);
    return s;
}

void addAcousticPianos (Lib& L)
{
    const char* A = kAcoustic;

    //--------------------------------------------------------------- grands
    L.add (A, "Grand", "Concert Grand", "steinway classical hall concert", grand());
    {
        auto s = grand(); auto& p = s.layer[0].piano;
        p.width = 0.85f; p.mechNoise = 0.15f; space (s, Space::Studio, 0.08f);
        L.add (A, "Grand", "Concert Grand Close", "close mic player position", s);
    }
    {
        auto s = grand(); space (s, Space::Hall, 0.3f); s.gfx.revDecay = 2.9f;
        L.add (A, "Grand", "Grand in the Hall", "hall concert classical wet", s);
    }
    {
        auto s = grand(); space (s, Space::Cathedral, 0.36f);
        L.add (A, "Grand", "Cathedral Grand", "church huge reverb", s);
    }
    {
        auto s = grand(); auto& l = s.layer[0]; auto& p = l.piano;
        p.hardness = 0.72f; p.bodyTone = 0.4f; p.unison = 0.9f;
        eq (l, 1.0f, 0.0f, 1000.0f, 3.0f); l.fx.compAmt = 0.3f; space (s, Space::Room, 0.12f);
        L.add (A, "Grand", "Bright Pop Grand", "pop bright modern mix", s);
    }
    {
        auto s = grand(); auto& l = s.layer[0]; auto& p = l.piano;
        p.hardness = 0.82f; p.bodyTone = 0.5f; p.unison = 1.2f; p.velSens = 0.45f;
        l.fx.compAmt = 0.5f; eq (l, 1.5f, 2.0f, 2200.0f, 2.0f, 40.0f); space (s, Space::Room, 0.1f);
        L.add (A, "Grand", "Rock Piano", "rock bright compressed band", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano;
        p.hardness = 0.38f; p.bodyTone = -0.15f; p.decay = 1.15f; p.velSens = 0.7f; p.pedalRes = 0.55f;
        space (s, Space::Hall, 0.25f);
        L.add (A, "Grand", "Ballad Grand", "ballad soft warm emotional", s);
    }
    {
        auto s = grand(); auto& l = s.layer[0]; auto& p = l.piano;
        p.hardness = 0.25f; p.bodyTone = -0.6f; p.decay = 1.3f; p.pedalRes = 0.7f;
        eq (l, 2.0f, -2.0f, 2500.0f, -3.0f, 30.0f, 7000.0f); space (s, Space::Cathedral, 0.4f);
        L.add (A, "Grand", "Dark Cinematic Grand", "film score dark soft ambient", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano;
        p.hardness = 0.55f; p.unison = 1.0f; p.bodyTone = 0.1f; space (s, Space::Chamber, 0.16f);
        L.add (A, "Grand", "Jazz Club Grand", "jazz club trio", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano;
        p.hardness = 0.66f; p.bodyTone = 0.25f; p.hammerPos = 0.115f; p.width = 0.75f; p.unison = 0.6f;
        space (s, Space::Studio, 0.1f);
        L.add (A, "Grand", "LA Studio Grand", "c7 yamaha studio pop record", s);
    }
    {
        auto s = grand(); auto& l = s.layer[0]; auto& p = l.piano;
        p.hardness = 0.42f; p.bodyTone = -0.25f; p.bodySize = 1.0f; p.decay = 1.2f; p.aftersound = 0.5f;
        eq (l, 2.0f, 0.0f, 1000.0f, -1.0f); space (s, Space::Hall, 0.2f);
        L.add (A, "Grand", "Vienna Imperial Grand", "bosendorfer warm deep bass", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano;
        p.hardness = 0.55f; p.bodyTone = 0.05f; p.decay = 1.1f; space (s, Space::Hall, 0.18f);
        L.add (A, "Grand", "Hamburg D Grand", "steinway concert d", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano;
        p.bodySize = 0.5f; p.inharm = 1.45f; p.decay = 0.8f; p.hardness = 0.52f; space (s, Space::Room, 0.14f);
        L.add (A, "Grand", "Baby Grand", "small grand living room", s);
    }
    {
        auto s = grand(); space (s, Space::Hall, 0.28f); s.gfx.revPre = 32;
        s.layer[0].piano.velSens = 0.75f;
        L.add (A, "Grand", "Classical Recital", "classical chopin debussy dynamics", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano;
        p.pedalRes = 0.95f; p.aftersound = 0.65f; p.decay = 1.25f; space (s, Space::Hall, 0.2f);
        L.add (A, "Grand", "Resonant Grand", "sympathetic resonance pedal bloom", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano;
        p.hardness = 0.33f; p.mechNoise = 0.3f; p.releaseNoise = 0.45f; p.width = 0.85f; space (s, Space::Studio, 0.06f);
        L.add (A, "Grand", "Intimate Grand", "soft close quiet singer songwriter", s);
    }
    {
        auto s = grand(); auto& l = s.layer[0]; l.piano.hardness = 0.62f; l.fx.compAmt = 0.25f; space (s, Space::Stage, 0.14f);
        L.add (A, "Grand", "Stage Grand", "live stage band", s);
    }
    {
        auto s = grand(); s.layer[0].piano.width = 0.0f; s.gfx.width = 0.0f; space (s, Space::Room, 0.1f);
        L.add (A, "Grand", "Mono Grand", "mono mix", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano; p.hardness = 0.4f; p.width = 0.9f;
        echo (s, 3, 0.35f, 0.18f, 3500.0f); space (s, Space::Ambient, 0.35f);
        L.add (A, "Grand", "Ambient Grand", "ambient delay wide atmospheric", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano; p.hardness = 0.45f;
        lofi (s, 11, 20, 0.35f, 0.25f, 5200); space (s, Space::Room, 0.18f);
        L.add (A, "Grand", "Lo-Fi Grand", "lofi hip hop vinyl dusty", s);
    }
    {
        auto s = grand(); auto& l = s.layer[0]; amp (l, Tape, 0.45f); lofi (s, 24, 48, 0.08f, 0.15f, 9000);
        space (s, Space::Room, 0.15f);
        L.add (A, "Grand", "Tape Grand", "tape saturation warm vintage", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano; p.hardness = 0.6f; p.unison = 1.6f; p.bodyTone = 0.15f;
        auto& l = s.layer[0]; mod (l, 1, ModFx::Chorus, 0.35f, 0.25f, 0.25f); space (s, Space::Plate, 0.2f);
        L.add (A, "Grand", "Shiny Chorus Grand", "chorus 80s 90s pop", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano; p.hardness = 0.7f; p.unison = 0.3f; p.inharm = 0.6f; p.stretch = 1.3f;
        space (s, Space::Hall, 0.18f);
        L.add (A, "Grand", "Pure Tuned Grand", "precise tuned clean modern", s);
    }

    //--------------------------------------------------------------- uprights
    L.add (A, "Upright", "Upright Piano", "upright home", upright());
    {
        auto s = upright(); auto& p = s.layer[0].piano; p.hardness = 0.7f; p.bodyTone = 0.3f; s.layer[0].fx.compAmt = 0.2f;
        L.add (A, "Upright", "Bright Upright", "pop upright bright", s);
    }
    {
        auto s = upright(); auto& l = s.layer[0]; auto& p = l.piano;
        p.unison = 3.0f; p.inharm = 3.0f; p.hfDamp = 0.95f; p.mechNoise = 0.4f;
        eq (l, 0.0f, 2.0f, 900.0f, -3.0f, 60.0f, 9000.0f); space (s, Space::Room, 0.16f);
        L.add (A, "Upright", "Vintage Upright", "old vintage warm parlour", s);
    }
    {
        auto s = upright(); auto& p = s.layer[0].piano; p.unison = 14.0f; p.hardness = 0.65f;
        L.add (A, "Upright", "Honky Tonk", "honky tonk detuned western", s);
    }
    {
        auto s = upright(); auto& p = s.layer[0].piano; p.unison = 20.0f; p.tack = 0.25f; p.hardness = 0.75f;
        lofi (s, 24, 48, 0.12f, 0.0f, 12000);
        L.add (A, "Upright", "Saloon Piano", "saloon ragtime western detuned", s);
    }
    {
        auto s = upright(); auto& p = s.layer[0].piano; p.tack = 0.8f; p.hardness = 0.85f; p.unison = 4.0f;
        L.add (A, "Upright", "Tack Piano", "tack thumbtack ragtime metallic", s);
    }
    {
        auto s = upright(); auto& p = s.layer[0].piano; p.tack = 0.55f; p.hardness = 0.7f; p.unison = 2.5f; p.bodySize = 0.4f;
        space (s, Space::Studio, 0.1f);
        L.add (A, "Upright", "Wing Tack Upright", "tack studio indie", s);
    }
    {
        auto s = upright(); auto& l = s.layer[0]; l.piano.hardness = 0.62f; l.fx.compAmt = 0.35f; eq (l, 0.5f, 0.0f, 1000.0f, 2.0f);
        L.add (A, "Upright", "Pop Upright", "pop indie singer songwriter", s);
    }
    {
        auto s = upright(); auto& p = s.layer[0].piano; p.felt = 0.55f; p.mechNoise = 0.55f; p.hardness = 0.35f;
        L.add (A, "Upright", "Muted Upright", "moderator felt practice pedal", s);
    }
    {
        auto s = upright(); auto& p = s.layer[0].piano; p.unison = 9.0f; p.stretch = 2.0f; p.inharm = 4.5f; p.decay = 0.5f;
        lofi (s, 24, 48, 0.1f, 0.12f, 8000);
        L.add (A, "Upright", "Broken Upright", "detuned broken old character", s);
    }
    {
        auto s = upright(); auto& p = s.layer[0].piano; p.hardness = 0.5f;
        lofi (s, 12, 24, 0.4f, 0.3f, 4800); space (s, Space::Room, 0.2f);
        L.add (A, "Upright", "Dusty Upright", "lofi hip hop chill vinyl", s);
    }
    {
        auto s = upright(); auto& p = s.layer[0].piano; p.bodySize = 0.0f; p.inharm = 4.0f; p.decay = 0.4f; p.hardness = 0.6f; p.unison = 2.0f;
        L.add (A, "Mini", "Mini Student Piano", "small spinet butterfly student", s);
    }

    //--------------------------------------------------------------- felt
    L.add (A, "Felt", "Felt Piano", "felt soft intimate", feltPiano());
    {
        auto s = feltPiano(); auto& p = s.layer[0].piano; p.mechNoise = 0.95f; p.width = 0.9f; space (s, Space::Studio, 0.05f);
        L.add (A, "Felt", "Felt Close & Noisy", "felt close mechanics noise intimate", s);
    }
    {
        auto s = feltPiano(); s.layer[0].piano.felt = 0.9f; echo (s, 1, 0.3f, 0.16f, 2500.0f); space (s, Space::Ambient, 0.35f);
        L.add (A, "Felt", "Felt Dream", "felt ambient dreamy delay", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano; p.felt = 0.7f; p.hardness = 0.3f; p.mechNoise = 0.4f;
        space (s, Space::Hall, 0.18f);
        L.add (A, "Felt", "Double Felt Grand", "felt grand soft", s);
    }
    {
        auto s = feltPiano(); auto& p = s.layer[0].piano; p.felt = 0.95f; p.hardness = 0.15f;
        lofi (s, 24, 48, 0.15f, 0.12f, 8000); space (s, Space::Room, 0.22f);
        L.add (A, "Felt", "Felt Lullaby", "lullaby soft tape felt", s);
    }
    {
        auto s = feltPiano(); auto& p = s.layer[0].piano; p.felt = 0.75f; p.unison = 2.5f; p.decay = 0.7f;
        lofi (s, 24, 48, 0.25f, 0.35f, 6500);
        L.add (A, "Felt", "Felt Cassette", "felt tape wobble lofi", s);
    }

    //--------------------------------------------------------------- electric grands
    L.add (A, "Electric Grand", "Electric Grand CP", "cp70 cp80 yamaha 80s", electricGrand());
    {
        auto s = electricGrand(); auto& l = s.layer[0]; l.fx.m1Depth = 0.6f; l.fx.m1Mix = 0.5f; space (s, Space::Hall, 0.22f);
        L.add (A, "Electric Grand", "CP Ballad Chorus", "80s ballad chorus cp80", s);
    }
    {
        auto s = electricGrand(); auto& l = s.layer[0]; mod (l, 1, ModFx::Phaser, 0.3f, 0.6f, 0.6f, 0.35f);
        L.add (A, "Electric Grand", "CP Phaser", "phaser 70s", s);
    }
    {
        auto s = electricGrand(); auto& l = s.layer[0]; mod (l, 1, ModFx::Off, 0, 0, 0); amp (l, Tube, 0.35f, 0.1f, EPSpeaker);
        space (s, Space::Room, 0.1f);
        L.add (A, "Electric Grand", "CP Dry Stage", "cp70 stage dry", s);
    }
    {
        auto s = grand(); auto& l = s.layer[0]; auto& p = l.piano;
        p.piezo = 0.8f; p.inharm = 1.6f; p.decay = 0.6f; p.body = 0.1f; p.hardness = 0.6f;
        amp (l, Tube, 0.4f, 0.0f, EPSpeaker); mod (l, 1, ModFx::Tremolo, 5.0f, 0.25f, 1.0f);
        L.add (A, "Electric Grand", "Electra Piano", "hohner electra string piano", s);
    }

    //--------------------------------------------------------------- special
    {
        auto s = grand(); auto& p = s.layer[0].piano; p.prep = 0.8f; p.hardness = 0.6f; space (s, Space::Room, 0.18f);
        L.add (A, "Prepared", "Prepared Piano", "cage prepared bolts experimental", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano; p.prep = 0.45f; p.tack = 0.35f; p.decay = 0.7f; space (s, Space::Room, 0.14f);
        L.add (A, "Prepared", "Prepared Screws", "prepared metallic percussive", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano; p.prep = 0.25f; p.hardness = 0.9f; p.decay = 1.3f; p.inharm = 2.0f;
        space (s, Space::Hall, 0.25f);
        L.add (A, "Prepared", "Glass Piano", "glass bright crystal", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano;
        p.hammerPos = 0.1f; p.hardness = 0.62f; p.unison = 1.5f; p.bodySize = 0.35f; p.decay = 0.55f; p.inharm = 1.6f;
        p.hfDamp = 0.8f; p.bodyTone = 0.2f; p.mechNoise = 0.3f; space (s, Space::Chamber, 0.18f);
        L.add (A, "Historical", "Fortepiano 1800", "fortepiano mozart beethoven historical", s);
    }
    {
        auto s = grand(); auto& p = s.layer[0].piano; p.decay = 3.5f; p.release = 2.0f; p.pedalRes = 1.0f;
        lofi (s, 24, 48, 0.0f, 0.12f, 9000); space (s, Space::Ambient, 0.55f);
        L.add (A, "Grand", "Frozen Grand", "infinite sustain ambient frozen", s);
    }

    //--------------------------------------------------------------- toy pianos (rods)
    L.add (A, "Toy", "Toy Piano", "toy kids metallic rods", toyPiano (1.0f, 0.6f));
    L.add (A, "Toy", "Toy Piano Glock", "toy glockenspiel bright", toyPiano (1.4f, 1.0f));
    {
        auto s = toyPiano (0.8f, 0.3f); lofi (s, 24, 48, 0.2f, 0.3f, 7000);
        L.add (A, "Toy", "Haunted Toy Piano", "toy creepy horror lofi", s);
    }
    {
        auto s = toyPiano (1.2f, 0.8f); s.layer[0].common.transpose = 0;
        L.add (A, "Toy", "Grand Toy Piano", "toy grand big low", s);
    }
}
} // namespace kv::banks
