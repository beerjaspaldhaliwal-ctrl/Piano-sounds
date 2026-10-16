// Keyverse - plug-in editor (Analog Lab / Keyscape style browser + macros).
#pragma once

#include "PluginProcessor.h"

//==============================================================================
namespace kvui
{
struct Colours
{
    static inline const juce::Colour bg       { 0xff121419 };
    static inline const juce::Colour panel    { 0xff1b1e25 };
    static inline const juce::Colour panelHi  { 0xff252933 };
    static inline const juce::Colour outline  { 0xff2e333f };
    static inline const juce::Colour accent   { 0xffff8a3d };
    static inline const juce::Colour accent2  { 0xff4fc3c8 };
    static inline const juce::Colour text     { 0xffe9e9ec };
    static inline const juce::Colour dim      { 0xff8b909c };
};

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float min, float max,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (juce::FontOptions (13.0f)); }
    juce::Font getTextButtonFont (juce::TextButton&, int h) override { return juce::Font (juce::FontOptions (juce::jmin (14.0f, (float) h * 0.55f))); }
};

// One generic parameter row for the Edit page.
class ParamRow : public juce::Component
{
public:
    ParamRow (const kv::PD& d, float* target, std::function<void (bool)> changed);
    void resized() override;
    void paint (juce::Graphics&) override;

private:
    kv::PD desc;
    float* value;
    std::function<void (bool)> onChange;
    juce::Label label;
    std::unique_ptr<juce::Slider> slider;
    std::unique_ptr<juce::ComboBox> combo;
    std::unique_ptr<juce::ToggleButton> toggle;
};

class SectionHeader : public juce::Component
{
public:
    explicit SectionHeader (juce::String t) : title (std::move (t)) {}
    void paint (juce::Graphics& g) override;

private:
    juce::String title;
};
} // namespace kvui

//==============================================================================
class KeyverseEditor : public juce::AudioProcessorEditor,
                       private juce::Timer
{
public:
    explicit KeyverseEditor (KeyverseProcessor&);
    ~KeyverseEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //--------------------------------------------------------------------------
    class CategoryModel : public juce::ListBoxModel
    {
    public:
        explicit CategoryModel (KeyverseEditor& e) : ed (e) {}
        int getNumRows() override { return (int) ed.categoryNames.size(); }
        void paintListBoxItem (int row, juce::Graphics&, int w, int h, bool selected) override;
        void selectedRowsChanged (int row) override { ed.categoryChanged (row); }

    private:
        KeyverseEditor& ed;
    };

    class PresetModel : public juce::ListBoxModel
    {
    public:
        explicit PresetModel (KeyverseEditor& e) : ed (e) {}
        int getNumRows() override { return (int) ed.visible.size(); }
        void paintListBoxItem (int row, juce::Graphics&, int w, int h, bool selected) override;
        void listBoxItemClicked (int row, const juce::MouseEvent&) override;
        void returnKeyPressed (int row) override;

    private:
        KeyverseEditor& ed;
    };

    void timerCallback() override;
    void categoryChanged (int row);
    void refilter();
    void refreshHeader();
    void rebuildEditPanel();
    void showPage (bool edit);
    void selectCurrentInList();
    void savePresetDialog();
    void sendCC (int cc, int value);

    KeyverseProcessor& proc;
    kvui::LookAndFeel lnf;

    // header
    juce::Label logo, presetName, presetInfo, voicesLabel;
    juce::TextButton prevButton { "<" }, nextButton { ">" }, saveButton { "Save" }, favButton;
    juce::TextButton performTab { "PERFORM" }, editTab { "EDIT" };

    // browser
    std::vector<juce::String> categoryNames;
    std::vector<int> categoryCounts;
    juce::String currentCategory = "All";
    std::vector<int> visible;
    CategoryModel categoryModel { *this };
    PresetModel presetModel { *this };
    juce::ListBox categoryList { "categories", &categoryModel };
    juce::ListBox presetList { "presets", &presetModel };
    juce::TextEditor search;
    juce::Label countLabel;

    // perform page
    juce::Component performPage;
    juce::Slider macroKnobs[KeyverseProcessor::numMacros];
    juce::Label macroLabels[KeyverseProcessor::numMacros];
    juce::Slider volumeKnob;
    juce::Label volumeLabel, soundInfo;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;

    // edit page
    juce::Viewport editViewport;
    juce::Component editContent;
    juce::TextButton layerTabs[3];
    int editLayer = 0;
    std::vector<std::unique_ptr<juce::Component>> editRows;
    bool editVisible = false;

    // performance strip
    juce::MidiKeyboardComponent keyboard;
    juce::Slider modWheel, pitchWheel;
    juce::TextButton sustainButton { "Sustain" };

    int lastVersion = -1;
    juce::String lastEngineSignature;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyverseEditor)
};
