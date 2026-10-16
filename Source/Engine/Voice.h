// Keyverse - voice / instrument-engine base classes.
#pragma once

#include "DSP.h"
#include "Params.h"

namespace kv
{
// Per-layer performance state, refreshed every control chunk.
struct VoiceCtx
{
    float sr = 48000.0f;
    float a4 = 440.0f;
    float transpose = 0.0f;   // semitones incl. fine tune
    float bend = 0.0f;        // semitones
    float vibrato = 0.0f;     // semitones (mod wheel / aftertouch vibrato, already scaled)
    float modWheel = 0.0f, aftertouch = 0.0f, expression = 1.0f;
    float pedal = 0.0f;       // sustain pedal 0..1 (continuous, for half pedalling)
    float softPedal = 0.0f;
    float tempo = 120.0f;

    float hz (float note) const { return midiToHz (note + transpose + bend + vibrato, a4); }
};

class EngineBase;

class Voice
{
public:
    virtual ~Voice() = default;

    int note = -1;          // MIDI key that triggered the voice
    float vel = 0.0f;       // 0..1 after velocity curve
    bool active = false;    // producing sound
    bool held = false;      // key is down
    bool sustained = false; // key up but held by the pedal
    bool releasing = false;
    bool dying = false;     // being faded out (stolen)
    uint32_t stamp = 0;
    float curNote = 60.0f, targetNote = 60.0f;  // for glide (semitones)
    float pan = 0.0f;       // extra pan from voice spreading

    virtual void start (int key, float velocity) = 0;
    virtual void release() = 0;
    virtual void kill (float fadeSeconds) = 0;
    virtual void render (float* L, float* R, int n) = 0;   // adds into L/R
    virtual void legatoTo (float newNote) { targetNote = newNote; }

protected:
    // Shared fade helper for kill()
    float fadeGain = 1.0f, fadeStep = 0.0f;
    void beginFade (float seconds, float sr)
    {
        dying = true;
        fadeStep = 1.0f / std::max (1.0f, seconds * sr);
        if (fadeGain > 1.0f) fadeGain = 1.0f;
    }
    // returns false once fade is complete
    bool applyFade (float& l, float& r)
    {
        if (! dying) return true;
        fadeGain -= fadeStep;
        if (fadeGain <= 0.0f) { fadeGain = 0.0f; l = r = 0.0f; return false; }
        l *= fadeGain; r *= fadeGain;
        return true;
    }
    void resetFade() { dying = false; fadeGain = 1.0f; fadeStep = 0.0f; }
};

class EngineBase
{
public:
    virtual ~EngineBase() = default;

    virtual void prepare (float sampleRate, int maxBlock) = 0;
    virtual int voiceCount() const = 0;
    virtual Voice& voiceAt (int i) = 0;
    const Voice& voiceAt (int i) const { return const_cast<EngineBase*> (this)->voiceAt (i); }
    virtual void processBus (float* /*L*/, float* /*R*/, int /*n*/) {}
    virtual void resetBus() {}
    // Called before a voice is started (e.g. organ percussion single trigger)
    virtual void onNoteOn (int /*heldNotes*/) {}
    // true when re-pressing a sounding key should fade the old voice (acoustic instruments)
    virtual bool stealSameNote() const { return true; }
    virtual float defaultStealFade() const { return 0.06f; }

    void beginBlock (const LayerParams& p, const VoiceCtx& c) { params = &p; ctx = &c; }
    void resetAll()
    {
        for (int i = 0; i < voiceCount(); ++i)
        {
            auto& v = voiceAt (i);
            v.active = v.held = v.sustained = v.releasing = v.dying = false;
            v.note = -1;
        }
        resetBus();
    }

    const LayerParams* params = nullptr;
    const VoiceCtx* ctx = nullptr;
    float sr = 48000.0f;
    int maxBlock = 512;
};

// Convenience: pool of a concrete voice type.
template <class V>
class PolyEngine : public EngineBase
{
public:
    explicit PolyEngine (int numVoices) : voices ((size_t) numVoices) {}
    int voiceCount() const override { return (int) voices.size(); }
    Voice& voiceAt (int i) override { return voices[(size_t) i]; }

protected:
    void prepareVoices()
    {
        for (auto& v : voices) v.setup (this);
    }
    std::vector<V> voices;
};

//==============================================================================
// Small 8-line feedback delay network used for soundboard/body bloom and
// sympathetic resonance (not the main reverb).
class Fdn8
{
public:
    void prepare (float sampleRate, float maxDelayMs)
    {
        sr = sampleRate;
        for (auto& d : dl) d.init ((int) (maxDelayMs * 0.001f * sr * 2.1f) + 8);
        reset();
    }
    void reset()
    {
        for (auto& d : dl) d.clear();
        for (auto& l : lp) l.reset();
    }
    void set (float meanDelayMs, float rt60, float dampHz)
    {
        static const float ratios[8] = { 0.59f, 0.71f, 0.83f, 0.93f, 1.07f, 1.19f, 1.31f, 1.43f };
        for (int i = 0; i < 8; ++i)
        {
            const float len = std::max (4.0f, meanDelayMs * 0.001f * sr * ratios[i]);
            len_[i] = std::min ((int) len | 1, dl[i].capacity() - 2);
            g[i] = std::pow (10.0f, -3.0f * (float) len_[i] / (std::max (rt60, 0.01f) * sr));
            lp[i].setCutoff (dampHz, sr);
        }
    }
    void process (float inL, float inR, float& outL, float& outR)
    {
        float x[8];
        for (int i = 0; i < 8; ++i)
            x[i] = lp[i].process (dl[i].tap (len_[i] - 1)) * g[i];
        outL = (x[0] - x[2] + x[4] - x[6]) * 0.5f;
        outR = (x[1] - x[3] + x[5] - x[7]) * 0.5f;
        // fast Walsh-Hadamard (orthogonal mixing)
        for (int h = 1; h < 8; h <<= 1)
            for (int i = 0; i < 8; i += h << 1)
                for (int j = i; j < i + h; ++j)
                {
                    const float a = x[j], b = x[j + h];
                    x[j] = a + b; x[j + h] = a - b;
                }
        const float s = 0.35355339f;
        for (int i = 0; i < 8; ++i)
            dl[i].push (x[i] * s + ((i & 1) ? inR : inL) * ((i & 2) ? -0.5f : 0.5f));
    }

private:
    float sr = 48000.0f;
    DelayLine dl[8];
    OnePoleLP lp[8];
    int len_[8] {};
    float g[8] {};
};

} // namespace kv
