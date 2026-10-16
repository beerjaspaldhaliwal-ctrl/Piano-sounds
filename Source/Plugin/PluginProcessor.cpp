// Keyverse - JUCE audio processor implementation.
#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
// Visits every parameter of a sound with a stable, unique id.
template <class F>
void visitSound (kv::SoundData& s, F&& f)
{
    for (int li = 0; li < 2; ++li)
    {
        auto& L = s.layer[li];
        const juce::String pre = "L" + juce::String (li) + ".";
        auto wrap = [&] (const char* group)
        {
            return [&, group] (const kv::PD& d, float& v) { f (pre + group + "." + d.id, d, v); };
        };
        L.common.visit (wrap ("common"));
        L.piano.visit (wrap ("piano"));
        L.ep.visit (wrap ("ep"));
        L.pluck.visit (wrap ("pluck"));
        L.mallet.visit (wrap ("mallet"));
        L.organ.visit (wrap ("organ"));
        L.analog.visit (wrap ("analog"));
        L.fm.visit (wrap ("fm"));
        L.fx.visit (wrap ("fx"));
    }
    s.gfx.visit ([&] (const kv::PD& d, float& v) { f (juce::String ("G.") + d.id, d, v); });
}
} // namespace

//==============================================================================
juce::String KeyverseProcessor::macroId (int i)
{
    static const char* ids[] = { "brightness", "timbre", "time", "movement", "modfx", "delay", "reverb" };
    return ids[juce::jlimit (0, numMacros - 1, i)];
}

juce::String KeyverseProcessor::macroName (int i)
{
    static const char* names[] = { "Brightness", "Timbre", "Time", "Movement", "Mod FX", "Delay", "Reverb" };
    return names[juce::jlimit (0, numMacros - 1, i)];
}

juce::AudioProcessorValueTreeState::ParameterLayout KeyverseProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (int i = 0; i < numMacros; ++i)
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { macroId (i), 1 }, macroName (i),
                                                                 juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "volume", 1 }, "Volume",
                                                             juce::NormalisableRange<float> (-30.0f, 6.0f, 0.1f), 0.0f,
                                                             juce::AudioParameterFloatAttributes().withLabel ("dB")));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "tune", 1 }, "Master Tune",
                                                             juce::NormalisableRange<float> (415.0f, 466.0f, 0.1f), 440.0f,
                                                             juce::AudioParameterFloatAttributes().withLabel ("Hz")));
    return layout;
}

KeyverseProcessor::KeyverseProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    for (int i = 0; i < numMacros; ++i) macroParams[i] = apvts.getRawParameterValue (macroId (i));
    volumeParam = apvts.getRawParameterValue ("volume");
    tuneParam = apvts.getRawParameterValue ("tune");
    loadFavourites();
    buildBrowser();
    if (! browser.empty())
    {
        currentEntry = 0;
        editSound = kv::factoryPresets()[0].data;
        currentName = browser[0].name;
        currentCategory = browser[0].category;
        pushSound (editSound, true);
    }
}

bool KeyverseProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void KeyverseProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.prepare (sampleRate, juce::jmax (64, samplesPerBlock));
    {
        const juce::SpinLock::ScopedLockType lock (pendingLock);
        synth.setSound (editSound, true);
        pendingFlag = false;
    }
    uiMidi.reset (sampleRate);
    keyboardState.reset();
    prepared = true;
}

void KeyverseProcessor::pushSound (const kv::SoundData& s, bool structural)
{
    const juce::SpinLock::ScopedLockType lock (pendingLock);
    pendingSound = s;
    if (structural) pendingStructural = true;
    pendingFlag = true;
}

void KeyverseProcessor::soundEdited (bool structural)
{
    modified = true;
    pushSound (editSound, structural);
}

void KeyverseProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();

    if (pendingFlag.load (std::memory_order_acquire))
    {
        const juce::SpinLock::ScopedTryLockType lock (pendingLock);
        if (lock.isLocked())
        {
            synth.setSound (pendingSound, pendingStructural.exchange (false));
            pendingFlag = false;
        }
    }

    kv::Macros m;
    m.brightness = macroParams[0]->load();
    m.timbre = macroParams[1]->load();
    m.time = macroParams[2]->load();
    m.movement = macroParams[3]->load();
    m.modFx = macroParams[4]->load();
    m.delay = macroParams[5]->load();
    m.reverb = macroParams[6]->load();
    synth.setMacros (m);
    synth.setTuning (tuneParam->load());

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                synth.setTempo ((float) *bpm);

    keyboardState.processNextMidiBuffer (midi, 0, n, true);
    uiMidi.removeNextBlockOfMessages (midi, n);

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    juce::HeapBlock<float> monoR;
    if (R == nullptr) { monoR.allocate ((size_t) n, true); R = monoR.get(); }

    int pos = 0;
    for (const auto meta : midi)
    {
        const int t = juce::jlimit (0, n, meta.samplePosition);
        if (t > pos) { synth.render (L + pos, R + pos, t - pos); pos = t; }
        synth.handleMidi (meta.data, meta.numBytes);
    }
    if (pos < n) synth.render (L + pos, R + pos, n - pos);

    const float g = juce::Decibels::decibelsToGain (volumeParam->load(), -100.0f);
    if (buffer.getNumChannels() > 1)
    {
        buffer.applyGain (0, 0, n, g);
        buffer.applyGain (1, 0, n, g);
        for (int ch = 2; ch < buffer.getNumChannels(); ++ch) buffer.clear (ch, 0, n);
    }
    else
    {
        for (int i = 0; i < n; ++i) L[i] = 0.5f * (L[i] + R[i]) * g;
    }
    voicesInUse = synth.activeVoices();
}

juce::AudioProcessorEditor* KeyverseProcessor::createEditor() { return new KeyverseEditor (*this); }

//==============================================================================
// Browser / presets
juce::File KeyverseProcessor::userPresetFolder() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Keyverse").getChildFile ("User Presets");
}

juce::File KeyverseProcessor::settingsFile() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Keyverse").getChildFile ("settings.xml");
}

void KeyverseProcessor::buildBrowser()
{
    browser.clear();
    const auto& fp = kv::factoryPresets();
    for (int i = 0; i < (int) fp.size(); ++i)
    {
        Entry e;
        e.name = fp[(size_t) i].name;
        e.category = fp[(size_t) i].category;
        e.subcategory = fp[(size_t) i].subcategory;
        e.tags = fp[(size_t) i].tags;
        e.factoryIndex = i;
        browser.push_back (e);
    }
    const auto folder = userPresetFolder();
    if (folder.isDirectory())
    {
        auto files = folder.findChildFiles (juce::File::findFiles, false, "*.kvpreset");
        files.sort();
        for (const auto& f : files)
        {
            if (auto xml = juce::XmlDocument::parse (f))
            {
                Entry e;
                e.name = xml->getStringAttribute ("name", f.getFileNameWithoutExtension());
                e.category = "User";
                e.subcategory = xml->getStringAttribute ("subcategory", "User");
                e.tags = xml->getStringAttribute ("tags");
                e.file = f;
                browser.push_back (e);
            }
        }
    }
}

void KeyverseProcessor::rescanUserPresets()
{
    const juce::String key = juce::isPositiveAndBelow (currentEntry, (int) browser.size()) ? browser[(size_t) currentEntry].key() : juce::String();
    buildBrowser();
    for (int i = 0; i < (int) browser.size(); ++i)
        if (browser[(size_t) i].key() == key) currentEntry = i;
}

void KeyverseProcessor::loadEntry (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) browser.size())) return;
    const auto& e = browser[(size_t) index];
    if (e.isUser())
    {
        auto xml = juce::XmlDocument::parse (e.file);
        if (xml == nullptr) return;
        kv::SoundData s;
        if (auto* snd = xml->getChildByName ("Sound"))
            treeToSound (juce::ValueTree::fromXml (*snd), s);
        editSound = s;
    }
    else
    {
        editSound = kv::factoryPresets()[(size_t) e.factoryIndex].data;
    }
    currentEntry = index;
    currentName = e.name;
    currentCategory = e.category;
    modified = false;
    for (int i = 0; i < numMacros; ++i)
        if (auto* p = apvts.getParameter (macroId (i)))
            p->setValueNotifyingHost (0.5f);
    pushSound (editSound, true);
    ++presetVersionCounter;
}

void KeyverseProcessor::step (int delta, const std::vector<int>& visible)
{
    if (visible.empty()) return;
    int pos = -1;
    for (int i = 0; i < (int) visible.size(); ++i) if (visible[(size_t) i] == currentEntry) pos = i;
    pos = pos < 0 ? 0 : (pos + delta + (int) visible.size()) % (int) visible.size();
    loadEntry (visible[(size_t) pos]);
}

bool KeyverseProcessor::saveUserPreset (const juce::String& name)
{
    const auto folder = userPresetFolder();
    if (! folder.createDirectory()) return false;
    const auto safe = juce::File::createLegalFileName (name.trim().isEmpty() ? juce::String ("My Preset") : name.trim());
    auto file = folder.getChildFile (safe + ".kvpreset");
    juce::XmlElement root ("KeyversePreset");
    root.setAttribute ("name", name.trim());
    root.setAttribute ("subcategory", currentCategory == "User" ? juce::String ("User") : currentCategory);
    root.setAttribute ("tags", "user");
    if (auto snd = soundToTree (editSound).createXml())
        root.addChildElement (snd.release());
    if (! root.writeTo (file)) return false;
    buildBrowser();
    for (int i = 0; i < (int) browser.size(); ++i)
        if (browser[(size_t) i].isUser() && browser[(size_t) i].file == file)
        {
            currentEntry = i;
            currentName = browser[(size_t) i].name;
            currentCategory = "User";
        }
    modified = false;
    ++presetVersionCounter;
    return true;
}

void KeyverseProcessor::toggleFavourite (const Entry& e)
{
    const auto k = e.key();
    if (favourites.count (k)) favourites.erase (k); else favourites.insert (k);
    saveFavourites();
}

void KeyverseProcessor::loadFavourites()
{
    favourites.clear();
    if (auto xml = juce::XmlDocument::parse (settingsFile()))
        for (auto* f : xml->getChildWithTagNameIterator ("Favourite"))
            favourites.insert (f->getStringAttribute ("key"));
}

void KeyverseProcessor::saveFavourites()
{
    juce::XmlElement root ("KeyverseSettings");
    for (const auto& k : favourites)
        root.createNewChildElement ("Favourite")->setAttribute ("key", k);
    settingsFile().getParentDirectory().createDirectory();
    root.writeTo (settingsFile());
}

//==============================================================================
// Host programs
int KeyverseProcessor::getNumPrograms() { return juce::jmax (1, (int) browser.size()); }

void KeyverseProcessor::setCurrentProgram (int index)
{
    if (index != currentEntry) loadEntry (index);
}

const juce::String KeyverseProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, (int) browser.size()) ? browser[(size_t) index].name : juce::String();
}

//==============================================================================
// State
juce::ValueTree KeyverseProcessor::soundToTree (kv::SoundData& s) const
{
    juce::ValueTree t ("Sound");
    t.setProperty ("numLayers", s.numLayers, nullptr);
    visitSound (s, [&] (const juce::String& id, const kv::PD&, float& v) { t.setProperty (juce::Identifier (id), v, nullptr); });
    return t;
}

void KeyverseProcessor::treeToSound (const juce::ValueTree& t, kv::SoundData& s)
{
    s.numLayers = juce::jlimit (1, 2, (int) t.getProperty ("numLayers", 1));
    visitSound (s, [&] (const juce::String& id, const kv::PD& d, float& v)
    {
        const juce::Identifier key (id);
        if (t.hasProperty (key)) v = juce::jlimit (juce::jmin (d.lo, d.hi), juce::jmax (d.lo, d.hi), (float) (double) t.getProperty (key));
    });
}

void KeyverseProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state ("KeyverseState");
    state.setProperty ("version", 1, nullptr);
    const bool valid = juce::isPositiveAndBelow (currentEntry, (int) browser.size());
    state.setProperty ("entryKey", valid ? browser[(size_t) currentEntry].key() : juce::String(), nullptr);
    state.setProperty ("name", currentName, nullptr);
    state.setProperty ("category", currentCategory, nullptr);
    state.setProperty ("modified", modified, nullptr);
    state.appendChild (soundToTree (editSound), nullptr);
    state.appendChild (apvts.copyState(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void KeyverseProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr) return;
    const auto state = juce::ValueTree::fromXml (*xml);
    if (! state.hasType ("KeyverseState")) return;

    const auto params = state.getChildWithName ("PARAMS");
    if (params.isValid()) apvts.replaceState (params);

    kv::SoundData s;
    const auto snd = state.getChildWithName ("Sound");
    if (snd.isValid()) treeToSound (snd, s);
    editSound = s;
    currentName = state.getProperty ("name").toString();
    currentCategory = state.getProperty ("category").toString();
    modified = (bool) state.getProperty ("modified", false);
    const auto key = state.getProperty ("entryKey").toString();
    for (int i = 0; i < (int) browser.size(); ++i)
        if (browser[(size_t) i].key() == key) currentEntry = i;
    pushSound (editSound, true);
    ++presetVersionCounter;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new KeyverseProcessor(); }
