// Keyverse - organs: tonewheel (9 drawbars, percussion, key click, scanner
// vibrato/chorus), transistor & Vox-style combo organs, pipe organ ranks
// (principal / flute / reed with chiff and wind), reed organ / harmonium and
// accordion (detuned musette reeds). Rotary speaker lives in the FX chain.
#pragma once

#include "Voice.h"

namespace kv
{
// Band-limited single-cycle wavetables (mip-mapped by harmonic count).
class WaveBank
{
public:
    static constexpr int N = 2048, Levels = 11;
    enum Wave { Tonewheel = 0, Transistor, Vox, Principal, Flute, PipeReed, ReedOrgan, Accordion, NumWaves };

    WaveBank()
    {
        data.assign ((size_t) NumWaves * Levels * (N + 1), 0.0f);
        std::vector<double> tmp ((size_t) N), st ((size_t) N);
        for (int i = 0; i < N; ++i) st[(size_t) i] = std::sin (2.0 * 3.14159265358979323846 * i / N);
        for (int w = 0; w < NumWaves; ++w)
        {
            float scale = 1.0f;
            for (int lv = 0; lv < Levels; ++lv)
            {
                const int maxH = std::max (1, 1024 >> lv);
                std::fill (tmp.begin(), tmp.end(), 0.0);
                for (int h = 1; h <= maxH; ++h)
                {
                    const double a = harmonic (w, h);
                    if (std::fabs (a) < 1.0e-6) continue;
                    for (int i = 0; i < N; ++i)
                        tmp[(size_t) i] += a * st[(size_t) ((h * i) & (N - 1))];
                }
                if (lv == 0)
                {
                    double pk = 1.0e-9;
                    for (double x : tmp) pk = std::max (pk, std::fabs (x));
                    scale = (float) (1.0 / pk);
                }
                float* t = table (w, lv);
                for (int i = 0; i < N; ++i) t[i] = (float) tmp[(size_t) i] * scale;
                t[N] = t[0];
            }
        }
    }

    float* table (int w, int lv) { return data.data() + ((size_t) w * Levels + (size_t) lv) * (N + 1); }
    const float* table (int w, int lv) const { return data.data() + ((size_t) w * Levels + (size_t) lv) * (N + 1); }

    static int levelFor (float freq, float sr)
    {
        const float allowed = 0.45f * sr / std::max (freq, 1.0f);
        int lv = 0;
        while (lv < Levels - 1 && (float) (1024 >> lv) > allowed) ++lv;
        return lv;
    }

    float read (const float* t, float phase) const
    {
        const float f = phase * N;
        const int i = (int) f;
        const float fr = f - (float) i;
        return t[i] + fr * (t[i + 1] - t[i]);
    }

    static const WaveBank& get() { static const WaveBank b; return b; }

private:
    static double harmonic (int w, int h)
    {
        const double hd = h;
        switch (w)
        {
            case Tonewheel:  return h == 1 ? 1.0 : (h == 2 ? 0.015 : (h == 3 ? 0.012 : 0.0));
            case Transistor: return (h % 2 ? 1.0 : 0.45) / hd * std::exp (-hd / 60.0);
            case Vox:        return h == 1 ? 1.0 : (h % 2 ? 0.32 / std::pow (hd, 1.3) : 0.08 / std::pow (hd, 1.5));
            case Principal:  return (h == 2 ? 1.15 : 1.0) / std::pow (hd, 1.25) * std::exp (-(hd - 1.0) / 14.0);
            case Flute:      return h == 1 ? 1.0 : (h == 2 ? 0.07 : (h == 3 ? 0.11 : (h == 4 ? 0.02 : 0.0)));
            case PipeReed:   return 1.0 / std::pow (hd, 0.75) * (1.0 + 1.2 * std::exp (-sqr ((float) (hd - 5.0)) / 6.0)) * std::exp (-hd / 40.0);
            case ReedOrgan:  return std::fabs (std::sin (3.14159265358979323846 * hd * 0.12)) / hd * 3.0 * std::exp (-hd / 50.0);
            case Accordion:  return std::fabs (std::sin (3.14159265358979323846 * hd * 0.27)) / std::pow (hd, 1.05) * 1.5 * std::exp (-hd / 35.0);
            default: return 0.0;
        }
    }
    std::vector<float> data;
};

class OrganEngine;

class OrganVoice : public Voice
{
public:
    void setup (EngineBase* e);
    void start (int keyIn, float velocity) override;
    void release() override
    {
        releasing = true;
        const auto& P = eng->params->organ;
        relMul = std::exp (-6.9f / (std::max (P.release, 0.003f) * eng->sr));
        envTarget = 0.0f;
        clickEnv = P.click * 0.06f * (0.5f + 0.5f * rng.uni());
    }
    void kill (float s) override { beginFade (s, eng->sr); }
    void render (float* L, float* R, int n) override;

    bool percTrigger = false;

private:
    void computeIncs();
    // Drawbar levels (~3 dB per step like a tonewheel organ); called every
    // chunk so registration changes and macros affect held notes.
    float updateDrawbars()
    {
        const auto& P = eng->params->organ;
        float total = 0.0f;
        for (int d = 0; d < 9; ++d)
        {
            float db = P.db[d];
            if (model == 0 && P.percOn > 0.5f && d == 8) db = 0.0f;   // 1' bus used by percussion
            amp[d] = db > 0.05f ? dbToGain ((db - 8.0f) * 3.0f) : 0.0f;
            total += amp[d];
        }
        const float levelNorm = 0.16f / std::max (1.0f, std::sqrt (total * 1.6f)) / std::sqrt ((float) nVoices);
        for (int d = 0; d < 9; ++d) amp[d] *= levelNorm * velGain;
        return levelNorm;
    }
    float velGain = 1.0f;
    EngineBase* eng = nullptr;
    float phase[9][3] {}, inc[9][3] {}, amp[9] {};
    const float* tab[9] {};
    int nVoices = 1, model = 0, key = 60;
    float env = 0.0f, envTarget = 1.0f, attInc = 1.0f, relMul = 0.99f;
    float percEnv = 0.0f, percMul = 0.0f, percPhase = 0.0f, percInc = 0.0f;
    const float* percTab = nullptr;
    float leakPhase[2] {}, leakInc[2] {}, leakAmp = 0.0f;
    float clickEnv = 0.0f, clickMul = 0.0f, chiffEnv = 0.0f, chiffMul = 0.0f, windAmt = 0.0f;
    Biquad clickBp, chiffBp;
    OnePoleLP windLp;
    float baseHz = 440.0f, lastHz = 0.0f;
    float panL = 0.7f, panR = 0.7f;
    Rng rng;
};

class OrganEngine : public PolyEngine<OrganVoice>
{
public:
    OrganEngine() : PolyEngine<OrganVoice> (48) {}
    void prepare (float sampleRate, int maxBlk) override
    {
        sr = sampleRate; maxBlock = maxBlk;
        prepareVoices();
        vibL.init ((int) (sr * 0.01f));
        vibR.init ((int) (sr * 0.01f));
        resetBus();
    }
    void resetBus() override
    {
        vibL.clear(); vibR.clear();
        toneL.reset(); toneR.reset();
        scanPhase = 0.0f;
    }
    bool stealSameNote() const override { return true; }
    float defaultStealFade() const override { return 0.005f; }

    void onNoteOn (int heldNotes) override { nextPercTrigger = heldNotes == 0; }
    bool consumePercTrigger() { const bool t = nextPercTrigger; nextPercTrigger = false; return t; }

    void processBus (float* L, float* R, int n) override
    {
        const auto& P = params->organ;
        toneL.setCutoff (P.tone, sr); toneR.setCutoff (P.tone, sr);
        const int vib = (int) P.vibrato;
        static const float depthMs[] = { 0.0f, 0.18f, 0.36f, 0.62f, 0.18f, 0.36f, 0.62f };
        const float depth = depthMs[std::clamp (vib, 0, 6)] * 0.001f * sr;
        const bool chorus = vib >= 4;
        const float inc = 6.9f / sr;
        const float drive = P.drive;
        const float dg = 1.0f + 9.0f * drive * drive;
        const float dn = 1.0f / fastTanh (dg * 0.5f) * 0.5f;
        for (int i = 0; i < n; ++i)
        {
            float l = toneL.process (L[i]);
            float r = toneR.process (R[i]);
            if (vib > 0)
            {
                vibL.push (l); vibR.push (r);
                scanPhase += inc; if (scanPhase >= 1.0f) scanPhase -= 1.0f;
                const float tri = 1.0f - 4.0f * std::fabs (scanPhase - 0.5f);
                const float d = 1.0f + depth * (1.0f + tri);
                const float wl = vibL.tapLinear (d), wr = vibR.tapLinear (d);
                l = chorus ? 0.5f * (l + wl) * 1.3f : wl;
                r = chorus ? 0.5f * (r + wr) * 1.3f : wr;
            }
            if (drive > 0.01f)
            {
                l = asymSat (l * dg, 0.12f) * dn;
                r = asymSat (r * dg, 0.12f) * dn;
            }
            L[i] = l; R[i] = r;
        }
    }

private:
    bool nextPercTrigger = true;
    DelayLine vibL, vibR;
    OnePoleLP toneL, toneR;
    float scanPhase = 0.0f;
};

//==============================================================================
inline void OrganVoice::setup (EngineBase* e)
{
    eng = e;
    rng.seed ((uint32_t) (reinterpret_cast<uintptr_t> (this) & 0xffffffffu) | 9u);
}

static constexpr float kFootage[9] = { 0.5f, 1.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f };

inline void OrganVoice::computeIncs()
{
    const auto& P = eng->params->organ;
    const float sr = eng->sr;
    const auto& bank = WaveBank::get();
    static const int waveOf[8] = { WaveBank::Tonewheel, WaveBank::Transistor, WaveBank::Vox, WaveBank::Principal,
                                   WaveBank::Flute, WaveBank::PipeReed, WaveBank::ReedOrgan, WaveBank::Accordion };
    const int wave = waveOf[std::clamp (model, 0, 7)];
    const float hz = lastHz;
    for (int d = 0; d < 9; ++d)
    {
        float f = hz * kFootage[d];
        if (model == 0)
        {
            while (f < 32.7f) f *= 2.0f;   // tonewheel foldback
            while (f > 5920.0f) f *= 0.5f;
        }
        else
        {
            while (f > sr * 0.45f) f *= 0.5f;
        }
        tab[d] = bank.table (wave, WaveBank::levelFor (f, sr));
        for (int k = 0; k < nVoices; ++k)
        {
            const float det = k == 0 ? 0.0f : (k == 1 ? P.musette : -P.musette * 0.9f);
            inc[d][k] = f * centsToRatio (det) / sr;
        }
    }
    float pf = hz * (P.percHarm > 0.5f ? 3.0f : 2.0f);
    while (pf > 5920.0f) pf *= 0.5f;
    percInc = pf / sr;
    percTab = bank.table (wave, WaveBank::levelFor (pf, sr));
    leakInc[0] = hz * 1.4983f / sr;
    leakInc[1] = hz * 2.9966f / sr;
}

inline void OrganVoice::start (int keyIn, float velocity)
{
    const auto& P = eng->params->organ;
    const auto& C = *eng->ctx;
    const float sr = eng->sr;
    resetFade();
    active = true; releasing = false;
    vel = velocity;
    key = std::clamp (keyIn + (int) std::lround (C.transpose), 0, 127);
    model = (int) P.model;
    nVoices = std::clamp ((int) P.musetteVoices, 1, 3);
    baseHz = lastHz = C.hz (curNote);
    computeIncs();

    const bool pipe = model >= 3 && model <= 5;
    const bool reed = model >= 6;
    // tonewheels start at random phases (key click character)
    for (int d = 0; d < 9; ++d)
        for (int k = 0; k < 3; ++k) phase[d][k] = rng.uni();
    // reed/pipe organs respond to velocity a little, tonewheels don't
    velGain = (pipe || model == 0 || model == 1 || model == 2) ? 1.0f : (0.55f + 0.45f * vel);
    if (model == 5) velGain *= 1.5f;          // pipe reeds
    if (model >= 6) velGain *= 1.7f;          // reed organ / accordion
    const float levelNorm = updateDrawbars();

    env = 0.0f;
    envTarget = 1.0f;
    attInc = 1.0f / std::max (1.0f, P.attack * sr * (pipe ? (1.0f + 0.3f * (float) (72 - key) / 36.0f) : 1.0f));
    relMul = 1.0f;

    auto* oe = static_cast<OrganEngine*> (eng);
    const bool trig = oe->consumePercTrigger();
    percEnv = (model == 0 && P.percOn > 0.5f && trig) ? (P.percSoft > 0.5f ? 0.5f : 1.0f) * levelNorm * 2.2f : 0.0f;
    percMul = std::exp (-6.9f / ((P.percFast > 0.5f ? 0.45f : 1.4f) * sr));
    percPhase = 0.0f;

    leakAmp = model == 0 ? P.leakage * 0.004f : 0.0f;
    clickEnv = (model <= 2) ? P.click * 0.12f * (0.6f + 0.4f * rng.uni()) : 0.0f;
    clickMul = std::exp (-1.0f / (0.0025f * sr));
    clickBp.setBandpass (std::min (2500.0f + 1500.0f * rng.uni(), sr * 0.4f), 0.7, sr);
    chiffEnv = pipe ? P.chiff * 0.08f : 0.0f;
    chiffMul = std::exp (-1.0f / (0.045f * sr));
    chiffBp.setBandpass (std::min (baseHz * 3.0f, sr * 0.4f), 2.5, sr);
    windAmt = (pipe || reed) ? P.wind * 0.012f : 0.0f;
    windLp.setCutoff (1800.0f, sr);

    const float keyPan = clampf ((float) (key - 64) / 60.0f, -0.4f, 0.4f);
    panGains (keyPan, panL, panR);
}

inline void OrganVoice::render (float* L, float* R, int n)
{
    if (! active) return;
    const auto& bank = WaveBank::get();
    const float hz = eng->ctx->hz (curNote);
    if (std::fabs (hz - lastHz) > 1.0e-3f * lastHz) { lastHz = hz; computeIncs(); }
    updateDrawbars();

    for (int i = 0; i < n; ++i)
    {
        if (envTarget > 0.5f) { env += attInc; if (env > 1.0f) env = 1.0f; }
        else { env *= relMul; if (env < 1.0e-5f) { active = false; return; } }

        float s = 0.0f;
        for (int d = 0; d < 9; ++d)
        {
            if (amp[d] <= 0.0f) continue;
            float o = 0.0f;
            for (int k = 0; k < nVoices; ++k)
            {
                float& ph = phase[d][k];
                o += bank.read (tab[d], ph);
                ph += inc[d][k]; if (ph >= 1.0f) ph -= 1.0f;
            }
            s += o * amp[d];
        }
        if (percEnv > 1.0e-6f)
        {
            s += bank.read (percTab, percPhase) * percEnv;
            percPhase += percInc; if (percPhase >= 1.0f) percPhase -= 1.0f;
            percEnv *= percMul;
        }
        if (leakAmp > 0.0f)
        {
            s += (sin01 (leakPhase[0]) + 0.6f * sin01 (leakPhase[1])) * leakAmp;
            leakPhase[0] += leakInc[0]; if (leakPhase[0] >= 1.0f) leakPhase[0] -= 1.0f;
            leakPhase[1] += leakInc[1]; if (leakPhase[1] >= 1.0f) leakPhase[1] -= 1.0f;
        }
        s *= env;
        if (windAmt > 0.0f) s += windLp.process (rng.bi()) * windAmt * env;
        if (clickEnv > 1.0e-6f) { s += clickBp.process (rng.bi()) * clickEnv; clickEnv *= clickMul; }
        if (chiffEnv > 1.0e-6f) { s += chiffBp.process (rng.bi()) * chiffEnv * 4.0f; chiffEnv *= chiffMul; }

        float l = s * panL, r = s * panR;
        if (! applyFade (l, r)) { active = false; return; }
        L[i] += l; R[i] += r;
    }
}

} // namespace kv
