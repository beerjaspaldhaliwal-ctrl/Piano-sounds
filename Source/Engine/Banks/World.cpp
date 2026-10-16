// World instruments: sitar, tanpura, koto, shamisen, banjo, mandolin,
// ukulele, harps, dulcimer, santoor, cimbalom, oud, bouzouki, balalaika,
// guzheng, pipa, kantele, kora, charango, zither, erhu and bansuri.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

static SoundData str (int excite, float bright, float pluckPos, float decay, float damping, int body, float bodyMix)
{
    auto s = make (Engine::Pluck);
    auto& l = s.layer[0];
    l.common.poly = 16;
    auto& p = l.pluck;
    p.excite = (float) excite; p.bright = bright; p.pluckPos = pluckPos; p.decay = decay; p.damping = damping;
    p.body = (float) body; p.bodyMix = bodyMix; p.release = 0.25f; p.relNoise = 0.12f; p.pickNoise = 0.25f; p.width = 0.4f;
    l.common.level = decay < 1.0f ? 6.0f : 3.0f;   // short-decay instruments need more level
    space (s, Space::Room, 0.16f);
    return s;
}

void addWorld (Lib& L)
{
    const char* W = kWorld;

    {
        auto s = str (0, 0.75f, 0.12f, 1.4f, 0.3f, 6, 0.7f); auto& l = s.layer[0]; auto& p = l.pluck;
        p.buzz = 0.6f; p.course = 2; p.courseDetune = 6.0f; p.courseDelay = 2.0f; p.twang = 8.0f; l.common.mwVib = 0.6f; l.common.bendRange = 5;
        space (s, Space::Hall, 0.2f);
        L.add (W, "India", "Sitar", "sitar india raga ravi shankar", s);
    }
    {
        auto s = str (1, 0.6f, 0.2f, 3.0f, 0.2f, 6, 0.7f); auto& p = s.layer[0].pluck;
        p.buzz = 0.7f; p.course = 2; p.courseDetune = 2.0f; p.repeat = 0.7f; p.sustainHold = 1;
        space (s, Space::Hall, 0.25f);
        L.add (W, "India", "Tanpura Drone", "tanpura tambura drone india", s);
    }
    {
        auto s = str (0, 0.7f, 0.22f, 1.2f, 0.3f, 10, 0.7f); auto& l = s.layer[0]; auto& p = l.pluck;
        p.stiffness = 0.2f; p.twang = 15.0f; l.common.mwVib = 0.5f; l.common.bendRange = 2;
        space (s, Space::Hall, 0.2f);
        L.add (W, "Japan", "Koto", "koto japan zither", s);
    }
    {
        auto s = str (0, 0.8f, 0.07f, 0.6f, 0.35f, 4, 0.6f); auto& p = s.layer[0].pluck; p.buzz = 0.4f; p.twang = 10.0f;
        L.add (W, "Japan", "Shamisen", "shamisen japan sawari", s);
    }
    {
        auto s = str (0, 0.85f, 0.1f, 0.5f, 0.3f, 4, 0.65f); auto& p = s.layer[0].pluck; p.stiffness = 0.3f; p.pickNoise = 0.4f;
        L.add (W, "Americana", "Banjo", "banjo bluegrass country", s);
    }
    {
        auto s = str (0, 0.75f, 0.12f, 0.7f, 0.3f, 8, 0.6f); auto& p = s.layer[0].pluck; p.course = 2; p.courseDetune = 3.0f; p.courseDelay = 1.5f;
        L.add (W, "Americana", "Mandolin", "mandolin bluegrass italian", s);
    }
    {
        auto s = str (0, 0.75f, 0.12f, 0.7f, 0.3f, 8, 0.6f); auto& p = s.layer[0].pluck; p.course = 2; p.courseDetune = 3.0f; p.repeat = 13.0f;
        space (s, Space::Hall, 0.2f);
        L.add (W, "Americana", "Mandolin Tremolo", "mandolin tremolo picking", s);
    }
    {
        auto s = str (1, 0.45f, 0.16f, 0.5f, 0.45f, 8, 0.7f); s.layer[0].common.strum = 25; s.layer[0].pluck.pickNoise = 0.1f;
        L.add (W, "Pacific", "Ukulele", "ukulele hawaii island", s);
    }
    {
        auto s = str (1, 0.5f, 0.3f, 2.0f, 0.3f, 5, 0.6f); auto& p = s.layer[0].pluck; p.sustainHold = 1; p.pickNoise = 0.05f;
        s.layer[0].common.poly = 32; space (s, Space::Hall, 0.3f);
        L.add (W, "Harp", "Concert Harp", "harp orchestra glissando pedal harp", s);
    }
    {
        auto s = str (1, 0.65f, 0.25f, 1.4f, 0.25f, 5, 0.5f); auto& p = s.layer[0].pluck; p.sustainHold = 1;
        s.layer[0].common.poly = 32; space (s, Space::Hall, 0.25f);
        L.add (W, "Harp", "Celtic Harp", "celtic harp irish folk", s);
    }
    {
        auto s = str (3, 0.7f, 0.12f, 2.0f, 0.2f, 8, 0.5f); auto& p = s.layer[0].pluck; p.course = 2; p.courseDetune = 4.0f; p.courseDelay = 0.0f; p.sustainHold = 1;
        s.layer[0].common.poly = 32; space (s, Space::Hall, 0.22f);
        L.add (W, "Struck", "Hammered Dulcimer", "hammered dulcimer appalachian", s);
    }
    {
        auto s = str (3, 0.8f, 0.1f, 1.5f, 0.2f, 8, 0.5f); auto& p = s.layer[0].pluck; p.course = 3; p.courseDetune = 5.0f; p.courseDelay = 0.0f; p.sustainHold = 1;
        s.layer[0].common.poly = 32; space (s, Space::Hall, 0.25f);
        L.add (W, "Struck", "Santoor", "santoor persian kashmir", s);
    }
    {
        auto s = str (3, 0.5f, 0.12f, 2.5f, 0.3f, 5, 0.5f); auto& p = s.layer[0].pluck; p.course = 3; p.courseDetune = 3.0f; p.courseDelay = 0.0f; p.release = 0.4f;
        s.layer[0].common.poly = 32;
        L.add (W, "Struck", "Cimbalom", "cimbalom hungarian gypsy", s);
    }
    {
        auto s = str (0, 0.55f, 0.15f, 0.8f, 0.4f, 11, 0.7f); auto& l = s.layer[0]; auto& p = l.pluck;
        p.course = 2; p.courseDetune = 2.0f; p.twang = 10.0f; l.common.mwVib = 0.4f;
        L.add (W, "Middle East", "Oud", "oud arabic maqam", s);
    }
    {
        auto s = str (0, 0.75f, 0.12f, 1.0f, 0.3f, 8, 0.55f); auto& p = s.layer[0].pluck; p.course = 2; p.courseOct = 1; p.courseDelay = 2.0f;
        L.add (W, "Mediterranean", "Bouzouki", "bouzouki greek irish", s);
    }
    {
        auto s = str (0, 0.8f, 0.1f, 0.5f, 0.35f, 8, 0.6f); s.layer[0].pluck.repeat = 11.0f;
        L.add (W, "Europe", "Balalaika", "balalaika russian tremolo", s);
    }
    {
        auto s = str (0, 0.65f, 0.18f, 1.8f, 0.25f, 10, 0.65f); auto& l = s.layer[0]; l.pluck.twang = 20.0f; l.common.mwVib = 0.6f; l.common.bendRange = 2;
        space (s, Space::Hall, 0.25f);
        L.add (W, "China", "Guzheng", "guzheng chinese zither", s);
    }
    {
        auto s = str (0, 0.8f, 0.12f, 0.8f, 0.3f, 8, 0.6f); s.layer[0].pluck.repeat = 14.0f;
        L.add (W, "China", "Pipa", "pipa chinese lute tremolo", s);
    }
    {
        auto s = str (1, 0.6f, 0.2f, 2.5f, 0.25f, 5, 0.45f); auto& p = s.layer[0].pluck; p.sustainHold = 1;
        s.layer[0].common.poly = 32; space (s, Space::Hall, 0.25f);
        L.add (W, "Europe", "Kantele", "kantele finnish psaltery", s);
    }
    {
        auto s = str (1, 0.6f, 0.22f, 1.3f, 0.3f, 6, 0.6f); s.layer[0].pluck.sustainHold = 1; s.layer[0].common.poly = 24;
        L.add (W, "Africa", "Kora", "kora west africa harp lute", s);
    }
    {
        auto s = str (0, 0.85f, 0.12f, 0.5f, 0.3f, 8, 0.6f); auto& p = s.layer[0].pluck; p.course = 2; p.courseOct = 1; p.courseDelay = 1.0f;
        s.layer[0].common.strum = 18;
        L.add (W, "South America", "Charango", "charango andes bolivia", s);
    }
    {
        auto s = str (0, 0.6f, 0.15f, 2.0f, 0.25f, 5, 0.5f); auto& p = s.layer[0].pluck; p.course = 2; p.courseDetune = 3.0f; p.sustainHold = 1;
        L.add (W, "Europe", "Zither", "zither third man austrian", s);
    }
    {
        auto s = analogPatch (0, -1, 0, 1600, 0.2f, 0.5f, 0.08f, 0.5f, 0.9f, 0.25f, 0.08f, 0.5f, 0.8f, 0.25f);
        mono (s, 0.12f); auto& a = s.layer[0].analog; a.fType = 2; a.cutoff = 1300; a.res = 0.35f; a.l1Pitch = 0.3f; a.l1Rate = 5.5f; a.l1Delay = 0.25f;
        a.noiseLevel = 0.04f; s.layer[0].common.mwVib = 0.5f;
        space (s, Space::Hall, 0.22f);
        L.add (W, "China", "Erhu", "erhu chinese violin bowed", s);
    }
    {
        auto s = analogPatch (3, -1, 0, 3000, 0.1f, 0.5f, 0.06f, 0.5f, 0.9f, 0.3f, 0.06f, 0.5f, 0.9f, 0.3f);
        mono (s, 0.08f); auto& a = s.layer[0].analog; a.noiseLevel = 0.2f; a.l1Pitch = 0.2f; a.l1Delay = 0.4f; a.l1Rate = 5.0f; a.velAmp = 0.6f;
        space (s, Space::Hall, 0.3f);
        L.add (W, "India", "Bansuri Flute", "bansuri shakuhachi bamboo flute", s);
    }
    {
        auto s = make (Engine::Organ); auto& o = s.layer[0].organ; o.model = 6; drawbars (o, "808000000"); o.musette = 8; o.musetteVoices = 2;
        o.wind = 0.3f; o.attack = 0.02f; o.release = 0.05f; o.tone = 5000; s.layer[0].common.poly = 6; space (s, Space::Hall, 0.2f);
        L.add (W, "Europe", "Bagpipe Chanter", "bagpipe scottish highland", s);
    }
}
} // namespace kv::banks
