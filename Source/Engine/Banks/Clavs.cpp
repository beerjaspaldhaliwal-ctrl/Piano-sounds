// Clavinets, harpsichords, clavichord, spinet, virginal and other plucked /
// struck keyboard instruments.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

void addClavs (Lib& L)
{
    const char* C = kClavs;

    //--------------------------------------------------------------- clavinet
    L.add (C, "Clavinet", "Clavinet D6", "hohner clavinet funk stevie", clavinet());
    {
        auto s = clavinet(); wah (s.layer[0], 1, 750.0f, 7.0f, 0.8f);
        L.add (C, "Clavinet", "Clav Funk Wah", "clavinet auto wah envelope funk", s);
    }
    {
        auto s = clavinet(); auto& l = s.layer[0]; wah (l, 1, 650.0f, 8.0f, 0.9f); amp (l, Tube, 0.45f, 0.2f); l.fx.compAmt = 0.35f;
        L.add (C, "Clavinet", "Superstitious Clav", "clavinet superstition funk wah", s);
    }
    {
        auto s = clavinet(); mod (s.layer[0], 1, ModFx::Phaser, 0.5f, 0.7f, 0.6f, 0.4f);
        L.add (C, "Clavinet", "Clav Phaser", "clavinet phaser 70s", s);
    }
    {
        auto s = clavinet(); auto& p = s.layer[0].pluck; p.mute = 0.6f; p.relNoise = 0.7f;
        L.add (C, "Clavinet", "Clav Muted", "clavinet mute damper", s);
    }
    {
        auto s = clavinet(); auto& l = s.layer[0]; l.pluck.pickup2Mix = 0.0f; eq (l, -2.0f, 0.0f, 1000.0f, 5.0f);
        L.add (C, "Clavinet", "Clav Brilliant", "clavinet bright bridge pickup", s);
    }
    {
        auto s = clavinet(); auto& l = s.layer[0]; l.pluck.pickupPos = 0.15f; l.pluck.pickup2Pos = 0.0f; eq (l, 2.0f, 0.0f, 1000.0f, -3.0f);
        L.add (C, "Clavinet", "Clav Soft Neck", "clavinet mellow neck", s);
    }
    {
        auto s = clavinet(); amp (s.layer[0], CrunchAmp, 0.4f, 0.1f, Combo2x12, -2.0f);
        L.add (C, "Clavinet", "Clav Overdrive", "clavinet distortion rock", s);
    }
    {
        auto s = clavinet(); mod (s.layer[0], 1, ModFx::Rotary, 6.7f, 0.5f, 1.0f);
        L.add (C, "Clavinet", "Clav Rotary", "clavinet leslie", s);
    }
    {
        auto s = clavinet(); mod (s.layer[0], 1, ModFx::AutoPan, 4.0f, 0.5f, 1.0f); mod (s.layer[0], 2, ModFx::Chorus, 0.5f, 0.3f, 0.35f);
        L.add (C, "Clavinet", "Clav Auto-Pan", "clavinet stereo pan chorus", s);
    }
    {
        auto s = clavinet(); auto& p = s.layer[0].pluck; p.pickup2Mix = -0.6f;
        L.add (C, "Clavinet", "Clav Out of Phase", "clavinet thin out of phase pickups", s);
    }
    {
        auto s = stack (clavinet(), pianet(), 0.0f, -4.0f);
        L.add (C, "Clavinet", "Clavinet-Pianet Duo", "clavinet pianet duo layer", s);
    }

    //--------------------------------------------------------------- harpsichord
    L.add (C, "Harpsichord", "Harpsichord 8' 4'", "harpsichord baroque bach", harpsichord());
    {
        auto s = harpsichord(); s.layer[0].pluck.course = 1;
        L.add (C, "Harpsichord", "Harpsichord 8'", "harpsichord single", s);
    }
    {
        auto s = harpsichord(); auto& p = s.layer[0].pluck; p.course = 3; p.courseOct = 2; p.courseDetune = 3.0f;
        L.add (C, "Harpsichord", "Harpsichord 8' 8' 4'", "harpsichord full double manual", s);
    }
    {
        auto s = harpsichord(); auto& p = s.layer[0].pluck; p.course = 1; p.mute = 0.55f; p.bright = 0.55f;
        L.add (C, "Harpsichord", "Harpsichord Lute Stop", "harpsichord buff lute", s);
    }
    {
        auto s = harpsichord(); space (s, Space::Hall, 0.25f);
        L.add (C, "Harpsichord", "Baroque Hall Harpsichord", "harpsichord hall concert", s);
    }
    {
        auto s = harpsichord(); auto& l = s.layer[0]; l.pluck.pickupPos = 0.1f; l.pluck.body = 0;
        amp (l, Tube, 0.3f, 0.2f); mod (l, 1, ModFx::Chorus, 0.5f, 0.4f, 0.4f); space (s, Space::Plate, 0.18f);
        L.add (C, "Harpsichord", "Electric Harpsichord", "baldwin electric harpsichord 60s", s);
    }
    {
        auto s = harpsichord(); auto& p = s.layer[0].pluck; p.course = 1; p.body = 8; p.bright = 0.9f; p.decay = 0.6f;
        L.add (C, "Harpsichord", "Spinet", "spinet small bright", s);
    }
    {
        auto s = harpsichord(); auto& p = s.layer[0].pluck; p.course = 1; p.body = 8; p.pluckPos = 0.22f; p.bright = 0.7f;
        L.add (C, "Harpsichord", "Virginal", "virginal renaissance muselar", s);
    }
    {
        auto s = harpsichord(); lofi (s, 24, 48, 0.2f, 0.25f, 7000);
        L.add (C, "Harpsichord", "Old Record Harpsichord", "harpsichord vintage record", s);
    }

    //--------------------------------------------------------------- clavichord & co
    {
        auto s = make (Engine::Pluck); auto& l = s.layer[0]; auto& p = l.pluck;
        l.common.poly = 16; l.common.mwVib = 0.25f;
        p.excite = 3; p.bright = 0.55f; p.pluckPos = 0.1f; p.decay = 0.7f; p.damping = 0.35f; p.course = 2; p.courseDetune = 1.5f;
        p.courseDelay = 0.0f; p.body = 8; p.bodyMix = 0.5f; p.release = 0.05f; p.relNoise = 0.35f; p.velSens = 0.8f;
        space (s, Space::Room, 0.12f);
        L.add (C, "Clavichord", "Clavichord", "clavichord bebung tangent quiet", s);
    }
    {
        auto s = make (Engine::Pluck); auto& l = s.layer[0]; auto& p = l.pluck;
        l.common.poly = 16;
        p.excite = 3; p.bright = 0.75f; p.pluckPos = 0.08f; p.decay = 0.8f; p.damping = 0.2f; p.course = 2; p.courseDetune = 6.0f;
        p.body = 8; p.bodyMix = 0.6f; p.release = 0.2f; p.stiffness = 0.15f;
        space (s, Space::Room, 0.14f);
        L.add (C, "Struck Zither", "Dolceola", "dolceola toy zither keyboard", s);
    }
    {
        auto s = clavinet(); auto& p = s.layer[0].pluck; p.excite = 4; p.decay = 0.4f; p.pickupPos = 0.08f; p.pickup2Pos = 0.0f;
        amp (s.layer[0], Tube, 0.35f, 0.3f);
        L.add (C, "Clavinet", "Plucked E-Clavi", "electric plucked clavinet bright", s);
    }
}
} // namespace kv::banks
