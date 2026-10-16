// Keyverse - layers (voice allocation, mono/legato/glide, strum, arpeggiator,
// key/velocity splits, insert FX) and the top-level Synth (two layers, global
// FX, performance macros, MIDI handling).
#pragma once

#include "Analog.h"
#include "EPiano.h"
#include "Effects.h"
#include "FM.h"
#include "Mallet.h"
#include "Organ.h"
#include "Piano.h"
#include "Pluck.h"
#include <memory>

namespace kv
{
//==============================================================================
// Analog-Lab style macro mapping: 0.5 = sound as designed.
inline void applyLayerMacros (LayerParams& p, const Macros& m)
{
    const float b = m.brightness, t = m.timbre, tm = m.time, mv = m.movement;
    switch (p.engine())
    {
        case Engine::Piano:
        {
            auto& q = p.piano;
            q.hardness = macroMap (q.hardness, b, 0.0f, 1.0f);
            q.bodyTone = macroMap (q.bodyTone, b, -1.0f, 1.0f);
            q.unison = macroMap (q.unison, t, 0.0f, std::max (8.0f, q.unison * 3.0f));
            q.inharm = macroScale (q.inharm, t, 2.5f);
            q.decay = macroScale (q.decay, tm, 3.0f);
            q.release = macroScale (q.release, tm, 5.0f);
            q.pedalRes = macroMap (q.pedalRes, mv, 0.0f, 1.0f);
            break;
        }
        case Engine::EPiano:
        {
            auto& q = p.ep;
            q.hardness = macroMap (q.hardness, b, 0.0f, 1.0f);
            q.bell = macroMap (q.bell, b, 0.0f, 1.0f);
            q.drive = macroMap (q.drive, t, 0.0f, 1.0f);
            q.bark = macroMap (q.bark, t, 0.0f, 1.0f);
            q.decay = macroScale (q.decay, tm, 3.0f);
            q.release = macroScale (q.release, tm, 5.0f);
            break;
        }
        case Engine::Pluck:
        {
            auto& q = p.pluck;
            q.bright = macroMap (q.bright, b, 0.0f, 1.0f);
            q.damping = macroMap (q.damping, b, 1.0f, 0.0f);
            if (q.pickupPos > 0.001f) q.pickupPos = macroMap (q.pickupPos, t, 0.04f, 0.4f);
            else q.pluckPos = macroMap (q.pluckPos, t, 0.03f, 0.45f);
            q.decay = macroScale (q.decay, tm, 4.0f);
            q.release = macroScale (q.release, tm, 8.0f);
            break;
        }
        case Engine::Mallet:
        {
            auto& q = p.mallet;
            q.hardness = macroMap (q.hardness, b, 0.0f, 1.0f);
            q.spread = macroMap (q.spread, t, -0.3f, 0.3f);
            q.decay = macroScale (q.decay, tm, 4.0f);
            q.release = macroScale (q.release, tm, 6.0f);
            q.bowAttack = macroScale (q.bowAttack, tm, 4.0f);
            break;
        }
        case Engine::Organ:
        {
            auto& q = p.organ;
            for (int i = 4; i < 9; ++i) q.db[i] = macroMap (q.db[i], b, 0.0f, 8.0f);
            q.tone = clampf (macroScale (q.tone, b, 3.0f), 800.0f, 20000.0f);
            q.drive = macroMap (q.drive, t, 0.0f, 1.0f);
            q.click = macroMap (q.click, t, 0.0f, 1.0f);
            q.attack = macroScale (q.attack, tm, 8.0f);
            q.release = macroScale (q.release, tm, 8.0f);
            break;
        }
        case Engine::Analog:
        {
            auto& q = p.analog;
            q.cutoff = clampf (macroScale (q.cutoff, b, 12.0f), 20.0f, 20000.0f);
            q.res = macroMap (q.res, t, 0.0f, 0.95f);
            q.uniDetune = macroMap (q.uniDetune, t, 0.0f, std::max (40.0f, q.uniDetune));
            for (float* x : { &q.aA, &q.aD, &q.aR, &q.fA, &q.fD, &q.fR }) *x = macroScale (*x, tm, 5.0f);
            q.l1Cut = macroMap (q.l1Cut, mv, 0.0f, std::max (1.5f, q.l1Cut * 2.0f));
            q.l2Cut = macroMap (q.l2Cut, mv, 0.0f, std::max (1.5f, q.l2Cut * 2.0f));
            break;
        }
        case Engine::FM:
        {
            auto& q = p.fm;
            const auto& a = fmAlg ((int) q.alg);
            for (int i = 0; i < 6; ++i)
                if (! (a.carriers & (1u << i)) && q.op[i].level > 0.0f)
                    q.op[i].level = clampf (q.op[i].level + (b - 0.5f) * 0.4f, 0.0f, 1.0f);
            q.feedback = macroMap (q.feedback, t, 0.0f, 1.0f);
            for (auto& o : q.op) { o.D = macroScale (o.D, tm, 4.0f); o.R = macroScale (o.R, tm, 4.0f); }
            q.lfoAmp = macroMap (q.lfoAmp, mv, 0.0f, 0.6f);
            break;
        }
        case Engine::None:
        case Engine::Count:
            break;
    }

    auto& fx = p.fx;
    auto modSlot = [&] (float& type, float& rate, float& depth, float& mix)
    {
        const int ty = (int) type;
        if (ty == ModFx::Rotary)
        {
            if (mv > 0.75f) rate = 6.7f; else if (mv < 0.25f) rate = 0.8f;
            mix = macroMap (mix, m.modFx, 0.0f, 1.0f);
            return;
        }
        rate = macroScale (rate, mv, 3.0f);
        if (ty == ModFx::Tremolo || ty == ModFx::AutoPan)
            depth = macroMap (depth, std::min (mv, m.modFx * 2.0f), 0.0f, 1.0f) * std::min (1.0f, m.modFx * 2.0f);
        else
        {
            depth = macroMap (depth, mv, 0.0f, 1.0f);
            mix = macroMap (mix, m.modFx, 0.0f, 1.0f);
        }
    };
    modSlot (fx.m1Type, fx.m1Rate, fx.m1Depth, fx.m1Mix);
    modSlot (fx.m2Type, fx.m2Rate, fx.m2Depth, fx.m2Mix);
}

inline void applyGlobalMacros (GlobalFxParams& g, const Macros& m)
{
    g.dlyMix = macroMap (g.dlyMix, m.delay, 0.0f, std::max (0.5f, g.dlyMix));
    g.revMix = macroMap (g.revMix, m.reverb, 0.0f, std::max (0.6f, g.revMix));
    g.revDecay *= 1.0f + std::max (0.0f, m.reverb - 0.5f) * 2.0f;
}

//==============================================================================
class Layer
{
public:
    static constexpr int kChunk = 32;

    Layer()
    {
        engines[(int) Engine::Piano]  = std::make_unique<PianoEngine>();
        engines[(int) Engine::EPiano] = std::make_unique<EPianoEngine>();
        engines[(int) Engine::Pluck]  = std::make_unique<PluckEngine>();
        engines[(int) Engine::Mallet] = std::make_unique<MalletEngine>();
        engines[(int) Engine::Organ]  = std::make_unique<OrganEngine>();
        engines[(int) Engine::Analog] = std::make_unique<AnalogEngine>();
        engines[(int) Engine::FM]     = std::make_unique<FMEngine>();
    }

    void prepare (float sampleRate, int maxBlock)
    {
        sr = sampleRate;
        ctx.sr = sr;
        for (auto& e : engines) if (e) { e->beginBlock (eff, ctx); e->prepare (sr, maxBlock); }
        fx.prepare (sr);
        bufL.assign ((size_t) std::max (maxBlock, kChunk), 0.0f);
        bufR.assign ((size_t) std::max (maxBlock, kChunk), 0.0f);
        reset();
    }

    void reset()
    {
        for (auto& e : engines) if (e) e->resetAll();
        fx.reset();
        nPending = 0; monoCount = 0; chordCount = 0; arpCount = 0; arpPlaying = 0;
        std::fill (std::begin (keyDown), std::end (keyDown), false);
        lastNote = -1;
    }

    void setParams (const LayerParams& p, bool structural)
    {
        base = p;
        updateEff();
        if (structural) reset();
    }

    void setMacros (const Macros& m) { macros = m; updateEff(); }

    bool enabled() const { return base.engine() != Engine::None; }
    const LayerParams& params() const { return base; }

    //--------------------------------------------------------------------------
    void noteOn (int note, int vel127)
    {
        const auto& c = base.common;
        if (! enabled()) return;
        if (note < (int) c.keyLo || note > (int) c.keyHi || vel127 < (int) c.velLo || vel127 > (int) c.velHi) return;
        keyDown[note] = true;
        float v = (float) vel127 / 127.0f;
        v = std::pow (v, std::exp2 (-c.velCurve * 1.2f));
        if ((int) c.arpMode > 0) { arpAdd (note, v); return; }
        if (c.strum > 0.5f) { strumOn (note, v); return; }
        startNote (note, v);
    }

    void noteOff (int note)
    {
        if (note < 0 || note > 127 || ! keyDown[note]) return;
        keyDown[note] = false;
        if ((int) base.common.arpMode > 0) { if (! sustainDown) arpRemove (note); return; }
        // drop pending strummed notes that never started
        for (int i = 0; i < nPending; ++i)
            if (pending[i].note == note) { pending[i] = pending[--nPending]; return; }
        stopNote (note);
    }

    void setSustain (float v)
    {
        const bool down = v >= 0.5f;
        ctx.pedal = v;
        if (sustainDown && ! down)
        {
            if (auto* e = current())
                for (int i = 0; i < e->voiceCount(); ++i)
                {
                    auto& vc = e->voiceAt (i);
                    if (vc.active && vc.sustained && ! vc.held) { vc.sustained = false; vc.release(); }
                }
            if ((int) base.common.arpMode > 0)
                for (int i = arpCount - 1; i >= 0; --i) if (! keyDown[arpNotes[i]]) arpRemove (arpNotes[i]);
        }
        sustainDown = down;
    }
    void setSoftPedal (float v) { ctx.softPedal = v; }
    void setModWheel (float v) { ctx.modWheel = v; }
    void setAftertouch (float v) { ctx.aftertouch = v; }
    void setExpression (float v) { ctx.expression = v; }
    void setBend (float norm) { bendNorm = norm; }

    void allNotesOff (bool hard)
    {
        nPending = 0; arpCount = 0; monoCount = 0;
        if (arpPlaying > 0) arpStopPlaying();
        std::fill (std::begin (keyDown), std::end (keyDown), false);
        if (auto* e = current())
            for (int i = 0; i < e->voiceCount(); ++i)
            {
                auto& v = e->voiceAt (i);
                if (! v.active) continue;
                v.held = v.sustained = false;
                if (hard) v.kill (0.01f); else v.release();
            }
    }

    int activeVoices() const
    {
        int n = 0;
        if (auto* e = currentConst())
            for (int i = 0; i < e->voiceCount(); ++i) n += e->voiceAt (i).active ? 1 : 0;
        return n;
    }

    //--------------------------------------------------------------------------
    void process (int n, float* outL, float* outR, float* revL, float* revR, float* dlyL, float* dlyR,
                  const Macros& m, float tempo, float a4)
    {
        if (! enabled()) return;
        macros = m;
        ctx.a4 = a4;
        ctx.tempo = tempo;
        updateEff();
        const auto& c = eff.common;
        auto* e = current();
        if (e == nullptr) return;
        e->beginBlock (eff, ctx);

        std::fill (bufL.begin(), bufL.begin() + n, 0.0f);
        std::fill (bufR.begin(), bufR.begin() + n, 0.0f);

        for (int pos = 0; pos < n; pos += kChunk)
        {
            const int len = std::min (kChunk, n - pos);
            // performance state
            ctx.bend = bendNorm * c.bendRange;
            vibPhase += c.vibRate * (float) len / sr;
            vibPhase -= std::floor (vibPhase);
            ctx.vibrato = c.mwVib * ctx.modWheel * sin01 (vibPhase);
            if (eff.engine() == Engine::Analog) static_cast<AnalogEngine*> (e)->advanceLfo2 (len);

            processPending (len);
            if ((int) c.arpMode > 0) arpTick (len);

            const float glideCoef = c.glide > 0.0005f ? 1.0f - std::exp (-(float) len / (c.glide * sr)) : 1.0f;
            for (int i = 0; i < e->voiceCount(); ++i)
            {
                auto& v = e->voiceAt (i);
                if (! v.active) continue;
                v.curNote += (v.targetNote - v.curNote) * glideCoef;
                v.render (bufL.data() + pos, bufR.data() + pos, len);
            }
            sampleClock += (uint64_t) len;
        }

        e->processBus (bufL.data(), bufR.data(), n);
        fx.process (bufL.data(), bufR.data(), n, eff.fx, ctx.modWheel);

        float gl, gr;
        panGains (c.pan, gl, gr);
        const float g = dbToGain (c.level) * ctx.expression * 1.41421356f;
        gl *= g; gr *= g;
        const float rs = c.revSend, ds = c.dlySend;
        for (int i = 0; i < n; ++i)
        {
            const float l = bufL[(size_t) i] * gl, r = bufR[(size_t) i] * gr;
            outL[i] += l; outR[i] += r;
            revL[i] += l * rs; revR[i] += r * rs;
            dlyL[i] += l * ds; dlyR[i] += r * ds;
        }
    }

private:
    void updateEff()
    {
        eff = base;
        applyLayerMacros (eff, macros);
        ctx.transpose = eff.common.transpose + eff.common.fine * 0.01f;
    }

    EngineBase* current() { const int t = (int) base.common.engine; return (t > 0 && t < (int) Engine::Count) ? engines[t].get() : nullptr; }
    const EngineBase* currentConst() const { const int t = (int) base.common.engine; return (t > 0 && t < (int) Engine::Count) ? engines[t].get() : nullptr; }

    //--------------------------------------------------------------------------
    void startNote (int note, float v)
    {
        auto* e = current();
        if (e == nullptr) return;
        const auto& c = base.common;
        e->beginBlock (eff, ctx);
        const int poly = std::clamp ((int) c.poly, 1, e->voiceCount() - 4);

        if (poly == 1) { monoStart (*e, note, v); return; }

        if (e->stealSameNote())
            for (int i = 0; i < e->voiceCount(); ++i)
            {
                auto& vc = e->voiceAt (i);
                if (vc.active && ! vc.dying && vc.note == note) { vc.held = vc.sustained = false; vc.kill (e->defaultStealFade()); }
            }

        e->onNoteOn (countHeld (*e));
        Voice* vc = allocate (*e, poly);
        vc->note = note;
        vc->held = true;
        vc->sustained = false;
        vc->stamp = ++stampCounter;
        vc->targetNote = (float) note;
        vc->curNote = (c.glide > 0.0005f && lastNote >= 0) ? (float) lastNote : (float) note;
        vc->pan = 0.0f;
        vc->active = false;
        vc->start (note, v);
        lastNote = note;
    }

    void stopNote (int note)
    {
        auto* e = current();
        if (e == nullptr) return;
        const int poly = std::clamp ((int) base.common.poly, 1, e->voiceCount() - 4);
        if (poly == 1) { monoStop (*e, note); return; }
        for (int i = 0; i < e->voiceCount(); ++i)
        {
            auto& vc = e->voiceAt (i);
            if (vc.active && vc.held && vc.note == note && ! vc.dying)
            {
                vc.held = false;
                if (sustainDown) vc.sustained = true; else vc.release();
            }
        }
    }

    int countHeld (EngineBase& e) const
    {
        int n = 0;
        for (int i = 0; i < e.voiceCount(); ++i) { auto& v = e.voiceAt (i); n += (v.active && v.held && ! v.dying) ? 1 : 0; }
        return n;
    }

    Voice* allocate (EngineBase& e, int limit)
    {
        int count = 0;
        for (int i = 0; i < e.voiceCount(); ++i) { auto& v = e.voiceAt (i); count += (v.active && ! v.dying) ? 1 : 0; }
        while (count >= limit)
        {
            Voice* victim = nullptr;
            int best = 4;
            for (int i = 0; i < e.voiceCount(); ++i)
            {
                auto& v = e.voiceAt (i);
                if (! v.active || v.dying) continue;
                const int cls = (! v.held && ! v.sustained) ? 0 : (v.sustained ? 1 : 2);
                if (cls < best || (cls == best && victim && v.stamp < victim->stamp)) { best = cls; victim = &v; }
            }
            if (! victim) break;
            victim->kill (0.008f);
            --count;
        }
        for (int i = 0; i < e.voiceCount(); ++i) if (! e.voiceAt (i).active) return &e.voiceAt (i);
        Voice* oldest = &e.voiceAt (0);
        for (int i = 1; i < e.voiceCount(); ++i) if (e.voiceAt (i).stamp < oldest->stamp) oldest = &e.voiceAt (i);
        return oldest;
    }

    // ---- mono / legato
    void monoStart (EngineBase& e, int note, float v)
    {
        for (int i = 0; i < monoCount; ++i) if (monoStack[i] == note) { for (int j = i; j < monoCount - 1; ++j) monoStack[j] = monoStack[j + 1]; --monoCount; break; }
        if (monoCount < 32) monoStack[monoCount++] = note;
        Voice* cur = nullptr;
        for (int i = 0; i < e.voiceCount(); ++i) { auto& vc = e.voiceAt (i); if (vc.active && ! vc.dying) { if (! cur || vc.stamp > cur->stamp) cur = &vc; } }
        const auto& c = base.common;
        const bool legato = c.legato > 0.5f && cur && cur->held;
        if (cur)
        {
            for (int i = 0; i < e.voiceCount(); ++i) { auto& vc = e.voiceAt (i); if (&vc != cur && vc.active && ! vc.dying) vc.kill (0.01f); }
            cur->note = note; cur->held = true; cur->sustained = false; cur->stamp = ++stampCounter;
            cur->targetNote = (float) note;
            if (c.glide <= 0.0005f) cur->curNote = (float) note;
            if (! legato) cur->start (note, v); else cur->legatoTo ((float) note);
        }
        else
        {
            e.onNoteOn (0);
            Voice* vc = allocate (e, 1);
            vc->note = note; vc->held = true; vc->sustained = false; vc->stamp = ++stampCounter;
            vc->targetNote = (float) note;
            vc->curNote = (c.glide > 0.0005f && lastNote >= 0) ? (float) lastNote : (float) note;
            vc->active = false;
            vc->start (note, v);
        }
        lastNote = note;
        lastMonoVel = v;
    }

    void monoStop (EngineBase& e, int note)
    {
        for (int i = 0; i < monoCount; ++i) if (monoStack[i] == note) { for (int j = i; j < monoCount - 1; ++j) monoStack[j] = monoStack[j + 1]; --monoCount; break; }
        for (int i = 0; i < e.voiceCount(); ++i)
        {
            auto& vc = e.voiceAt (i);
            if (! vc.active || vc.dying || vc.note != note || ! vc.held) continue;
            if (monoCount > 0)
            {
                const int prev = monoStack[monoCount - 1];
                vc.note = prev;
                vc.targetNote = (float) prev;
                if (base.common.glide <= 0.0005f) vc.curNote = (float) prev;
                if (base.common.legato > 0.5f) vc.legatoTo ((float) prev); else vc.start (prev, lastMonoVel);
                lastNote = prev;
            }
            else
            {
                vc.held = false;
                if (sustainDown) vc.sustained = true; else vc.release();
            }
        }
    }

    // ---- strum
    struct Pending { int note; float vel; int delay; };
    void strumOn (int note, float v)
    {
        const int window = (int) (0.045f * sr);
        if ((int64_t) (sampleClock - lastStrumClock) > window) chordCount = 0;
        lastStrumClock = sampleClock;
        const int step = (int) (base.common.strum * 0.001f * sr);
        int rank = 0;
        for (int i = 0; i < chordCount; ++i) if (chord[i] < note) ++rank;
        for (int i = 0; i < nPending; ++i) if (pending[i].note > note) pending[i].delay += step;
        if (chordCount < 16) chord[chordCount++] = note;
        const int delay = rank * step;
        if (delay <= 0 || nPending >= 32) { startNote (note, v); return; }
        pending[nPending++] = { note, v * (1.0f - 0.04f * (float) rank), delay };
    }
    void processPending (int len)
    {
        for (int i = 0; i < nPending;)
        {
            pending[i].delay -= len;
            if (pending[i].delay <= 0)
            {
                const Pending p = pending[i];
                pending[i] = pending[--nPending];
                startNote (p.note, p.vel);
            }
            else ++i;
        }
    }

    // ---- arpeggiator
    void arpAdd (int note, float v)
    {
        for (int i = 0; i < arpCount; ++i) if (arpNotes[i] == note) return;
        if (arpCount >= 32) return;
        arpNotes[arpCount] = note;
        arpOrder[arpCount] = note;
        ++arpCount;
        // keep arpNotes sorted by pitch, arpOrder by arrival
        std::sort (arpNotes, arpNotes + arpCount);
        arpVel = v;
        if (arpCount == 1) { arpStep = 0; arpDir = 1; arpPos = 1.0e9; }
    }
    void arpRemove (int note)
    {
        auto rem = [&] (int* a)
        {
            for (int i = 0; i < arpCount; ++i) if (a[i] == note) { for (int j = i; j < arpCount - 1; ++j) a[j] = a[j + 1]; return; }
        };
        bool found = false;
        for (int i = 0; i < arpCount; ++i) found |= arpNotes[i] == note;
        if (! found) return;
        rem (arpNotes); rem (arpOrder);
        --arpCount;
        if (arpCount == 0) arpStopPlaying();
    }
    void arpStopPlaying()
    {
        for (int i = 0; i < arpPlaying; ++i) stopNote (arpPlayingNotes[i]);
        arpPlaying = 0;
    }
    void arpTick (int len)
    {
        if (arpCount == 0) return;
        const auto& c = base.common;
        static const float beats[8] = { 1.0f, 0.5f, 1.0f / 3.0f, 0.25f, 1.0f / 6.0f, 0.125f, 2.0f / 3.0f, 2.0f };
        const double stepLen = 60.0 / std::max (ctx.tempo, 20.0f) * beats[std::clamp ((int) c.arpRate, 0, 7)] * sr;
        const double swing = (arpStep & 1) ? (1.0 - c.arpSwing) : (1.0 + c.arpSwing);
        const double thisLen = stepLen * swing;
        arpPos += len;
        if (arpPlaying > 0 && arpPos >= thisLen * c.arpGate && arpPos < 1.0e8) arpStopPlaying();
        if (arpPos < thisLen) return;
        arpPos = arpPos >= 1.0e8 ? 0.0 : arpPos - thisLen;
        arpStopPlaying();

        const int oct = std::clamp ((int) c.arpOct, 1, 4);
        const int total = arpCount * oct;
        const int mode = (int) c.arpMode;
        if (mode == 6)
        {
            for (int i = 0; i < arpCount && arpPlaying < 32; ++i)
            {
                const int nn = arpNotes[i] + 12 * (arpStep % oct);
                if (nn > 127) continue;
                startNote (nn, arpVel);
                arpPlayingNotes[arpPlaying++] = nn;
            }
            ++arpStep;
            return;
        }
        int idx = 0;
        switch (mode)
        {
            case 1: idx = arpStep % total; break;
            case 2: idx = total - 1 - (arpStep % total); break;
            case 3:
            {
                const int period = std::max (1, 2 * total - 2);
                const int ph = arpStep % period;
                idx = total == 1 ? 0 : (ph < total ? ph : period - ph);
                break;
            }
            case 4: idx = (int) (arpRng.uni() * (float) total) % total; break;
            default: idx = arpStep % total; break;
        }
        const int o = idx / arpCount, k = idx % arpCount;
        const int src = mode == 5 ? arpOrder[k] : arpNotes[k];
        const int nn = std::min (127, src + 12 * o);
        startNote (nn, arpVel * (arpStep % 4 == 0 ? 1.0f : 0.85f));
        arpPlayingNotes[arpPlaying++] = nn;
        ++arpStep;
    }

    float sr = 48000.0f;
    std::unique_ptr<EngineBase> engines[(int) Engine::Count];
    LayerParams base, eff;
    Macros macros;
    VoiceCtx ctx;
    InsertChain fx;
    std::vector<float> bufL, bufR;
    bool keyDown[128] {};
    bool sustainDown = false;
    float bendNorm = 0.0f, vibPhase = 0.0f;
    uint32_t stampCounter = 0;
    int lastNote = -1;
    uint64_t sampleClock = 0, lastStrumClock = 0;

    int monoStack[32] {}, monoCount = 0;
    float lastMonoVel = 0.8f;
    Pending pending[32] {};
    int nPending = 0;
    int chord[16] {}, chordCount = 0;

    int arpNotes[32] {}, arpOrder[32] {}, arpCount = 0;
    int arpPlayingNotes[32] {}, arpPlaying = 0;
    int arpStep = 0, arpDir = 1;
    double arpPos = 0.0;
    float arpVel = 0.8f;
    Rng arpRng;
};

//==============================================================================
class Synth
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        sr = (float) sampleRate;
        maxBlk = std::max (maxBlock, 64);
        for (auto& l : layers) l.prepare (sr, maxBlk);
        delay.prepare (sr); reverb.prepare (sr); lofi.prepare (sr); limiter.prepare (sr);
        for (auto* b : { &revL, &revR, &dlyL, &dlyR, &wetL, &wetR }) b->assign ((size_t) maxBlk, 0.0f);
        setSound (snd, true);
    }

    void setSound (const SoundData& s, bool structural)
    {
        snd = s;
        for (int i = 0; i < 2; ++i)
        {
            LayerParams lp = s.layer[i];
            if (i >= s.numLayers) lp.common.engine = 0;
            layers[i].setParams (lp, structural);
        }
        if (structural) { delay.reset(); reverb.reset(); lofi.reset(); limiter.reset(); }
    }

    const SoundData& sound() const { return snd; }
    void setMacros (const Macros& m) { macros = m; for (auto& l : layers) l.setMacros (m); }
    void setTempo (float bpm) { tempo = bpm > 0.0f ? bpm : 120.0f; }
    void setTuning (float a4Hz) { a4 = a4Hz; }

    void noteOn (int note, int vel) { if (vel <= 0) { noteOff (note); return; } for (auto& l : layers) l.noteOn (note, vel); }
    void noteOff (int note) { for (auto& l : layers) l.noteOff (note); }
    void allNotesOff (bool hard = false) { for (auto& l : layers) l.allNotesOff (hard); }

    void controller (int cc, int value)
    {
        const float v = (float) value / 127.0f;
        switch (cc)
        {
            case 1:  for (auto& l : layers) l.setModWheel (v); break;
            case 11: for (auto& l : layers) l.setExpression (v); break;
            case 64: for (auto& l : layers) l.setSustain (v); break;
            case 67: for (auto& l : layers) l.setSoftPedal (v); break;
            case 120: allNotesOff (true); break;
            case 121: for (auto& l : layers) { l.setModWheel (0); l.setExpression (1); l.setSustain (0); l.setBend (0); } break;
            case 123: allNotesOff (false); break;
            default: break;
        }
    }
    void pitchBend (int value14) { const float b = ((float) value14 - 8192.0f) / 8192.0f; for (auto& l : layers) l.setBend (clampf (b, -1.0f, 1.0f)); }
    void channelPressure (int value) { for (auto& l : layers) l.setAftertouch ((float) value / 127.0f); }

    // Raw MIDI bytes (status + data), any channel.
    void handleMidi (const uint8_t* d, int len)
    {
        if (len < 1) return;
        const uint8_t st = d[0] & 0xF0;
        if (st == 0x90 && len >= 3) noteOn (d[1], d[2]);
        else if (st == 0x80 && len >= 3) noteOff (d[1]);
        else if (st == 0xB0 && len >= 3) controller (d[1], d[2]);
        else if (st == 0xE0 && len >= 3) pitchBend (d[1] | (d[2] << 7));
        else if (st == 0xD0 && len >= 2) channelPressure (d[1]);
    }

    void render (float* L, float* R, int n)
    {
        int done = 0;
        while (done < n)
        {
            const int len = std::min (maxBlk, n - done);
            renderBlock (L + done, R + done, len);
            done += len;
        }
    }

    int activeVoices() const { return layers[0].activeVoices() + layers[1].activeVoices(); }

private:
    void renderBlock (float* L, float* R, int n)
    {
        std::fill (L, L + n, 0.0f);
        std::fill (R, R + n, 0.0f);
        for (auto* b : { &revL, &revR, &dlyL, &dlyR }) std::fill (b->begin(), b->begin() + n, 0.0f);
        for (auto& l : layers) l.process (n, L, R, revL.data(), revR.data(), dlyL.data(), dlyR.data(), macros, tempo, a4);

        GlobalFxParams g = snd.gfx;
        applyGlobalMacros (g, macros);
        // delay feeds a little into the reverb for a cohesive space
        std::fill (wetL.begin(), wetL.begin() + n, 0.0f);
        std::fill (wetR.begin(), wetR.begin() + n, 0.0f);
        delay.process (dlyL.data(), dlyR.data(), wetL.data(), wetR.data(), n, g, tempo);
        for (int i = 0; i < n; ++i)
        {
            L[i] += wetL[(size_t) i]; R[i] += wetR[(size_t) i];
            revL[(size_t) i] += wetL[(size_t) i] * 0.3f; revR[(size_t) i] += wetR[(size_t) i] * 0.3f;
        }
        reverb.process (revL.data(), revR.data(), L, R, n, g);
        lofi.process (L, R, n, g);

        const float mg = dbToGain (g.master);
        const float w = g.width;
        for (int i = 0; i < n; ++i)
        {
            const float m = 0.5f * (L[i] + R[i]), s = 0.5f * (L[i] - R[i]) * w;
            L[i] = (m + s) * mg; R[i] = (m - s) * mg;
        }
        limiter.process (L, R, n);
    }

    float sr = 48000.0f, tempo = 120.0f, a4 = 440.0f;
    int maxBlk = 512;
    SoundData snd;
    Macros macros;
    Layer layers[2];
    StereoDelay delay;
    Reverb reverb;
    LoFi lofi;
    Limiter limiter;
    std::vector<float> revL, revR, dlyL, dlyR, wetL, wetR;
};

} // namespace kv
