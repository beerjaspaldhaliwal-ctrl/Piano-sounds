// Keyverse - effects: per-layer insert chain (compressor, wah, drive/amp +
// cabinet, EQ, two modulation slots incl. rotary speaker) and the global
// chain (stereo/ping-pong delay, FDN reverb, lo-fi, width, limiter).
#pragma once

#include "Params.h"
#include "DSP.h"

namespace kv
{
//==============================================================================
class Compressor
{
public:
    void prepare (float sr_) { sr = sr_; env = 0.0f; }
    void reset() { env = 0.0f; }
    void process (float* L, float* R, int n, float amount)
    {
        if (amount <= 0.001f) return;
        const float thr = -8.0f - 26.0f * amount;
        const float ratio = 2.0f + 6.0f * amount;
        const float makeup = -thr * (1.0f - 1.0f / ratio) * 0.55f;
        const float att = std::exp (-1.0f / (0.004f * sr)), rel = std::exp (-1.0f / (0.12f * sr));
        for (int i = 0; i < n; ++i)
        {
            const float x = std::max (std::fabs (L[i]), std::fabs (R[i]));
            env = x > env ? x + (env - x) * att : x + (env - x) * rel;
            const float over = gainToDb (env) - thr;
            float gr = 0.0f;
            if (over > 3.0f) gr = over * (1.0f - 1.0f / ratio);
            else if (over > -3.0f) gr = (1.0f - 1.0f / ratio) * sqr (over + 3.0f) / 12.0f;
            const float g = dbToGain (makeup - gr);
            L[i] *= g; R[i] *= g;
        }
    }

private:
    float sr = 48000.0f, env = 0.0f;
};

//==============================================================================
class Wah
{
public:
    void prepare (float sr_) { sr = sr_; reset(); }
    void reset() { fl.reset(); fr.reset(); env = 0.0f; phase = 0.0f; pos = 0.0f; }
    void process (float* L, float* R, int n, const InsertFxParams& p, float modWheel)
    {
        const int mode = (int) p.wahMode;
        if (mode <= 0) return;
        const float att = std::exp (-1.0f / (0.004f * sr)), rel = std::exp (-1.0f / (0.09f * sr));
        for (int i = 0; i < n; i += 8)
        {
            const int m = std::min (8, n - i);
            float target = 0.0f;
            if (mode == 1)
            {
                float pk = 0.0f;
                for (int k = 0; k < m; ++k) pk = std::max (pk, std::fabs (L[i + k]) + std::fabs (R[i + k]));
                env = pk > env ? pk + (env - pk) * std::pow (att, (float) m) : pk + (env - pk) * std::pow (rel, (float) m);
                // square-root response: sweeps across the whole playing range instead of pinning open
                target = clampf (std::sqrt (env) * 0.85f * (0.3f + p.wahDepth), 0.0f, 1.0f);
            }
            else if (mode == 2)
            {
                phase += p.wahRate * (float) m / sr; phase -= std::floor (phase);
                target = 0.5f + 0.5f * sin01 (phase);
            }
            else target = modWheel;
            pos += (target - pos) * 0.25f;
            const float f = clampf (p.wahFreq * std::exp2 ((pos - 0.35f) * (0.6f + 3.4f * p.wahDepth)), 120.0f, sr * 0.4f);
            fl.setQ (f, p.wahQ, sr); fr.setQ (f, p.wahQ, sr);
            for (int k = 0; k < m; ++k)
            {
                float lp, bp, hp;
                // normalised band-pass (peak gain 1) plus a little low end, like a wah pedal
                fl.process (L[i + k], lp, bp, hp); L[i + k] = bp * fl.k * 2.6f + lp * 0.25f;
                fr.process (R[i + k], lp, bp, hp); R[i + k] = bp * fr.k * 2.6f + lp * 0.25f;
            }
        }
    }

private:
    float sr = 48000.0f, env = 0.0f, phase = 0.0f, pos = 0.0f;
    Svf fl, fr;
};

//==============================================================================
class AmpSim
{
public:
    void prepare (float sr_) { sr = sr_; lastKey = -1.0f; reset(); }
    void reset()
    {
        osL.reset(); osR.reset();
        for (auto& b : pre)  for (auto& f : b) f.reset();
        for (auto& b : tone) for (auto& f : b) f.reset();
        for (auto& b : cab)  for (auto& f : b) f.reset();
        dcL.reset(); dcR.reset();
    }

    void process (float* L, float* R, int n, const InsertFxParams& p)
    {
        const int type = (int) p.ampType;
        const int cabT = (int) p.ampCab;
        if (type <= 0 && cabT <= 0) return;
        const float key = (float) type * 1000.0f + (float) cabT * 100.0f + p.ampGain * 10.0f + p.ampTone + p.ampLevel * 0.001f;
        if (std::fabs (key - lastKey) > 1.0e-6f) configure (type, cabT, p);
        dcL.setup (sr); dcR.setup (sr);
        for (int i = 0; i < n; ++i)
        {
            L[i] = channel (L[i], 0, osL, dcL, type);
            R[i] = channel (R[i], 1, osR, dcR, type);
        }
    }

private:
    float shape (float x, int type) const
    {
        switch (type)
        {
            case 1: return asymSat (x, 0.18f);                                   // tube warmth
            case 2: return fastTanh (x + 0.1f * x * x);                          // overdrive (asym soft)
            case 3: { const float y = clampf (x + 0.25f, -1.0f, 1.0f); return fastTanh (2.0f * y) * 0.8f; } // fuzz
            case 4: return asymSat (x, 0.08f);                                   // clean amp
            case 5: return fastTanh (asymSat (x, 0.22f) * 2.2f);                 // crunch
            case 6: return fastTanh (fastTanh (asymSat (x, 0.2f) * 3.0f) * 3.0f); // lead
            case 7: return asymSat (x, 0.05f);                                   // bass
            case 8: return asymSat (x, 0.3f) + 0.05f * x;                        // organ (Leslie 122)
            case 9: return fastTanh (x) * 0.9f + 0.1f * x;                       // tape
            default: return x;
        }
    }

    float channel (float x, int ch, HalfbandOversampler& os, DcBlock& dc, int type)
    {
        x *= inGain;
        for (int k = 0; k < nPre; ++k) x = pre[ch][k].process (x);
        if (type > 0)
        {
            float a, b;
            os.up (x, a, b);
            a = shape (a, type); b = shape (b, type);
            x = os.down (a, b) * outNorm;
            x = dc.process (x);
        }
        for (int k = 0; k < nTone; ++k) x = tone[ch][k].process (x);
        for (int k = 0; k < nCab; ++k)  x = cab[ch][k].process (x);
        return x * outGain;
    }

    void configure (int type, int cabT, const InsertFxParams& p)
    {
        lastKey = (float) type * 1000.0f + (float) cabT * 100.0f + p.ampGain * 10.0f + p.ampTone + p.ampLevel * 0.001f;
        const float g = p.ampGain;
        static const float gMin[10] = { 1, 1.0f, 2.0f, 6.0f, 1.0f, 2.0f, 6.0f, 1.0f, 1.0f, 1.0f };
        static const float gMax[10] = { 1, 8.0f, 60.0f, 400.0f, 8.0f, 40.0f, 400.0f, 10.0f, 16.0f, 6.0f };
        inGain = type > 0 ? gMin[type] * std::pow (gMax[type] / gMin[type], g) : 1.0f;
        outNorm = type > 0 ? 0.3f / std::max (1.0e-3f, std::fabs (shape (0.3f * inGain, type))) : 1.0f;
        if (type == 2 || type == 3 || type == 5 || type == 6) outNorm *= 0.55f;
        outGain = dbToGain (p.ampLevel);

        nPre = 0;
        if (type == 2) { pre[0][nPre].setHighpass (180.0, 0.7, sr); ++nPre; pre[0][nPre].setPeak (750.0, 0.8, 6.0, sr); ++nPre; }
        if (type == 6) { pre[0][nPre].setHighpass (120.0, 0.7, sr); ++nPre; pre[0][nPre].setPeak (900.0, 0.7, 4.0, sr); ++nPre; }
        if (type == 3) { pre[0][nPre].setHighpass (90.0, 0.7, sr); ++nPre; }
        for (int k = 0; k < nPre; ++k) pre[1][k] = pre[0][k];

        nTone = 0;
        const float t = p.ampTone;
        tone[0][nTone++].setLowShelf (140.0, -3.0 * t + (type == 7 ? 3.0 : 0.0), sr);
        tone[0][nTone++].setHighShelf (2800.0, 7.0 * t, sr);
        if (type == 2 || type == 3 || type == 6) tone[0][nTone++].setLowpass (6500.0, 0.7, sr);
        if (type == 9) tone[0][nTone++].setLowpass (11000.0, 0.6, sr);
        for (int k = 0; k < nTone; ++k) tone[1][k] = tone[0][k];

        nCab = 0;
        auto& c = cab[0];
        switch (cabT)
        {
            case 1: c[nCab++].setHighpass (75, 0.7, sr); c[nCab++].setPeak (120, 1.2, 3, sr); c[nCab++].setPeak (500, 1.0, -2, sr);
                    c[nCab++].setPeak (2200, 1.5, 3, sr); c[nCab++].setLowpass (5500, 0.7, sr); break;
            case 2: c[nCab++].setHighpass (70, 0.7, sr); c[nCab++].setPeak (110, 1.2, 3, sr); c[nCab++].setPeak (1800, 1.4, 2, sr);
                    c[nCab++].setLowpass (5000, 0.75, sr); break;
            case 3: c[nCab++].setHighpass (80, 1.0, sr); c[nCab++].setPeak (100, 1.4, 5, sr); c[nCab++].setPeak (420, 1.0, -3, sr);
                    c[nCab++].setPeak (2500, 2.0, 4, sr); c[nCab++].setLowpass (4200, 0.9, sr); break;
            case 4: c[nCab++].setHighpass (40, 0.7, sr); c[nCab++].setPeak (80, 1.0, 3, sr); c[nCab++].setPeak (800, 1.0, -3, sr);
                    c[nCab++].setLowpass (3500, 0.7, sr); break;
            case 5: c[nCab++].setHighpass (90, 0.7, sr); c[nCab++].setPeak (200, 1.0, 2, sr); c[nCab++].setPeak (1500, 1.2, 2, sr);
                    c[nCab++].setLowpass (7000, 0.7, sr); break;
            default: break;
        }
        for (int k = 0; k < nCab; ++k) cab[1][k] = cab[0][k];
    }

    float sr = 48000.0f, lastKey = -1.0f;
    float inGain = 1.0f, outNorm = 1.0f, outGain = 1.0f;
    HalfbandOversampler osL, osR;
    Biquad pre[2][3], tone[2][4], cab[2][6];
    int nPre = 0, nTone = 0, nCab = 0;
    DcBlock dcL, dcR;
};

//==============================================================================
class Equalizer
{
public:
    void prepare (float sr_) { sr = sr_; last[0] = -1e9f; reset(); }
    void reset() { for (auto& c : f) for (auto& b : c) b.reset(); }
    void process (float* L, float* R, int n, const InsertFxParams& p)
    {
        const bool active = std::fabs (p.eqLow) > 0.05f || std::fabs (p.eqMid) > 0.05f || std::fabs (p.eqHigh) > 0.05f
                         || p.lowCut > 21.0f || p.highCut < 19900.0f;
        if (! active) return;
        const float k[6] = { p.eqLow, p.eqMid, p.eqMidF, p.eqHigh, p.lowCut, p.highCut };
        if (std::memcmp (k, last, sizeof (k)) != 0)
        {
            std::memcpy (last, k, sizeof (k));
            f[0][0].setLowShelf (120.0, p.eqLow, sr);
            f[0][1].setPeak (p.eqMidF, 0.8, p.eqMid, sr);
            f[0][2].setHighShelf (6000.0, p.eqHigh, sr);
            if (p.lowCut > 21.0f) f[0][3].setHighpass (p.lowCut, 0.707, sr); else f[0][3].setBypass();
            if (p.highCut < 19900.0f) f[0][4].setLowpass (p.highCut, 0.707, sr); else f[0][4].setBypass();
            for (int i = 0; i < 5; ++i) { f[1][i].b0 = f[0][i].b0; f[1][i].b1 = f[0][i].b1; f[1][i].b2 = f[0][i].b2; f[1][i].a1 = f[0][i].a1; f[1][i].a2 = f[0][i].a2; }
        }
        for (int i = 0; i < n; ++i)
        {
            float l = L[i], r = R[i];
            for (int b = 0; b < 5; ++b) { l = f[0][b].process (l); r = f[1][b].process (r); }
            L[i] = l; R[i] = r;
        }
    }

private:
    float sr = 48000.0f;
    float last[6] {};
    Biquad f[2][5];
};

//==============================================================================
// One modulation slot: chorus, ensemble, phaser, flanger, tremolo, auto-pan,
// rotary speaker, vibrato, dimension.
class ModFx
{
public:
    enum Type { Off = 0, Chorus, Ensemble, Phaser, Flanger, Tremolo, AutoPan, Rotary, Vibrato, Dimension };

    void prepare (float sr_)
    {
        sr = sr_;
        dL.init ((int) (sr * 0.06f)); dR.init ((int) (sr * 0.06f));
        hornL.init ((int) (sr * 0.01f)); hornR.init ((int) (sr * 0.01f));
        drumL.init ((int) (sr * 0.01f)); drumR.init ((int) (sr * 0.01f));
        xoLp.setLowpass (800.0, 0.707, sr); xoHp.setHighpass (800.0, 0.707, sr);
        xoLp2 = xoLp; xoHp2 = xoHp;
        reset();
    }
    void reset()
    {
        dL.clear(); dR.clear(); hornL.clear(); hornR.clear(); drumL.clear(); drumR.clear();
        for (auto& a : apL) a = 0.0f;
        for (auto& a : apR) a = 0.0f;
        phase = 0.0f; phase2 = 0.0f; fbL = fbR = 0.0f;
        hornAng = 0.0f; drumAng = 0.3f; hornSpd = 0.8f; drumSpd = 0.65f;
        xoLp.reset(); xoHp.reset(); xoLp2.reset(); xoHp2.reset();
        lastType = -1;
    }

    void process (float* L, float* R, int n, int type, float rate, float depth, float mix, float fb, float modWheel)
    {
        if (type <= 0) return;
        if (type != lastType) { lastType = type; dL.clear(); dR.clear(); fbL = fbR = 0.0f; }
        switch (type)
        {
            case Chorus:    chorus (L, R, n, rate, depth, mix, 7.0f, 4.0f, fb); break;
            case Dimension: dimension (L, R, n, rate, depth, mix); break;
            case Ensemble:  ensemble (L, R, n, rate, depth, mix); break;
            case Phaser:    phaser (L, R, n, rate, depth, mix, fb); break;
            case Flanger:   chorus (L, R, n, rate, depth, mix, 0.4f, 4.5f, fb); break;
            case Tremolo:   tremolo (L, R, n, rate, depth, fb, false); break;
            case AutoPan:   tremolo (L, R, n, rate, depth, fb, true); break;
            case Rotary:    rotary (L, R, n, rate, depth, mix, modWheel); break;
            case Vibrato:   vibrato (L, R, n, rate, depth, mix); break;
            default: break;
        }
    }

private:
    void chorus (float* L, float* R, int n, float rate, float depth, float mix, float baseMs, float depthMs, float fb)
    {
        const float inc = rate / sr;
        const float base = baseMs * 0.001f * sr, dep = depthMs * 0.001f * sr * depth;
        for (int i = 0; i < n; ++i)
        {
            phase += inc; if (phase >= 1.0f) phase -= 1.0f;
            const float ml = sin01 (phase), mr = sin01 (phase + 0.5f);
            dL.push (L[i] + fbL * fb);
            dR.push (R[i] + fbR * fb);
            const float wl = dL.tapCubic (base + dep * (0.5f + 0.5f * ml));
            const float wr = dR.tapCubic (base + dep * (0.5f + 0.5f * mr));
            fbL = wl; fbR = wr;
            L[i] = L[i] * (1.0f - mix) + wl * mix;
            R[i] = R[i] * (1.0f - mix) + wr * mix;
        }
    }
    void dimension (float* L, float* R, int n, float rate, float depth, float mix)
    {
        const float inc = rate / sr;
        const float base = 0.004f * sr, dep = 0.0012f * sr * (0.3f + depth);
        for (int i = 0; i < n; ++i)
        {
            phase += inc; if (phase >= 1.0f) phase -= 1.0f;
            const float tri = 1.0f - 4.0f * std::fabs (phase - 0.5f);
            const float m = 0.5f * (L[i] + R[i]);
            dL.push (m);
            const float a = dL.tapCubic (base + dep * (0.5f + 0.5f * tri));
            const float b = dL.tapCubic (base + dep * (0.5f - 0.5f * tri));
            L[i] += mix * (a - 0.6f * b) * 0.7f;
            R[i] += mix * (b - 0.6f * a) * 0.7f;
        }
    }
    void ensemble (float* L, float* R, int n, float rate, float depth, float mix)
    {
        const float inc1 = rate / sr, inc2 = (rate * 9.0f + 3.0f) / sr;
        const float base = 0.0055f * sr;
        const float d1 = 0.0018f * sr * depth, d2 = 0.00025f * sr * depth;
        for (int i = 0; i < n; ++i)
        {
            phase += inc1; if (phase >= 1.0f) phase -= 1.0f;
            phase2 += inc2; if (phase2 >= 1.0f) phase2 -= 1.0f;
            const float m = 0.5f * (L[i] + R[i]);
            dL.push (m);
            float wl = 0.0f, wr = 0.0f;
            for (int k = 0; k < 3; ++k)
            {
                const float o = (float) k / 3.0f;
                wl += dL.tapLinear (base + d1 * (1.0f + sin01 (phase + o)) + d2 * (1.0f + sin01 (phase2 + o)));
                wr += dL.tapLinear (base + d1 * (1.0f + sin01 (phase + o + 0.1667f)) + d2 * (1.0f + sin01 (phase2 + o + 0.1667f)));
            }
            L[i] = L[i] * (1.0f - mix) + wl * 0.4f * mix;
            R[i] = R[i] * (1.0f - mix) + wr * 0.4f * mix;
        }
    }
    void phaser (float* L, float* R, int n, float rate, float depth, float mix, float fb)
    {
        const float inc = rate / sr;
        for (int i = 0; i < n; i += 8)
        {
            const int m = std::min (8, n - i);
            phase += inc * (float) m; phase -= std::floor (phase);
            const float fl = 180.0f * std::exp2 (depth * 5.0f * (0.5f + 0.5f * sin01 (phase)));
            const float fr = 180.0f * std::exp2 (depth * 5.0f * (0.5f + 0.5f * sin01 (phase + 0.25f)));
            const float tl = std::tan (kPi * std::min (fl, sr * 0.45f) / sr), tr = std::tan (kPi * std::min (fr, sr * 0.45f) / sr);
            const float al = (tl - 1.0f) / (tl + 1.0f), ar = (tr - 1.0f) / (tr + 1.0f);
            for (int k = 0; k < m; ++k)
            {
                float x = L[i + k] + fbL * fb;
                for (int s = 0; s < 6; ++s) { const float y = al * x + apL[s]; apL[s] = x - al * y; x = y; }
                fbL = x;
                L[i + k] = L[i + k] * (1.0f - mix * 0.5f) + x * mix * 0.5f;
                float z = R[i + k] + fbR * fb;
                for (int s = 0; s < 6; ++s) { const float y = ar * z + apR[s]; apR[s] = z - ar * y; z = y; }
                fbR = z;
                R[i + k] = R[i + k] * (1.0f - mix * 0.5f) + z * mix * 0.5f;
            }
        }
    }
    void tremolo (float* L, float* R, int n, float rate, float depth, float shape, bool pan)
    {
        const float inc = rate / sr;
        for (int i = 0; i < n; ++i)
        {
            phase += inc; if (phase >= 1.0f) phase -= 1.0f;
            float s = sin01 (phase);
            if (shape > 0.0f) s = fastTanh (s * (1.0f + 8.0f * shape)) / fastTanh (1.0f + 8.0f * shape);
            if (pan)
            {
                const float gl = 1.0f - depth * (0.5f + 0.5f * s), gr = 1.0f - depth * (0.5f - 0.5f * s);
                L[i] *= gl * (1.0f + 0.3f * depth); R[i] *= gr * (1.0f + 0.3f * depth);
            }
            else
            {
                const float g = 1.0f - depth * (0.5f + 0.5f * s);
                L[i] *= g; R[i] *= g;
            }
        }
    }
    void vibrato (float* L, float* R, int n, float rate, float depth, float mix)
    {
        const float inc = rate / sr;
        const float base = 0.003f * sr, dep = 0.0035f * sr * depth;
        for (int i = 0; i < n; ++i)
        {
            phase += inc; if (phase >= 1.0f) phase -= 1.0f;
            const float m = sin01 (phase);
            dL.push (L[i]); dR.push (R[i]);
            const float wl = dL.tapCubic (base + dep * (0.5f + 0.5f * m)), wr = dR.tapCubic (base + dep * (0.5f + 0.5f * m));
            L[i] = L[i] * (1.0f - mix) + wl * mix;
            R[i] = R[i] * (1.0f - mix) + wr * mix;
        }
    }
    void rotary (float* L, float* R, int n, float rate, float depth, float mix, float modWheel)
    {
        const bool fast = (rate >= 2.0f) != (modWheel > 0.5f);
        const float hT = fast ? 6.7f : 0.82f, dT = fast ? 5.9f : 0.67f;
        const float hC = std::exp (-1.0f / ((fast ? 0.7f : 0.9f) * sr)), dC = std::exp (-1.0f / ((fast ? 3.5f : 4.5f) * sr));
        const float hornDep = (0.25f + 0.25f * depth) * 0.001f * sr, drumDep = 0.3f * 0.001f * sr;
        const float base = 0.0012f * sr;
        for (int i = 0; i < n; ++i)
        {
            hornSpd = hT + (hornSpd - hT) * hC;
            drumSpd = dT + (drumSpd - dT) * dC;
            hornAng += hornSpd / sr; if (hornAng >= 1.0f) hornAng -= 1.0f;
            drumAng += drumSpd / sr; if (drumAng >= 1.0f) drumAng -= 1.0f;
            const float m = 0.5f * (L[i] + R[i]);
            const float lo = xoLp.process (xoLp2.process (m));
            const float hi = xoHp.process (xoHp2.process (m));
            hornL.push (hi); drumL.push (lo);
            const float hs = sin01 (hornAng), hc = sin01 (hornAng + 0.25f);
            const float hs2 = sin01 (hornAng + 0.32f), hc2 = sin01 (hornAng + 0.57f);
            const float ds = sin01 (drumAng), dc = sin01 (drumAng + 0.25f);
            const float ds2 = sin01 (drumAng + 0.32f), dc2 = sin01 (drumAng + 0.57f);
            const float am = 0.35f + 0.35f * depth;
            const float hl = hornL.tapLinear (base + hornDep * (1.0f + hs)) * (1.0f + am * hc);
            const float hr = hornL.tapLinear (base + hornDep * (1.0f + hs2)) * (1.0f + am * hc2);
            const float dl = drumL.tapLinear (base + drumDep * (1.0f + ds)) * (1.0f + 0.18f * dc);
            const float dr = drumL.tapLinear (base + drumDep * (1.0f + ds2)) * (1.0f + 0.18f * dc2);
            const float wl = (hl + dl) * 0.9f, wr = (hr + dr) * 0.9f;
            L[i] = L[i] * (1.0f - mix) + wl * mix;
            R[i] = R[i] * (1.0f - mix) + wr * mix;
        }
    }

    float sr = 48000.0f;
    DelayLine dL, dR, hornL, hornR, drumL, drumR;
    float apL[6] {}, apR[6] {};
    float phase = 0.0f, phase2 = 0.0f, fbL = 0.0f, fbR = 0.0f;
    float hornAng = 0.0f, drumAng = 0.0f, hornSpd = 0.8f, drumSpd = 0.65f;
    Biquad xoLp, xoHp, xoLp2, xoHp2;
    int lastType = -1;
};

//==============================================================================
class InsertChain
{
public:
    void prepare (float sr) { comp.prepare (sr); wah.prepare (sr); amp.prepare (sr); eq.prepare (sr); m1.prepare (sr); m2.prepare (sr); }
    void reset() { comp.reset(); wah.reset(); amp.reset(); eq.reset(); m1.reset(); m2.reset(); }
    void process (float* L, float* R, int n, const InsertFxParams& p, float modWheel)
    {
        comp.process (L, R, n, p.compAmt);
        wah.process (L, R, n, p, modWheel);
        amp.process (L, R, n, p);
        eq.process (L, R, n, p);
        m1.process (L, R, n, (int) p.m1Type, p.m1Rate, p.m1Depth, p.m1Mix, p.m1Fb, modWheel);
        m2.process (L, R, n, (int) p.m2Type, p.m2Rate, p.m2Depth, p.m2Mix, p.m2Fb, modWheel);
    }

private:
    Compressor comp;
    Wah wah;
    AmpSim amp;
    Equalizer eq;
    ModFx m1, m2;
};

//==============================================================================
class StereoDelay
{
public:
    void prepare (float sr_)
    {
        sr = sr_;
        dl.init ((int) (sr * 2.6f)); dr.init ((int) (sr * 2.6f));
        reset();
    }
    void reset() { dl.clear(); dr.clear(); lpL.reset(); lpR.reset(); hpL.reset(); hpR.reset(); phase = 0.0f; timeSm.reset (-1.0f); }

    static float syncBeats (int s)
    {
        static const float b[12] = { 0, 1.0f, 0.5f, 0.75f, 1.0f / 3.0f, 0.25f, 0.375f, 1.0f / 6.0f, 2.0f, 1.5f, 2.0f / 3.0f, 0.125f };
        return b[std::clamp (s, 0, 11)];
    }

    // in: send buffers; out: added into L/R
    void process (const float* inL, const float* inR, float* L, float* R, int n, const GlobalFxParams& p, float tempo)
    {
        if (p.dlyMix <= 0.001f) { if (! silent) { dl.clear(); dr.clear(); silent = true; } return; }
        silent = false;
        float ms = p.dlyTime;
        if (p.dlySync >= 1.0f) ms = 60000.0f / std::max (tempo, 20.0f) * syncBeats ((int) p.dlySync);
        const float target = clampf (ms * 0.001f * sr, 4.0f, (float) dl.capacity() - 8.0f);
        if (timeSm.y < 0.0f) timeSm.reset (target);
        timeSm.setTime (0.25f, sr);
        lpL.setCutoff (p.dlyTone, sr); lpR.setCutoff (p.dlyTone, sr);
        hpL.setCutoff (120.0f, sr); hpR.setCutoff (120.0f, sr);
        const bool ping = p.dlyPing > 0.5f;
        const float fb = p.dlyFb, mix = p.dlyMix;
        const float modDepth = p.dlyMod * 0.002f * sr;
        for (int i = 0; i < n; ++i)
        {
            const float d = timeSm.process (target);
            phase += 0.6f / sr; if (phase >= 1.0f) phase -= 1.0f;
            const float mod = modDepth * (1.0f + sin01 (phase));
            const float wl = dl.tapLinear (d + mod), wr = dr.tapLinear (d + mod * 0.9f);
            const float fl = hpL.process (lpL.process (fastTanh (wl * fb * 1.1f)));
            const float fr = hpR.process (lpR.process (fastTanh (wr * fb * 1.1f)));
            if (ping)
            {
                dl.push (0.5f * (inL[i] + inR[i]) + fr);
                dr.push (fl);
            }
            else
            {
                dl.push (inL[i] + fl);
                dr.push (inR[i] + fr);
            }
            L[i] += wl * mix;
            R[i] += wr * mix;
        }
    }

private:
    float sr = 48000.0f, phase = 0.0f;
    bool silent = true;
    DelayLine dl, dr;
    OnePoleLP lpL, lpR;
    OnePoleHP hpL, hpR;
    Smoother timeSm;
};

//==============================================================================
// 8-channel FDN reverb: multi-step Hadamard diffuser + Householder feedback
// network with per-line damping and slow delay modulation.
class Reverb
{
public:
    static constexpr int C = 8, Steps = 4;

    void prepare (float sr_)
    {
        sr = sr_;
        pre.init ((int) (sr * 0.3f));
        preR.init ((int) (sr * 0.3f));
        for (int s = 0; s < Steps; ++s) for (int c = 0; c < C; ++c) diff[s][c].init ((int) (sr * 0.06f));
        for (int c = 0; c < C; ++c) fbk[c].init ((int) (sr * 0.32f));
        Rng r; r.seed (12345u);
        for (int s = 0; s < Steps; ++s) for (int c = 0; c < C; ++c) { diffRand[s][c] = r.uni(); flip[s][c] = r.uni() < 0.5f; }
        for (int c = 0; c < C; ++c) modPh[c] = r.uni();
        lastKey = -1.0f;
        reset();
    }
    void reset()
    {
        pre.clear(); preR.clear();
        for (auto& st : diff) for (auto& d : st) d.clear();
        for (auto& d : fbk) d.clear();
        for (auto& l : damp) l.reset();
        lcL.reset(); lcR.reset();
    }

    void process (const float* inL, const float* inR, float* L, float* R, int n, const GlobalFxParams& p)
    {
        if (p.revMix <= 0.001f) return;
        const float key = p.revSize * 7.0f + p.revDecay * 3.0f + p.revDamp * 11.0f + p.revLowCut * 0.01f;
        if (std::fabs (key - lastKey) > 1.0e-6f) configure (p);
        const float preD = std::max (1.0f, p.revPre * 0.001f * sr);
        const float mix = p.revMix;
        const float wid = p.revWidth;
        const float modInc = (0.25f + 0.5f * p.revMod) / sr;
        const float modDep = p.revMod * 0.0011f * sr;
        for (int i = 0; i < n; ++i)
        {
            pre.push (inL[i]);
            preR.push (inR[i]);
            const float pl = pre.tapLinear (preD);
            const float pr = preR.tapLinear (preD);
            float x[C];
            for (int c = 0; c < C; ++c) x[c] = (c & 1) ? pr : pl;
            // diffusion
            for (int s = 0; s < Steps; ++s)
            {
                for (int c = 0; c < C; ++c)
                {
                    diff[s][c].push (x[c]);
                    x[c] = diff[s][c].tap (diffLen[s][c]);
                }
                hadamard (x);
                for (int c = 0; c < C; ++c) if (flip[s][c]) x[c] = -x[c];
            }
            // feedback network
            float y[C];
            float sum = 0.0f;
            for (int c = 0; c < C; ++c)
            {
                modPh[c] += modInc * (1.0f + 0.13f * (float) c); if (modPh[c] >= 1.0f) modPh[c] -= 1.0f;
                const float d = fbLen[c] + (c < 4 ? modDep * (1.0f + sin01 (modPh[c])) : 0.0f);
                y[c] = damp[c].process (fbk[c].tapLinear (d)) * gain[c];
                sum += y[c];
            }
            sum *= 2.0f / (float) C;
            float outL = 0.0f, outR = 0.0f;
            for (int c = 0; c < C; ++c)
            {
                fbk[c].push (y[c] - sum + x[c]);
                if (c & 1) outR += y[c]; else outL += y[c];
            }
            outL = lcL.process (outL * 0.5f + x[0] * 0.15f);
            outR = lcR.process (outR * 0.5f + x[1] * 0.15f);
            const float mid = 0.5f * (outL + outR), side = 0.5f * (outL - outR) * wid;
            L[i] += (mid + side) * mix;
            R[i] += (mid - side) * mix;
        }
    }

private:
    static void hadamard (float* x)
    {
        for (int h = 1; h < C; h <<= 1)
            for (int i = 0; i < C; i += h << 1)
                for (int j = i; j < i + h; ++j)
                {
                    const float a = x[j], b = x[j + h];
                    x[j] = a + b; x[j + h] = a - b;
                }
        const float s = 0.35355339f;
        for (int i = 0; i < C; ++i) x[i] *= s;
    }

    void configure (const GlobalFxParams& p)
    {
        lastKey = p.revSize * 7.0f + p.revDecay * 3.0f + p.revDamp * 11.0f + p.revLowCut * 0.01f;
        const float size = 0.25f + 0.75f * p.revSize;
        for (int s = 0; s < Steps; ++s)
        {
            const float range = (0.004f * std::exp2 ((float) s)) * size * sr;   // 4, 8, 16, 32 ms (scaled)
            for (int c = 0; c < C; ++c)
                diffLen[s][c] = std::clamp ((int) (range * (0.15f + 0.85f * (((float) c + diffRand[s][c]) / (float) C))), 1, diff[s][c].capacity() - 2);
        }
        const float baseMs = 45.0f + 95.0f * p.revSize;
        for (int c = 0; c < C; ++c)
        {
            const float ms = baseMs * std::exp2 ((float) c / (float) C);
            fbLen[c] = std::min (ms * 0.001f * sr, (float) fbk[c].capacity() - sr * 0.004f);
            gain[c] = std::pow (10.0f, -3.0f * fbLen[c] / (std::max (p.revDecay, 0.1f) * sr));
            damp[c].setCutoff (20000.0f * std::exp2 (-p.revDamp * 4.2f), sr);
        }
        lcL.setHighpass (p.revLowCut, 0.6, sr); lcR.setHighpass (p.revLowCut, 0.6, sr);
    }

    float sr = 48000.0f, lastKey = -1.0f;
    DelayLine pre, preR;
    DelayLine diff[Steps][C];
    int diffLen[Steps][C] {};
    float diffRand[Steps][C] {};
    bool flip[Steps][C] {};
    DelayLine fbk[C];
    float fbLen[C] {}, gain[C] {}, modPh[C] {};
    OnePoleLP damp[C];
    Biquad lcL, lcR;
};

//==============================================================================
class LoFi
{
public:
    void prepare (float sr_)
    {
        sr = sr_;
        wowL.init ((int) (sr * 0.02f)); wowR.init ((int) (sr * 0.02f));
        reset();
    }
    void reset() { wowL.clear(); wowR.clear(); holdL = holdR = 0.0f; ph = 0.0f; wph = fph = 0.0f; toneL.reset(); toneR.reset(); hiss.reset(); crk.reset(); }

    void process (float* L, float* R, int n, const GlobalFxParams& p)
    {
        const bool bits = p.lofiBits < 23.5f, rate = p.lofiRate < 47.5f, noise = p.lofiNoise > 0.001f, wow = p.lofiWow > 0.001f, tone = p.lofiTone < 19500.0f;
        if (! (bits || rate || noise || wow || tone)) return;
        const float q = std::exp2 (std::round (p.lofiBits) - 1.0f);
        const float rInc = std::min (1.0f, p.lofiRate * 1000.0f / sr);
        if (tone) { toneL.setLowpass (p.lofiTone, 0.6, sr); toneR.setLowpass (p.lofiTone, 0.6, sr); }
        hiss.setCutoff (6000.0f, sr);
        crk.setCutoff (2500.0f, sr);
        const float wd = p.lofiWow * 0.0025f * sr;
        for (int i = 0; i < n; ++i)
        {
            float l = L[i], r = R[i];
            if (wow)
            {
                wph += 0.55f / sr; if (wph >= 1.0f) wph -= 1.0f;
                fph += 7.3f / sr;  if (fph >= 1.0f) fph -= 1.0f;
                const float d = 2.0f + wd * (1.0f + sin01 (wph) + 0.15f * sin01 (fph));
                wowL.push (l); wowR.push (r);
                l = wowL.tapLinear (d); r = wowR.tapLinear (d);
            }
            if (rate)
            {
                ph += rInc;
                if (ph >= 1.0f) { ph -= 1.0f; holdL = l; holdR = r; }
                l = holdL; r = holdR;
            }
            if (bits) { l = std::round (l * q) / q; r = std::round (r * q) / q; }
            if (tone) { l = toneL.process (l); r = toneR.process (r); }
            if (noise)
            {
                const float h = hiss.process (rng.bi()) * 0.02f * p.lofiNoise;
                float c = 0.0f;
                if (rng.uni() < 0.0004f * p.lofiNoise) c = rng.bi() * 0.6f;
                c = crk.process (c) * p.lofiNoise;
                l += h + c; r += h * 0.9f + c;
            }
            L[i] = l; R[i] = r;
        }
    }

private:
    float sr = 48000.0f, holdL = 0.0f, holdR = 0.0f, ph = 0.0f, wph = 0.0f, fph = 0.0f;
    DelayLine wowL, wowR;
    Biquad toneL, toneR;
    OnePoleLP hiss, crk;
    Rng rng;
};

//==============================================================================
class Limiter
{
public:
    void prepare (float sr_) { sr = sr_; env = 1.0f; }
    void reset() { env = 1.0f; }
    void process (float* L, float* R, int n)
    {
        const float thr = 0.89f;
        const float rel = std::exp (-1.0f / (0.12f * sr));
        for (int i = 0; i < n; ++i)
        {
            const float pk = std::max (std::fabs (L[i]), std::fabs (R[i]));
            const float g = pk > thr ? thr / pk : 1.0f;
            env = g < env ? g : g + (env - g) * rel;
            float l = L[i] * env, r = R[i] * env;
            // safety soft clip
            l = l > 0.95f ? 0.95f + 0.05f * fastTanh ((l - 0.95f) * 20.0f) : (l < -0.95f ? -0.95f + 0.05f * fastTanh ((l + 0.95f) * 20.0f) : l);
            r = r > 0.95f ? 0.95f + 0.05f * fastTanh ((r - 0.95f) * 20.0f) : (r < -0.95f ? -0.95f + 0.05f * fastTanh ((r + 0.95f) * 20.0f) : r);
            if (! std::isfinite (l)) l = 0.0f;
            if (! std::isfinite (r)) r = 0.0f;
            L[i] = l; R[i] = r;
        }
    }

private:
    float sr = 48000.0f, env = 1.0f;
};

} // namespace kv
