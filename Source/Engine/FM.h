// Keyverse - 6-operator FM synthesis (DX-style phase modulation).
// Twelve routing algorithms, per-operator ADSR, velocity & key scaling,
// operator feedback, fixed-frequency operators and an LFO.
#pragma once

#include "Voice.h"

namespace kv
{
struct FMAlg
{
    uint8_t mods[6];     // bitmask of operators modulating op i (always higher index)
    uint8_t carriers;    // bitmask of operators sent to the output
    int fb;              // operator with self feedback
};

inline const FMAlg& fmAlg (int i)
{
    static const FMAlg algs[12] = {
        { { 1u << 1, 1u << 2, 1u << 3, 1u << 4, 1u << 5, 0 }, 1u << 0, 5 },                                  // 6-op stack
        { { 1u << 1, 1u << 2, 0, 1u << 4, 1u << 5, 0 }, (1u << 0) | (1u << 3), 5 },                          // two 3-op stacks
        { { 1u << 1, 0, 1u << 3, 0, 1u << 5, 0 }, (1u << 0) | (1u << 2) | (1u << 4), 5 },                   // three pairs
        { { 1u << 1, 0, 1u << 3, 1u << 4, 1u << 5, 0 }, (1u << 0) | (1u << 2), 5 },                         // 4-stack + pair
        { { 1u << 1, 0, 1u << 3, 1u << 4, 1u << 5, 0 }, (1u << 0) | (1u << 2), 1 },                         // pair + 4-stack, fb op2
        { { 1u << 1, 0, 1u << 5, 1u << 5, 1u << 5, 0 }, (1u << 0) | (1u << 2) | (1u << 3) | (1u << 4), 5 }, // one mod -> 3 carriers
        { { (1u << 1) | (1u << 2) | (1u << 4), 0, 1u << 3, 0, 1u << 5, 0 }, 1u << 0, 5 },                  // three branches -> 1
        { { 0, 0, 0, 0, 0, 0 }, 0x3fu, 5 },                                                                  // six carriers
        { { (1u << 1) | (1u << 2), 0, 0, (1u << 4) | (1u << 5), 0, 0 }, (1u << 0) | (1u << 3), 5 },        // two mods -> 2 carriers
        { { 1u << 1, 1u << 2, 0, 1u << 4, 0, 0 }, (1u << 0) | (1u << 3) | (1u << 5), 5 },                  // 3-stack + pair + carrier
        { { 1u << 1, 0, 0, 0, 0, 0 }, (1u << 0) | (1u << 2) | (1u << 3) | (1u << 4) | (1u << 5), 5 },       // pair + 4 carriers
        { { (1u << 1) | (1u << 3), 1u << 2, 0, 1u << 4, 1u << 5, 0 }, 1u << 0, 5 },                         // Y-branch stack
    };
    return algs[std::clamp (i, 0, 11)];
}

// Operator level (0..1) -> linear amplitude, ~72 dB range like classic FM.
inline float fmLevel (float l) { return l <= 0.001f ? 0.0f : std::pow (10.0f, (l - 1.0f) * 3.6f); }

class FMVoice : public Voice
{
public:
    void setup (EngineBase* e)
    {
        eng = e;
        rng.seed ((uint32_t) (reinterpret_cast<uintptr_t> (this) & 0xffffffffu) | 13u);
    }

    void start (int keyIn, float velocity) override
    {
        const auto& P = eng->params->fm;
        const float sr = eng->sr;
        const bool wasActive = active && ! dying;
        resetFade();
        active = true; releasing = false;
        vel = velocity;
        key = keyIn;
        alg = fmAlg ((int) P.alg);
        const float kt = (float) (key - 60) / 24.0f;
        const float rs = std::exp2 (-P.rateScale * kt);
        int nc = 0;
        for (int i = 0; i < 6; ++i)
        {
            const auto& o = P.op[i];
            env[i].set (o.A * rs, o.D * rs, o.S, o.R * rs, sr);
            env[i].noteOn (! wasActive);
            const float velScale = 1.0f - o.velSens * (1.0f - vel);
            const float keyScale = std::exp2 (-o.keyScale * kt * 1.5f);
            lvl[i] = fmLevel (o.level) * velScale * keyScale;
            if (alg.carriers & (1u << i)) ++nc;
            if (! wasActive) { phase[i] = 0.0f; out[i] = 0.0f; }
        }
        fb1 = fb2 = 0.0f;
        carrierNorm = 0.36f / std::pow ((float) std::max (nc, 1), 0.6f);
        lfoPhase = 0.0f;
        lfoFade = 0.0f;
    }

    void release() override
    {
        releasing = true;
        for (auto& e : env) e.noteOff();
    }
    void kill (float s) override { beginFade (s, eng->sr); }

    void render (float* L, float* R, int n) override
    {
        if (! active) return;
        const auto& P = eng->params->fm;
        const auto& C = *eng->ctx;
        const float sr = eng->sr;

        lfoPhase += P.lfoRate * (float) n / sr;
        lfoPhase -= std::floor (lfoPhase);
        lfoFade = std::min (1.0f, lfoFade + (float) n / (std::max (P.lfoDelay, 0.01f) * sr));
        const float lfo = sin01 (lfoPhase) * lfoFade;
        const float hz = C.hz (curNote + lfo * P.lfoPitch);
        float inc[6];
        for (int i = 0; i < 6; ++i)
        {
            const auto& o = P.op[i];
            const float f = o.fixed > 0.5f ? o.fixedHz : hz * o.ratio * centsToRatio (o.detune);
            inc[i] = std::min (f / sr, 0.49f);
        }
        const float ampMod = 1.0f - P.lfoAmp * (0.5f + 0.5f * lfo);
        const float fbAmt = P.feedback * P.feedback * 0.35f;
        constexpr float modScale = 1.6f;   // cycles of phase deviation at full level
        bool anyActive = false;

        for (int s = 0; s < n; ++s)
        {
            float mix = 0.0f;
            for (int i = 5; i >= 0; --i)
            {
                const float e = env[i].next();
                float pm = 0.0f;
                const uint8_t m = alg.mods[i];
                if (m)
                    for (int j = i + 1; j < 6; ++j)
                        if (m & (1u << j)) pm += out[j];
                pm *= modScale;
                if (i == alg.fb) pm += (fb1 + fb2) * fbAmt;
                const float y = sin01 (phase[i] + pm) * e * lvl[i];
                if (i == alg.fb) { fb2 = fb1; fb1 = y; }
                out[i] = y;
                phase[i] += inc[i];
                if (phase[i] >= 1.0f) phase[i] -= 1.0f;
                if (alg.carriers & (1u << i)) mix += y;
            }
            float v = mix * carrierNorm * ampMod;
            float l = v * 0.707f, r = v * 0.707f;
            if (! applyFade (l, r)) { active = false; return; }
            L[s] += l; R[s] += r;
        }
        for (int i = 0; i < 6; ++i)
            if ((alg.carriers & (1u << i)) && env[i].active() && lvl[i] > 0.0f) anyActive = true;
        if (! anyActive) active = false;
    }

private:
    EngineBase* eng = nullptr;
    FMAlg alg {};
    Adsr env[6];
    float phase[6] {}, out[6] {}, lvl[6] {};
    float fb1 = 0.0f, fb2 = 0.0f, carrierNorm = 0.3f, lfoPhase = 0.0f, lfoFade = 0.0f;
    int key = 60;
    Rng rng;
};

class FMEngine : public PolyEngine<FMVoice>
{
public:
    FMEngine() : PolyEngine<FMVoice> (32) {}
    void prepare (float sampleRate, int maxBlk) override
    {
        sr = sampleRate; maxBlock = maxBlk;
        prepareVoices();
    }
    bool stealSameNote() const override { return true; }
    float defaultStealFade() const override { return 0.01f; }
};

} // namespace kv
