// Keyverse - core DSP building blocks.
// Header-only, dependency-free C++17 so the engine can be built and tested
// without JUCE (see tools/) and dropped into the plugin unchanged.
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#if defined(_MSC_VER)
 #define KV_SIMD_REDUCE(...)
#else
 #define KV_PRAGMA(x) _Pragma(#x)
 #define KV_SIMD_REDUCE(...) KV_PRAGMA(omp simd reduction(+:__VA_ARGS__))
#endif

namespace kv
{
constexpr float kPi    = 3.14159265358979323846f;
constexpr float kTwoPi = 6.28318530717958647692f;

inline float clampf (float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
inline float lerpf (float a, float b, float t)    { return a + (b - a) * t; }
inline float dbToGain (float db)                  { return std::pow (10.0f, db * 0.05f); }
inline float gainToDb (float g)                   { return 20.0f * std::log10 (std::max (g, 1.0e-9f)); }
inline float centsToRatio (float c)               { return std::exp2 (c * (1.0f / 1200.0f)); }
inline float midiToHz (float note, float a4 = 440.0f) { return a4 * std::exp2 ((note - 69.0f) * (1.0f / 12.0f)); }
inline float t60ToCoef (float t60, float sr)      { return std::exp (-6.907755f / (std::max (t60, 1.0e-4f) * sr)); }
inline float sqr (float x)                        { return x * x; }

// Map a 0..1 macro around a preset's base value: 0 -> lo, 0.5 -> base, 1 -> hi.
inline float macroMap (float base, float m, float lo, float hi)
{
    return m < 0.5f ? lerpf (lo, base, m * 2.0f) : lerpf (base, hi, (m - 0.5f) * 2.0f);
}

// Multiplicative macro: 0 -> base/range, 0.5 -> base, 1 -> base*range.
inline float macroScale (float base, float m, float range)
{
    return base * std::pow (range, (m - 0.5f) * 2.0f);
}

// Rational tanh approximation (Lambert continued fraction), exact enough for audio.
inline float fastTanh (float x)
{
    if (x > 4.97f)  return 1.0f;
    if (x < -4.97f) return -1.0f;
    const float x2 = x * x;
    const float a = x * (135135.0f + x2 * (17325.0f + x2 * (378.0f + x2)));
    const float b = 135135.0f + x2 * (62370.0f + x2 * (3150.0f + x2 * 28.0f));
    return a / b;
}

// Smooth asymmetric saturator (tube-ish): adds even harmonics.
inline float asymSat (float x, float bias)
{
    return fastTanh (x + bias) - fastTanh (bias);
}

//==============================================================================
struct Rng
{
    uint32_t s = 0x9E3779B9u;
    void seed (uint32_t v) { s = v ? v : 0x9E3779B9u; }
    uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    float uni() { return (float) (next() >> 8) * (1.0f / 16777216.0f); }
    float bi()  { return uni() * 2.0f - 1.0f; }
};

// Deterministic per-note pseudo random value in [-1,1] (stable across sessions).
inline float hashNoise (uint32_t a, uint32_t b)
{
    uint32_t h = a * 0x27d4eb2du ^ (b + 0x165667b1u) * 0x9E3779B1u;
    h ^= h >> 15; h *= 0x85ebca6bu; h ^= h >> 13; h *= 0xc2b2ae35u; h ^= h >> 16;
    return (float) (h >> 8) * (2.0f / 16777216.0f) - 1.0f;
}

//==============================================================================
struct SineTable
{
    static constexpr int N = 4096;
    float t[N + 1];
    SineTable()
    {
        for (int i = 0; i <= N; ++i)
            t[i] = (float) std::sin (2.0 * 3.14159265358979323846 * i / N);
    }
    // phase in cycles, any value (wrapped)
    float operator() (float phase) const
    {
        phase -= std::floor (phase);
        const float f = phase * N;
        const int i = (int) f;
        const float fr = f - (float) i;
        return t[i] + fr * (t[i + 1] - t[i]);
    }
};
inline const SineTable& sine() { static const SineTable s; return s; }
inline float sin01 (float phase) { return sine() (phase); }

//==============================================================================
struct Smoother
{
    float y = 0.0f, a = 0.0f;
    void setTime (float sec, float sr) { a = std::exp (-1.0f / (std::max (sec, 1.0e-5f) * sr)); }
    void reset (float v) { y = v; }
    float process (float x) { y = x + a * (y - x); return y; }
};

//==============================================================================
// Analog-style ADSR: exponential segments, overshooting attack target.
struct Adsr
{
    enum Stage { Idle, Attack, Decay, Sustain, Release };
    Stage stage = Idle;
    float v = 0.0f;
    float attCoef = 0.0f, decCoef = 0.0f, relCoef = 0.0f, sus = 1.0f;

    void set (float a, float d, float s, float r, float sr)
    {
        a = std::max (a, 0.0005f);
        const float tauA = a / 1.466f;            // reaches 1.0 aiming at 1.3
        attCoef = 1.0f - std::exp (-1.0f / (tauA * sr));
        decCoef = std::exp (-4.6f / (std::max (d, 0.001f) * sr));
        relCoef = std::exp (-6.9f / (std::max (r, 0.002f) * sr));
        sus = clampf (s, 0.0f, 1.0f);
    }
    void noteOn (bool fromZero) { stage = Attack; if (fromZero) v = 0.0f; }
    void noteOff() { if (stage != Idle) stage = Release; }
    void kill() { stage = Idle; v = 0.0f; }
    bool active() const { return stage != Idle; }

    float next()
    {
        switch (stage)
        {
            case Attack:
                v += (1.3f - v) * attCoef;
                if (v >= 1.0f) { v = 1.0f; stage = Decay; }
                break;
            case Decay:
                v = sus + (v - sus) * decCoef;
                if (v - sus < 1.0e-4f) { v = sus; stage = Sustain; }
                break;
            case Sustain:
                v = sus;
                if (sus <= 1.0e-5f) { stage = Idle; v = 0.0f; }
                break;
            case Release:
                v *= relCoef;
                if (v < 1.0e-5f) { v = 0.0f; stage = Idle; }
                break;
            case Idle: default: break;
        }
        return v;
    }
};

//==============================================================================
struct Lfo
{
    enum Shape { Sine = 0, Triangle, SawUp, SawDown, Square, SampleHold, SmoothRandom, NumShapes };
    float phase = 0.0f, inc = 0.0f, sh = 0.0f, shPrev = 0.0f;
    int shape = Sine;
    Rng rng;

    void setRate (float hz, float sr) { inc = hz / sr; }
    void reset (float ph) { phase = ph - std::floor (ph); }

    // advance by n samples and return value in [-1,1]
    float advance (int n)
    {
        phase += inc * (float) n;
        bool wrapped = false;
        if (phase >= 1.0f) { phase -= std::floor (phase); wrapped = true; }
        if (wrapped) { shPrev = sh; sh = rng.bi(); }
        return value();
    }
    float value() const
    {
        switch (shape)
        {
            case Triangle:     return 1.0f - 4.0f * std::fabs (phase - 0.5f);
            case SawUp:        return phase * 2.0f - 1.0f;
            case SawDown:      return 1.0f - phase * 2.0f;
            case Square:       return phase < 0.5f ? 1.0f : -1.0f;
            case SampleHold:   return sh;
            case SmoothRandom: { const float t = phase * phase * (3.0f - 2.0f * phase); return shPrev + (sh - shPrev) * t; }
            case Sine: default: return sin01 (phase);
        }
    }
};

//==============================================================================
struct OnePoleLP
{
    float a = 0.0f, z = 0.0f;
    void setCutoff (float hz, float sr) { a = std::exp (-kTwoPi * clampf (hz, 1.0f, sr * 0.49f) / sr); }
    float process (float x) { z = x + a * (z - x); return z; }
    void reset() { z = 0.0f; }
};

struct OnePoleHP
{
    OnePoleLP lp;
    void setCutoff (float hz, float sr) { lp.setCutoff (hz, sr); }
    float process (float x) { return x - lp.process (x); }
    void reset() { lp.reset(); }
};

// DC blocker
struct DcBlock
{
    float x1 = 0.0f, y1 = 0.0f, R = 0.995f;
    void setup (float sr) { R = 1.0f - (kTwoPi * 10.0f / sr); }
    float process (float x) { const float y = x - x1 + R * y1; x1 = x; y1 = y; return y; }
    void reset() { x1 = y1 = 0.0f; }
};

//==============================================================================
// RBJ biquad, transposed direct form II.
struct Biquad
{
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;

    void reset() { z1 = z2 = 0.0f; }
    float process (float x)
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void setBypass() { b0 = 1.0f; b1 = b2 = a1 = a2 = 0.0f; }

    void norm (double B0, double B1, double B2, double A0, double A1, double A2)
    {
        b0 = (float) (B0 / A0); b1 = (float) (B1 / A0); b2 = (float) (B2 / A0);
        a1 = (float) (A1 / A0); a2 = (float) (A2 / A0);
    }
    static double w0 (double f, double sr) { return 2.0 * 3.14159265358979323846 * std::min (std::max (f, 1.0), sr * 0.49) / sr; }

    void setLowpass (double f, double q, double sr)
    {
        const double w = w0 (f, sr), c = std::cos (w), al = std::sin (w) / (2.0 * q);
        norm ((1 - c) / 2, 1 - c, (1 - c) / 2, 1 + al, -2 * c, 1 - al);
    }
    void setHighpass (double f, double q, double sr)
    {
        const double w = w0 (f, sr), c = std::cos (w), al = std::sin (w) / (2.0 * q);
        norm ((1 + c) / 2, -(1 + c), (1 + c) / 2, 1 + al, -2 * c, 1 - al);
    }
    void setBandpass (double f, double q, double sr) // 0 dB peak
    {
        const double w = w0 (f, sr), c = std::cos (w), al = std::sin (w) / (2.0 * q);
        norm (al, 0, -al, 1 + al, -2 * c, 1 - al);
    }
    void setNotch (double f, double q, double sr)
    {
        const double w = w0 (f, sr), c = std::cos (w), al = std::sin (w) / (2.0 * q);
        norm (1, -2 * c, 1, 1 + al, -2 * c, 1 - al);
    }
    void setPeak (double f, double q, double gainDb, double sr)
    {
        const double A = std::pow (10.0, gainDb / 40.0), w = w0 (f, sr), c = std::cos (w), al = std::sin (w) / (2.0 * q);
        norm (1 + al * A, -2 * c, 1 - al * A, 1 + al / A, -2 * c, 1 - al / A);
    }
    void setLowShelf (double f, double gainDb, double sr, double S = 1.0)
    {
        const double A = std::pow (10.0, gainDb / 40.0), w = w0 (f, sr), c = std::cos (w), s = std::sin (w);
        const double al = s / 2.0 * std::sqrt ((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0), sq = 2.0 * std::sqrt (A) * al;
        norm (A * ((A + 1) - (A - 1) * c + sq), 2 * A * ((A - 1) - (A + 1) * c), A * ((A + 1) - (A - 1) * c - sq),
              (A + 1) + (A - 1) * c + sq, -2 * ((A - 1) + (A + 1) * c), (A + 1) + (A - 1) * c - sq);
    }
    void setHighShelf (double f, double gainDb, double sr, double S = 1.0)
    {
        const double A = std::pow (10.0, gainDb / 40.0), w = w0 (f, sr), c = std::cos (w), s = std::sin (w);
        const double al = s / 2.0 * std::sqrt ((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0), sq = 2.0 * std::sqrt (A) * al;
        norm (A * ((A + 1) + (A - 1) * c + sq), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - sq),
              (A + 1) - (A - 1) * c + sq, 2 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - sq);
    }
    void setAllpass (double f, double q, double sr)
    {
        const double w = w0 (f, sr), c = std::cos (w), al = std::sin (w) / (2.0 * q);
        norm (1 - al, -2 * c, 1 + al, 1 + al, -2 * c, 1 - al);
    }
};

//==============================================================================
// Cytomic / Zavalishin TPT state variable filter.
struct Svf
{
    float ic1 = 0.0f, ic2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f, a3 = 0.0f, k = 1.0f;

    void set (float fc, float res, float sr)   // res 0..1 (1 = self oscillation)
    {
        const float g = std::tan (kPi * clampf (fc, 5.0f, sr * 0.48f) / sr);
        k = 2.0f - 1.98f * clampf (res, 0.0f, 1.0f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }
    void setQ (float fc, float q, float sr)
    {
        const float g = std::tan (kPi * clampf (fc, 5.0f, sr * 0.48f) / sr);
        k = 1.0f / std::max (q, 0.05f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }
    void process (float v0, float& lp, float& bp, float& hp)
    {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        lp = v2; bp = v1; hp = v0 - k * v1 - v2;
    }
    float lowpass (float x)  { float l, b, h; process (x, l, b, h); return l; }
    float bandpass (float x) { float l, b, h; process (x, l, b, h); return b; }
    float highpass (float x) { float l, b, h; process (x, l, b, h); return h; }
    void reset() { ic1 = ic2 = 0.0f; }
};

//==============================================================================
// 4-pole transistor ladder (zero-delay feedback, saturated input stage).
struct Ladder
{
    float s[4] = { 0, 0, 0, 0 };
    float G = 0.0f, g = 0.0f, k = 0.0f;

    void set (float fc, float res, float sr)
    {
        g = std::tan (kPi * clampf (fc, 5.0f, sr * 0.47f) / sr);
        G = g / (1.0f + g);
        k = 4.0f * clampf (res, 0.0f, 1.05f);
    }
    float process (float x, float drive)
    {
        const float inv = 1.0f / (1.0f + g);
        const float G2 = G * G, G3 = G2 * G, G4 = G3 * G;
        const float S = G3 * s[0] * inv + G2 * s[1] * inv + G * s[2] * inv + s[3] * inv;
        const float comp = 1.0f + 0.45f * k;
        const float xin = x * drive * comp;
        const float y4 = (G4 * xin + S) / (1.0f + k * G4);
        float u = fastTanh (xin - k * y4);
        for (int i = 0; i < 4; ++i)
        {
            const float v = (u - s[i]) * G;
            const float y = v + s[i];
            s[i] = y + v;
            u = y;
        }
        return u / std::max (drive, 0.2f);
    }
    void reset() { s[0] = s[1] = s[2] = s[3] = 0.0f; }
};

//==============================================================================
struct DelayLine
{
    std::vector<float> buf;
    int mask = 0, w = 0;

    void init (int maxSamples)
    {
        int size = 16;
        while (size < maxSamples + 4) size <<= 1;
        buf.assign ((size_t) size, 0.0f);
        mask = size - 1;
        w = 0;
    }
    void clear() { std::fill (buf.begin(), buf.end(), 0.0f); w = 0; }
    int capacity() const { return mask - 3; }
    void push (float x) { buf[(size_t) w] = x; w = (w + 1) & mask; }
    // d = 0 -> most recently pushed sample
    float tap (int d) const { return buf[(size_t) ((w - 1 - d) & mask)]; }
    float tapLinear (float d) const
    {
        const int i = (int) d;
        const float f = d - (float) i;
        const float a = tap (i), b = tap (i + 1);
        return a + (b - a) * f;
    }
    float tapCubic (float d) const
    {
        const int i = (int) d;
        const float f = d - (float) i;
        const float xm1 = tap (i - 1 < 0 ? 0 : i - 1), x0 = tap (i), x1 = tap (i + 1), x2 = tap (i + 2);
        const float c1 = 0.5f * (x1 - xm1);
        const float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
        const float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
        return ((c3 * f + c2) * f + c1) * f + x0;
    }
};

//==============================================================================
// PolyBLEP helpers for alias-reduced classic waveforms.
inline float polyBlep (float t, float dt)
{
    if (t < dt)        { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

//==============================================================================
// Asymmetric "hammer/mallet force" pulse with smooth spectrum (gamma shape).
// corner = frequency where the spectrum starts to roll off; p = 1 (hard) .. 4 (soft).
// The pulse has unit area so low partials keep their level across dynamics.
inline int makeForcePulse (float* out, int maxLen, float sr, float corner, float p)
{
    corner = clampf (corner, 20.0f, sr * 0.45f);
    const float tau = 1.0f / (kTwoPi * corner);
    int len = (int) std::ceil ((p + 9.0f) * tau * sr) + 2;
    len = std::min (std::max (len, 2), maxLen);
    double sum = 0.0;
    for (int i = 0; i < len; ++i)
    {
        const float t = ((float) i + 0.5f) / (sr * tau);
        const float v = std::pow (t, p) * std::exp (-t);
        out[i] = v;
        sum += v;
    }
    const float inv = sum > 0.0 ? (float) (1.0 / sum) : 0.0f;
    for (int i = 0; i < len; ++i) out[i] *= inv;
    return len;
}

// Magnitude of the force pulse spectrum at frequency f (matches makeForcePulse).
inline float forcePulseMag (float f, float corner, float p)
{
    const float x = f / std::max (corner, 1.0f);
    return std::pow (1.0f + x * x, -(p + 1.0f) * 0.5f);
}

//==============================================================================
// 2x oversampler with a Kaiser-windowed half-band FIR (used by drive stages).
class HalfbandOversampler
{
public:
    static constexpr int kTaps = 31;      // must be 4k+3
    static constexpr int kHalf = (kTaps + 1) / 2;  // even-index taps (non-zero)

    HalfbandOversampler()
    {
        const int M = (kTaps - 1) / 2;
        const double beta = 8.0;
        auto bessel0 = [] (double x)
        {
            double sum = 1.0, term = 1.0;
            for (int k = 1; k < 30; ++k) { term *= (x / (2.0 * k)) * (x / (2.0 * k)); sum += term; }
            return sum;
        };
        const double denom = bessel0 (beta);
        for (int j = 0; j < kHalf; ++j)
        {
            const int n = 2 * j;
            const double m = n - M;  // odd distance from centre
            const double sinc = std::sin (0.5 * 3.14159265358979323846 * m) / (3.14159265358979323846 * m);
            const double r = (double) (n - M) / M;
            const double w = bessel0 (beta * std::sqrt (std::max (0.0, 1.0 - r * r))) / denom;
            h[j] = (float) (sinc * w);
        }
        reset();
    }
    void reset()
    {
        std::fill (std::begin (upHist), std::end (upHist), 0.0f);
        std::fill (std::begin (dnOdd), std::end (dnOdd), 0.0f);
        std::fill (std::begin (dnEven), std::end (dnEven), 0.0f);
        upPos = dnPos = 0;
    }
    // Produces two samples at 2x rate from one input sample.
    void up (float x, float& y0, float& y1)
    {
        upPos = (upPos + 1) % kHalf;
        upHist[upPos] = x;
        float acc = 0.0f;
        for (int j = 0; j < kHalf; ++j)
            acc += h[j] * upHist[(upPos - j + kHalf) % kHalf];
        y0 = 2.0f * acc;
        y1 = upHist[(upPos - kDelay + kHalf) % kHalf];
    }
    // Consumes two 2x-rate samples, returns one base-rate sample.
    float down (float v0, float v1)
    {
        dnPos = (dnPos + 1) % kHalf;
        dnEven[dnPos] = v0;
        dnOdd[dnPos] = v1;
        float acc = 0.0f;
        for (int j = 0; j < kHalf; ++j)
            acc += h[j] * dnOdd[(dnPos - j + kHalf) % kHalf];
        return acc + 0.5f * dnEven[(dnPos - kDelay + kHalf) % kHalf];
    }

private:
    static constexpr int kDelay = (kTaps - 1) / 4;  // centre tap offset in base-rate samples
    float h[kHalf] {};
    float upHist[kHalf] {}, dnOdd[kHalf] {}, dnEven[kHalf] {};
    int upPos = 0, dnPos = 0;
};

//==============================================================================
inline void panGains (float pan, float& l, float& r)   // pan -1..1, constant power
{
    const float p = (clampf (pan, -1.0f, 1.0f) + 1.0f) * 0.125f;  // 0..1/4 cycle
    l = sin01 (p + 0.25f);   // cos
    r = sin01 (p);           // sin
}

} // namespace kv
