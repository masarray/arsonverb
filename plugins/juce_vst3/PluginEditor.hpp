#pragma once
#include "PluginProcessor.hpp"
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <memory>

namespace arplugin {

class ReverbEditor final : public juce::AudioProcessorEditor {
public:
    explicit ReverbEditor (ReverbProcessor&);
    ~ReverbEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ReverbProcessor& processor;
    juce::LookAndFeel_V4 theme;
    juce::Label title;
    juce::Label subtitle;
    juce::ComboBox presetBox;
    juce::ToggleButton bypassButton;
    std::array<juce::Slider, arverb::parameterCount> sliders;
    std::array<juce::Label, arverb::parameterCount> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,
               arverb::parameterCount> sliderAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbEditor)
};
} // namespace arplugin
