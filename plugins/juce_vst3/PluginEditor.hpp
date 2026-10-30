#pragma once

#include "PluginProcessor.hpp"
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <memory>

namespace arplugin {

// Entirely vector-rendered on the message thread; no textures, WebView,
// timers, or allocations are added to the real-time DSP callback.
class HardwareLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float,
                           float, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int,
                       juce::ComboBox&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool, bool) override;
};

class ReverbEditor final : public juce::AudioProcessorEditor {
public:
    explicit ReverbEditor (ReverbProcessor&);
    ~ReverbEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void refreshDecayReadout();
    void refreshParameterReadout (std::size_t index);
    void rebuildFaceplateCache();
    ReverbProcessor& processor;
    HardwareLookAndFeel theme;
    juce::ComboBox presetBox;
    juce::ToggleButton bypassButton;
    juce::Label decayReadout;
    juce::Label parameterReadout;
    juce::Image faceplateCache;
    int activeParameter = -1;
    std::array<juce::Slider, arverb::parameterCount> sliders;
    std::array<juce::Label, arverb::parameterCount> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,
               arverb::parameterCount> sliderAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbEditor)
};
} // namespace arplugin
