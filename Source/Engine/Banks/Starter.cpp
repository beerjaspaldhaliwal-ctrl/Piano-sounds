// Temporary calibration bank: one representative sound per engine.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

void addPads (Lib& L)
{
    auto s = analog();
    auto& a = s.layer[0].analog;
    osc (a, 1, 0, 0, 0, 0, 0.8f);
    osc (a, 2, 0, 0, 0, 8, 0.8f);
    a.unison = 3; a.uniDetune = 18; a.uniSpread = 0.8f;
    filt (a, 0, 1800, 0.15f, 1.5f);
    ampEnv (a, 0.8f, 1.0f, 0.85f, 1.8f);
    fltEnv (a, 1.2f, 2.0f, 0.5f, 2.0f);
    a.l2Cut = 0.6f; a.l2Rate = 0.2f;
    space (s, Space::Hall, 0.35f);
    L.add (kPads, "Analog", "Warm Pad", "", s);
}
void addLeads (Lib& L)
{
    auto s = analog();
    auto& a = s.layer[0].analog;
    s.layer[0].common.poly = 1; s.layer[0].common.legato = 1; s.layer[0].common.glide = 0.06f;
    osc (a, 1, 0, 0, 0, 0, 0.8f); osc (a, 2, 1, 0, 0, 6, 0.6f);
    filt (a, 0, 2500, 0.35f, 2.0f);
    ampEnv (a, 0.005f, 0.3f, 0.9f, 0.2f);
    fltEnv (a, 0.005f, 0.4f, 0.4f, 0.3f);
    echo (s, 3, 0.3f, 0.2f);
    space (s, Space::Plate, 0.15f);
    L.add (kLeads, "Mono", "Classic Lead", "minimoog", s);
}
void addSynthBass (Lib& L)
{
    auto s = analog();
    auto& a = s.layer[0].analog;
    s.layer[0].common.poly = 1;
    s.layer[0].common.transpose = -12;
    osc (a, 1, 0, 0, 0, 0, 0.9f); osc (a, 2, 1, -1, 0, 0, 0.6f);
    filt (a, 0, 400, 0.3f, 3.0f, 0.3f, 0.4f);
    ampEnv (a, 0.002f, 0.4f, 0.8f, 0.08f);
    fltEnv (a, 0.001f, 0.25f, 0.15f, 0.1f);
    L.add (kSynthBass, "Analog", "Moog Bass", "", s);
}
void addStringsBrass (Lib& L)
{
    auto s = analog();
    auto& a = s.layer[0].analog;
    osc (a, 1, 0, 0, 0, 0, 0.9f);
    filt (a, 0, 1200, 0.1f, 2.5f);
    ampEnv (a, 0.06f, 0.6f, 0.8f, 0.3f);
    fltEnv (a, 0.08f, 0.5f, 0.4f, 0.3f);
    space (s, Space::Hall, 0.2f);
    L.add (kStrBrass, "Brass", "Synth Brass", "", s);
}
void addPlucks (Lib& L)
{
    auto s = analog();
    auto& a = s.layer[0].analog;
    osc (a, 1, 1, 0, 0, 0, 0.9f, 0.3f);
    filt (a, 0, 300, 0.3f, 5.0f);
    ampEnv (a, 0.001f, 0.5f, 0.0f, 0.3f);
    fltEnv (a, 0.001f, 0.25f, 0.0f, 0.2f);
    s.layer[0].common.arpMode = 1; s.layer[0].common.arpOct = 2;
    echo (s, 3, 0.35f, 0.25f);
    space (s, Space::Hall, 0.2f);
    L.add (kPlucks, "Arp", "Pluck Arp", "", s);
}
void addWorld (Lib& L)
{
    auto s = steelGuitar();
    auto& p = s.layer[0].pluck;
    p.buzz = 0.6f; p.body = 6; p.course = 2; p.courseDetune = 6;
    L.add (kWorld, "Sitar", "Sitar", "", s);
}
void addLayers (Lib& L) { L.add (kLayers, "Piano + Pad", "Grand & Strings", "", stack (grand(), analog(), 0.0f, -12.0f)); }
void addFx (Lib& L)
{
    auto s = mallet ({ 1.0f, 2.32f, 4.25f }, { 0, -6, -12 }, { 1, 0.7f, 0.5f }, 8.0f, 0.5f);
    s.layer[0].mallet.excite = 2; s.layer[0].mallet.bowAttack = 0.6f;
    space (s, Space::Ambient, 0.4f);
    L.add (kFx, "Bowed", "Glass Harmonica", "", s);
}
} // namespace kv::banks
