// Keyverse - JUCE audio processor.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Engine/Presets.h"
#include "../Engine/Synth.h"

#include <atomic>
#include <set>

class KeyverseProcessor : public juce::AudioProcessor
{
public:
    // One browser entry: either a factory preset or a user preset file.
    struct Entry
    {
        juce::String name, category, subcategory, tags;
        int factoryIndex = -1;      // >= 0 for factory presets
        juce::File file;            // user presets
        bool isUser() const { return factoryIndex < 0; }
        juce::String key() const { return isUser() ? "user:" + file.getFileName() : "factory:" + name; }
    };

    KeyverseProcessor();
    ~KeyverseProcessor() override = default;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Keyverse"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return juce::jmax (0, currentEntry); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    // Message-thread API used by the editor
    const std::vector<Entry>& entries() const { return browser; }
    int currentEntryIndex() const { return currentEntry; }
    const juce::String& currentPresetName() const { return currentName; }
    const juce::String& currentPresetCategory() const { return currentCategory; }
    void loadEntry (int index);
    void step (int delta, const std::vector<int>& visibleEntries);

    kv::SoundData& editableSound() { return editSound; }
    void soundEdited (bool structural);   // push editSound to the audio thread
    bool isModified() const { return modified; }

    bool saveUserPreset (const juce::String& name);
    void rescanUserPresets();
    juce::File userPresetFolder() const;

    bool isFavourite (const Entry& e) const { return favourites.count (e.key()) > 0; }
    void toggleFavourite (const Entry& e);

    int activeVoices() const { return voicesInUse.load(); }

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    juce::MidiMessageCollector uiMidi;

    // Incremented whenever the current preset changes (editor polls it).
    int presetVersion() const { return presetVersionCounter.load(); }

    static juce::String macroId (int i);
    static juce::String macroName (int i);
    static constexpr int numMacros = 7;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void pushSound (const kv::SoundData& s, bool structural);
    void buildBrowser();
    void loadFavourites();
    void saveFavourites();
    juce::File settingsFile() const;
    juce::ValueTree soundToTree (kv::SoundData& s) const;
    static void treeToSound (const juce::ValueTree& t, kv::SoundData& s);

    kv::Synth synth;
    kv::SoundData editSound;
    std::vector<Entry> browser;
    int currentEntry = 0;
    juce::String currentName, currentCategory;
    bool modified = false;
    std::set<juce::String> favourites;

    // audio thread hand-off
    juce::SpinLock pendingLock;
    kv::SoundData pendingSound;
    std::atomic<bool> pendingFlag { false }, pendingStructural { false };
    std::atomic<int> voicesInUse { 0 };
    std::atomic<int> presetVersionCounter { 0 };

    std::atomic<float>* macroParams[numMacros] {};
    std::atomic<float>* volumeParam = nullptr;
    std::atomic<float>* tuneParam = nullptr;
    bool prepared = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyverseProcessor)
};
