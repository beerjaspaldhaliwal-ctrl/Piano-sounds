// Keyverse - parameter model.
// Every sound is a plain-old-data struct of floats. Each struct exposes
// visit() so the UI, state serialisation and preset tools can discover the
// parameters without any extra registration code.
#pragma once

#include <cstdint>

namespace kv
{
enum class PK : uint8_t { Lin, Log, Int, Choice, Toggle };

struct PD
{
    const char* id;
    const char* label;
    float lo, hi;
    PK kind;
    const char* choices;   // '|' separated, for PK::Choice
    const char* unit;
};

inline PD lin (const char* id, const char* label, float lo, float hi, const char* unit = "")   { return { id, label, lo, hi, PK::Lin, nullptr, unit }; }
inline PD logp (const char* id, const char* label, float lo, float hi, const char* unit = "")  { return { id, label, lo, hi, PK::Log, nullptr, unit }; }
inline PD intp (const char* id, const char* label, float lo, float hi, const char* unit = "")  { return { id, label, lo, hi, PK::Int, nullptr, unit }; }
inline PD tog (const char* id, const char* label)                                            { return { id, label, 0.0f, 1.0f, PK::Toggle, nullptr, "" }; }
inline PD cho (const char* id, const char* label, float numChoices, const char* choices)     { return { id, label, 0.0f, numChoices - 1.0f, PK::Choice, choices, "" }; }

enum class Engine : int { None = 0, Piano, EPiano, Pluck, Mallet, Organ, Analog, FM, Count };
inline const char* engineChoices() { return "None|Acoustic Piano|Electric Piano|Plucked String|Mallet & Bell|Organ|Analog Synth|FM Synth"; }

//==============================================================================
struct CommonParams
{
    float engine = 0, level = 0, pan = 0, transpose = 0, fine = 0;
    float keyLo = 0, keyHi = 127, velLo = 1, velHi = 127, velCurve = 0;
    float poly = 32, legato = 0, glide = 0, strum = 0, bendRange = 2;
    float mwVib = 0.3f, vibRate = 5.5f, revSend = 1, dlySend = 1;
    float arpMode = 0, arpRate = 3, arpOct = 1, arpGate = 0.6f, arpSwing = 0;

    template <class V> void visit (V&& v)
    {
        v (cho ("engine", "Engine", 8, engineChoices()), engine);
        v (lin ("level", "Level", -40, 12, "dB"), level);
        v (lin ("pan", "Pan", -1, 1), pan);
        v (intp ("transpose", "Transpose", -36, 36, "st"), transpose);
        v (lin ("fine", "Fine Tune", -100, 100, "ct"), fine);
        v (intp ("keyLo", "Key Low", 0, 127), keyLo);
        v (intp ("keyHi", "Key High", 0, 127), keyHi);
        v (intp ("velLo", "Velocity Low", 1, 127), velLo);
        v (intp ("velHi", "Velocity High", 1, 127), velHi);
        v (lin ("velCurve", "Velocity Curve", -1, 1), velCurve);
        v (intp ("poly", "Polyphony", 1, 64), poly);
        v (tog ("legato", "Legato"), legato);
        v (logp ("glide", "Glide", 0, 2, "s"), glide);
        v (lin ("strum", "Strum", 0, 120, "ms"), strum);
        v (intp ("bendRange", "Bend Range", 0, 24, "st"), bendRange);
        v (lin ("mwVib", "ModWheel Vibrato", 0, 1, "st"), mwVib);
        v (lin ("vibRate", "Vibrato Rate", 2, 9, "Hz"), vibRate);
        v (lin ("revSend", "Reverb Send", 0, 1), revSend);
        v (lin ("dlySend", "Delay Send", 0, 1), dlySend);
        v (cho ("arpMode", "Arpeggiator", 7, "Off|Up|Down|Up-Down|Random|As Played|Chord"), arpMode);
        v (cho ("arpRate", "Arp Rate", 8, "1/4|1/8|1/8T|1/16|1/16T|1/32|1/4T|1/2"), arpRate);
        v (intp ("arpOct", "Arp Octaves", 1, 4), arpOct);
        v (lin ("arpGate", "Arp Gate", 0.05f, 1), arpGate);
        v (lin ("arpSwing", "Arp Swing", 0, 0.5f), arpSwing);
    }
};

//==============================================================================
struct InsertFxParams
{
    float compAmt = 0;
    float wahMode = 0, wahFreq = 900, wahQ = 4, wahDepth = 0.6f, wahRate = 1.5f;
    float ampType = 0, ampGain = 0.3f, ampTone = 0, ampLevel = 0, ampCab = 0;
    float eqLow = 0, eqMid = 0, eqMidF = 1000, eqHigh = 0, lowCut = 20, highCut = 20000;
    float m1Type = 0, m1Rate = 0.8f, m1Depth = 0.5f, m1Mix = 0.5f, m1Fb = 0;
    float m2Type = 0, m2Rate = 0.8f, m2Depth = 0.5f, m2Mix = 0.5f, m2Fb = 0;

    static const char* modChoices() { return "Off|Chorus|Ensemble|Phaser|Flanger|Tremolo|Auto-Pan|Rotary|Vibrato|Dimension"; }

    template <class V> void visit (V&& v)
    {
        v (lin ("compAmt", "Compressor", 0, 1), compAmt);
        v (cho ("wahMode", "Wah", 4, "Off|Auto (Envelope)|LFO|Mod Wheel"), wahMode);
        v (logp ("wahFreq", "Wah Freq", 250, 3500, "Hz"), wahFreq);
        v (lin ("wahQ", "Wah Q", 1, 14), wahQ);
        v (lin ("wahDepth", "Wah Depth", 0, 1), wahDepth);
        v (logp ("wahRate", "Wah Rate", 0.05f, 10, "Hz"), wahRate);
        v (cho ("ampType", "Drive / Amp", 10, "Off|Tube Warmth|Overdrive|Fuzz|Clean Amp|Crunch Amp|Lead Amp|Bass Amp|Organ Amp|Tape"), ampType);
        v (lin ("ampGain", "Drive", 0, 1), ampGain);
        v (lin ("ampTone", "Amp Tone", -1, 1), ampTone);
        v (lin ("ampLevel", "Amp Level", -24, 12, "dB"), ampLevel);
        v (cho ("ampCab", "Cabinet", 6, "Off|1x12 Open|2x12 Combo|4x12 Closed|Bass 1x15|EP Speaker"), ampCab);
        v (lin ("eqLow", "EQ Low", -15, 15, "dB"), eqLow);
        v (lin ("eqMid", "EQ Mid", -15, 15, "dB"), eqMid);
        v (logp ("eqMidF", "EQ Mid Freq", 150, 6000, "Hz"), eqMidF);
        v (lin ("eqHigh", "EQ High", -15, 15, "dB"), eqHigh);
        v (logp ("lowCut", "Low Cut", 20, 1000, "Hz"), lowCut);
        v (logp ("highCut", "High Cut", 800, 20000, "Hz"), highCut);
        v (cho ("m1Type", "Mod FX 1", 10, modChoices()), m1Type);
        v (logp ("m1Rate", "Mod 1 Rate", 0.03f, 12, "Hz"), m1Rate);
        v (lin ("m1Depth", "Mod 1 Depth", 0, 1), m1Depth);
        v (lin ("m1Mix", "Mod 1 Mix", 0, 1), m1Mix);
        v (lin ("m1Fb", "Mod 1 Feedback", 0, 0.95f), m1Fb);
        v (cho ("m2Type", "Mod FX 2", 10, modChoices()), m2Type);
        v (logp ("m2Rate", "Mod 2 Rate", 0.03f, 12, "Hz"), m2Rate);
        v (lin ("m2Depth", "Mod 2 Depth", 0, 1), m2Depth);
        v (lin ("m2Mix", "Mod 2 Mix", 0, 1), m2Mix);
        v (lin ("m2Fb", "Mod 2 Feedback", 0, 0.95f), m2Fb);
    }
};

//==============================================================================
struct PianoParams
{
    float hardness = 0.5f, hammerPos = 0.12f, unison = 0.6f, decay = 1.0f, release = 0.25f;
    float hfDamp = 0.5f, inharm = 1.0f, stretch = 1.0f, aftersound = 0.35f;
    float hammerNoise = 0.35f, releaseNoise = 0.3f, mechNoise = 0.1f;
    float body = 0.35f, bodySize = 0.7f, bodyTone = 0.0f, pedalRes = 0.35f, width = 0.6f;
    float velSens = 0.6f, tack = 0.0f, piezo = 0.0f, prep = 0.0f, felt = 0.0f, maxPartials = 96;

    template <class V> void visit (V&& v)
    {
        v (lin ("hardness", "Hammer Hardness", 0, 1), hardness);
        v (lin ("hammerPos", "Strike Position", 0.04f, 0.3f), hammerPos);
        v (lin ("unison", "Unison Detune", 0, 25, "ct"), unison);
        v (logp ("decay", "Sustain Length", 0.1f, 4), decay);
        v (logp ("release", "Damper Time", 0.03f, 3, "s"), release);
        v (lin ("hfDamp", "String Damping", 0, 2), hfDamp);
        v (logp ("inharm", "Inharmonicity", 0.0f, 8), inharm);
        v (lin ("stretch", "Stretch Tuning", 0, 2), stretch);
        v (lin ("aftersound", "Aftersound", 0, 1), aftersound);
        v (lin ("hammerNoise", "Hammer Noise", 0, 1), hammerNoise);
        v (lin ("releaseNoise", "Release Noise", 0, 1), releaseNoise);
        v (lin ("mechNoise", "Mechanics Noise", 0, 1), mechNoise);
        v (lin ("body", "Soundboard", 0, 1), body);
        v (lin ("bodySize", "Instrument Size", 0, 1), bodySize);
        v (lin ("bodyTone", "Body Tone", -1, 1), bodyTone);
        v (lin ("pedalRes", "Pedal Resonance", 0, 1), pedalRes);
        v (lin ("width", "Stereo Width", 0, 1), width);
        v (lin ("velSens", "Dynamics", 0, 1), velSens);
        v (lin ("tack", "Tacks", 0, 1), tack);
        v (lin ("piezo", "Piezo Pickup", 0, 1), piezo);
        v (lin ("prep", "Preparation", 0, 1), prep);
        v (lin ("felt", "Felt Mute", 0, 1), felt);
        v (intp ("maxPartials", "Partials", 8, 96), maxPartials);
    }
};

//==============================================================================
struct EPianoParams
{
    float pickup = 0, excite = 0, hardness = 0.5f, bell = 0.4f, bellRatio = 6.27f, bellDecay = 0.25f;
    float mode3 = 0.1f, toneBar = 0.4f, beat = 0.7f, offset = 0.45f, drive = 0.5f, distance = 0.6f;
    float decay = 1.0f, release = 0.15f, thump = 0.3f, relNoise = 0.25f, velSens = 0.6f, width = 0.3f;
    float bark = 0.5f;

    template <class V> void visit (V&& v)
    {
        v (cho ("pickup", "Pickup", 2, "Electromagnetic (Tine)|Electrostatic (Reed)"), pickup);
        v (cho ("excite", "Excitation", 2, "Hammer|Pluck (Pianet)"), excite);
        v (lin ("hardness", "Hammer Tip", 0, 1), hardness);
        v (lin ("bell", "Bell / Tine", 0, 1), bell);
        v (lin ("bellRatio", "Bell Ratio", 2, 12), bellRatio);
        v (logp ("bellDecay", "Bell Decay", 0.01f, 2, "s"), bellDecay);
        v (lin ("mode3", "Upper Mode", 0, 1), mode3);
        v (lin ("toneBar", "Tone Bar", 0, 1), toneBar);
        v (lin ("beat", "Tone Bar Beat", 0, 4, "Hz"), beat);
        v (lin ("offset", "Pickup Voicing", 0, 1.5f), offset);
        v (lin ("drive", "Pickup Drive", 0, 1), drive);
        v (lin ("distance", "Pickup Distance", 0.2f, 1.5f), distance);
        v (logp ("decay", "Sustain Length", 0.1f, 4), decay);
        v (logp ("release", "Damper Time", 0.02f, 2, "s"), release);
        v (lin ("thump", "Hammer Thump", 0, 1), thump);
        v (lin ("relNoise", "Key Release Noise", 0, 1), relNoise);
        v (lin ("velSens", "Dynamics", 0, 1), velSens);
        v (lin ("width", "Key Spread", 0, 1), width);
        v (lin ("bark", "Bark (velocity)", 0, 1), bark);
    }
};

//==============================================================================
struct PluckParams
{
    float excite = 0, bright = 0.6f, pluckPos = 0.15f, decay = 1.0f, damping = 0.4f, stiffness = 0.0f;
    float pickupPos = 0.0f, pickup2Pos = 0.0f, pickup2Mix = 0.0f, body = 0, bodyMix = 0.5f;
    float course = 1, courseDetune = 3, courseOct = 0, courseDelay = 4;
    float buzz = 0, twang = 6, release = 0.12f, relNoise = 0.2f, mute = 0, repeat = 0;
    float pickNoise = 0.3f, velBright = 0.5f, width = 0.4f, velSens = 0.6f, sustainHold = 0;

    template <class V> void visit (V&& v)
    {
        v (cho ("excite", "Excitation", 5, "Pick|Finger|Slap / Pop|Strike|Quill"), excite);
        v (lin ("bright", "Brightness", 0, 1), bright);
        v (lin ("pluckPos", "Pluck Position", 0.02f, 0.5f), pluckPos);
        v (logp ("decay", "Sustain Length", 0.05f, 6), decay);
        v (lin ("damping", "String Damping", 0, 1), damping);
        v (lin ("stiffness", "Stiffness", 0, 1), stiffness);
        v (lin ("pickupPos", "Pickup Position", 0, 0.5f), pickupPos);
        v (lin ("pickup2Pos", "Pickup 2 Position", 0, 0.5f), pickup2Pos);
        v (lin ("pickup2Mix", "Pickup 2 Mix", -1, 1), pickup2Mix);
        v (cho ("body", "Body", 12, "None|Nylon Guitar|Steel Guitar|Archtop|Banjo|Harp|Sitar|Harpsichord|Small Body|Double Bass|Koto|Oud"), body);
        v (lin ("bodyMix", "Body Amount", 0, 1), bodyMix);
        v (intp ("course", "Strings per Note", 1, 3), course);
        v (lin ("courseDetune", "Course Detune", 0, 30, "ct"), courseDetune);
        v (cho ("courseOct", "Course Tuning", 3, "Unison|Octave|Octave + Unison"), courseOct);
        v (lin ("courseDelay", "Course Delay", 0, 30, "ms"), courseDelay);
        v (lin ("buzz", "Bridge Buzz", 0, 1), buzz);
        v (lin ("twang", "Pitch Twang", 0, 40, "ct"), twang);
        v (logp ("release", "Release Damping", 0.02f, 4, "s"), release);
        v (lin ("relNoise", "Release Noise", 0, 1), relNoise);
        v (lin ("mute", "Palm Mute", 0, 1), mute);
        v (lin ("repeat", "Tremolo Picking", 0, 16, "Hz"), repeat);
        v (lin ("pickNoise", "Pick Noise", 0, 1), pickNoise);
        v (lin ("velBright", "Velocity > Brightness", 0, 1), velBright);
        v (lin ("width", "Key Spread", 0, 1), width);
        v (lin ("velSens", "Dynamics", 0, 1), velSens);
        v (tog ("sustainHold", "Let Ring"), sustainHold);
    }
};

//==============================================================================
struct MalletParams
{
    float excite = 0, hardness = 0.5f, noise = 0.2f, noiseFreq = 2500, decay = 2.0f, decayTrack = 0.6f;
    float release = 0.2f, damped = 0;
    float r[6] = { 1.0f, 2.756f, 5.404f, 8.933f, 13.34f, 18.64f };
    float g[6] = { 0.0f, -10.0f, -18.0f, -26.0f, -60.0f, -60.0f };
    float d[6] = { 1.0f, 0.5f, 0.3f, 0.2f, 0.15f, 0.1f };
    float beat = 0.0f, beatMix = 0.0f, resonator = 0.0f, spread = 0.0f, velSens = 0.6f, width = 0.5f;
    float bowAttack = 0.3f, buzz = 0.0f, pitchDrop = 0.0f, velBright = 0.6f;

    template <class V> void visit (V&& v)
    {
        v (cho ("excite", "Excitation", 3, "Mallet|Pluck|Bow"), excite);
        v (lin ("hardness", "Mallet Hardness", 0, 1), hardness);
        v (lin ("noise", "Impact Noise", 0, 1), noise);
        v (logp ("noiseFreq", "Impact Tone", 200, 10000, "Hz"), noiseFreq);
        v (logp ("decay", "Decay @C4", 0.05f, 30, "s"), decay);
        v (lin ("decayTrack", "Decay Key Track", 0, 1.5f), decayTrack);
        v (logp ("release", "Damper Time", 0.02f, 10, "s"), release);
        v (tog ("damped", "Dampers"), damped);
        static const char* rid[] = { "r1", "r2", "r3", "r4", "r5", "r6" };
        static const char* gid[] = { "g1", "g2", "g3", "g4", "g5", "g6" };
        static const char* did[] = { "d1", "d2", "d3", "d4", "d5", "d6" };
        static const char* rl[] = { "Mode 1 Ratio", "Mode 2 Ratio", "Mode 3 Ratio", "Mode 4 Ratio", "Mode 5 Ratio", "Mode 6 Ratio" };
        static const char* gl[] = { "Mode 1 Level", "Mode 2 Level", "Mode 3 Level", "Mode 4 Level", "Mode 5 Level", "Mode 6 Level" };
        static const char* dl[] = { "Mode 1 Decay", "Mode 2 Decay", "Mode 3 Decay", "Mode 4 Decay", "Mode 5 Decay", "Mode 6 Decay" };
        for (int i = 0; i < 6; ++i)
        {
            v (lin (rid[i], rl[i], 0.1f, 30), r[i]);
            v (lin (gid[i], gl[i], -60, 6, "dB"), g[i]);
            v (logp (did[i], dl[i], 0.01f, 3), d[i]);
        }
        v (lin ("beat", "Doublet Beat", 0, 6, "Hz"), beat);
        v (lin ("beatMix", "Doublet Mix", 0, 1), beatMix);
        v (lin ("resonator", "Resonator Tube", 0, 1), resonator);
        v (lin ("spread", "Inharmonic Spread", -0.5f, 0.5f), spread);
        v (lin ("velSens", "Dynamics", 0, 1), velSens);
        v (lin ("width", "Key Spread", 0, 1), width);
        v (logp ("bowAttack", "Bow Attack", 0.01f, 4, "s"), bowAttack);
        v (lin ("buzz", "Rattle", 0, 1), buzz);
        v (lin ("pitchDrop", "Strike Pitch Drop", -100, 100, "ct"), pitchDrop);
        v (lin ("velBright", "Velocity > Hardness", 0, 1), velBright);
    }
};

//==============================================================================
struct OrganParams
{
    float model = 0;
    float db[9] = { 8, 8, 8, 0, 0, 0, 0, 0, 0 };
    float percOn = 0, percHarm = 1, percFast = 1, percSoft = 0;
    float click = 0.4f, leakage = 0.15f, attack = 0.004f, release = 0.01f, chiff = 0, wind = 0;
    float vibrato = 0, musette = 0, musetteVoices = 1, tone = 12000, drive = 0;

    template <class V> void visit (V&& v)
    {
        v (cho ("model", "Organ Model", 8, "Tonewheel|Transistor Combo|Vox Combo|Pipe Principal|Pipe Flute|Pipe Reed|Reed Organ|Accordion"), model);
        static const char* ids[] = { "db1", "db2", "db3", "db4", "db5", "db6", "db7", "db8", "db9" };
        static const char* lab[] = { "16'", "5 1/3'", "8'", "4'", "2 2/3'", "2'", "1 3/5'", "1 1/3'", "1'" };
        for (int i = 0; i < 9; ++i) v (lin (ids[i], lab[i], 0, 8), db[i]);
        v (tog ("percOn", "Percussion"), percOn);
        v (cho ("percHarm", "Perc Harmonic", 2, "Second|Third"), percHarm);
        v (tog ("percFast", "Perc Fast"), percFast);
        v (tog ("percSoft", "Perc Soft"), percSoft);
        v (lin ("click", "Key Click", 0, 1), click);
        v (lin ("leakage", "Leakage", 0, 1), leakage);
        v (logp ("attack", "Attack", 0.001f, 2, "s"), attack);
        v (logp ("release", "Release", 0.005f, 3, "s"), release);
        v (lin ("chiff", "Pipe Chiff", 0, 1), chiff);
        v (lin ("wind", "Wind Noise", 0, 1), wind);
        v (cho ("vibrato", "Vibrato / Chorus", 7, "Off|V1|V2|V3|C1|C2|C3"), vibrato);
        v (lin ("musette", "Reed Detune", 0, 40, "ct"), musette);
        v (intp ("musetteVoices", "Reeds", 1, 3), musetteVoices);
        v (logp ("tone", "Tone", 800, 20000, "Hz"), tone);
        v (lin ("drive", "Preamp Drive", 0, 1), drive);
    }
};

//==============================================================================
struct AnalogParams
{
    float o1Wave = 0, o1Oct = 0, o1Semi = 0, o1Fine = 0, o1Level = 1, o1PW = 0.5f;
    float o2Wave = 0, o2Oct = 0, o2Semi = 0, o2Fine = 7, o2Level = 0, o2PW = 0.5f;
    float o3Wave = 0, o3Oct = -1, o3Semi = 0, o3Fine = 0, o3Level = 0, o3PW = 0.5f;
    float sync = 0, ring = 0, fm = 0, subLevel = 0, noiseLevel = 0;
    float unison = 1, uniDetune = 15, uniSpread = 0.6f, drift = 0.3f;
    float fType = 0, cutoff = 4000, res = 0.2f, fDrive = 0.2f, keyTrack = 0.5f, fEnv = 2, fVel = 1, hpCut = 20, vowel = 0;
    float aA = 0.005f, aD = 0.5f, aS = 0.8f, aR = 0.4f;
    float fA = 0.005f, fD = 0.6f, fS = 0.3f, fR = 0.5f;
    float pEnv = 0, pDecay = 0.1f, velAmp = 0.5f;
    float l1Rate = 5, l1Shape = 0, l1Pitch = 0, l1Cut = 0, l1Amp = 0, l1Pw = 0, l1Delay = 0;
    float l2Rate = 0.3f, l2Shape = 0, l2Cut = 0, l2Pan = 0, l2Pw = 0;
    float mwCut = 0, atCut = 0, atVib = 0;

    static const char* waves() { return "Saw|Square / Pulse|Triangle|Sine|Noise|SuperSaw"; }

    template <class V> void visit (V&& v)
    {
        v (cho ("o1Wave", "Osc 1 Wave", 6, waves()), o1Wave);
        v (intp ("o1Oct", "Osc 1 Octave", -3, 3), o1Oct);
        v (intp ("o1Semi", "Osc 1 Semi", -12, 12), o1Semi);
        v (lin ("o1Fine", "Osc 1 Fine", -100, 100, "ct"), o1Fine);
        v (lin ("o1Level", "Osc 1 Level", 0, 1), o1Level);
        v (lin ("o1PW", "Osc 1 Pulse Width", 0.05f, 0.95f), o1PW);
        v (cho ("o2Wave", "Osc 2 Wave", 6, waves()), o2Wave);
        v (intp ("o2Oct", "Osc 2 Octave", -3, 3), o2Oct);
        v (intp ("o2Semi", "Osc 2 Semi", -12, 12), o2Semi);
        v (lin ("o2Fine", "Osc 2 Fine", -100, 100, "ct"), o2Fine);
        v (lin ("o2Level", "Osc 2 Level", 0, 1), o2Level);
        v (lin ("o2PW", "Osc 2 Pulse Width", 0.05f, 0.95f), o2PW);
        v (cho ("o3Wave", "Osc 3 Wave", 6, waves()), o3Wave);
        v (intp ("o3Oct", "Osc 3 Octave", -3, 3), o3Oct);
        v (intp ("o3Semi", "Osc 3 Semi", -12, 12), o3Semi);
        v (lin ("o3Fine", "Osc 3 Fine", -100, 100, "ct"), o3Fine);
        v (lin ("o3Level", "Osc 3 Level", 0, 1), o3Level);
        v (lin ("o3PW", "Osc 3 Pulse Width", 0.05f, 0.95f), o3PW);
        v (tog ("sync", "Osc 2 Hard Sync"), sync);
        v (lin ("ring", "Ring Mod", 0, 1), ring);
        v (lin ("fm", "Osc 3 > 1 FM", 0, 1), fm);
        v (lin ("subLevel", "Sub Osc", 0, 1), subLevel);
        v (lin ("noiseLevel", "Noise", 0, 1), noiseLevel);
        v (intp ("unison", "Unison Voices", 1, 8), unison);
        v (lin ("uniDetune", "Unison Detune", 0, 60, "ct"), uniDetune);
        v (lin ("uniSpread", "Unison Spread", 0, 1), uniSpread);
        v (lin ("drift", "Analog Drift", 0, 1), drift);
        v (cho ("fType", "Filter Type", 7, "Ladder LP 24|SVF LP 12|SVF Band Pass|SVF High Pass|SVF Notch|LP 24 + HP|Formant"), fType);
        v (logp ("cutoff", "Cutoff", 20, 20000, "Hz"), cutoff);
        v (lin ("res", "Resonance", 0, 1), res);
        v (lin ("fDrive", "Filter Drive", 0, 1), fDrive);
        v (lin ("keyTrack", "Key Track", 0, 1.2f), keyTrack);
        v (lin ("fEnv", "Filter Env", -8, 8, "oct"), fEnv);
        v (lin ("fVel", "Velocity > Cutoff", 0, 4, "oct"), fVel);
        v (logp ("hpCut", "High Pass", 20, 2000, "Hz"), hpCut);
        v (lin ("vowel", "Vowel", 0, 4), vowel);
        v (logp ("aA", "Amp Attack", 0.0005f, 10, "s"), aA);
        v (logp ("aD", "Amp Decay", 0.005f, 20, "s"), aD);
        v (lin ("aS", "Amp Sustain", 0, 1), aS);
        v (logp ("aR", "Amp Release", 0.005f, 20, "s"), aR);
        v (logp ("fA", "Filter Attack", 0.0005f, 10, "s"), fA);
        v (logp ("fD", "Filter Decay", 0.005f, 20, "s"), fD);
        v (lin ("fS", "Filter Sustain", 0, 1), fS);
        v (logp ("fR", "Filter Release", 0.005f, 20, "s"), fR);
        v (lin ("pEnv", "Pitch Env", -24, 24, "st"), pEnv);
        v (logp ("pDecay", "Pitch Env Decay", 0.002f, 4, "s"), pDecay);
        v (lin ("velAmp", "Velocity > Amp", 0, 1), velAmp);
        v (logp ("l1Rate", "LFO 1 Rate", 0.02f, 30, "Hz"), l1Rate);
        v (cho ("l1Shape", "LFO 1 Shape", 7, "Sine|Triangle|Saw Up|Saw Down|Square|Sample & Hold|Smooth Random"), l1Shape);
        v (lin ("l1Pitch", "LFO 1 > Pitch", 0, 12, "st"), l1Pitch);
        v (lin ("l1Cut", "LFO 1 > Cutoff", 0, 4, "oct"), l1Cut);
        v (lin ("l1Amp", "LFO 1 > Amp", 0, 1), l1Amp);
        v (lin ("l1Pw", "LFO 1 > PW", 0, 0.45f), l1Pw);
        v (logp ("l1Delay", "LFO 1 Fade In", 0, 5, "s"), l1Delay);
        v (logp ("l2Rate", "LFO 2 Rate", 0.02f, 30, "Hz"), l2Rate);
        v (cho ("l2Shape", "LFO 2 Shape", 7, "Sine|Triangle|Saw Up|Saw Down|Square|Sample & Hold|Smooth Random"), l2Shape);
        v (lin ("l2Cut", "LFO 2 > Cutoff", 0, 4, "oct"), l2Cut);
        v (lin ("l2Pan", "LFO 2 > Pan", 0, 1), l2Pan);
        v (lin ("l2Pw", "LFO 2 > PW", 0, 0.45f), l2Pw);
        v (lin ("mwCut", "ModWheel > Cutoff", -4, 4, "oct"), mwCut);
        v (lin ("atCut", "Aftertouch > Cutoff", 0, 4, "oct"), atCut);
        v (lin ("atVib", "Aftertouch > Vibrato", 0, 1, "st"), atVib);
    }
};

//==============================================================================
struct FMOp
{
    float ratio = 1, detune = 0, fixed = 0, fixedHz = 440, level = 0;
    float A = 0.002f, D = 1.0f, S = 0.5f, R = 0.4f, velSens = 0.5f, keyScale = 0;
};

struct FMParams
{
    float alg = 0, feedback = 0;
    FMOp op[6];
    float lfoRate = 5, lfoPitch = 0, lfoAmp = 0, lfoDelay = 0.3f, rateScale = 0.3f;

    static const char* algChoices()
    {
        return "6-Op Stack|Two 3-Op Stacks|Three Pairs (E.Piano)|4-Stack + Pair|Pair + 4-Stack (FB 2)|One Mod > 3 Carriers + Pair"
               "|Three Branches > 1|Six Carriers (Organ)|Two Mods > 2 Carriers|3-Stack + Pair + Carrier|Pair + 4 Carriers|Y-Branch Stack";
    }

    template <class V> void visit (V&& v)
    {
        v (cho ("alg", "Algorithm", 12, algChoices()), alg);
        v (lin ("feedback", "Feedback", 0, 1), feedback);
        static const char* idR[] = { "op1Ratio", "op2Ratio", "op3Ratio", "op4Ratio", "op5Ratio", "op6Ratio" };
        static const char* idT[] = { "op1Det", "op2Det", "op3Det", "op4Det", "op5Det", "op6Det" };
        static const char* idX[] = { "op1Fixed", "op2Fixed", "op3Fixed", "op4Fixed", "op5Fixed", "op6Fixed" };
        static const char* idH[] = { "op1Hz", "op2Hz", "op3Hz", "op4Hz", "op5Hz", "op6Hz" };
        static const char* idL[] = { "op1Level", "op2Level", "op3Level", "op4Level", "op5Level", "op6Level" };
        static const char* idA[] = { "op1A", "op2A", "op3A", "op4A", "op5A", "op6A" };
        static const char* idD[] = { "op1D", "op2D", "op3D", "op4D", "op5D", "op6D" };
        static const char* idS[] = { "op1S", "op2S", "op3S", "op4S", "op5S", "op6S" };
        static const char* idRl[] = { "op1R", "op2R", "op3R", "op4R", "op5R", "op6R" };
        static const char* idV[] = { "op1Vel", "op2Vel", "op3Vel", "op4Vel", "op5Vel", "op6Vel" };
        static const char* idK[] = { "op1Key", "op2Key", "op3Key", "op4Key", "op5Key", "op6Key" };
        static const char* lR[] = { "Op1 Ratio", "Op2 Ratio", "Op3 Ratio", "Op4 Ratio", "Op5 Ratio", "Op6 Ratio" };
        static const char* lT[] = { "Op1 Detune", "Op2 Detune", "Op3 Detune", "Op4 Detune", "Op5 Detune", "Op6 Detune" };
        static const char* lX[] = { "Op1 Fixed", "Op2 Fixed", "Op3 Fixed", "Op4 Fixed", "Op5 Fixed", "Op6 Fixed" };
        static const char* lH[] = { "Op1 Fixed Hz", "Op2 Fixed Hz", "Op3 Fixed Hz", "Op4 Fixed Hz", "Op5 Fixed Hz", "Op6 Fixed Hz" };
        static const char* lL[] = { "Op1 Level", "Op2 Level", "Op3 Level", "Op4 Level", "Op5 Level", "Op6 Level" };
        static const char* lA[] = { "Op1 Attack", "Op2 Attack", "Op3 Attack", "Op4 Attack", "Op5 Attack", "Op6 Attack" };
        static const char* lD[] = { "Op1 Decay", "Op2 Decay", "Op3 Decay", "Op4 Decay", "Op5 Decay", "Op6 Decay" };
        static const char* lS[] = { "Op1 Sustain", "Op2 Sustain", "Op3 Sustain", "Op4 Sustain", "Op5 Sustain", "Op6 Sustain" };
        static const char* lRl[] = { "Op1 Release", "Op2 Release", "Op3 Release", "Op4 Release", "Op5 Release", "Op6 Release" };
        static const char* lV[] = { "Op1 Vel Sens", "Op2 Vel Sens", "Op3 Vel Sens", "Op4 Vel Sens", "Op5 Vel Sens", "Op6 Vel Sens" };
        static const char* lK[] = { "Op1 Key Scale", "Op2 Key Scale", "Op3 Key Scale", "Op4 Key Scale", "Op5 Key Scale", "Op6 Key Scale" };
        for (int i = 0; i < 6; ++i)
        {
            auto& o = op[i];
            v (lin (idR[i], lR[i], 0.125f, 32), o.ratio);
            v (lin (idT[i], lT[i], -50, 50, "ct"), o.detune);
            v (tog (idX[i], lX[i]), o.fixed);
            v (logp (idH[i], lH[i], 1, 8000, "Hz"), o.fixedHz);
            v (lin (idL[i], lL[i], 0, 1), o.level);
            v (logp (idA[i], lA[i], 0.0005f, 10, "s"), o.A);
            v (logp (idD[i], lD[i], 0.005f, 30, "s"), o.D);
            v (lin (idS[i], lS[i], 0, 1), o.S);
            v (logp (idRl[i], lRl[i], 0.005f, 20, "s"), o.R);
            v (lin (idV[i], lV[i], 0, 1), o.velSens);
            v (lin (idK[i], lK[i], -1, 1), o.keyScale);
        }
        v (logp ("lfoRate", "LFO Rate", 0.05f, 20, "Hz"), lfoRate);
        v (lin ("lfoPitch", "LFO > Pitch", 0, 1, "st"), lfoPitch);
        v (lin ("lfoAmp", "LFO > Amp", 0, 1), lfoAmp);
        v (logp ("lfoDelay", "LFO Delay", 0, 5, "s"), lfoDelay);
        v (lin ("rateScale", "Env Rate Scaling", 0, 1), rateScale);
    }
};

//==============================================================================
struct LayerParams
{
    CommonParams common;
    PianoParams piano;
    EPianoParams ep;
    PluckParams pluck;
    MalletParams mallet;
    OrganParams organ;
    AnalogParams analog;
    FMParams fm;
    InsertFxParams fx;

    Engine engine() const { return (Engine) (int) common.engine; }

    // Visits common + fx + only the active engine's parameters, with prefixes.
    template <class V> void visitActive (V&& v)
    {
        common.visit (v);
        switch (engine())
        {
            case Engine::Piano:  piano.visit (v); break;
            case Engine::EPiano: ep.visit (v); break;
            case Engine::Pluck:  pluck.visit (v); break;
            case Engine::Mallet: mallet.visit (v); break;
            case Engine::Organ:  organ.visit (v); break;
            case Engine::Analog: analog.visit (v); break;
            case Engine::FM:     fm.visit (v); break;
            default: break;
        }
        fx.visit (v);
    }
};

struct GlobalFxParams
{
    float dlyMix = 0, dlyTime = 375, dlySync = 0, dlyFb = 0.35f, dlyTone = 5000, dlyPing = 1, dlyMod = 0.1f;
    float revMix = 0.2f, revSize = 0.6f, revDecay = 1.8f, revDamp = 0.4f, revPre = 15, revWidth = 1, revLowCut = 120, revMod = 0.3f;
    float lofiBits = 24, lofiRate = 48, lofiNoise = 0, lofiWow = 0, lofiTone = 20000;
    float width = 1, master = 0;

    template <class V> void visit (V&& v)
    {
        v (lin ("dlyMix", "Delay Mix", 0, 1), dlyMix);
        v (logp ("dlyTime", "Delay Time", 10, 2000, "ms"), dlyTime);
        v (cho ("dlySync", "Delay Sync", 12, "Off|1/4|1/8|1/8 Dotted|1/8 Triplet|1/16|1/16 Dotted|1/16 Triplet|1/2|1/4 Dotted|1/4 Triplet|1/32"), dlySync);
        v (lin ("dlyFb", "Delay Feedback", 0, 0.95f), dlyFb);
        v (logp ("dlyTone", "Delay Tone", 500, 20000, "Hz"), dlyTone);
        v (tog ("dlyPing", "Ping-Pong"), dlyPing);
        v (lin ("dlyMod", "Delay Wobble", 0, 1), dlyMod);
        v (lin ("revMix", "Reverb Mix", 0, 1), revMix);
        v (lin ("revSize", "Reverb Size", 0, 1), revSize);
        v (logp ("revDecay", "Reverb Decay", 0.2f, 30, "s"), revDecay);
        v (lin ("revDamp", "Reverb Damping", 0, 1), revDamp);
        v (lin ("revPre", "Pre-Delay", 0, 250, "ms"), revPre);
        v (lin ("revWidth", "Reverb Width", 0, 1), revWidth);
        v (logp ("revLowCut", "Reverb Low Cut", 20, 1000, "Hz"), revLowCut);
        v (lin ("revMod", "Reverb Modulation", 0, 1), revMod);
        v (intp ("lofiBits", "Bit Depth", 4, 24), lofiBits);
        v (logp ("lofiRate", "Sample Rate", 2, 48, "kHz"), lofiRate);
        v (lin ("lofiNoise", "Vinyl / Hiss", 0, 1), lofiNoise);
        v (lin ("lofiWow", "Wow & Flutter", 0, 1), lofiWow);
        v (logp ("lofiTone", "Lo-Fi Tone", 800, 20000, "Hz"), lofiTone);
        v (lin ("width", "Stereo Width", 0, 2), width);
        v (lin ("master", "Master Level", -24, 12, "dB"), master);
    }
};

// The complete sound (no strings - safe to copy on the audio thread).
struct SoundData
{
    int numLayers = 1;
    LayerParams layer[2];
    GlobalFxParams gfx;
};

// Performance macros (Analog-Lab style). 0.5 = as designed in the preset.
struct Macros
{
    float brightness = 0.5f, timbre = 0.5f, time = 0.5f, movement = 0.5f;
    float modFx = 0.5f, delay = 0.5f, reverb = 0.5f;
};

} // namespace kv
