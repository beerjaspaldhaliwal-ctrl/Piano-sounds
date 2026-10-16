// Keyverse - virtual analog polysynth.
// 3 oscillators (PolyBLEP saw / pulse / triangle / sine / noise / supersaw),
// up to 8-voice unison with stereo spread, sub osc, hard sync, ring mod and
// osc3 -> osc1 FM; transistor ladder, multimode SVF, MS-20 style LP+HP and
// a vowel/formant filter; two ADSRs, pitch envelope, two LFOs, analog drift.
#pragma once

#include "Voice.h"

namespace kv
{
class AnalogEngine;

struct FormantBank
{
    Svf bp[3];
    float gains[3] { 1, 0.5f, 0.1f };
    void set (float vowel, float shift, float res, float sr)
    {
        static const float F[5][3] = { { 800, 1150, 2800 }, { 350, 2000, 2800 }, { 270, 2140, 2950 }, { 450, 800, 2830 }, { 325, 700, 2700 } };
        static const float G[5][3] = { { 0, -6, -18 }, { 0, -14, -18 }, { 0, -12, -20 }, { 0, -10, -18 }, { 0, -16, -28 } };
        const float v = clampf (vowel, 0.0f, 3.999f);
        const int i = (int) v;
        const float t = v - (float) i;
        for (int k = 0; k < 3; ++k)
        {
            const float f = lerpf (F[i][k], F[i + 1][k], t) * shift;
            const float g = lerpf (G[i][k], G[i + 1][k], t);
            bp[k].setQ (f, 4.0f + 14.0f * res, sr);
            gains[k] = dbToGain (g);
        }
    }
    float process (float x)
    {
        float y = 0.0f;
        for (int k = 0; k < 3; ++k) y += bp[k].bandpass (x) * gains[k];
        return y * 2.0f;
    }
    void reset() { for (auto& b : bp) b.reset(); }
};

class AnalogVoice : public Voice
{
public:
    static constexpr int kMaxUni = 8;

    void setup (EngineBase* e)
    {
        eng = e;
        rng.seed ((uint32_t) (reinterpret_cast<uintptr_t> (this) & 0xffffffffu) | 11u);
        for (int k = 0; k < kMaxUni; ++k) { ph1[k] = rng.uni(); ph2[k] = rng.uni(); }
    }

    void start (int keyIn, float velocity) override
    {
        const auto& P = eng->params->analog;
        const float sr = eng->sr;
        const bool wasActive = active && ! dying;
        resetFade();
        active = true; releasing = false;
        vel = velocity;
        key = keyIn;
        ampEnv.set (P.aA, P.aD, P.aS, P.aR, sr);
        filtEnv.set (P.fA, P.fD, P.fS, P.fR, sr);
        ampEnv.noteOn (! wasActive);
        filtEnv.noteOn (! wasActive);
        pEnv = 1.0f;
        pEnvMul = std::exp (-1.0f / (std::max (P.pDecay, 0.001f) * sr * 0.25f));
        lfo1.shape = (int) P.l1Shape;
        lfo1.reset (0.0f);
        lfo1Fade = P.l1Delay > 0.01f ? 0.0f : 1.0f;
        lfo1FadeInc = 32.0f / (std::max (P.l1Delay, 0.01f) * sr);
        if (! wasActive)
        {
            ladL.reset(); ladR.reset(); svfL.reset(); svfR.reset(); hpL.reset(); hpR.reset();
            svf2L.reset(); svf2R.reset(); fmtL.reset(); fmtR.reset();
            for (int k = 0; k < kMaxUni; ++k) { ph1[k] = rng.uni(); ph2[k] = rng.uni(); }
            ph3 = rng.uni(); phSub = 0.0f;
        }
        velGain = 1.0f - P.velAmp * (1.0f - vel * vel) * 0.9f;
        for (int k = 0; k < 3; ++k) driftV[k] = rng.bi() * 0.3f;
    }

    void release() override
    {
        releasing = true;
        ampEnv.noteOff();
        filtEnv.noteOff();
    }

    void kill (float s) override { beginFade (s, eng->sr); }

    void render (float* L, float* R, int n) override;

private:
    float osc (int wave, float& ph, float inc, float pw)
    {
        float v = 0.0f;
        switch (wave)
        {
            case 1:
            {
                v = ph < pw ? 1.0f : -1.0f;
                v += polyBlep (ph, inc);
                float t2 = ph + 1.0f - pw; if (t2 >= 1.0f) t2 -= 1.0f;
                v -= polyBlep (t2, inc);
                break;
            }
            case 2: v = 1.0f - 4.0f * std::fabs (ph - 0.5f); break;
            case 3: v = sin01 (ph); break;
            case 4: v = rng.bi(); break;
            default: v = 2.0f * ph - 1.0f - polyBlep (ph, inc); break;
        }
        ph += inc;
        if (ph >= 1.0f) ph -= 1.0f;
        return v;
    }

    EngineBase* eng = nullptr;
    int key = 60;
    float ph1[kMaxUni] {}, ph2[kMaxUni] {}, ph3 = 0.0f, phSub = 0.0f;
    Adsr ampEnv, filtEnv;
    float pEnv = 0.0f, pEnvMul = 0.0f, velGain = 1.0f;
    Lfo lfo1;
    float lfo1Fade = 1.0f, lfo1FadeInc = 0.0f;
    Ladder ladL, ladR;
    Svf svfL, svfR, svf2L, svf2R;
    OnePoleHP hpL, hpR;
    FormantBank fmtL, fmtR;
    float driftV[3] {}, driftT[3] {};
    Rng rng;
};

class AnalogEngine : public PolyEngine<AnalogVoice>
{
public:
    AnalogEngine() : PolyEngine<AnalogVoice> (36) {}
    void prepare (float sampleRate, int maxBlk) override
    {
        sr = sampleRate; maxBlock = maxBlk;
        prepareVoices();
    }
    bool stealSameNote() const override { return false; }
    float defaultStealFade() const override { return 0.006f; }

    // global LFO 2 (shared by voices so pads move together)
    float lfo2Value = 0.0f;
    void advanceLfo2 (int n)
    {
        const auto& P = params->analog;
        lfo2.shape = (int) P.l2Shape;
        lfo2.setRate (P.l2Rate, sr);
        lfo2Value = lfo2.advance (n);
    }
    void processBus (float*, float*, int) override {}

private:
    Lfo lfo2;
};

inline void AnalogVoice::render (float* L, float* R, int n)
{
    if (! active) return;
    const auto& P = eng->params->analog;
    const auto& C = *eng->ctx;
    auto* ae = static_cast<AnalogEngine*> (eng);
    const float sr = eng->sr;

    // ---- control-rate modulation
    lfo1.setRate (P.l1Rate, sr);
    const float l1 = lfo1.advance (n) * lfo1Fade;
    lfo1Fade = std::min (1.0f, lfo1Fade + lfo1FadeInc * (float) n / 32.0f);
    const float l2 = ae->lfo2Value;
    for (int k = 0; k < 3; ++k)
    {
        if (rng.uni() < 0.02f) driftT[k] = rng.bi();
        driftV[k] += (driftT[k] - driftV[k]) * 0.01f;
    }
    const float drift = P.drift * 0.12f;
    const float pe = P.pEnv * pEnv;
    pEnv *= std::pow (pEnvMul, (float) n);
    const float vib = l1 * P.l1Pitch + C.aftertouch * P.atVib * sin01 ((float) key * 0.1f);
    const float baseNote = curNote + pe + vib;

    int uni = std::clamp ((int) P.unison, 1, kMaxUni);
    const int w1 = (int) P.o1Wave, w2 = (int) P.o2Wave, w3 = (int) P.o3Wave;
    const bool super = (w1 == 5 || w2 == 5);
    if (super) uni = std::max (uni, 7);
    const float detune = P.uniDetune * (super ? 1.2f : 1.0f);

    const float n1 = baseNote + P.o1Oct * 12.0f + P.o1Semi + P.o1Fine * 0.01f + drift * driftV[0];
    const float n2 = baseNote + P.o2Oct * 12.0f + P.o2Semi + P.o2Fine * 0.01f + drift * driftV[1];
    const float n3 = baseNote + P.o3Oct * 12.0f + P.o3Semi + P.o3Fine * 0.01f + drift * driftV[2];
    const float f1 = C.hz (n1), f2 = C.hz (n2), f3 = C.hz (n3);
    float inc1[kMaxUni], inc2[kMaxUni], gL[kMaxUni], gR[kMaxUni];
    const bool stereo = uni > 1 && P.uniSpread > 0.01f;
    for (int k = 0; k < uni; ++k)
    {
        const float pos = uni > 1 ? (float) k / (float) (uni - 1) * 2.0f - 1.0f : 0.0f;
        const float c = centsToRatio (pos * detune * 0.5f);
        inc1[k] = std::min (f1 * c / sr, 0.45f);
        inc2[k] = std::min (f2 * c / sr, 0.45f);
        const float upan = pos * P.uniSpread * ((k & 1) ? 1.0f : -1.0f) * (k == 0 && (uni & 1) ? 0.0f : 1.0f);
        panGains (upan, gL[k], gR[k]);
    }
    const float uniNorm = 1.0f / std::sqrt ((float) uni);
    const float inc3 = std::min (f3 / sr, 0.45f);
    const float incSub = std::min (f1 * 0.5f / sr, 0.45f);

    const float pw1 = clampf (P.o1PW + l1 * P.l1Pw + l2 * P.l2Pw, 0.05f, 0.95f);
    const float pw2 = clampf (P.o2PW + l1 * P.l1Pw + l2 * P.l2Pw, 0.05f, 0.95f);
    const float pw3 = clampf (P.o3PW, 0.05f, 0.95f);

    // filter cutoff (octaves)
    const float cutOct = std::log2 (std::max (P.cutoff, 10.0f))
                       + P.keyTrack * (curNote - 60.0f) / 12.0f
                       + P.fVel * (vel - 0.5f)
                       + l1 * P.l1Cut + l2 * P.l2Cut
                       + C.modWheel * P.mwCut + C.aftertouch * P.atCut;
    const int ft = (int) P.fType;
    const float res = P.res;
    const float drive = 1.0f + 3.0f * P.fDrive;
    hpL.setCutoff (P.hpCut, sr); hpR.setCutoff (P.hpCut, sr);
    if (ft == 6) { const float shift = std::pow (2.0f, clampf (cutOct - 10.55f, -2.0f, 2.0f) * 0.35f);
                   fmtL.set (P.vowel, shift, res, sr); fmtR.set (P.vowel, shift, res, sr); }

    const float l1Amp = 1.0f - P.l1Amp * (0.5f + 0.5f * l1);
    float panPos = pan + l2 * P.l2Pan;
    float vpl, vpr;
    panGains (panPos, vpl, vpr);
    const float lvl1 = P.o1Level, lvl2 = P.o2Level, lvl3 = P.o3Level;
    const float fm = P.fm * 0.6f;
    const bool sync = P.sync > 0.5f;
    const float out = velGain * 0.32f;

    for (int i = 0; i < n; ++i)
    {
        // oscillators
        float sl = 0.0f, sr_ = 0.0f;
        float o3 = 0.0f;
        if (lvl3 > 0.0f || fm > 0.0f) o3 = osc (w3 == 5 ? 0 : w3, ph3, inc3, pw3);
        float first1 = 0.0f, first2 = 0.0f;
        for (int k = 0; k < uni; ++k)
        {
            float a = 0.0f, b = 0.0f;
            if (lvl1 > 0.0f || P.ring > 0.0f)
            {
                const float before = ph1[k];
                if (fm > 0.0f) { float pm = ph1[k] + fm * o3; pm -= std::floor (pm); float tmp = pm; a = osc (w1 == 5 ? 0 : w1, tmp, inc1[k], pw1); ph1[k] += inc1[k]; if (ph1[k] >= 1.0f) ph1[k] -= 1.0f; }
                else a = osc (w1 == 5 ? 0 : w1, ph1[k], inc1[k], pw1);
                if (sync && ph1[k] < before) ph2[k] = ph1[k] * inc2[k] / std::max (inc1[k], 1.0e-6f);
            }
            if (lvl2 > 0.0f || P.ring > 0.0f) b = osc (w2 == 5 ? 0 : w2, ph2[k], inc2[k], pw2);
            if (k == 0) { first1 = a; first2 = b; }
            const float m = a * lvl1 + b * lvl2;
            sl += m * gL[k]; sr_ += m * gR[k];
        }
        sl *= uniNorm * 1.41f; sr_ *= uniNorm * 1.41f;
        float mono = o3 * lvl3;
        if (P.subLevel > 0.0f) { mono += (phSub < 0.5f ? 1.0f : -1.0f) * P.subLevel * 0.8f; phSub += incSub; if (phSub >= 1.0f) phSub -= 1.0f; }
        if (P.noiseLevel > 0.0f) mono += rng.bi() * P.noiseLevel * 0.7f;
        if (P.ring > 0.0f) mono += first1 * first2 * P.ring;
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
                yl = ft == 1 ? lp : ft == 2 ? bp : ft == 3 ? hp : (lp + hp);
                if (stereo)
                {
                    svfR.set (fc, res, sr);
                    svfR.process (fastTanh (sr_ * drive * 0.7f) / (drive * 0.7f), lp, bp, hp);
                    yr = ft == 1 ? lp : ft == 2 ? bp : ft == 3 ? hp : (lp + hp);
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
