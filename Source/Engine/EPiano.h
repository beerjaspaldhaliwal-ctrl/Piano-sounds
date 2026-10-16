// Keyverse - electric pianos (tine / reed / pianet families).
//
// The vibrating element (tine + tone bar, or steel reed) is a handful of
// resonant modes excited by a hammer pulse (or a pluck for Pianet-style
// instruments). Its displacement drives a nonlinear pickup model:
//  - electromagnetic (Rhodes-style): flux follows a bell-shaped curve of the
//    tine position; voicing offsets the tine from the pole piece, trading
//    fundamental against the even-harmonic "bell"; hard playing pushes the
//    tine through the curve, producing the famous bark.
//  - electrostatic (Wurlitzer-style): capacitance ~ 1/(d - x), asymmetric,
//    giving the reedy, nasal growl.
// Output is the time derivative of flux/charge, as in the real pickups.
#pragma once

#include "Voice.h"

namespace kv
{
class EPianoVoice : public Voice
{
public:
    void setup (EngineBase* e)
    {
        eng = e;
        pulse.assign ((size_t) (eng->sr * 0.03f) + 16, 0.0f);
        rng.seed ((uint32_t) (reinterpret_cast<uintptr_t> (this) & 0xffffffffu) | 7u);
    }

    void start (int keyIn, float velocity) override
    {
        const auto& P = eng->params->ep;
        const auto& C = *eng->ctx;
        const float sr = eng->sr;
        resetFade();
        active = true; releasing = false;
        vel = velocity;
        key = std::clamp (keyIn + (int) std::lround (C.transpose), 0, 127);
        baseHz = C.hz (curNote);
        appliedRatio = 1.0f;
        const float f0 = baseHz;
        const float v = vel;

        // Modes: fundamental, tone bar (beating partner), bell overtone, upper mode.
        const float t60 = std::max (0.2f, P.decay * 14.0f * std::exp2 (-(float) (key - 28) / 22.0f));
        const float bellT = std::max (0.01f, P.bellDecay * std::exp2 (-(float) (key - 60) / 30.0f));
        const float keyBell = clampf (0.55f + (float) (key - 48) / 50.0f, 0.25f, 1.4f);
        const float fmax = sr * 0.45f;
        float freq[4] = { f0, f0 + P.beat * clampf (f0 / 300.0f, 0.3f, 2.0f), f0 * P.bellRatio, f0 * P.bellRatio * 2.8f };
        float t[4]    = { t60, t60 * 1.6f, bellT, bellT * 0.5f };
        float a[4]    = { 1.0f, 0.5f * P.toneBar, 0.9f * P.bell * keyBell, 0.5f * P.mode3 * keyBell };
        nModes = 0;
        for (int m = 0; m < 4; ++m)
        {
            if (freq[m] >= fmax || a[m] <= 0.0f) continue;
            const int k = nModes++;
            wm[k] = kTwoPi * freq[m] / sr;
            sg[k] = 6.9078f / t[m];
            const float r = std::exp (-sg[k] / sr);
            c1[k] = 2.0f * r * std::cos (wm[k]);
            c2[k] = r * r;
            amp[k] = a[m];
            sinw[k] = std::sin (wm[k]);
            y1[k] = y2[k] = 0.0f;
        }

        // Displacement amplitude into the pickup (the bark lives here).
        dispAmp = (0.2f + 0.8f * v) * (0.25f + 1.5f * P.drive) * (1.0f + 1.6f * P.bark * v * v) * 0.5f;

        if (P.excite < 0.5f)
        {
            // hammer
            const float corner = 700.0f * std::exp2 ((float) (key - 60) / 24.0f) * (0.3f + 1.4f * P.hardness) * (0.35f + 1.2f * v);
            const float p = clampf (3.0f - 1.6f * v * (0.3f + P.hardness), 1.0f, 3.5f);
            pulseLen = makeForcePulse (pulse.data(), (int) pulse.size(), sr, corner, p);
            pulsePos = 0;
            for (int k = 0; k < nModes; ++k) gin[k] = sinw[k] * amp[k];
        }
        else
        {
            // pluck: start every mode at its displacement maximum
            pulseLen = pulsePos = 0;
            const float bright = 0.3f + 0.7f * P.hardness * (0.5f + 0.5f * v);
            for (int k = 0; k < nModes; ++k)
            {
                const float A = amp[k] * (k >= 2 ? bright * 0.4f : 1.0f);
                const float r = std::sqrt (c2[k]);
                y1[k] = A * std::cos (wm[k]) / r;
                y2[k] = A * std::cos (2.0f * wm[k]) / (r * r);
                gin[k] = 0.0f;
            }
        }

        // Pickup normalisation: measure the swing of the transfer curve.
        const bool es = P.pickup > 0.5f;
        h = std::max (0.1f, P.distance);
        off = P.offset;
        float lo = 1.0e9f, hi = -1.0e9f;
        for (int i = 0; i < 64; ++i)
        {
            const float x = dispAmp * std::sin (kTwoPi * (float) i / 64.0f);
            const float y = transfer (x, es);
            lo = std::min (lo, y); hi = std::max (hi, y);
        }
        const float swing = std::max (hi - lo, 1.0e-4f);
        const float dynDb = (v - 1.0f) * (14.0f + 26.0f * P.velSens);
        const float typeGain = (es ? 1.45f : 0.72f) * (P.excite > 0.5f ? 1.8f : 1.0f);
        outGain = dbToGain (dynDb) * 0.64f * typeGain / swing / std::max (wm[0], 1.0e-4f);
        prevPhi = transfer (0.0f, es);
        isES = es;

        const float keyPan = P.width * clampf ((float) (key - 64) / 36.0f, -1.0f, 1.0f);
        panGains (keyPan, panL, panR);

        thumpEnv = P.thump * (0.3f + 0.7f * v) * 0.08f * dbToGain (dynDb);
        thumpMul = std::exp (-1.0f / (0.008f * sr));
        thumpLp.setCutoff (700.0f + 900.0f * P.hardness, sr);
        relEnv = 0.0f;
        relLp.setCutoff (500.0f, sr);
        cull = 0;
    }

    void release() override
    {
        releasing = true;
        const auto& P = eng->params->ep;
        const float extra = 6.9078f / std::max (P.release, 0.01f);
        const float sr = eng->sr;
        for (int k = 0; k < nModes; ++k)
        {
            const float r = std::exp (-(sg[k] + extra) / sr);
            c1[k] = 2.0f * r * std::cos (wm[k] * appliedRatio);
            c2[k] = r * r;
        }
        relEnv = P.relNoise * 0.05f * (0.5f + 0.5f * vel);
        relMul = std::exp (-1.0f / (0.025f * sr));
    }

    void kill (float s) override { beginFade (s, eng->sr); }

    void render (float* L, float* R, int n) override
    {
        if (! active) return;
        updatePitch();
        const float g = outGain;
        float peak = 0.0f;
        for (int i = 0; i < n; ++i)
        {
            const float x = pulsePos < pulseLen ? pulse[(size_t) pulsePos++] : 0.0f;
            float disp = 0.0f;
            for (int k = 0; k < nModes; ++k)
            {
                const float y = c1[k] * y1[k] - c2[k] * y2[k] + gin[k] * x;
                y2[k] = y1[k]; y1[k] = y;
                disp += y;
            }
            const float phi = transfer (disp * dispAmp, isES);
            float s = (phi - prevPhi) * g;
            prevPhi = phi;

            if (thumpEnv > 1.0e-6f) { s += thumpLp.process (rng.bi()) * thumpEnv; thumpEnv *= thumpMul; }
            if (relEnv > 1.0e-6f)   { s += relLp.process (rng.bi()) * relEnv; relEnv *= relMul; }

            float l = s * panL, r = s * panR;
            if (! applyFade (l, r)) { active = false; return; }
            L[i] += l; R[i] += r;
            peak = std::max (peak, std::fabs (s));
        }
        if (++cull >= 8)
        {
            cull = 0;
            float e = 0.0f;
            for (int k = 0; k < nModes; ++k) e += y1[k] * y1[k] + y2[k] * y2[k];
            if (pulsePos >= pulseLen && e * dispAmp * dispAmp * outGain * outGain * wm[0] * wm[0] < 1.0e-12f
                && thumpEnv < 1.0e-6f && relEnv < 1.0e-6f)
                active = false;
        }
        (void) peak;
    }

private:
    float transfer (float x, bool es) const
    {
        if (es)
        {
            // electrostatic: charge ~ 1/(d - x), asymmetric
            const float u = clampf (x * (0.35f + 0.45f / h), -3.0f, 0.82f);
            return 1.0f / (1.0f - u) + 0.15f * off * u;
        }
        // electromagnetic: Lorentzian flux curve around the pole piece
        const float u = (x + off) / h;
        return 1.0f / (1.0f + u * u);
    }

    void updatePitch()
    {
        const float ratio = eng->ctx->hz (curNote) / baseHz;
        if (std::fabs (ratio - appliedRatio) < 1.0e-5f) return;
        appliedRatio = ratio;
        for (int k = 0; k < nModes; ++k)
            c1[k] = 2.0f * std::sqrt (c2[k]) * std::cos (std::min (wm[k] * ratio, kPi * 0.98f));
    }

    EngineBase* eng = nullptr;
    float y1[4] {}, y2[4] {}, c1[4] {}, c2[4] {}, gin[4] {}, wm[4] {}, sg[4] {}, amp[4] {}, sinw[4] {};
    int nModes = 0;
    std::vector<float> pulse;
    int pulseLen = 0, pulsePos = 0;
    float dispAmp = 0.5f, h = 0.6f, off = 0.45f, outGain = 1.0f, prevPhi = 0.0f;
    bool isES = false;
    float panL = 0.7f, panR = 0.7f;
    float baseHz = 440.0f, appliedRatio = 1.0f;
    int key = 60, cull = 0;
    Rng rng;
    float thumpEnv = 0, thumpMul = 0, relEnv = 0, relMul = 0;
    OnePoleLP thumpLp, relLp;
};

class EPianoEngine : public PolyEngine<EPianoVoice>
{
public:
    EPianoEngine() : PolyEngine<EPianoVoice> (40) {}
    void prepare (float sampleRate, int maxBlk) override
    {
        sr = sampleRate; maxBlock = maxBlk;
        prepareVoices();
    }
    float defaultStealFade() const override { return 0.03f; }
};

} // namespace kv
