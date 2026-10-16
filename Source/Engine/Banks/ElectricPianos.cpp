// Electric pianos: tine (Rhodes-style), reed (Wurlitzer-style), Pianet,
// Claviset/Cembalet, tine bass and FM/digital electric pianos.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

void addElectricPianos (Lib& L)
{
    const char* E = kElectric;

    //--------------------------------------------------------------- tine
    L.add (E, "Tine", "Stage Tine 73", "rhodes mark i stage classic 70s", tineEP());
    {
        auto s = tineEP(); auto& e = s.layer[0].ep; e.bark = 0.65f; e.drive = 0.5f; e.thump = 0.4f;
        L.add (E, "Tine", "Mark I Classic", "rhodes mark 1 warm bark", s);
    }
    {
        auto s = tineEP(); auto& e = s.layer[0].ep; e.hardness = 0.62f; e.bell = 0.58f; e.offset = 0.38f;
        eq (s.layer[0], 0.0f, 0.0f, 1000.0f, 2.0f);
        L.add (E, "Tine", "Mark II Bright", "rhodes mark 2 bright", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; mod (l, 1, ModFx::AutoPan, 4.5f, 0.75f, 1.0f, 0.2f);
        amp (l, Tube, 0.25f, 0.0f, EPSpeaker);
        L.add (E, "Tine", "Suitcase Tremolo", "rhodes suitcase stereo vibrato tremolo pan", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; mod (l, 1, ModFx::AutoPan, 1.2f, 0.6f, 1.0f);
        space (s, Space::Room, 0.16f);
        L.add (E, "Tine", "Suitcase Slow Pan", "rhodes suitcase slow pan soul", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; auto& e = l.ep;
        e.drive = 0.82f; e.bark = 0.92f; e.offset = 0.55f; e.hardness = 0.6f; amp (l, Tube, 0.55f, 0.1f, EPSpeaker);
        L.add (E, "Tine", "Barking Tine", "rhodes bark growl aggressive", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; auto& e = l.ep;
        e.offset = 0.22f; e.bell = 0.82f; e.hardness = 0.75f; e.drive = 0.4f;
        eq (l, 0.0f, -2.0f, 600.0f, 4.0f); mod (l, 1, ModFx::Chorus, 0.6f, 0.35f, 0.4f);
        L.add (E, "Tine", "Dyno Bell Tine", "dyno my piano bright bell 80s", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; auto& e = l.ep;
        e.hardness = 0.3f; e.bell = 0.3f; e.drive = 0.3f; e.bark = 0.3f;
        mod (l, 1, ModFx::Chorus, 0.4f, 0.25f, 0.3f); space (s, Space::Hall, 0.22f);
        L.add (E, "Tine", "Soft Ballad Tine", "rhodes soft ballad mellow", s);
    }
    {
        auto s = tineEP(); mod (s.layer[0], 1, ModFx::Chorus, 0.65f, 0.55f, 0.5f);
        L.add (E, "Tine", "Tine Chorus", "rhodes chorus", s);
    }
    {
        auto s = tineEP(); mod (s.layer[0], 1, ModFx::Phaser, 0.4f, 0.7f, 0.7f, 0.4f);
        L.add (E, "Tine", "Tine Phaser 70s", "rhodes phaser small stone", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0];
        mod (l, 1, ModFx::Phaser, 0.3f, 0.6f, 0.6f, 0.3f); mod (l, 2, ModFx::AutoPan, 3.5f, 0.5f, 1.0f);
        L.add (E, "Tine", "Tine Phase & Pan", "rhodes phaser tremolo pan", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; auto& e = l.ep;
        e.hardness = 0.48f; e.drive = 0.55f; e.bark = 0.5f; l.fx.compAmt = 0.3f;
        mod (l, 1, ModFx::AutoPan, 3.2f, 0.35f, 1.0f); space (s, Space::Room, 0.12f);
        L.add (E, "Tine", "Neo Soul Tine", "neo soul rnb gospel", s);
    }
    {
        auto s = tineEP(); auto& e = s.layer[0].ep; e.hardness = 0.4f;
        lofi (s, 12, 24, 0.35f, 0.3f, 6000); space (s, Space::Room, 0.15f);
        L.add (E, "Tine", "Lo-Fi Dusty Tine", "lofi hip hop chill rhodes", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; amp (l, Overdrive, 0.45f, 0.0f, Combo2x12, -2.0f);
        L.add (E, "Tine", "Overdriven Tine", "rhodes overdrive distortion rock", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; amp (l, CleanAmp, 0.45f, 0.15f, Combo2x12); space (s, Space::Spring, 0.18f);
        L.add (E, "Tine", "Tine Through Amp", "rhodes twin amp spring", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; wah (l, 1, 700.0f, 6.0f, 0.75f); amp (l, CleanAmp, 0.35f, 0.0f, Open1x12);
        L.add (E, "Tine", "Tine Funk Wah", "rhodes auto wah envelope funk", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; auto& e = l.ep;
        e.bell = 0.92f; e.bellRatio = 7.0f; e.offset = 0.15f; e.hardness = 0.7f;
        mod (l, 1, ModFx::Chorus, 0.45f, 0.4f, 0.45f); space (s, Space::Plate, 0.25f);
        L.add (E, "Tine", "Glassy Tine", "glass bell shimmer rhodes", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; auto& e = l.ep;
        e.hardness = 0.55f; e.offset = 0.55f; e.bark = 0.5f; e.decay = 1.2f; e.toneBar = 0.55f;
        eq (l, 1.0f, 0.0f, 1000.0f, 2.0f);
        L.add (E, "Tine", "Modern Vibe Tine", "vintage vibe modern tine", s);
    }
    {
        auto s = tineEP(); auto& e = s.layer[0].ep; e.offset = 0.7f; e.hardness = 0.35f; e.bell = 0.25f; e.drive = 0.35f;
        L.add (E, "Tine", "Mellow 77 Tine", "rhodes 1977 mellow warm", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; amp (l, CrunchAmp, 0.45f, 0.1f, Closed4x12, -3.0f);
        L.add (E, "Tine", "Hard Rock Tine", "rhodes crunch rock amp", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; mod (l, 1, ModFx::Rotary, 0.8f, 0.6f, 1.0f); amp (l, OrganAmp, 0.3f);
        L.add (E, "Tine", "Rotary Tine", "rhodes leslie rotary", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; mod (l, 1, ModFx::Chorus, 0.5f, 0.3f, 0.35f);
        echo (s, 6, 0.35f, 0.22f, 3500.0f); space (s, Space::Hall, 0.28f);
        L.add (E, "Tine", "Tine Dream Delay", "rhodes dreamy delay ambient", s);
    }
    {
        auto s = tineEP(); amp (s.layer[0], AmpOff, 0.0f); space (s, Space::Studio, 0.05f);
        L.add (E, "Tine", "Studio Tine DI", "rhodes direct dry studio", s);
    }
    {
        auto s = tineEP(); auto& l = s.layer[0]; auto& e = l.ep;
        l.common.keyHi = 64; e.decay = 1.4f; e.bell = 0.2f; e.drive = 0.5f; e.width = 0.0f;
        L.add (E, "Tine Bass", "Tine Piano Bass", "rhodes piano bass doors", s);
    }

    //--------------------------------------------------------------- reed
    L.add (E, "Reed", "Reed 200A", "wurlitzer 200a classic", reedEP());
    {
        auto s = reedEP(); s.layer[0].fx.m1Depth = 0.55f;
        L.add (E, "Reed", "Reed 200A Tremolo", "wurlitzer tremolo vibrato", s);
    }
    {
        auto s = reedEP(); auto& l = s.layer[0]; auto& e = l.ep; e.hardness = 0.78f; e.drive = 0.62f; e.bell = 0.35f;
        eq (l, 0.0f, 3.0f, 1500.0f, 3.0f);
        L.add (E, "Reed", "Reed 140B Bright", "wurlitzer 140b bright early", s);
    }
    {
        auto s = reedEP(); auto& l = s.layer[0]; auto& e = l.ep; e.drive = 0.88f; e.bark = 0.95f; amp (l, Tube, 0.55f, 0.0f, EPSpeaker);
        L.add (E, "Reed", "Reed Growl", "wurlitzer growl bark dirty", s);
    }
    {
        auto s = reedEP(); auto& e = s.layer[0].ep; e.hardness = 0.45f; e.drive = 0.35f; e.bark = 0.4f; space (s, Space::Room, 0.18f);
        L.add (E, "Reed", "Soft Pop Reed", "wurlitzer soft pop indie", s);
    }
    {
        auto s = reedEP(); mod (s.layer[0], 2, ModFx::Chorus, 0.6f, 0.45f, 0.45f);
        L.add (E, "Reed", "Reed Chorus", "wurlitzer chorus", s);
    }
    {
        auto s = reedEP(); lofi (s, 12, 22, 0.3f, 0.3f, 5500);
        L.add (E, "Reed", "Lo-Fi Reed", "wurlitzer lofi tape", s);
    }
    {
        auto s = reedEP(); auto& l = s.layer[0]; auto& e = l.ep; e.drive = 0.72f; l.fx.compAmt = 0.35f;
        mod (l, 1, ModFx::Off, 0, 0, 0); mod (l, 2, ModFx::Dimension, 0.5f, 0.5f, 0.5f);
        L.add (E, "Reed", "Reed Rock 70s", "wurlitzer supertramp rock", s);
    }
    {
        auto s = reedEP(); mod (s.layer[0], 2, ModFx::Phaser, 0.35f, 0.65f, 0.6f, 0.35f);
        L.add (E, "Reed", "Reed Phaser", "wurlitzer phaser", s);
    }
    {
        auto s = reedEP(); auto& l = s.layer[0]; amp (l, CrunchAmp, 0.35f, 0.0f, Combo2x12, -2.0f);
        L.add (E, "Reed", "Reed Amp Crunch", "wurlitzer amp overdrive", s);
    }
    {
        auto s = reedEP(); auto& l = s.layer[0]; mod (l, 1, ModFx::Tremolo, 6.2f, 0.65f, 1.0f, 0.4f); space (s, Space::Spring, 0.2f);
        L.add (E, "Reed", "Reed Surf Trem", "wurlitzer deep tremolo spring", s);
    }

    //--------------------------------------------------------------- pianet / claviset
    L.add (E, "Pianet", "Pianet T", "hohner pianet t beatles", pianet());
    {
        auto s = pianet(); auto& e = s.layer[0].ep; e.pickup = 1; e.hardness = 0.72f; e.drive = 0.4f; e.offset = 0.4f;
        L.add (E, "Pianet", "Pianet N", "hohner pianet n bright", s);
    }
    {
        auto s = pianet(); amp (s.layer[0], Fuzz, 0.35f, 0.0f, Combo2x12, -4.0f);
        L.add (E, "Pianet", "Pianet Fuzz", "pianet fuzz 60s garage", s);
    }
    {
        auto s = pianet(); mod (s.layer[0], 1, ModFx::Tremolo, 5.5f, 0.5f, 1.0f);
        L.add (E, "Pianet", "Pianet Tremolo", "pianet tremolo", s);
    }
    {
        auto s = pianet(); auto& e = s.layer[0].ep; e.bell = 0.45f; e.hardness = 0.9f; e.drive = 0.5f; e.decay = 0.3f;
        amp (s.layer[0], Tube, 0.3f, 0.2f, EPSpeaker);
        L.add (E, "Pianet", "Claviset", "weltmeister claviset east german", s);
    }
    {
        auto s = pianet(); auto& e = s.layer[0].ep; e.pickup = 1; e.bell = 0.4f; e.hardness = 0.95f; e.decay = 0.25f; e.drive = 0.55f;
        L.add (E, "Pianet", "Cembalet", "hohner cembalet", s);
    }
    {
        auto s = pianet(); auto& l = s.layer[0]; mod (l, 1, ModFx::Vibrato, 4.5f, 0.15f, 1.0f); mod (l, 2, ModFx::Chorus, 0.5f, 0.4f, 0.4f);
        L.add (E, "Pianet", "Vibanet", "pianet vibrato chorus", s);
    }
    {
        auto s = reedEP(); auto& l = s.layer[0]; auto& e = l.ep; l.common.keyHi = 60; e.decay = 0.8f; e.width = 0.0f; e.drive = 0.6f;
        mod (l, 1, ModFx::Off, 0, 0, 0);
        L.add (E, "Reed Bass", "Basset Reed Bass", "weltmeister basset bass", s);
    }

    //--------------------------------------------------------------- FM / digital
    L.add (E, "FM", "DX E.Piano", "dx7 fm 80s classic tine", dxEP());
    {
        auto s = dxEP (0.7f, 0.78f, 0.75f, 5.0f);
        L.add (E, "FM", "DX Full Tines", "dx7 bright bell 80s", s);
    }
    {
        auto s = dxEP (0.45f, 0.62f, 0.55f, 2.0f); s.layer[0].fx.m1Depth = 0.45f; s.layer[0].fx.m1Mix = 0.45f; space (s, Space::Hall, 0.25f);
        L.add (E, "FM", "DX Ballad", "dx7 soft ballad chorus", s);
    }
    {
        auto a = dxEP (0.62f, 0.72f, 0.66f, 4.0f), b = dxEP (0.55f, 0.68f, 0.62f, -4.0f);
        b.layer[0].common.fine = 7; b.layer[0].common.pan = 0.4f; a.layer[0].common.pan = -0.4f; a.layer[0].common.fine = -7;
        auto s = stack (a, b, -2.0f, -2.0f);
        L.add (E, "FM", "TX Stacked EP", "tx816 stacked detuned wide", s);
    }
    {
        auto s = dxEP (0.78f, 0.5f, 0.8f, 3.0f); auto& f = s.layer[0].fm;
        f.op[3].ratio = 7.0f; f.op[5].ratio = 3.5f; f.op[4].D = 4.0f;
        space (s, Space::Plate, 0.22f);
        L.add (E, "FM", "FM Bell EP", "fm bell electric piano glass", s);
    }
    {
        auto s = dxEP (0.55f, 0.58f, 0.82f, 6.0f); auto& f = s.layer[0].fm;
        f.op[5].ratio = 4.0f; f.op[5].D = 2.5f;
        mod (s.layer[0], 1, ModFx::Chorus, 0.7f, 0.6f, 0.5f); space (s, Space::Hall, 0.3f);
        L.add (E, "Digital", "Crystal EP", "jd800 crystal digital 90s", s);
    }
    {
        auto s = dxEP (0.4f, 0.66f, 0.45f, 2.0f); auto& f = s.layer[0].fm; f.feedback = 0.1f;
        mod (s.layer[0], 1, ModFx::Chorus, 0.4f, 0.4f, 0.35f);
        L.add (E, "Digital", "MK Digital EP", "mk80 digital rhodes 80s", s);
    }
    {
        auto s = dxEP (0.6f, 0.7f, 0.6f, 3.0f); mod (s.layer[0], 1, ModFx::Phaser, 0.3f, 0.6f, 0.5f, 0.3f);
        L.add (E, "FM", "DX Phaser EP", "dx7 phaser", s);
    }
    {
        auto s = dxEP (0.66f, 0.74f, 0.7f, 3.0f); auto& l = s.layer[0]; l.fx.compAmt = 0.3f;
        mod (l, 2, ModFx::AutoPan, 2.5f, 0.35f, 1.0f); echo (s, 6, 0.3f, 0.18f, 4000.0f);
        L.add (E, "FM", "City Pop EP", "city pop 80s japan dx", s);
    }
}
} // namespace kv::banks
