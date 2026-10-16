// Keyverse - acoustic piano: modal string synthesis.
//
// Each note is a bank of two-pole resonators, one per string partial and per
// unison string. Partials follow the stiff-string law f_n = n f0 sqrt(1 + B n^2)
// with key-dependent inharmonicity B, and decay with frequency-dependent loss.
// A felt hammer is modelled as an asymmetric force pulse whose spectrum
// brightens with velocity (nonlinear felt), so timbre changes continuously with
// dynamics. Slightly detuned unison strings give beating and the characteristic
// two-stage (prompt + aftersound) decay. The voices feed a soundboard network
// and a pedal-dependent sympathetic resonance on the layer bus.
#pragma once

#include "Voice.h"

namespace kv
{
class PianoEngine;

class PianoVoice : public Voice
{
public:
    static constexpr int kMaxRes = 3 * 96 + 16;

    void setup (EngineBase* e);
    void start (int key, float velocity) override;
    void release() override;
    void kill (float fadeSeconds) override { beginFade (fadeSeconds, eng->sr); }
    void render (float* L, float* R, int n) override;

private:
    void addResonator (float freq, float sigmaHz, float inGain, float gL, float gR);
    void applyDamping (float extraSigma);
    void updatePitch();

    EngineBase* eng = nullptr;
    alignas (32) float y1[kMaxRes], y2[kMaxRes], c1[kMaxRes], c2[kMaxRes], gin[kMaxRes], gl[kMaxRes], gr[kMaxRes];
    float w[kMaxRes], sigma[kMaxRes], sin2[kMaxRes];
    int nRes = 0;

    std::vector<float> pulse;
    int pulseLen = 0, pulsePos = 0;
    float exciteGain = 0.0f;

    Rng rng;
    float knockEnv = 0, knockMul = 0, thumpEnv = 0, thumpMul = 0, relEnv = 0, relMul = 0, mechEnv = 0, mechMul = 0;
    Biquad knockBp, mechBp;
    OnePoleLP thumpLp;
    Biquad relLp;
    float panL = 0.7f, panR = 0.7f;
    float baseHz = 440.0f, appliedRatio = 1.0f;
    int key = 60;
    bool damped = false;
    int cullCounter = 0;
};

class PianoEngine : public PolyEngine<PianoVoice>
{
public:
    PianoEngine() : PolyEngine<PianoVoice> (64) {}

    void prepare (float sampleRate, int maxBlk) override
    {
        sr = sampleRate;
        maxBlock = maxBlk;
        prepareVoices();
        board.prepare (sr, 40.0f);
        sympa.prepare (sr, 120.0f);
        resetBus();
    }
    void resetBus() override
    {
        board.reset(); sympa.reset();
        for (auto* b : { &lsL, &lsR, &hsL, &hsR, &pkL, &pkR }) b->reset();
        pedalSmooth = 0.0f;
    }
    float defaultStealFade() const override { return 0.08f; }

    void processBus (float* L, float* R, int n) override
    {
        const auto& P = params->piano;
        const float size = P.bodySize;
        const float bodyAmt = P.body * (1.0f - 0.85f * P.piezo);
        board.set (2.0f + 6.0f * size, 0.12f + 0.35f * size, 2500.0f + 3000.0f * (P.bodyTone + 1.0f));
        sympa.set (25.0f + 30.0f * size, 2.5f + 2.0f * size, 3500.0f);
        const float tone = P.bodyTone;
        lsL.setLowShelf (180.0, -2.5 * tone + 1.5 * (size - 0.5), sr);  lsR.b0 = lsL.b0; lsR.b1 = lsL.b1; lsR.b2 = lsL.b2; lsR.a1 = lsL.a1; lsR.a2 = lsL.a2;
        hsL.setHighShelf (3500.0, 5.0 * tone + 3.0 * P.piezo, sr);       hsR.b0 = hsL.b0; hsR.b1 = hsL.b1; hsR.b2 = hsL.b2; hsR.a1 = hsL.a1; hsR.a2 = hsL.a2;
        // uprights: boxy low-mid resonance; piezo: nasal upper mids
        const float boxy = (1.0f - size) * 3.5f;
        pkL.setPeak (P.piezo > 0.3f ? 1800.0 : 420.0, 0.9, P.piezo > 0.3f ? 4.0 * P.piezo : boxy, sr);
        pkR.b0 = pkL.b0; pkR.b1 = pkL.b1; pkR.b2 = pkL.b2; pkR.a1 = pkL.a1; pkR.a2 = pkL.a2;

        const float pedTarget = ctx->pedal;
        const float pedCoef = std::exp (-1.0f / (0.05f * sr));
        const float sympAmt = P.pedalRes * 0.25f;
        for (int i = 0; i < n; ++i)
        {
            pedalSmooth = pedTarget + (pedalSmooth - pedTarget) * pedCoef;
            float bl, br, sl, sR;
            board.process (L[i], R[i], bl, br);
            const float sIn = pedalSmooth * sympAmt;
            sympa.process (L[i] * sIn, R[i] * sIn, sl, sR);
            float l = L[i] + bl * bodyAmt * 0.6f + sl;
            float r = R[i] + br * bodyAmt * 0.6f + sR;
            l = pkL.process (hsL.process (lsL.process (l)));
            r = pkR.process (hsR.process (lsR.process (r)));
            L[i] = l; R[i] = r;
        }
    }

private:
    Fdn8 board, sympa;
    Biquad lsL, lsR, hsL, hsR, pkL, pkR;
    float pedalSmooth = 0.0f;
};

//==============================================================================
inline void PianoVoice::setup (EngineBase* e)
{
    eng = e;
    pulse.assign ((size_t) (eng->sr * 0.05f) + 16, 0.0f);
    rng.seed ((uint32_t) (reinterpret_cast<uintptr_t> (this) & 0xffffffffu) | 1u);
}

// Inharmonicity coefficient for a concert grand, by key (log-linear fit).
inline float pianoInharm (int key)
{
    static const float k[] = { 21, 38, 60, 84, 108 };
    static const float v[] = { -3.6f, -4.0f, -3.4f, -2.55f, -1.8f };
    float x = clampf ((float) key, 21.0f, 108.0f);
    int i = 0;
    while (i < 3 && x > k[i + 1]) ++i;
    const float t = (x - k[i]) / (k[i + 1] - k[i]);
    return std::pow (10.0f, v[i] + t * (v[i + 1] - v[i]));
}

// Railsback-style stretch tuning in cents.
inline float pianoStretch (int key)
{
    const float d = (float) (key - 62);
    return d >= 0.0f ? 2.3e-4f * d * d * d : 3.3e-4f * d * d * d;
}

inline void PianoVoice::addResonator (float freq, float sigmaHz, float inGain, float gL, float gR)
{
    if (nRes >= kMaxRes) return;
    const float sr = eng->sr;
    const float wk = kTwoPi * freq / sr;
    const float r = std::exp (-sigmaHz / sr);
    const int k = nRes++;
    w[k] = wk;
    sigma[k] = sigmaHz;
    const float s = std::sin (wk);
    sin2[k] = s * s;
    c1[k] = 2.0f * r * std::cos (wk);
    c2[k] = r * r;
    gin[k] = s * inGain;
    gl[k] = gL;
    gr[k] = gR;
    y1[k] = y2[k] = 0.0f;
}

inline void PianoVoice::start (int keyIn, float velocity)
{
    const auto& P = eng->params->piano;
    const auto& C = *eng->ctx;
    const float sr = eng->sr;
    resetFade();
    active = true;
    releasing = false;
    damped = false;
    vel = velocity;
    key = std::clamp (keyIn + (int) std::lround (C.transpose), 0, 127);
    nRes = 0;
    cullCounter = 0;

    baseHz = C.hz (curNote) * centsToRatio (P.stretch * pianoStretch (key));
    appliedRatio = 1.0f;
    const float f0 = baseHz;
    const float B = pianoInharm (key) * P.inharm * (1.0f + 1.5f * P.piezo);

    const float felt = P.felt;
    const float soft = C.softPedal;
    const float fmax = std::min (0.45f * sr, 18000.0f);
    const int maxN = (int) P.maxPartials;

    // Sustain: T60 of the fundamental by register.
    const float t60 = std::max (0.25f, P.decay * 22.0f * std::exp2 (-(float) (key - 21) / 21.0f) * (1.0f - 0.5f * P.piezo));
    const float sigma1 = 6.9078f / t60;
    const float hf = P.hfDamp * (1.0f + 2.0f * felt + 1.5f * P.prep);

    // Unison strings (single strings modelled with two polarisations).
    const int strings = key <= 30 ? 1 : (key <= 46 ? 2 : 3);
    const int K = strings == 1 ? 2 : 3;
    float det[3], dm[3], amp[3], cp[3];
    const float u = P.unison;
    if (K == 2)
    {
        det[0] = 0.0f;            dm[0] = 1.0f;  amp[0] = 1.0f;                    cp[0] = -0.15f;
        det[1] = 0.12f * u + 0.1f; dm[1] = 0.3f;  amp[1] = 0.25f + 0.55f * P.aftersound; cp[1] = 0.15f;
        det[2] = 0; dm[2] = 1; amp[2] = 0; cp[2] = 0;
    }
    else
    {
        const float r0 = 0.7f + 0.3f * std::fabs (hashNoise ((uint32_t) key, 11u));
        const float r1 = 0.7f + 0.3f * std::fabs (hashNoise ((uint32_t) key, 23u));
        det[0] = -u * 0.5f * r0;  dm[0] = 1.0f;   amp[0] = 1.0f;                  cp[0] = -0.25f;
        det[1] = u * 0.45f * r1;  dm[1] = 1.12f;  amp[1] = (strings == 2 ? 0.9f : 1.0f) * (1.0f - 0.45f * soft); cp[1] = 0.25f;
        det[2] = u * 0.1f;        dm[2] = 0.27f;  amp[2] = 0.15f + 0.6f * P.aftersound; cp[2] = 0.0f;
    }

    const float x0 = clampf (P.hammerPos * (1.0f + 0.08f * hashNoise ((uint32_t) key, 3u)), 0.02f, 0.45f);
    const float fh = (P.piezo > 0.5f ? 9000.0f : 3200.0f) * std::exp2 (P.bodyTone + felt * -1.0f);
    const float width = P.width;
    const float keyPan = width * clampf ((float) (key - 64) / 40.0f, -1.0f, 1.0f);
    panGains (keyPan, panL, panR);

    // Normalisation over the partial set.
    float norm2 = 0.0f;
    float an[128];
    int N = 0;
    for (int n = 1; n <= maxN && n < 128; ++n)
    {
        const float fn = (float) n * f0 * std::sqrt (1.0f + B * (float) (n * n));
        if (fn > fmax) break;
        const float comb = std::sin (kPi * (float) n * x0);
        const float rad = 1.0f / (1.0f + sqr ((P.piezo > 0.5f ? 40.0f : 95.0f) / fn));
        const float board = 1.0f / std::sqrt (1.0f + sqr (fn / fh));
        an[n] = comb * rad * board * (1.0f - 0.5f * P.prep * (float) (n % 2));
        norm2 += an[n] * an[n];
        N = n;
    }
    const float norm = 1.0f / std::sqrt (std::max (norm2, 1.0e-6f)) / (K == 3 ? 2.0f : 1.2f);

    for (int c = 0; c < K; ++c)
    {
        float pl, pr;
        panGains (keyPan + cp[c] * width, pl, pr);
        const float ratio = centsToRatio (det[c]);
        for (int n = 1; n <= N; ++n)
        {
            const float fn = (float) n * f0 * std::sqrt (1.0f + B * (float) (n * n)) * ratio;
            const float fk = fn * 0.001f;
            const float sg = sigma1 * dm[c] * (1.0f + 0.15f * (float) (n - 1) / (float) N)
                           + hf * (0.6f * fk + 0.8f * fk * fk) * (c == 2 ? 1.6f : 1.0f);
            addResonator (fn, sg, an[n] * amp[c], pl * norm, pr * norm);
        }
    }

    // Tacks: bright metallic jangle (inharmonic, fast decaying).
    if (P.tack > 0.01f)
    {
        static const float tr[] = { 6.93f, 9.37f, 12.71f, 15.23f, 19.1f };
        for (int i = 0; i < 5; ++i)
        {
            const float f = f0 * tr[i] * (1.0f + 0.02f * hashNoise ((uint32_t) key, 100u + (uint32_t) i));
            if (f < fmax) addResonator (f, 6.9f / (0.12f + 0.1f * (float) (i & 1)), P.tack * 0.5f, panL * norm, panR * norm);
        }
    }
    // Preparations (bolts, screws, rubber): extra inharmonic partials.
    if (P.prep > 0.01f)
    {
        for (int i = 0; i < 6; ++i)
        {
            const float rr = 1.3f + 9.0f * (0.5f + 0.5f * hashNoise ((uint32_t) key, 200u + (uint32_t) i));
            const float f = f0 * rr;
            if (f < fmax) addResonator (f, 6.9f / (0.25f + 1.2f * (0.5f + 0.5f * hashNoise ((uint32_t) key, 300u + (uint32_t) i))),
                                        P.prep * 0.6f, 0.7f * norm, 0.7f * norm);
        }
    }

    // Hammer force pulse: velocity and hardness set its spectrum.
    const float v = vel;
    const float baseCorner = 1100.0f * std::exp2 ((float) (key - 60) / 30.0f);
    float corner = baseCorner * (0.4f + 1.4f * std::pow (v, 1.5f)) * (0.35f + 1.3f * P.hardness);
    corner *= (1.0f - 0.65f * felt) * (1.0f + 3.0f * P.tack) * (1.0f - 0.3f * soft) * (1.0f + 0.6f * P.piezo);
    const float p = clampf (1.8f - 1.4f * v * (0.4f + P.hardness), 0.5f, 2.2f) + 1.2f * felt;
    pulseLen = makeForcePulse (pulse.data(), (int) pulse.size(), sr, corner, p);
    pulsePos = 0;
    const float dynDb = (v - 1.0f) * (18.0f + 30.0f * P.velSens);
    exciteGain = dbToGain (dynDb) * 0.55f * (1.0f - 0.3f * soft) * (1.0f + 1.6f * felt) * (1.0f + 0.6f * P.piezo);

    // Mechanical noises.
    const float hn = P.hammerNoise;
    knockEnv = hn * v * v * 0.05f * (1.0f + 3.0f * P.tack) * exciteGain / 0.55f;
    knockMul = std::exp (-1.0f / (0.005f * sr));
    knockBp.setBandpass (std::min (1600.0f + 2500.0f * P.hardness + 5000.0f * P.tack, sr * 0.4f), 0.9, sr);
    thumpEnv = hn * v * 0.12f * (key < 64 ? 1.0f : 0.5f) * exciteGain / 0.55f;
    thumpMul = std::exp (-1.0f / (0.012f * sr));
    thumpLp.setCutoff (140.0f, sr);
    mechEnv = P.mechNoise * (0.3f + 0.4f * v) * 0.04f;
    mechMul = std::exp (-1.0f / (0.02f * sr));
    mechBp.setBandpass (380.0, 1.2, sr);
    relEnv = 0.0f;
    relLp.setLowpass (260.0, 0.8, sr);
}

inline void PianoVoice::applyDamping (float extra)
{
    const float sr = eng->sr;
    for (int k = 0; k < nRes; ++k)
    {
        const float r = std::exp (-(sigma[k] + extra) / sr);
        c1[k] = 2.0f * r * std::cos (w[k] * appliedRatio);
        c2[k] = r * r;
    }
}

inline void PianoVoice::release()
{
    releasing = true;
    const auto& P = eng->params->piano;
    if (key >= 89) return;   // top strings have no dampers
    damped = true;
    const float halfPedal = clampf (eng->ctx->pedal * 2.0f, 0.0f, 1.0f);
    const float t = P.release * (1.0f + std::max (0.0f, (float) (48 - key)) / 30.0f) * (1.0f + 10.0f * halfPedal * halfPedal);
    applyDamping (6.9078f / std::max (t, 0.01f));
    relEnv = (P.releaseNoise * (0.4f + 0.6f * vel) * 0.025f + P.mechNoise * 0.015f) * std::sqrt (exciteGain / 0.55f);
    relMul = std::exp (-1.0f / (0.035f * eng->sr));
}

inline void PianoVoice::updatePitch()
{
    const float hz = eng->ctx->hz (curNote) * centsToRatio (eng->params->piano.stretch * pianoStretch (key));
    const float ratio = hz / baseHz;
    if (std::fabs (ratio - appliedRatio) < 1.0e-5f) return;
    appliedRatio = ratio;
    for (int k = 0; k < nRes; ++k)
    {
        const float r = std::sqrt (c2[k]);
        c1[k] = 2.0f * r * std::cos (std::min (w[k] * ratio, kPi * 0.98f));
    }
}

inline void PianoVoice::render (float* L, float* R, int n)
{
    if (! active) return;
    updatePitch();
    const int nr = nRes;
    float* __restrict Y1 = y1;
    float* __restrict Y2 = y2;
    const float* __restrict C1 = c1;
    const float* __restrict C2 = c2;
    const float* __restrict GI = gin;
    const float* __restrict GL = gl;
    const float* __restrict GR = gr;

    for (int i = 0; i < n; ++i)
    {
        float sL = 0.0f, sR = 0.0f;
        if (pulsePos < pulseLen)
        {
            const float x = pulse[(size_t) pulsePos++] * exciteGain;
            KV_SIMD_REDUCE (sL, sR)
            for (int k = 0; k < nr; ++k)
            {
                const float y = C1[k] * Y1[k] - C2[k] * Y2[k] + GI[k] * x;
                Y2[k] = Y1[k]; Y1[k] = y;
                sL += GL[k] * y; sR += GR[k] * y;
            }
        }
        else
        {
            KV_SIMD_REDUCE (sL, sR)
            for (int k = 0; k < nr; ++k)
            {
                const float y = C1[k] * Y1[k] - C2[k] * Y2[k];
                Y2[k] = Y1[k]; Y1[k] = y;
                sL += GL[k] * y; sR += GR[k] * y;
            }
        }

        // hammer knock, low thump, damper/key noises (mono, panned with key)
        float nz = 0.0f;
        if (knockEnv > 1.0e-6f) { nz += knockBp.process (rng.bi()) * knockEnv; knockEnv *= knockMul; }
        if (thumpEnv > 1.0e-6f) { nz += thumpLp.process (rng.bi()) * thumpEnv * 4.0f; thumpEnv *= thumpMul; }
        if (mechEnv > 1.0e-6f)  { nz += mechBp.process (rng.bi()) * mechEnv; mechEnv *= mechMul; }
        if (relEnv > 1.0e-6f)   { nz += relLp.process (rng.bi()) * relEnv * 3.0f; relEnv *= relMul; }
        sL += nz * panL;
        sR += nz * panR;

        if (! applyFade (sL, sR)) { active = false; L[i] += sL; R[i] += sR; return; }
        L[i] += sL;
        R[i] += sR;
    }

    // Cull inaudible partials (keeps CPU proportional to what is heard).
    if (++cullCounter >= 4 && pulsePos >= pulseLen)
    {
        cullCounter = 0;
        const float thr = 1.0e-10f;
        for (int k = 0; k < nRes;)
        {
            const float e = Y1[k] * Y1[k] + Y2[k] * Y2[k] - C1[k] * Y1[k] * Y2[k];
            const float gm = std::max (GL[k], GR[k]);
            if (e * gm * gm < thr * sin2[k])
            {
                const int last = --nRes;
                y1[k] = y1[last]; y2[k] = y2[last]; c1[k] = c1[last]; c2[k] = c2[last];
                gin[k] = gin[last]; gl[k] = gl[last]; gr[k] = gr[last];
                w[k] = w[last]; sigma[k] = sigma[last]; sin2[k] = sin2[last];
            }
            else ++k;
        }
        if (nRes == 0 && knockEnv < 1.0e-6f && relEnv < 1.0e-6f && thumpEnv < 1.0e-6f && mechEnv < 1.0e-6f)
            active = false;
    }
}

} // namespace kv
