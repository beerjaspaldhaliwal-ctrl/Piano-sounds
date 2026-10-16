// Keyverse - mallets, bells & tuned percussion (modal synthesis).
//
// Up to six vibration modes with free ratios/levels/decays (bars, plates,
// tines, tubes, bells, glasses), optional detuned doublets for beating,
// a resonator tube for marimba/vibes, and three excitation types:
// mallet strike (force pulse), pluck (music box, kalimba) or bow (sustained
// noise excitation: glass harmonica, bowed vibes, singing bowls).
#pragma once

#include "Voice.h"

namespace kv
{
class MalletVoice : public Voice
{
public:
    static constexpr int kMax = 14;

    void setup (EngineBase* e)
    {
        eng = e;
        pulse.assign ((size_t) (eng->sr * 0.03f) + 16, 0.0f);
        rng.seed ((uint32_t) (reinterpret_cast<uintptr_t> (this) & 0xffffffffu) | 5u);
    }

    void start (int keyIn, float velocity) override
    {
        const auto& P = eng->params->mallet;
        const auto& C = *eng->ctx;
        const float sr = eng->sr;
        resetFade();
        active = true; releasing = false;
        vel = velocity;
        key = std::clamp (keyIn + (int) std::lround (C.transpose), 0, 127);
        baseHz = C.hz (curNote);
        appliedRatio = 0.0f;
        dampExtra = 0.0f;
        ageSamples = 0;
        mode = (int) P.excite;

        const float f0 = baseHz;
        const float fmax = sr * 0.45f;
        const float tBase = P.decay * std::exp2 (-(float) (key - 60) * P.decayTrack / 12.0f);
        n = 0;
        float norm2 = 0.0f;
        for (int m = 0; m < 6; ++m)
        {
            const float ratio = m == 0 ? P.r[0] : 1.0f + (P.r[m] - 1.0f) * (1.0f + P.spread);
            const float f = f0 * ratio;
            const float a = P.g[m] <= -59.0f ? 0.0f : dbToGain (P.g[m]);
            if (f >= fmax || a <= 0.0f) continue;
            const float t = std::max (0.01f, tBase * P.d[m]);
            add (f, t, a);
            norm2 += a * a;
            if (P.beatMix > 0.01f && P.beat > 0.0f)
            {
                const float fb = f + P.beat * (0.6f + 0.4f * hashNoise ((uint32_t) key, (uint32_t) m)) * (1.0f + 0.3f * (float) m);
                if (fb < fmax) add (fb, t * 0.9f, a * P.beatMix);
            }
        }
        if (P.resonator > 0.01f && f0 * P.r[0] < fmax)
            add (f0 * P.r[0], tBase * P.d[0] * 1.6f + 0.3f, P.resonator * 0.8f);
        norm = 1.0f / std::sqrt (std::max (norm2, 1.0e-4f));

        const float v = vel;
        const float hard = clampf (P.hardness * (1.0f - P.velBright * 0.5f) + P.velBright * 0.5f * v, 0.0f, 1.0f);
        const float dynDb = (v - 1.0f) * (14.0f + 26.0f * P.velSens);
        gain = dbToGain (dynDb) * 0.4f * norm;

        pulseLen = pulsePos = 0;
        bowEnv = 0.0f;
        if (mode == 0)
        {
            const float corner = 300.0f * std::exp2 (hard * 5.5f) * std::exp2 ((float) (key - 60) / 36.0f);
            pulseLen = makeForcePulse (pulse.data(), (int) pulse.size(), sr, corner, 2.5f - 1.5f * hard);
            for (int k = 0; k < n; ++k) gin[k] = sinw[k] * amp[k];
        }
        else if (mode == 1)
        {
            // pluck: cosine-phase start, upper modes scaled by hardness
            for (int k = 0; k < n; ++k)
            {
                const float A = amp[k] * (k == 0 ? 1.0f : 0.3f + 0.7f * hard);
                const float r = std::sqrt (c2[k]);
                y1[k] = A * std::cos (wv[k]) / r;
                y2[k] = A * std::cos (2.0f * wv[k]) / (r * r);
                gin[k] = 0.0f;
            }
        }
        else
        {
            // bow: noise excitation normalised to the resonator bandwidth
            for (int k = 0; k < n; ++k)
                gin[k] = sinw[k] * amp[k] * std::sqrt (std::max (1.0f - c2[k], 1.0e-7f)) * 1.4f;
            bowLp.setCutoff (std::min (f0 * 6.0f, sr * 0.4f), sr);
            bowAtt = 1.0f - std::exp (-1.0f / (std::max (P.bowAttack, 0.005f) * sr * 0.3f));
            bowTarget = 1.0f;
        }

        noiseEnv = mode == 2 ? 0.0f : P.noise * (0.2f + 0.8f * v * v) * 0.2f * dbToGain (dynDb);
        noiseMul = std::exp (-1.0f / (0.006f * sr));
        noiseBp.setBandpass (std::min (P.noiseFreq, sr * 0.4f), 1.2, sr);

        const float keyPan = P.width * clampf ((float) (key - 66) / 30.0f, -1.0f, 1.0f);
        panGains (keyPan, panL, panR);
        buzzLp.setCutoff (3000.0f, sr);
        cull = 0;
        updatePitch();
    }

    void release() override
    {
        releasing = true;
        const auto& P = eng->params->mallet;
        bowTarget = 0.0f;
        if (P.damped < 0.5f && mode != 2) return;
        const float extra = 6.9078f / std::max (P.release, 0.01f);
        dampExtra = extra;
        updatePitch (true);
    }

    void kill (float s) override { beginFade (s, eng->sr); }

    void render (float* L, float* R, int nS) override
    {
        if (! active) return;
        const auto& P = eng->params->mallet;
        updatePitch();
        float energy = 0.0f;
        for (int i = 0; i < nS; ++i)
        {
            float x = 0.0f;
            if (mode == 0) { if (pulsePos < pulseLen) x = pulse[(size_t) pulsePos++]; }
            else if (mode == 2)
            {
                bowEnv += (bowTarget - bowEnv) * (bowTarget > bowEnv ? bowAtt : bowAtt * 0.5f);
                x = bowLp.process (rng.bi()) * bowEnv;
            }
            float s = 0.0f;
            for (int k = 0; k < n; ++k)
            {
                const float y = c1[k] * y1[k] - c2[k] * y2[k] + gin[k] * x;
                y2[k] = y1[k]; y1[k] = y;
                s += y;
            }
            s *= gain;
            if (P.buzz > 0.01f)
                s += buzzLp.process (rng.bi()) * std::fabs (s) * P.buzz * 1.5f;
            if (noiseEnv > 1.0e-6f) { s += noiseBp.process (rng.bi()) * noiseEnv; noiseEnv *= noiseMul; }
            float l = s * panL, r = s * panR;
            if (! applyFade (l, r)) { active = false; return; }
            L[i] += l; R[i] += r;
            energy = std::max (energy, std::fabs (s));
        }
        ageSamples += nS;
        if (++cull >= 8)
        {
            cull = 0;
            float e = 0.0f;
            for (int k = 0; k < n; ++k) e += (y1[k] * y1[k] + y2[k] * y2[k]);
            const bool exciting = (mode == 0 && pulsePos < pulseLen) || (mode == 2 && (bowTarget > 0.0f || bowEnv > 1.0e-4f));
            if (! exciting && e * gain * gain < 1.0e-11f && noiseEnv < 1.0e-6f) active = false;
        }
    }

private:
    void add (float f, float t60, float a)
    {
        if (n >= kMax) return;
        const float sr = eng->sr;
        const int k = n++;
        wv[k] = kTwoPi * f / sr;
        sg[k] = 6.9078f / t60;
        sinw[k] = std::sin (wv[k]);
        amp[k] = a;
        const float r = std::exp (-sg[k] / sr);
        c1[k] = 2.0f * r * std::cos (wv[k]);
        c2[k] = r * r;
        y1[k] = y2[k] = 0.0f;
    }

    void updatePitch (bool force = false)
    {
        const auto& P = eng->params->mallet;
        const float age = (float) ageSamples / eng->sr;
        const float drop = P.pitchDrop * std::exp (-age / 0.08f);
        const float ratio = eng->ctx->hz (curNote) / baseHz * centsToRatio (drop);
        if (! force && std::fabs (ratio - appliedRatio) < 1.0e-5f) return;
        appliedRatio = ratio;
        const float sr = eng->sr;
        for (int k = 0; k < n; ++k)
        {
            const float r = std::exp (-(sg[k] + dampExtra) / sr);
            c1[k] = 2.0f * r * std::cos (std::min (wv[k] * ratio, kPi * 0.98f));
            c2[k] = r * r;
        }
    }

    EngineBase* eng = nullptr;
    float y1[kMax] {}, y2[kMax] {}, c1[kMax] {}, c2[kMax] {}, gin[kMax] {}, wv[kMax] {}, sg[kMax] {}, sinw[kMax] {}, amp[kMax] {};
    int n = 0, mode = 0, key = 60, cull = 0, ageSamples = 0;
    std::vector<float> pulse;
    int pulseLen = 0, pulsePos = 0;
    float gain = 1.0f, norm = 1.0f, baseHz = 440.0f, appliedRatio = 0.0f, dampExtra = 0.0f;
    float bowEnv = 0.0f, bowTarget = 0.0f, bowAtt = 0.001f;
    OnePoleLP bowLp, buzzLp;
    float noiseEnv = 0.0f, noiseMul = 0.0f;
    Biquad noiseBp;
    float panL = 0.7f, panR = 0.7f;
    Rng rng;

public:
    void resetDamp() { dampExtra = 0.0f; }
};

class MalletEngine : public PolyEngine<MalletVoice>
{
public:
    MalletEngine() : PolyEngine<MalletVoice> (40) {}
    void prepare (float sampleRate, int maxBlk) override
    {
        sr = sampleRate; maxBlock = maxBlk;
        prepareVoices();
    }
    float defaultStealFade() const override { return 0.03f; }
};

} // namespace kv
