// Keyverse offline renderer / regression test.
//
// Renders every factory preset through a short performance (chord, melody,
// bass note, sustain pedal) and checks:
//   - no NaN/Inf, sane peak & loudness,
//   - all voices end after release,
//   - tuning of a sustained A3 (autocorrelation pitch estimate),
//   - real-time CPU factor.
// Optionally writes a WAV per preset (--wav DIR) for listening.
//
// usage: kvrender [--wav DIR] [--filter TEXT] [--sr RATE] [--quiet] [--list]
//        kvrender --one "Preset Name" NOTE VELOCITY SECONDS OUT.wav
#include "../Source/Engine/Presets.h"
#include "../Source/Engine/Synth.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#if defined(__SSE__) || defined(_M_X64)
 #include <xmmintrin.h>
 #include <pmmintrin.h>
#endif

using namespace kv;

static void writeWav (const std::string& path, const std::vector<float>& L, const std::vector<float>& R, int sr)
{
    std::ofstream f (path, std::ios::binary);
    const uint32_t n = (uint32_t) L.size();
    const uint32_t dataBytes = n * 2 * 2;
    auto u32 = [&] (uint32_t v) { f.write ((const char*) &v, 4); };
    auto u16 = [&] (uint16_t v) { f.write ((const char*) &v, 2); };
    f.write ("RIFF", 4); u32 (36 + dataBytes); f.write ("WAVE", 4);
    f.write ("fmt ", 4); u32 (16); u16 (1); u16 (2); u32 ((uint32_t) sr); u32 ((uint32_t) sr * 4); u16 (4); u16 (16);
    f.write ("data", 4); u32 (dataBytes);
    for (uint32_t i = 0; i < n; ++i)
    {
        const int16_t l = (int16_t) std::lround (clampf (L[i], -1.0f, 1.0f) * 32767.0);
        const int16_t r = (int16_t) std::lround (clampf (R[i], -1.0f, 1.0f) * 32767.0);
        f.write ((const char*) &l, 2); f.write ((const char*) &r, 2);
    }
}

struct Ev { double t; int type; int a; int b; };   // type 0 on, 1 off, 2 cc

// Estimate the fundamental near the expected frequency with a fine windowed
// DFT scan (+-80 cents). Returns 0 when no clear peak is found in range.
static float estimatePitch (const std::vector<float>& x, int start, int len, int sr, float expected)
{
    if (start + len > (int) x.size()) return 0.0f;
    double best = -1.0, bestF = 0.0, edge = 0.0;
    for (double c = -80.0; c <= 80.0; c += 0.5)
    {
        const double f = expected * std::pow (2.0, c / 1200.0);
        double re = 0.0, im = 0.0;
        const double w = 2.0 * 3.14159265358979 * f / sr;
        for (int i = 0; i < len; ++i)
        {
            const double win = 0.5 - 0.5 * std::cos (2.0 * 3.14159265358979 * i / (len - 1));
            re += x[(size_t) (start + i)] * win * std::cos (w * i);
            im -= x[(size_t) (start + i)] * win * std::sin (w * i);
        }
        const double m = re * re + im * im;
        if (m > best) { best = m; bestF = f; }
        if (c == -80.0 || c == 80.0) edge = std::max (edge, m);
    }
    if (best <= 1.0e-12 || best <= edge * 1.01) return 0.0f;
    return (float) bestF;
}

int main (int argc, char** argv)
{
#if defined(__SSE__) || defined(_M_X64)
    _MM_SET_FLUSH_ZERO_MODE (_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE (_MM_DENORMALS_ZERO_ON);
#endif
    std::string wavDir, filter;
    int sr = 48000;
    bool quiet = false, list = false;
    for (int i = 1; i < argc; ++i)
    {
        if (! std::strcmp (argv[i], "--wav") && i + 1 < argc) wavDir = argv[++i];
        else if (! std::strcmp (argv[i], "--filter") && i + 1 < argc) filter = argv[++i];
        else if (! std::strcmp (argv[i], "--sr") && i + 1 < argc) sr = std::atoi (argv[++i]);
        else if (! std::strcmp (argv[i], "--quiet")) quiet = true;
        else if (! std::strcmp (argv[i], "--list")) list = true;
    }

    const auto& presets = factoryPresets();

    // --one "Preset Name" NOTE VELOCITY SECONDS OUT.wav : render one held note
    for (int i = 1; i + 5 < argc; ++i)
    {
        if (std::strcmp (argv[i], "--one") != 0) continue;
        for (const auto& pr : presets)
        {
            if (pr.name != argv[i + 1]) continue;
            Synth s;
            s.prepare (sr, 512);
            s.setSound (pr.data, true);
            const float secs = (float) std::atof (argv[i + 4]);
            const int n = (int) (secs * (float) sr), hold = (int) (secs * 0.6f * (float) sr);
            std::vector<float> L ((size_t) n), R ((size_t) n);
            s.noteOn (std::atoi (argv[i + 2]), std::atoi (argv[i + 3]));
            s.render (L.data(), R.data(), hold);
            s.noteOff (std::atoi (argv[i + 2]));
            s.render (L.data() + hold, R.data() + hold, n - hold);
            writeWav (argv[i + 5], L, R, sr);
            return 0;
        }
        std::fprintf (stderr, "preset not found: %s\n", argv[i + 1]);
        return 1;
    }

    if (list)
    {
        for (const auto& p : presets) std::printf ("%s | %s | %s\n", p.category.c_str(), p.subcategory.c_str(), p.name.c_str());
        std::printf ("%zu presets\n", presets.size());
        return 0;
    }

    const std::vector<Ev> perf = {
        { 0.00, 0, 48, 100 }, { 0.00, 0, 55, 96 }, { 0.00, 0, 60, 104 }, { 0.00, 0, 64, 92 }, { 0.00, 0, 67, 100 },
        { 1.20, 1, 48, 0 }, { 1.20, 1, 55, 0 }, { 1.20, 1, 60, 0 }, { 1.20, 1, 64, 0 }, { 1.20, 1, 67, 0 },
        { 1.50, 0, 72, 70 }, { 1.75, 1, 72, 0 }, { 1.75, 0, 76, 90 }, { 2.00, 1, 76, 0 }, { 2.00, 0, 79, 110 }, { 2.25, 1, 79, 0 },
        { 2.25, 0, 84, 127 }, { 2.60, 1, 84, 0 },
        { 2.80, 2, 64, 127 }, { 2.80, 0, 36, 112 }, { 2.90, 0, 43, 80 }, { 3.00, 0, 52, 60 }, { 3.10, 1, 36, 0 }, { 3.10, 1, 43, 0 }, { 3.10, 1, 52, 0 },
        { 3.90, 2, 64, 0 },
        { 4.20, 0, 57, 1 }, { 4.40, 1, 57, 0 },     // ppp note
        { 4.50, 0, 96, 127 }, { 4.70, 1, 96, 0 },   // high fff note
    };
    const double perfLen = 5.0, tailLen = 10.0;

    int failures = 0, idx = 0;
    double totalCpu = 0.0, totalAudio = 0.0;
    for (const auto& pr : presets)
    {
        ++idx;
        const std::string full = pr.category + " / " + pr.name;
        if (! filter.empty() && full.find (filter) == std::string::npos) continue;

        Synth s;
        s.prepare (sr, 512);
        s.setSound (pr.data, true);
        const int total = (int) ((perfLen + tailLen) * sr);
        std::vector<float> L ((size_t) total), R ((size_t) total);
        size_t ev = 0;
        const int block = 256;
        const auto t0 = std::chrono::steady_clock::now();
        for (int pos = 0; pos < total; pos += block)
        {
            const int n = std::min (block, total - pos);
            while (ev < perf.size() && perf[ev].t * sr < pos + n)
            {
                const auto& e = perf[ev++];
                if (e.type == 0) s.noteOn (e.a, e.b);
                else if (e.type == 1) s.noteOff (e.a);
                else s.controller (e.a, e.b);
            }
            s.render (L.data() + pos, R.data() + pos, n);
        }
        const double cpu = std::chrono::duration<double> (std::chrono::steady_clock::now() - t0).count();
        totalCpu += cpu; totalAudio += perfLen + tailLen;

        float peak = 0.0f; double sum = 0.0; int bad = 0;
        for (int i = 0; i < (int) (perfLen * sr); ++i)
        {
            if (! std::isfinite (L[(size_t) i]) || ! std::isfinite (R[(size_t) i])) ++bad;
            peak = std::max (peak, std::max (std::fabs (L[(size_t) i]), std::fabs (R[(size_t) i])));
            sum += 0.5 * (L[(size_t) i] * L[(size_t) i] + R[(size_t) i] * R[(size_t) i]);
        }
        const float rmsDb = 10.0f * std::log10 ((float) (sum / (perfLen * sr)) + 1e-12f);
        // long natural decays (bells, drones) are fine; stuck voices are not
        int voicesLeft = s.activeVoices();
        double silenceAt = perfLen + tailLen;
        if (voicesLeft > 0)
        {
            std::vector<float> tl ((size_t) sr), tr ((size_t) sr);
            for (int sec = 0; sec < 40 && s.activeVoices() > 0; ++sec)
            {
                s.render (tl.data(), tr.data(), sr);
                silenceAt += 1.0;
            }
            voicesLeft = s.activeVoices();
        }

        // tuning check on a sustained A3 (dry)
        float cents = 0.0f; bool pitched = false;
        {
            Synth t;
            t.prepare (sr, 512);
            SoundData d = pr.data;
            d.gfx.revMix = 0; d.gfx.dlyMix = 0; d.gfx.lofiWow = 0;
            for (auto& l : d.layer) { l.fx.m1Type = 0; l.fx.m2Type = 0; l.common.arpMode = 0; l.common.strum = 0; l.analog.l1Pitch = 0; l.analog.drift = 0; l.fm.lfoPitch = 0; }
            t.setSound (d, true);
            const int n = (int) (1.2 * sr);
            std::vector<float> a ((size_t) n), b ((size_t) n);
            t.noteOn (57, 100);
            t.render (a.data(), b.data(), n);
            const int tr = (int) d.layer[0].common.transpose;
            const float expected = midiToHz (57.0f + (float) tr + d.layer[0].common.fine * 0.01f);
            std::vector<float> m ((size_t) n);
            for (int i = 0; i < n; ++i) m[(size_t) i] = a[(size_t) i] + b[(size_t) i];
            const float f = estimatePitch (m, (int) (0.25 * sr), (int) (0.25 * sr), sr, expected);
            if (f > 0.0f) { pitched = true; cents = 1200.0f * std::log2 (f / expected); }
        }

        const bool fail = bad > 0 || peak < 1.0e-3f || voicesLeft > 0;
        failures += fail ? 1 : 0;
        if (! quiet || fail)
            std::printf ("%3d %-44s peak %6.1f dB  rms %6.1f dB  cpu x%5.1f  silent@%4.0fs  tune %s%6.1f ct%s\n",
                         idx, full.substr (0, 44).c_str(), gainToDb (peak), rmsDb, (perfLen + tailLen) / cpu, silenceAt,
                         pitched ? "" : "?", cents, fail ? (voicesLeft > 0 ? "  <-- FAIL (stuck voices)" : "  <-- FAIL") : "");

        if (! wavDir.empty())
        {
            std::string fn = pr.category + "_" + pr.name;
            for (auto& ch : fn) if (! std::isalnum ((unsigned char) ch)) ch = '_';
            const int keep = (int) ((perfLen + 3.0) * sr);
            L.resize ((size_t) keep); R.resize ((size_t) keep);
            writeWav (wavDir + "/" + fn + ".wav", L, R, sr);
        }
    }
    std::printf ("\n%zu presets, %d failures, overall realtime factor x%.1f\n", presets.size(), failures, totalAudio / std::max (totalCpu, 1e-9));
    return failures == 0 ? 0 : 1;
}
