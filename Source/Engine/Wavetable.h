// Keyverse - wavetable synth engine.
//
// 16 built-in wavetables of 32 frames each are generated once at start-up
// (additive spectra or oversampled time-domain shapes, analysed with an FFT).
// Every frame is stored as a mip-map of 9 band-limited copies (512 down to 2
// harmonics, 4x oversampled), so oscillators stay alias-free across the
// keyboard. Playback uses 4-point Hermite interpolation and morphs smoothly
// between frames.
//
// Per voice: two wavetable oscillators with eight phase/shape warps, up to
// 8-voice unison with stereo spread and per-voice table spread, sine sub,
// noise, the analog filter set (ladder, SVF, LP+HP, formant), amp, filter and
// modulation envelopes, a per-voice and a global LFO, and mod wheel /
// channel or polyphonic pressure routing.
#pragma once

#include "Analog.h"
#include <complex>

namespace kv
{
//==============================================================================
namespace wtdetail
{
using cpx = std::complex<double>;

// In-place iterative radix-2 FFT. sign = -1 forward, +1 inverse (unnormalised).
inline void fft (std::vector<cpx>& a, int sign)
{
    const size_t n = a.size();
    for (size_t i = 1, j = 0; i < n; ++i)
    {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap (a[i], a[j]);
    }
    for (size_t len = 2; len <= n; len <<= 1)
    {
        const double ang = sign * 2.0 * 3.14159265358979323846 / (double) len;
        const cpx wl (std::cos (ang), std::sin (ang));
        for (size_t i = 0; i < n; i += len)
        {
            cpx w (1.0, 0.0);
            for (size_t j = 0; j < len / 2; ++j)
            {
                const cpx u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wl;
            }
        }
    }
}

inline double frac (double x) { return x - std::floor (x); }
inline double hash01 (uint32_t a, uint32_t b) { return 0.5 + 0.5 * (double) hashNoise (a, b); }
} // namespace wtdetail

//==============================================================================
class WavetableBank
{
public:
    static constexpr int kTables = WavetableParams::kNumTables;
    static constexpr int kFrames = 32;
    static constexpr int kLevels = 9;          // 512, 256, ... 2 harmonics
    static constexpr int kMaxHarm = 512;

    static int levelSize (int m) { return std::max (256, 2048 >> m); }
    static int levelHarm (int m) { return kMaxHarm >> m; }

    static const WavetableBank& get() { static const WavetableBank b; return b; }

    // Pointer to a band-limited frame of levelSize(m) samples, with wrap-around
    // guard samples at [-1], [N] and [N + 1] for the interpolator.
    const float* frame (int table, int f, int m) const
    {
        return data.data() + ((size_t) (table * kFrames + f) * (size_t) perFrame) + (size_t) levelOffset[m] + 1;
    }

private:
    using cpx = wtdetail::cpx;
    int levelOffset[kLevels] {};
    int perFrame = 0;
    std::vector<float> data;

    WavetableBank()
    {
        int off = 0;
        for (int m = 0; m < kLevels; ++m) { levelOffset[m] = off; off += levelSize (m) + 3; }
        perFrame = off;
        data.assign ((size_t) kTables * kFrames * (size_t) perFrame, 0.0f);
        std::vector<cpx> spec ((size_t) kMaxHarm + 1);
        for (int t = 0; t < kTables; ++t)
            for (int f = 0; f < kFrames; ++f)
            {
                std::fill (spec.begin(), spec.end(), cpx (0.0, 0.0));
                makeSpectrum (t, (double) f / (double) (kFrames - 1), spec);
                store (t, f, spec);
            }
    }

    // spec[k] = A_k * exp(i (phi_k - pi/2)) so harmonic k = A_k sin(2 pi k x + phi_k)
    static void addSine (std::vector<cpx>& s, int k, double amp, double phase = 0.0)
    {
        if (k >= 1 && k <= kMaxHarm) s[(size_t) k] += std::polar (amp, phase - 1.5707963267948966);
    }

    // Analyse one cycle given as a function of phase (0..1), 4096x oversampled.
    template <class F> static void fromTime (std::vector<cpx>& s, F&& fn)
    {
        const int N = 4096;
        std::vector<cpx> buf ((size_t) N);
        for (int i = 0; i < N; ++i) buf[(size_t) i] = cpx (fn ((double) i / N), 0.0);
        wtdetail::fft (buf, -1);
        for (int k = 1; k <= kMaxHarm; ++k) s[(size_t) k] = buf[(size_t) k] * (2.0 / N);
    }

    void store (int t, int f, const std::vector<cpx>& spec)
    {
        // synthesise every mip level, normalise all with the full-band frame's scale
        double scale = 1.0;
        for (int m = 0; m < kLevels; ++m)
        {
            const int N = levelSize (m), H = std::min (levelHarm (m), N / 2 - 1);
            std::vector<cpx> buf ((size_t) N, cpx (0.0, 0.0));
            for (int k = 1; k <= H; ++k) buf[(size_t) k] = spec[(size_t) k];
            wtdetail::fft (buf, +1);
            if (m == 0)
            {
                double peak = 0.0, sum = 0.0;
                for (int i = 0; i < N; ++i) { const double v = buf[(size_t) i].real(); peak = std::max (peak, std::fabs (v)); sum += v * v; }
                const double rms = std::sqrt (sum / N);
                scale = (peak < 1.0e-9) ? 0.0 : std::min (1.0 / peak, 0.5 / std::max (rms, 1.0e-9));
            }
            float* out = data.data() + ((size_t) (t * kFrames + f) * (size_t) perFrame) + (size_t) levelOffset[m] + 1;
            for (int i = 0; i < N; ++i) out[i] = (float) (buf[(size_t) i].real() * scale);
            out[-1] = out[N - 1]; out[N] = out[0]; out[N + 1] = out[1];
        }
    }

    static void makeSpectrum (int t, double x, std::vector<cpx>& s)
    {
        using namespace wtdetail;
        static constexpr double pi = 3.14159265358979323846, tau = 2.0 * pi;
        switch (t)
        {
            case 0:   // Basic Shapes: sine > triangle > saw > square > narrow pulse
            {
                auto shape = [] (int i, double p)
                {
                    switch (i)
                    {
                        case 0: return std::sin (tau * p);
                        case 1: return 1.0 - 4.0 * std::fabs (frac (p + 0.25) - 0.5);
                        case 2: return 1.0 - 2.0 * frac (p + 0.5);
                        case 3: return p < 0.5 ? 1.0 : -1.0;
                        default: return p < 0.2 ? 1.0 : -0.25;
                    }
                };
                const double y = x * 4.0;
                const int i = std::min (3, (int) y);
                const double w = y - i;
                fromTime (s, [&] (double p) { return shape (i, p) * (1.0 - w) + shape (i + 1, p) * w; });
                break;
            }
            case 1:   // Pulse Width 50% > 4%
            {
                const double pw = 0.5 - 0.46 * x;
                fromTime (s, [&] (double p) { return p < pw ? 1.0 : -1.0; });
                break;
            }
            case 2:   // Saw Sync: slave saw restarted every master cycle, ratio 1 > 8
            {
                const double r = 1.0 + 7.0 * x;
                fromTime (s, [&] (double p) { return (1.0 - 2.0 * frac (p * r)) * (1.0 - 0.3 * p); });
                break;
            }
            case 3:   // Harmonic Sweep: saw harmonics added one by one (1..48)
            {
                const double nH = 1.0 + 47.0 * x;
                for (int k = 1; k <= 49; ++k)
                {
                    const double g = clampf ((float) (nH - k + 1.0), 0.0f, 1.0f);
                    if (g > 0.0) addSine (s, k, g / k);
                }
                break;
            }
            case 4:   // Vocal Formants: A E I O U (male voice)
            case 14:  // Choir: U O A E I (female voice, softer tilt)
            {
                static const double F[5][3] = { { 800, 1150, 2800 }, { 400, 2000, 2550 }, { 280, 2250, 2900 }, { 450, 800, 2830 }, { 325, 700, 2530 } };
                static const double G[5][3] = { { 1.0, 0.5, 0.1 }, { 1.0, 0.25, 0.12 }, { 1.0, 0.2, 0.1 }, { 1.0, 0.35, 0.06 }, { 1.0, 0.25, 0.03 } };
                static const int choirOrder[5] = { 4, 3, 0, 1, 2 };
                const bool choir = t == 14;
                const double f0 = choir ? 220.0 : 110.0;
                const double v = std::min (x * 4.0, 3.9999);
                const int i = (int) v;
                const double w = v - i;
                const int a = choir ? choirOrder[i] : i, b = choir ? choirOrder[i + 1] : i + 1;
                for (int k = 1; k <= kMaxHarm; ++k)
                {
                    const double fk = k * f0;
                    if (fk > 9000.0) break;
                    double amp = 0.0;
                    for (int j = 0; j < 3; ++j)
                    {
                        const double fc = F[a][j] * (1.0 - w) + F[b][j] * w;
                        const double gc = G[a][j] * (1.0 - w) + G[b][j] * w;
                        const double bw = 60.0 + 40.0 * j;
                        amp += gc / (1.0 + sqr ((float) ((fk - fc) / bw)));
                    }
                    amp *= 1.0 / std::pow ((double) k, choir ? 0.35 : 0.2);
                    addSine (s, k, amp, choir ? 0.0 : (k & 1) * 0.5);
                }
                break;
            }
            case 5:   // Digital Steps: 8 distinct hashed harmonic sets
            {
                const int step = std::min (7, (int) (x * 8.0));
                for (int k = 1; k <= 96; ++k)
                {
                    const double h = hash01 ((uint32_t) step + 101u, (uint32_t) k);
                    if (k == 1 || h > 0.55) addSine (s, k, (k == 1 ? 1.0 : h) / std::pow ((double) k, 0.75));
                }
                break;
            }
            case 6:   // Drawbar Organ: morph through five registrations (16' fundamental)
            {
                static const int harm[9] = { 1, 3, 2, 4, 6, 8, 10, 12, 16 };
                static const char* reg[5] = { "008000000", "888000000", "888800000", "868000468", "888888888" };
                const double v = std::min (x * 4.0, 3.9999);
                const int i = (int) v;
                const double w = v - i;
                for (int d = 0; d < 9; ++d)
                {
                    const double lv = ((reg[i][d] - '0') * (1.0 - w) + (reg[i + 1][d] - '0') * w) / 8.0;
                    if (lv > 0.0) addSine (s, harm[d], std::pow (lv, 1.5));
                }
                break;
            }
            case 7:   // FM Sweep: 1:1 phase modulation, index 0 > 5
            {
                const double I = 5.0 * x;
                fromTime (s, [&] (double p) { return std::sin (tau * p + I * std::sin (tau * p)); });
                break;
            }
            case 8:   // Bell Spectra: sparse partials growing brighter
            {
                static const int part[8] = { 1, 2, 3, 5, 7, 11, 13, 17 };
                for (int j = 0; j < 8; ++j)
                {
                    const double g = std::exp (-(double) j / (0.6 + 6.0 * x)) * (0.6 + 0.4 * std::cos (j * 2.1 + x * 5.0));
                    addSine (s, part[j], std::fabs (g), j * 0.7);
                }
                break;
            }
            case 9:   // Wavefolder: sine folded with rising gain
            {
                const double g = 1.0 + 7.0 * x;
                fromTime (s, [&] (double p) { return std::sin (0.5 * pi * g * (std::sin (tau * p) + 0.15 * x)); });
                break;
            }
            case 10:  // Resonant Sweep: saw through a resonant low-pass, peak at harmonic 2 > 64
            {
                const double h = 2.0 + 62.0 * std::pow (x, 1.6), Q = 5.0;
                for (int k = 1; k <= kMaxHarm; ++k)
                {
                    const double r = k / h;
                    const double resp = 1.0 / std::sqrt (sqr ((float) (1.0 - r * r)) + sqr ((float) (r / Q)));
                    addSine (s, k, resp / k);
                }
                break;
            }
            case 11:  // Hollow Comb: saw with a moving comb notch
            {
                const double d = 0.5 * x;
                for (int k = 1; k <= kMaxHarm; ++k) addSine (s, k, std::fabs (std::cos (pi * k * d)) / k);
                break;
            }
            case 12:  // Spectral Noise: morph through four random spectra
            {
                const double v = std::min (x * 3.0, 2.9999);
                const int i = (int) v;
                const double w = v - i;
                for (int k = 1; k <= 160; ++k)
                {
                    const double a = sqr ((float) hash01 ((uint32_t) i + 7u, (uint32_t) k)), b = sqr ((float) hash01 ((uint32_t) i + 8u, (uint32_t) k));
                    const double ph = tau * hash01 (991u, (uint32_t) k);
                    addSine (s, k, (k == 1 ? 1.0 : (a * (1.0 - w) + b * w)) / std::pow ((double) k, 0.6), ph);
                }
                break;
            }
            case 13:  // Low Pass Sweep: dark > bright saw (2-pole roll-off)
            {
                const double c = 1.0 + 80.0 * x * x;
                for (int k = 1; k <= kMaxHarm; ++k) addSine (s, k, 1.0 / (k * (1.0 + sqr ((float) (k / c)))));
                break;
            }
            case 15:  // Metallic: harmonic comb |sin(k a)|^3
            default:
            {
                const double a = 0.3 + 2.5 * x;
                for (int k = 1; k <= 200; ++k) addSine (s, k, (k == 1 ? 1.0 : std::pow (std::fabs (std::sin (k * a)), 3.0)) / std::pow ((double) k, 0.6));
                break;
            }
        }
    }
};

//==============================================================================
class WavetableEngine;

class WavetableVoice : public Voice
{
public:
    static constexpr int kMaxUni = 8;

    void setup (EngineBase* e)
    {
        eng = e;
        rng.seed ((uint32_t) (reinterpret_cast<uintptr_t> (this) & 0xffffffffu) | 29u);
    }

    void start (int keyIn, float velocity) override
    {
        const auto& P = eng->params->wt;
        const float sr = eng->sr;
        const bool wasActive = active && ! dying;
        resetFade();
        active = true; releasing = false;
        vel = velocity; key = keyIn;
        ampEnv.set (P.aA, P.aD, P.aS, P.aR, sr);
        filtEnv.set (P.fA, P.fD, P.fS, P.fR, sr);
        modEnv.set (P.mA, P.mD, P.mS, P.mR, sr);
        ampEnv.noteOn (! wasActive);
        filtEnv.noteOn (! wasActive);
        modEnv.noteOn (! wasActive);
        lfo1.shape = (int) P.l1Shape;
        lfo1.reset (0.0f);
        lfo1Fade = P.l1Delay > 0.01f ? 0.0f : 1.0f;
        lfo1FadeInc = 1.0f / (std::max (P.l1Delay, 0.01f) * sr);
        if (! wasActive)
        {
            ladL.reset(); ladR.reset(); svfL.reset(); svfR.reset(); hpL.reset(); hpR.reset();
            svf2L.reset(); svf2R.reset(); fmtL.reset(); fmtR.reset();
            for (int k = 0; k < kMaxUni; ++k) { ph1[k] = rng.uni(); ph2[k] = rng.uni(); fb2[k] = 0.0f; }
            phSub = 0.0f;
            posInit = false;
        }
        velGain = 1.0f - P.velAmp * (1.0f - vel * vel) * 0.9f;
        for (int k = 0; k < 2; ++k) driftV[k] = rng.bi() * 0.3f;
    }

    void release() override { releasing = true; ampEnv.noteOff(); filtEnv.noteOff(); modEnv.noteOff(); }
    void kill (float s) override { beginFade (s, eng->sr); }
    void render (float* L, float* R, int n) override;

private:
    struct OscSetup
    {
        int table = 0, level = 0, size = 128, warp = 0;
        float warpAmt = 0.0f, gain = 0.0f;
    };

    // 4-point, 3rd-order Hermite interpolation around p[0]..p[1]
    static float hermite (const float* p, float f)
    {
        const float c1 = 0.5f * (p[1] - p[-1]);
        const float c2 = p[-1] - 2.5f * p[0] + 2.0f * p[1] - 0.5f * p[2];
        const float c3 = 0.5f * (p[2] - p[-1]) + 1.5f * (p[0] - p[1]);
        return ((c3 * f + c2) * f + c1) * f + p[0];
    }

    static float readFrame (const float* a, const float* b, float t, float ph, int size)
    {
        const float x = ph * (float) size;
        int i = (int) x;
        if (i >= size) i = size - 1;
        const float fr = x - (float) i;
        return hermite (a + i, fr) * (1.0f - t) + hermite (b + i, fr) * t;
    }

    // Reads oscillator o at phase ph with warp; pos in 0..1, fm = modulator input
    static float oscRead (const OscSetup& o, float pos, float ph, float fm)
    {
        const auto& B = WavetableBank::get();
        const float fpos = clampf (pos, 0.0f, 1.0f) * (float) (WavetableBank::kFrames - 1);
        int f = (int) fpos;
        if (f >= WavetableBank::kFrames - 1) f = WavetableBank::kFrames - 2;
        const float t = fpos - (float) f;
        const float* fa = B.frame (o.table, f, o.level);
        const float* fb = B.frame (o.table, f + 1, o.level);
        const float a = o.warpAmt;
        float p = ph;
        switch (o.warp)
        {
            case 1: p = std::pow (std::max (ph, 1.0e-6f), std::exp (-2.5f * a)); break;
            case 2: p = std::pow (std::max (ph, 1.0e-6f), std::exp (2.5f * a)); break;
            case 3: { p = ph * (1.0f + 7.0f * a); p -= std::floor (p); break; }
            case 4:
            {
                const float m = ph < 0.5f ? ph * 2.0f : (1.0f - ph) * 2.0f;   // forward then backward
                return readFrame (fa, fb, t, ph, o.size) * (1.0f - a) + readFrame (fa, fb, t, std::min (m, 0.99999f), o.size) * a;
            }
            case 5:
            {
                const float w = 0.5f - 0.45f * a;
                p = ph < w ? ph * 0.5f / w : 0.5f + (ph - w) * 0.5f / (1.0f - w);
                break;
            }
            case 6: { p = ph + a * 0.6f * fm; p -= std::floor (p); break; }
            case 7:
            {
                const float v = readFrame (fa, fb, t, ph, o.size);
                const float q = std::exp2 (1.0f + (1.0f - a) * 7.0f);
                return std::round (v * q) / q;
            }
            default: break;
        }
        if (p >= 1.0f) p = 0.99999f;
        return readFrame (fa, fb, t, p, o.size);
    }

    static OscSetup setupOsc (float tableF, float levelGain, float warpF, float warpAmt, float inc)
    {
        OscSetup o;
        o.table = std::clamp ((int) tableF, 0, WavetableBank::kTables - 1);
        o.warp = std::clamp ((int) warpF, 0, 7);
        o.warpAmt = clampf (warpAmt, 0.0f, 1.0f);
        o.gain = levelGain;
        // choose the mip level whose top harmonic stays below ~0.47 * Nyquist-limit
        float extra = 0.0f;
        if (o.warp != 0 && o.warpAmt > 0.001f)
            extra = (o.warp == 3 || o.warp == 6) ? 1.0f + 2.0f * o.warpAmt : (o.warp == 1 || o.warp == 2 ? 1.0f + o.warpAmt : 1.0f);
        const float h = (float) WavetableBank::kMaxHarm * inc / 0.47f;
        const int m = (int) std::ceil (std::log2 (std::max (h, 1.0f)) + extra - 1.0e-4f);
        o.level = std::clamp (m, 0, WavetableBank::kLevels - 1);
        o.size = WavetableBank::levelSize (o.level);
        return o;
    }

    EngineBase* eng = nullptr;
    int key = 60;
    float ph1[kMaxUni] {}, ph2[kMaxUni] {}, fb2[kMaxUni] {}, phSub = 0.0f;
    Adsr ampEnv, filtEnv, modEnv;
    float velGain = 1.0f;
    Lfo lfo1;
    float lfo1Fade = 1.0f, lfo1FadeInc = 0.0f;
    float pos1Cur = 0.0f, pos2Cur = 0.0f;
    bool posInit = false;
    Ladder ladL, ladR;
    Svf svfL, svfR, svf2L, svf2R;
    OnePoleHP hpL, hpR;
    FormantBank fmtL, fmtR;
    float driftV[2] {}, driftT[2] {};
    Rng rng;
};

class WavetableEngine : public PolyEngine<WavetableVoice>
{
public:
    WavetableEngine() : PolyEngine<WavetableVoice> (36) { (void) WavetableBank::get(); }
    void prepare (float sampleRate, int maxBlk) override
    {
        sr = sampleRate; maxBlock = maxBlk;
        prepareVoices();
    }
    bool stealSameNote() const override { return false; }
    float defaultStealFade() const override { return 0.006f; }

    float lfo2Value = 0.0f;
    void advanceLfo2 (int n)
    {
        const auto& P = params->wt;
        lfo2.shape = (int) P.l2Shape;
        lfo2.setRate (P.l2Rate, sr);
        lfo2Value = lfo2.advance (n);
    }

private:
    Lfo lfo2;
};

inline void WavetableVoice::render (float* L, float* R, int n)
{
    if (! active) return;
    const auto& P = eng->params->wt;
    const auto& C = *eng->ctx;
    auto* we = static_cast<WavetableEngine*> (eng);
    const float sr = eng->sr;

    // ---- control-rate modulation (once per chunk)
    lfo1.setRate (P.l1Rate, sr);
    const float l1 = lfo1.advance (n) * lfo1Fade;
    lfo1Fade = std::min (1.0f, lfo1Fade + lfo1FadeInc * (float) n);
    const float l2 = we->lfo2Value;
    for (int k = 0; k < 2; ++k)
    {
        if (rng.uni() < 0.02f) driftT[k] = rng.bi();
        driftV[k] += (driftT[k] - driftV[k]) * 0.01f;
    }
    const float press = std::max (C.aftertouch, pressure);
    const float drift = P.drift * 0.12f;
    const float menv0 = modEnv.v;
    const float vib = l1 * P.l1Pitch + press * P.atVib * sin01 ((float) key * 0.1f);
    const float baseNote = curNote + vib + P.mPitch * menv0;

    const int uni = std::clamp ((int) P.unison, 1, kMaxUni);
    const float n1 = baseNote + P.t1Oct * 12.0f + P.t1Semi + P.t1Fine * 0.01f + drift * driftV[0];
    const float n2 = baseNote + P.t2Oct * 12.0f + P.t2Semi + P.t2Fine * 0.01f + drift * driftV[1];
    const float f1 = C.hz (n1), f2 = C.hz (n2);
    float inc1[kMaxUni], inc2[kMaxUni], gL[kMaxUni], gR[kMaxUni], uPos[kMaxUni];
    const bool stereo = uni > 1 && P.uniSpread > 0.01f;
    for (int k = 0; k < uni; ++k)
    {
        const float posU = uni > 1 ? (float) k / (float) (uni - 1) * 2.0f - 1.0f : 0.0f;
        const float c = centsToRatio (posU * P.uniDetune * 0.5f);
        inc1[k] = std::min (f1 * c / sr, 0.45f);
        inc2[k] = std::min (f2 * c / sr, 0.45f);
        uPos[k] = posU * P.uniPos * ((k & 1) ? 1.0f : -1.0f);
        const float upan = posU * P.uniSpread * ((k & 1) ? 1.0f : -1.0f) * (k == 0 && (uni & 1) ? 0.0f : 1.0f);
        panGains (upan, gL[k], gR[k]);
    }
    const float uniNorm = 1.0f / std::sqrt ((float) uni);
    const float incSub = std::min (f1 * 0.5f / sr, 0.45f);

    const float warpMod = P.mWarp * menv0;
    const auto o1 = setupOsc (P.t1Table, P.t1Level, P.t1Warp, P.t1WarpAmt + warpMod, inc1[uni - 1]);
    const auto o2 = setupOsc (P.t2Table, P.t2Level, P.t2Warp, P.t2WarpAmt + warpMod, inc2[uni - 1]);
    const bool use1 = o1.gain > 0.0f, use2 = o2.gain > 0.0f || o1.warp == 6;

    // table position targets (modulation by velocity, LFOs, mod wheel, pressure)
    const float posMod = P.velPos * (vel - 0.5f) * 2.0f * 0.5f + l1 * P.l1Pos + l2 * P.l2Pos
                       + C.modWheel * P.mwPos + press * P.atPos;

    // filter cutoff (octaves)
    const float cutOct = std::log2 (std::max (P.cutoff, 10.0f))
                       + P.keyTrack * (curNote - 60.0f) / 12.0f
                       + P.fVel * (vel - 0.5f)
                       + l1 * P.l1Cut + l2 * P.l2Cut
                       + C.modWheel * P.mwCut + press * P.atCut;
    const int ft = (int) P.fType;
    const float res = P.res;
    const float drive = 1.0f + 3.0f * P.fDrive;
    hpL.setCutoff (P.hpCut, sr); hpR.setCutoff (P.hpCut, sr);
    if (ft == 6) { const float shift = std::pow (2.0f, clampf (cutOct - 10.55f, -2.0f, 2.0f) * 0.35f);
                   fmtL.set (P.vowel, shift, res, sr); fmtR.set (P.vowel, shift, res, sr); }

    const float l1Amp = 1.0f - P.l1Amp * (0.5f + 0.5f * l1);
    float vpl, vpr;
    panGains (pan + l2 * P.l2Pan, vpl, vpr);
    const float out = velGain * 0.4f * (eng->params->common.poly <= 1.0f ? 2.0f : 1.0f);

    // smoothed per-sample position ramps
    const float pos1T = P.t1Pos + posMod, pos2T = P.t2Pos + posMod;
    if (! posInit) { pos1Cur = pos1T; pos2Cur = pos2T; posInit = true; }
    const float d1 = (pos1T - pos1Cur) / (float) n, d2 = (pos2T - pos2Cur) / (float) n;

    for (int i = 0; i < n; ++i)
    {
        const float me = modEnv.next();
        pos1Cur += d1; pos2Cur += d2;
        const float p1 = pos1Cur + P.mPos1 * me, p2 = pos2Cur + P.mPos2 * me;

        float sl = 0.0f, sr_ = 0.0f;
        for (int k = 0; k < uni; ++k)
        {
            float b = 0.0f;
            if (use2)
            {
                b = oscRead (o2, p2 + uPos[k], ph2[k], fb2[k]);
                fb2[k] = b;
                ph2[k] += inc2[k]; if (ph2[k] >= 1.0f) ph2[k] -= 1.0f;
            }
            float a = 0.0f;
            if (use1)
            {
                a = oscRead (o1, p1 + uPos[k], ph1[k], b);
                ph1[k] += inc1[k]; if (ph1[k] >= 1.0f) ph1[k] -= 1.0f;
            }
            const float m = a * o1.gain + b * o2.gain;
            sl += m * gL[k]; sr_ += m * gR[k];
        }
        sl *= uniNorm * 1.41f; sr_ *= uniNorm * 1.41f;
        float mono = 0.0f;
        if (P.subLevel > 0.0f) { mono += sin01 (phSub) * P.subLevel * 0.9f; phSub += incSub; if (phSub >= 1.0f) phSub -= 1.0f; }
        if (P.noiseLevel > 0.0f) mono += rng.bi() * P.noiseLevel * 0.6f;
        sl += mono; sr_ += mono;
        if (! stereo) { sl = sr_ = (sl + sr_) * 0.5f; }

        // filter
        const float fe = filtEnv.next();
        const float fc = clampf (std::exp2 (cutOct + P.fEnv * fe), 20.0f, sr * 0.45f);
        float yl, yr;
        if (P.hpCut > 21.0f) { sl = hpL.process (sl); sr_ = hpR.process (sr_); }
        switch (ft)
        {
            case 0:
                ladL.set (fc, res, sr); yl = ladL.process (sl, drive);
                if (stereo) { ladR.set (fc, res, sr); yr = ladR.process (sr_, drive); } else yr = yl;
                break;
            case 5:
                svf2L.set (fc * 0.5f, res * 0.8f, sr); ladL.set (fc, res * 0.9f, sr);
                yl = ladL.process (svf2L.highpass (sl), drive);
                if (stereo) { svf2R.set (fc * 0.5f, res * 0.8f, sr); ladR.set (fc, res * 0.9f, sr); yr = ladR.process (svf2R.highpass (sr_), drive); } else yr = yl;
                break;
            case 6:
                yl = fmtL.process (fastTanh (sl * drive) / drive);
                yr = stereo ? fmtR.process (fastTanh (sr_ * drive) / drive) : yl;
                break;
            default:
            {
                svfL.set (fc, res, sr);
                float lp, bp, hp;
                svfL.process (fastTanh (sl * drive * 0.7f) / (drive * 0.7f), lp, bp, hp);
                yl = ft == 1 ? lp : ft == 2 ? bp * std::sqrt (svfL.k) : ft == 3 ? hp : (lp + hp);
                if (stereo)
                {
                    svfR.set (fc, res, sr);
                    svfR.process (fastTanh (sr_ * drive * 0.7f) / (drive * 0.7f), lp, bp, hp);
                    yr = ft == 1 ? lp : ft == 2 ? bp * std::sqrt (svfR.k) : ft == 3 ? hp : (lp + hp);
                }
                else yr = yl;
                break;
            }
        }

        const float a = ampEnv.next() * out * l1Amp;
        float l = yl * a * (stereo ? 1.0f : vpl * 1.41f);
        float r = yr * a * (stereo ? 1.0f : vpr * 1.41f);
        if (! applyFade (l, r)) { active = false; return; }
        L[i] += l; R[i] += r;
        if (! ampEnv.active()) { active = false; return; }
    }
}

} // namespace kv
