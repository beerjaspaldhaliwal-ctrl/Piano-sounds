// Mallets & bells: vibraphone, marimba, xylophone, glockenspiel, celeste,
// music box, kalimba, tubular & church bells, gamelan, steel pan, handpan,
// glass & singing bowls, tuning forks.
#include "Banks.h"

namespace kv::banks
{
using namespace pb;
using namespace inst;

void addMallets (Lib& L)
{
    const char* M = kMallets;

    //--------------------------------------------------------------- vibes
    L.add (M, "Vibraphone", "Vibraphone", "vibes motor jazz", vibraphone());
    {
        auto s = vibraphone(); s.layer[0].fx.m1Rate = 7.5f; s.layer[0].fx.m1Depth = 0.6f;
        L.add (M, "Vibraphone", "Vibes Fast Motor", "vibes fast tremolo", s);
    }
    {
        auto s = vibraphone(); mod (s.layer[0], 1, ModFx::Off, 0, 0, 0);
        L.add (M, "Vibraphone", "Vibes No Motor", "vibes dry no tremolo", s);
    }
    {
        auto s = vibraphone(); auto& m = s.layer[0].mallet; m.hardness = 0.22f; m.noise = 0.03f; s.layer[0].fx.m1Depth = 0.3f;
        L.add (M, "Vibraphone", "Soft Mallet Vibes", "vibes yarn soft mellow", s);
    }
    {
        auto s = vibraphone(); space (s, Space::Chamber, 0.2f); s.layer[0].fx.m1Rate = 4.0f; s.layer[0].fx.m1Depth = 0.35f;
        L.add (M, "Vibraphone", "Jazz Club Vibes", "vibes jazz club", s);
    }
    {
        auto s = vibraphone(); auto& m = s.layer[0].mallet; m.excite = 2; m.bowAttack = 0.5f; m.damped = 0;
        mod (s.layer[0], 1, ModFx::Off, 0, 0, 0); space (s, Space::Ambient, 0.4f);
        L.add (M, "Vibraphone", "Bowed Vibes", "bowed vibraphone ambient drone", s);
    }

    //--------------------------------------------------------------- wood
    L.add (M, "Marimba", "Marimba", "marimba rosewood", marimba());
    {
        auto s = marimba(); auto& m = s.layer[0].mallet; m.hardness = 0.22f; m.noise = 0.12f;
        L.add (M, "Marimba", "Soft Marimba", "marimba soft yarn mallets", s);
    }
    {
        auto s = marimba(); auto& m = s.layer[0].mallet; m.resonator = 1.0f; m.decay = 1.8f; s.layer[0].common.transpose = -12;
        L.add (M, "Marimba", "Bass Marimba", "marimba bass low", s);
    }
    {
        auto s = marimba(); auto& m = s.layer[0].mallet; m.hardness = 0.65f; echo (s, 4, 0.35f, 0.2f); space (s, Space::Hall, 0.25f);
        L.add (M, "Marimba", "Marimba Echoes", "marimba delay minimal", s);
    }
    {
        auto s = mallet ({ 1.0f, 3.0f, 6.0f, 9.5f }, { 0, -6, -14, -24 }, { 1.0f, 0.4f, 0.2f, 0.1f }, 0.6f, 0.85f, 0.3f, 2500.0f);
        s.layer[0].common.transpose = 12; s.layer[0].mallet.resonator = 0.3f; s.layer[0].common.level = 5.0f;
        L.add (M, "Xylophone", "Xylophone", "xylophone bright wood", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.6f, 4.3f }, { 0, -10, -16 }, { 1.0f, 0.5f, 0.3f }, 0.6f, 0.4f, 0.5f, 800.0f);
        s.layer[0].common.level = 5.0f;
        L.add (M, "Wood", "Log Drum", "log drum slit wooden", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.0f, 3.9f }, { 0, -8, -16 }, { 1.0f, 0.6f, 0.3f }, 1.8f, 0.35f, 0.15f, 1200.0f);
        L.add (M, "Wood", "Tongue Drum", "steel tongue drum", s);
    }

    //--------------------------------------------------------------- metal bars
    L.add (M, "Glockenspiel", "Glockenspiel", "glockenspiel orchestra bells", glockenspiel());
    {
        auto s = glockenspiel(); s.layer[0].mallet.hardness = 0.5f; space (s, Space::Hall, 0.3f);
        L.add (M, "Glockenspiel", "Soft Glock", "glockenspiel soft dreamy", s);
    }
    L.add (M, "Celeste", "Celeste", "celesta tchaikovsky sugar plum", celeste());
    {
        auto s = celeste(); echo (s, 3, 0.35f, 0.18f, 3000.0f); space (s, Space::Hall, 0.32f);
        L.add (M, "Celeste", "Celeste Dreams", "celesta dreamy delay", s);
    }
    {
        auto s = stack (celeste(), glockenspiel(), 0.0f, -10.0f);
        L.add (M, "Celeste", "Glock Celeste", "celesta glockenspiel duo", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.76f, 5.4f }, { 0, -12, -20 }, { 1.0f, 0.5f, 0.3f }, 2.5f, 0.6f, 0.1f, 4000.0f);
        auto& m = s.layer[0].mallet; m.damped = 1; m.release = 0.3f; s.layer[0].common.transpose = 12;
        L.add (M, "Celeste", "Chime Keys", "chimeatron chime keyboard", s);
    }
    {
        auto s = mallet ({ 1.0f, 6.27f }, { 0, -18 }, { 1.0f, 0.08f }, 4.0f, 0.35f, 0.05f, 2000.0f);
        auto& m = s.layer[0].mallet; m.damped = 1; m.release = 0.4f;
        L.add (M, "Celeste", "Dulcitone", "dulcitone tuning fork keyboard", s);
    }
    {
        auto s = stack (celeste(), mallet ({ 0.224f, 0.617f, 1.209f, 2.0f, 2.987f, 4.173f }, { -30, -14, -6, 0, -3, -8 },
                                           { 1.0f, 0.9f, 0.8f, 0.7f, 0.5f, 0.35f }, 6.0f, 0.6f, 0.1f, 3000.0f), 0.0f, -8.0f);
        L.add (M, "Celeste", "Celechimes", "celeste chimes duo", s);
    }

    //--------------------------------------------------------------- plucked tines
    L.add (M, "Music Box", "Music Box", "music box comb lullaby", musicBox());
    {
        auto s = musicBox(); space (s, Space::Hall, 0.35f); echo (s, 2, 0.3f, 0.15f, 3000.0f);
        L.add (M, "Music Box", "Music Box Lullaby", "music box lullaby nursery", s);
    }
    {
        auto s = musicBox(); lofi (s, 24, 48, 0.25f, 0.35f, 6000); space (s, Space::Room, 0.25f);
        L.add (M, "Music Box", "Haunted Music Box", "music box creepy horror wobble", s);
    }
    {
        auto s = mallet ({ 1.0f, 5.4f, 14.1f }, { 0, -14, -26 }, { 1.0f, 0.25f, 0.1f }, 1.6f, 0.5f, 0.15f, 2000.0f);
        auto& m = s.layer[0].mallet; m.excite = 1; m.buzz = 0.08f; space (s, Space::Room, 0.16f);
        L.add (M, "Kalimba", "Kalimba", "kalimba thumb piano africa", s);
    }
    {
        auto s = mallet ({ 1.0f, 5.9f, 13.0f }, { 0, -12, -24 }, { 1.0f, 0.3f, 0.12f }, 1.8f, 0.6f, 0.2f, 2500.0f);
        auto& m = s.layer[0].mallet; m.excite = 1; m.buzz = 0.55f;
        L.add (M, "Kalimba", "Mbira Buzz", "mbira buzz zimbabwe", s);
    }

    //--------------------------------------------------------------- bells
    {
        auto s = mallet ({ 0.224f, 0.617f, 1.209f, 2.0f, 2.987f, 4.173f }, { -30, -14, -6, 0, -3, -8 },
                         { 1.0f, 0.9f, 0.8f, 0.7f, 0.5f, 0.35f }, 8.0f, 0.65f, 0.15f, 3000.0f);
        s.layer[0].mallet.decayTrack = 0.3f; space (s, Space::Hall, 0.3f);
        L.add (M, "Bells", "Tubular Bells", "tubular bells chimes orchestra", s);
    }
    {
        auto s = mallet ({ 0.5f, 1.0f, 1.19f, 1.5f, 2.0f, 3.0f }, { -6, -4, -2, -10, 0, -6 }, { 1.8f, 1.2f, 1.0f, 0.6f, 0.7f, 0.45f }, 9.0f, 0.75f, 0.2f, 1800.0f);
        auto& m = s.layer[0].mallet; m.beat = 0.8f; m.beatMix = 0.6f; m.decayTrack = 0.3f; space (s, Space::Cathedral, 0.35f);
        s.layer[0].common.level = -5.0f;
        L.add (M, "Bells", "Church Bell", "church bell tower carillon", s);
    }
    {
        auto s = mallet ({ 1.0f, 3.0f, 5.4f }, { 0, -8, -20 }, { 1.0f, 0.5f, 0.25f }, 3.0f, 0.55f, 0.1f, 3000.0f);
        s.layer[0].mallet.beat = 1.0f; s.layer[0].mallet.beatMix = 0.3f; s.layer[0].common.transpose = 12;
        L.add (M, "Bells", "Handbells", "handbells christmas choir", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.29f, 3.91f, 5.79f }, { 0, -6, -12, -18 }, { 1.0f, 0.7f, 0.5f, 0.3f }, 6.0f, 0.8f, 0.1f, 6000.0f);
        s.layer[0].common.transpose = 24;
        L.add (M, "Bells", "Crotales", "crotales antique cymbals", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.0f, 3.2f, 4.7f }, { 0, -6, -12, -18 }, { 1.0f, 0.7f, 0.5f, 0.3f }, 4.0f, 0.75f, 0.05f, 5000.0f);
        auto& m = s.layer[0].mallet; m.beat = 1.5f; m.beatMix = 0.4f; s.layer[0].common.transpose = 12;
        echo (s, 3, 0.4f, 0.2f, 4000.0f); space (s, Space::Hall, 0.35f);
        L.add (M, "Bells", "Crystal Bells", "crystal bells sparkle christmas", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.76f, 5.4f, 8.9f }, { 0, -10, -14, -20 }, { 1.0f, 0.6f, 0.4f, 0.3f }, 3.0f, 0.9f, 0.05f, 8000.0f);
        auto& m = s.layer[0].mallet; m.spread = 0.15f; s.layer[0].common.transpose = 24;
        echo (s, 5, 0.45f, 0.25f, 5000.0f); space (s, Space::Hall, 0.35f);
        L.add (M, "Bells", "Fairy Bells", "fairy sparkle magic", s);
    }

    //--------------------------------------------------------------- world metal
    {
        auto s = mallet ({ 1.0f, 2.0f, 3.0f, 4.0f }, { 0, -6, -12, -20 }, { 1.0f, 0.8f, 0.5f, 0.3f }, 1.6f, 0.5f, 0.15f, 2000.0f);
        auto& m = s.layer[0].mallet; m.beat = 1.5f; m.beatMix = 0.3f; m.pitchDrop = 8.0f; space (s, Space::Room, 0.16f);
        L.add (M, "Steel Pan", "Steel Drum", "steel pan caribbean calypso", s);
    }
    {
        auto s = mallet ({ 1.0f, 1.52f, 3.46f, 3.92f }, { 0, -8, -6, -14 }, { 1.0f, 0.7f, 0.5f, 0.4f }, 3.0f, 0.6f, 0.15f, 2500.0f);
        auto& m = s.layer[0].mallet; m.beat = 2.5f; m.beatMix = 0.5f; space (s, Space::Hall, 0.25f);
        L.add (M, "Gamelan", "Gamelan Bonang", "gamelan bonang java bali gong", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.71f, 4.8f }, { 0, -10, -16 }, { 1.0f, 0.6f, 0.4f }, 2.5f, 0.7f, 0.2f, 3000.0f);
        auto& m = s.layer[0].mallet; m.beat = 3.0f; m.beatMix = 0.45f; space (s, Space::Hall, 0.25f);
        L.add (M, "Gamelan", "Gamelan Saron", "gamelan saron metallophone ombak", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.0f, 2.98f, 4.0f }, { 0, -4, -10, -16 }, { 1.0f, 0.8f, 0.6f, 0.4f }, 3.5f, 0.3f, 0.12f, 900.0f);
        auto& m = s.layer[0].mallet; m.beat = 0.7f; m.beatMix = 0.4f; space (s, Space::Hall, 0.28f);
        L.add (M, "Handpan", "Handpan", "handpan hang drum meditation", s);
    }

    //--------------------------------------------------------------- glass & bowls
    {
        auto s = mallet ({ 1.0f, 2.32f, 4.25f, 6.63f }, { 0, -8, -14, -20 }, { 1.0f, 0.8f, 0.6f, 0.4f }, 8.0f, 0.5f);
        auto& m = s.layer[0].mallet; m.excite = 2; m.bowAttack = 0.4f; space (s, Space::Hall, 0.3f);
        L.add (M, "Glass", "Wine Glass", "glass harp rubbed wine glasses", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.71f, 5.12f, 8.0f }, { 0, -6, -14, -20 }, { 1.0f, 0.8f, 0.6f, 0.4f }, 12.0f, 0.4f, 0.05f, 1500.0f);
        auto& m = s.layer[0].mallet; m.beat = 1.2f; m.beatMix = 0.7f; m.decayTrack = 0.2f; s.layer[0].common.transpose = -12;
        s.layer[0].common.level = -6.0f;
        space (s, Space::Hall, 0.3f);
        L.add (M, "Bowls", "Singing Bowl", "tibetan singing bowl meditation", s);
    }
    {
        auto s = mallet ({ 1.0f, 2.71f, 5.12f, 8.0f }, { 0, -6, -14, -20 }, { 1.0f, 0.8f, 0.6f, 0.4f }, 12.0f, 0.4f);
        auto& m = s.layer[0].mallet; m.excite = 2; m.bowAttack = 1.2f; m.beat = 1.2f; m.beatMix = 0.7f; s.layer[0].common.transpose = -12;
        space (s, Space::Ambient, 0.4f);
        L.add (M, "Bowls", "Rubbed Bowl", "singing bowl rubbed drone", s);
    }
    {
        auto s = mallet ({ 1.0f, 3.0f, 6.4f }, { 0, -8, -16 }, { 1.0f, 0.5f, 0.3f }, 1.5f, 0.7f, 0.1f, 5000.0f);
        space (s, Space::Hall, 0.3f);
        L.add (M, "Glass", "Glass Marimba", "glass marimba crystal", s);
    }
}
} // namespace kv::banks
