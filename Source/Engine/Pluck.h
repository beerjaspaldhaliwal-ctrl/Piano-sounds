// Keyverse - plucked & struck strings (guitars, basses, harpsichord, clavinet,
// harp, sitar, banjo, koto, mandolin ...).
//
// Extended Karplus-Strong digital waveguide per string:
//   delay line + Thiran fractional allpass (exact tuning)
//   + dispersion allpasses (string stiffness)
//   + one-pole loss filter (frequency dependent decay)
//   + optional nonlinear bridge (sitar jawari / fret buzz).
// Excitation is shaped by playing style (pick, finger, slap, hammer, quill)
// and comb-filtered at the pluck point; electric instruments read the string
// at one or two magnetic pickup positions. Courses (12-string, mandolin,
// harpsichord 8'+4') use several strings per key. A body/soundbox filter
// network runs on the layer bus.
#pragma once

#include "Voice.h"
#include <complex>

namespace kv
{
struct StringLoop
{
    DelayLine dl;
    int N = 2;
    float apC = 0.0f, apX = 0.0f, apY = 0.0f;   // fractional delay allpass
    float dC = 0.0f, dX[3] {}, dY[3] {};        // dispersion allpasses
    int nDisp = 0;
    float p = 0.2f, lp = 0.0f, g = 0.99f;       // loss filter
    float buzz = 0.0f, buzzThr = 1.0f;
    float dcR = 0.999f, dcX = 0.0f, dcY = 0.0f;  // in-loop DC blocker
    float level = 1.0f;

    void init (int maxLen) { dl.init (maxLen); reset(); }
    void reset()
    {
        dl.clear(); apX = apY = lp = dcX = dcY = 0.0f;
        for (int i = 0; i < 3; ++i) dX[i] = dY[i] = 0.0f;
    }

    // Tune the loop for frequency f with fundamental T60 t60.
    void tune (float f, float t60, float damp, float stiff, float sr)
    {
        f = clampf (f, 10.0f, sr * 0.3f);
        const float w = kTwoPi * f / sr;
        p = clampf (damp, 0.0f, 0.95f);
        nDisp = stiff > 0.01f ? 3 : 0;
        dC = -0.65f * clampf (stiff, 0.0f, 1.0f) * clampf (1.0f - f / 3000.0f, 0.1f, 1.0f);

        const std::complex<float> z1 = std::polar (1.0f, -w);
        const float lpDelay = std::atan2 (p * std::sin (w), 1.0f - p * std::cos (w)) / w;
        float dispDelay = 0.0f;
        if (nDisp > 0)
        {
            const std::complex<float> A = (dC + z1) / (1.0f + dC * z1);
            dispDelay = (float) nDisp * (-std::arg (A) / w);
        }
        // DC blocker corner well below the fundamental; its phase lead is compensated
        dcR = 1.0f - kTwoPi * (f * 0.125f) / sr;
        const std::complex<float> Hdc = (1.0f - z1) / (1.0f - dcR * z1);
        const float dcDelay = -std::arg (Hdc) / w;   // negative (phase lead)
        float total = sr / f - lpDelay - dispDelay - dcDelay;
        int n = (int) std::floor (total - 0.5f);
        n = std::clamp (n, 1, dl.capacity() - 2);
        const float d = clampf (total - (float) n, 0.1f, 1.9f);
        N = n;
        apC = (1.0f - d) / (1.0f + d);

        const float lpMag = (1.0f - p) / std::abs (1.0f - p * z1) * std::abs (Hdc);
        const float rho = std::pow (10.0f, -3.0f / (std::max (t60, 0.01f) * f));
        g = std::min (0.99995f, rho / std::max (lpMag, 1.0e-4f));
    }

    float tick (float in)
    {
        const float v = dl.tap (N - 1);
        const float fa = apC * v + apX - apC * apY;
        apX = v; apY = fa;
        float x = fa;
        for (int i = 0; i < nDisp; ++i)
        {
            const float y = dC * x + dX[i] - dC * dY[i];
            dX[i] = x; dY[i] = y; x = y;
        }
        lp = (1.0f - p) * x + p * lp;
        float out = g * lp;
        const float d = out - dcX + dcR * dcY;
        dcX = out; dcY = d; out = d;
        if (buzz > 0.0f && out < -buzzThr)   // one-sided bridge contact
            out = -buzzThr + (out + buzzThr) * (1.0f - buzz);
        const float wv = out + in;
        dl.push (wv);
        return wv;
    }
};

class PluckVoice : public Voice
{
public:
    void setup (EngineBase* e)
    {
        eng = e;
        const int maxLen = (int) (eng->sr / 12.0f) + 16;
        for (auto& s : str) s.init (maxLen);
        exc.assign ((size_t) maxLen * 2 + 64, 0.0f);
        rng.seed ((uint32_t) (reinterpret_cast<uintptr_t> (this) & 0xffffffffu) | 3u);
    }

    void start (int keyIn, float velocity) override
    {
        const auto& P = eng->params->pluck;
        const auto& C = *eng->ctx;
        resetFade();
        active = true; releasing = false;
        vel = velocity;
        key = std::clamp (keyIn + (int) std::lround (C.transpose), 0, 127);
        nStr = std::clamp ((int) P.course, 1, 3);
        for (int s = 0; s < nStr; ++s) str[s].reset();
        ageSamples = 0;
        silentChunks = 0;
        repeatPhase = 0.0f;
        repeatSign = 1.0f;

        const int octMode = (int) P.courseOct;
        for (int s = 0; s < nStr; ++s)
        {
            float ratio = 1.0f, det = 0.0f, lvl = 1.0f, dly = 0.0f;
            if (s == 1)
            {
                ratio = (octMode >= 1) ? 2.0f : 1.0f;
                det = (octMode >= 1) ? P.courseDetune * 0.3f : P.courseDetune;
                lvl = (octMode >= 1) ? 0.55f : 0.9f;
                dly = P.courseDelay;
            }
            else if (s == 2)
            {
                ratio = 1.0f;
                det = -P.courseDetune * (octMode == 2 ? 0.7f : 1.0f);
                lvl = 0.85f;
                dly = P.courseDelay * 0.5f;
            }
            strRatio[s] = ratio * centsToRatio (det);
            str[s].level = lvl;
            strDelay[s] = (int) (dly * 0.001f * eng->sr);
        }
        retune();
        buildExcitation();
        excPos[0] = excPos[1] = excPos[2] = 0;

        const float keyPan = P.width * clampf ((float) (key - 60) / 30.0f, -1.0f, 1.0f);
        panGains (keyPan, panL, panR);

        outDc.setup (eng->sr);
        outDc.reset();
        clickEnv = P.pickNoise * vel * 0.25f * dynGain;
        clickMul = std::exp (-1.0f / (0.0015f * eng->sr));
        clickHp.setCutoff (2500.0f, eng->sr);
        relEnv = 0.0f;
        relBp.setBandpass (1200.0, 1.5, eng->sr);
    }

    void release() override
    {
        releasing = true;
        const auto& P = eng->params->pluck;
        if (P.sustainHold > 0.5f) return;
        relEnv = P.relNoise * 0.06f * (0.4f + 0.6f * vel) * dynGain;
        relMul = std::exp (-1.0f / (0.02f * eng->sr));
        retune();
    }

    void kill (float s) override { beginFade (s, eng->sr); }

    void render (float* L, float* R, int n) override
    {
        if (! active) return;
        const auto& P = eng->params->pluck;
        // pitch twang (tension modulation) + bend/vibrato -> retune per chunk
        retune();

        // tremolo picking
        if (P.repeat > 0.1f && held)
        {
            repeatPhase += P.repeat * (float) n / eng->sr;
            if (repeatPhase >= 1.0f)
            {
                repeatPhase -= 1.0f;
                repeatSign = -repeatSign;
                excPos[0] = excPos[1] = excPos[2] = 0;
                excScale = repeatSign * (0.75f + 0.25f * rng.uni());
            }
        }

        float peak = 0.0f;
        const int puA = pickupTap (P.pickupPos), puB = pickupTap (P.pickup2Pos);
        const float mixB = P.pickup2Mix;
        // keep level roughly constant whatever the pickup position
        const float puGain = puA > 0 ? 1.5f / (0.5f + 2.0f * std::sin (kPi * std::max (P.pickupPos, 0.03f))) : 1.0f;
        for (int i = 0; i < n; ++i)
        {
            float s = 0.0f;
            for (int k = 0; k < nStr; ++k)
            {
                auto& st = str[k];
                float in = 0.0f;
                if (ageSamples >= strDelay[k] && excPos[k] < excLen)
                    in = exc[(size_t) excPos[k]++] * excScale;
                const float w = st.tick (in);
                float o = w;
                if (puA > 0)
                {
                    const float a = w - st.dl.tap (std::min (puA, st.N - 1));
                    o = a;
                    if (puB > 0)
                    {
                        const float b = w - st.dl.tap (std::min (puB, st.N - 1));
                        o = a * (1.0f - std::fabs (mixB)) + b * mixB;
                    }
                }
                s += o * st.level;
            }
            ++ageSamples;
            s = outDc.process (s * dynGain * puGain);
            if (clickEnv > 1.0e-6f) { s += clickHp.process (rng.bi()) * clickEnv; clickEnv *= clickMul; }
            if (relEnv > 1.0e-6f)   { s += relBp.process (rng.bi()) * relEnv; relEnv *= relMul; }
            float l = s * panL, r = s * panR;
            if (! applyFade (l, r)) { active = false; return; }
            L[i] += l; R[i] += r;
            peak = std::max (peak, std::fabs (s));
        }
        bool exciting = false;
        for (int k = 0; k < nStr; ++k) exciting |= excPos[k] < excLen;
        if (peak < 2.0e-5f && ! exciting && ! (P.repeat > 0.1f && held)) { if (++silentChunks > 8) active = false; }
        else silentChunks = 0;
    }

private:
    int pickupTap (float pos) const
    {
        if (pos <= 0.001f) return 0;
        return std::max (1, (int) std::lround (pos * (float) str[0].N));
    }

    float baseT60() const
    {
        const auto& P = eng->params->pluck;
        float t = 7.0f * std::exp2 (-(float) (key - 40) / 24.0f) * P.decay;
        t *= (1.0f - 0.92f * P.mute);
        if (releasing && P.sustainHold < 0.5f) t = std::min (t, P.release);
        return std::max (t, 0.01f);
    }

    void retune()
    {
        const auto& P = eng->params->pluck;
        const auto& C = *eng->ctx;
        const float sr = eng->sr;
        const float ageSec = (float) ageSamples / sr;
        const float twangCents = P.twang * vel * vel * std::exp (-ageSec / 0.07f);
        const float f0 = C.hz (curNote) * centsToRatio (twangCents);
        float damp = clampf (P.damping * (0.25f + 0.55f * (1.0f - (float) key / 127.0f)) + 0.35f * P.mute, 0.0f, 0.92f);
        if (releasing && P.sustainHold < 0.5f) damp = clampf (damp + 0.25f, 0.0f, 0.92f);
        const float t60 = baseT60();
        for (int s = 0; s < nStr; ++s)
        {
            auto& st = str[s];
            st.tune (f0 * strRatio[s], t60 * (s == 1 && strRatio[s] > 1.5f ? 0.7f : 1.0f), damp, P.stiffness, sr);
            st.buzz = P.buzz * 0.85f;
            st.buzzThr = 0.02f + 0.25f * (1.0f - P.buzz);
        }
    }

    void buildExcitation()
    {
        const auto& P = eng->params->pluck;
        const float sr = eng->sr;
        const int mode = (int) P.excite;
        const float period = (float) str[0].N + 1.0f;
        const float v = vel;
        const float bright = clampf (P.bright * (1.0f - P.velBright * 0.6f) + P.velBright * 0.6f * v - 0.35f * P.mute, 0.0f, 1.0f);

        int len = 0;
        OnePoleLP lp;
        lp.setCutoff (300.0f * std::exp2 (bright * 6.0f) * (0.6f + 0.4f * v), sr);
        switch (mode)
        {
            case 1: // finger: soft rounded displacement
            {
                len = std::max (4, (int) (period * (0.25f + 0.35f * (1.0f - bright))));
                for (int i = 0; i < len; ++i)
                {
                    const float t = (float) i / (float) len;
                    exc[(size_t) i] = 0.5f - 0.5f * std::cos (kTwoPi * t) + 0.15f * lp.process (rng.bi());
                }
                break;
            }
            case 2: // slap / pop: sharp + bright
            {
                len = std::max (8, (int) (period * 0.5f));
                for (int i = 0; i < len; ++i)
                    exc[(size_t) i] = (i < 3 ? 2.5f : 0.0f) + rng.bi() * 0.8f * std::exp (-(float) i / (period * 0.08f));
                break;
            }
            case 3: // strike (hammer): smooth force pulse
            {
                const float corner = 400.0f * std::exp2 (bright * 5.0f) * (0.4f + 0.8f * v);
                len = makeForcePulse (exc.data(), (int) exc.size() / 2, sr, corner, 2.0f - bright);
                for (int i = 0; i < len; ++i) exc[(size_t) i] *= period * 0.5f;
                break;
            }
            case 4: // quill: sharp ramp release (harpsichord)
            {
                len = std::max (4, (int) (period * 0.15f));
                for (int i = 0; i < len; ++i)
                    exc[(size_t) i] = (1.0f - (float) i / (float) len) * 1.5f + rng.bi() * 0.3f * bright;
                break;
            }
            default: // pick: filtered noise burst, one period
            {
                len = std::max (4, (int) (period * (0.5f + 0.5f * (1.0f - bright))));
                for (int i = 0; i < len; ++i) exc[(size_t) i] = lp.process (rng.bi()) * 1.6f + (i == 0 ? 0.8f * bright : 0.0f);
                break;
            }
        }
        // remove DC so the string doesn't drift, then comb at the pluck point
        float mean = 0.0f;
        for (int i = 0; i < len; ++i) mean += exc[(size_t) i];
        mean /= (float) len;
        if (mode == 0 || mode == 2) for (int i = 0; i < len; ++i) exc[(size_t) i] -= mean;
        const int combD = std::max (1, (int) std::lround (P.pluckPos * period));
        const int total = std::min (len + combD, (int) exc.size() - 1);
        for (int i = total - 1; i >= 0; --i)
        {
            const float a = i < len ? exc[(size_t) i] : 0.0f;
            const float b = (i - combD >= 0 && i - combD < len) ? exc[(size_t) (i - combD)] : 0.0f;
            exc[(size_t) i] = a - b;
        }
        excLen = total;
        excScale = 1.0f;
        // normalise energy per period (the loop recirculates one period of excitation)
        double e2 = 0.0;
        for (int i = 0; i < total; ++i) e2 += (double) exc[(size_t) i] * exc[(size_t) i];
        const float rms = (float) std::sqrt (e2 / std::max (period, 1.0f)) + 1.0e-6f;
        for (int i = 0; i < total; ++i) exc[(size_t) i] *= 0.45f / rms;

        const float dynDb = (v - 1.0f) * (14.0f + 24.0f * P.velSens);
        dynGain = dbToGain (dynDb) * 0.4f;
    }

    EngineBase* eng = nullptr;
    StringLoop str[3];
    float strRatio[3] { 1, 1, 1 };
    int strDelay[3] {};
    int nStr = 1;
    std::vector<float> exc;
    int excLen = 0, excPos[3] {};
    float excScale = 1.0f, dynGain = 1.0f;
    int key = 60, ageSamples = 0, silentChunks = 0;
    float repeatPhase = 0.0f, repeatSign = 1.0f;
    float panL = 0.7f, panR = 0.7f;
    Rng rng;
    float clickEnv = 0, clickMul = 0, relEnv = 0, relMul = 0;
    OnePoleHP clickHp;
    Biquad relBp;
    DcBlock outDc;
};

//==============================================================================
class PluckEngine : public PolyEngine<PluckVoice>
{
public:
    PluckEngine() : PolyEngine<PluckVoice> (32) {}
    void prepare (float sampleRate, int maxBlk) override
    {
        sr = sampleRate; maxBlock = maxBlk;
        prepareVoices();
        bodyFdn.prepare (sr, 30.0f);
        resetBus();
    }
    void resetBus() override
    {
        bodyFdn.reset();
        for (auto& b : eqL) b.reset();
        for (auto& b : eqR) b.reset();
        lastBody = -1;
    }
    float defaultStealFade() const override { return 0.02f; }

    void processBus (float* L, float* R, int n) override
    {
        const auto& P = params->pluck;
        const int body = (int) P.body;
        if (body <= 0) return;
        if (body != lastBody) configureBody (body);
        const float mix = P.bodyMix;
        bodyFdn.set (2.5f + 2.0f * (float) (body == 9) + 1.5f * (float) (body == 5), body == 4 ? 0.06f : 0.14f, 3500.0f);
        for (int i = 0; i < n; ++i)
        {
            float l = L[i], r = R[i];
            float el = l, er = r;
            for (int k = 0; k < nEq; ++k) { el = eqL[k].process (el); er = eqR[k].process (er); }
            float bl, br;
            bodyFdn.process (l, r, bl, br);
            L[i] = l * (1.0f - mix) + (el + bl * 0.3f) * mix;
            R[i] = r * (1.0f - mix) + (er + br * 0.3f) * mix;
        }
    }

private:
    void configureBody (int body)
    {
        lastBody = body;
        struct Pk { float f, q, g; };
        // Main air / top / back plate resonances per instrument family.
        static const Pk tables[12][6] = {
            { { 0, 0, 0 } },
            { { 98, 6, 7 }, { 200, 5, 5 }, { 390, 3, 2 }, { 1000, 1, -2 }, { 2600, 1.2f, 1 }, { 6000, 0.7f, -5 } },     // nylon
            { { 105, 5, 6 }, { 220, 4, 4 }, { 480, 3, 2 }, { 1200, 1, -1.5f }, { 2500, 1.5f, 2.5f }, { 9000, 0.7f, -2 } }, // steel
            { { 120, 4, 4 }, { 280, 3, 4 }, { 700, 2, 3 }, { 1600, 1, -2 }, { 3200, 1, -3 }, { 5000, 0.7f, -6 } },       // archtop
            { { 150, 1, -6 }, { 260, 2, 3 }, { 1200, 2, 5 }, { 3000, 2, 4 }, { 5500, 1, 2 }, { 0, 0, 0 } },               // banjo
            { { 90, 3, 4 }, { 180, 3, 3 }, { 400, 2, 2 }, { 1500, 1, -1 }, { 6000, 0.7f, -3 }, { 0, 0, 0 } },              // harp
            { { 140, 4, 4 }, { 600, 4, 3 }, { 1300, 3, 2 }, { 2400, 3, 5 }, { 4500, 2, 3 }, { 0, 0, 0 } },                 // sitar
            { { 100, 3, 3 }, { 250, 3, 3 }, { 700, 2, 1 }, { 1500, 2, 2 }, { 8000, 0.7f, 2 }, { 0, 0, 0 } },               // harpsichord
            { { 120, 1, -6 }, { 220, 5, 5 }, { 450, 4, 4 }, { 1100, 2, 2 }, { 3000, 1.5f, 1 }, { 0, 0, 0 } },              // small body
            { { 60, 4, 5 }, { 110, 4, 4 }, { 240, 3, 2 }, { 700, 1, -2 }, { 2500, 0.7f, -6 }, { 0, 0, 0 } },              // double bass
            { { 180, 4, 4 }, { 420, 3, 3 }, { 1800, 2, 3 }, { 3500, 2, 2 }, { 0, 0, 0 }, { 0, 0, 0 } },                    // koto
            { { 110, 5, 5 }, { 230, 4, 4 }, { 600, 3, 2 }, { 1400, 1, -1 }, { 5000, 0.7f, -4 }, { 0, 0, 0 } },             // oud
        };
        nEq = 0;
        const int b = std::clamp (body, 0, 11);
        for (int k = 0; k < 6; ++k)
        {
            const auto& t = tables[b][k];
            if (t.f <= 0.0f) continue;
            if (t.q < 1.0f) eqL[nEq].setHighShelf (t.f, t.g, sr);
            else            eqL[nEq].setPeak (t.f, t.q, t.g, sr);
            eqR[nEq] = eqL[nEq];
            eqL[nEq].reset(); eqR[nEq].reset();
            ++nEq;
        }
    }

    Fdn8 bodyFdn;
    Biquad eqL[6], eqR[6];
    int nEq = 0, lastBody = -1;
};

} // namespace kv
