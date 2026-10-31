#pragma once

#include "PluginProcessor.hpp"
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <memory>

namespace arplugin {

// Native JUCE renderer: one embedded physical-knob texture plus vector level
// indicator/pointer. No timers or allocations in the real-time DSP callback.
// In JUCE, the slider supplies a normalized 0..1 position. Both the pointer
// and progress arc must use this exact angle to stay synchronized.
inline float vintageRotaryAngle (float position, float start, float end) noexcept {
    return start + juce::jlimit (0.0f, 1.0f, position) * (end - start);
}
// Hardware-size optical corrections: small dials need more visible black
// socket, without changing the large Decay dial's successful proportions.
inline float vintageFaceRatio (float side) noexcept {
    return side < 100.0f ? 0.316f : 0.355f;
}
inline float vintageGrooveRatio (float side) noexcept {
    return side < 100.0f ? 0.480f : 0.465f;
}
inline float vintageArcRatio (float side) noexcept {
    return side < 100.0f ? 0.409f : 0.419f;
}

class HardwareLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float,
                           float, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int,
                       juce::ComboBox&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool, bool) override;
    void drawPopupMenuBackground (juce::Graphics&, int, int) override;
    void drawTooltip (juce::Graphics&, const juce::String&, int, int) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Font getPopupMenuFont() override;

private:
    // 2 image caches (12 small, 1 large); alpha asset pre-scaled on resize.
    // LookAndFeel keeps the artwork stationary while the level/pointer rotate.
    std::array<juce::Image, 2> goldFaceCache;
    std::array<int, 2> goldFaceSizes {{0, 0}};
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
    juce::Rectangle<int> decayDisplayBounds() const;
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
