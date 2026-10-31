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
// Larger face in both sizes matches the approved 2026-10-31 mockup.
inline float vintageFaceRatio (float side) noexcept {
    // The embedded PNG has ~15% transparent radius beyond the actual metal.
    // Enlarging only Decay makes the PHYSICAL gold-to-arc clearance match the
    // small knobs without changing their 98px/200px control hitboxes.
    return side < 125.0f ? 0.422f : 0.468f;
}
inline float vintageGoldVisibleRatio (float side) noexcept {
    constexpr float pngGoldRadiusFraction = 0.85f; // 160px PNG alpha extent
    return vintageFaceRatio (side) * pngGoldRadiusFraction;
}
inline float vintageLedPositionRatio (float side) noexcept {
    // A jewel seated on the metal surface, not hanging off its outer bezel.
    return vintageGoldVisibleRatio (side) * 0.77f;
}
inline float vintageGrooveRatio (float) noexcept { return 0.490f; }
inline float vintageArcRatio (float) noexcept { return 0.435f; }
// Keep one small-face and one large-face cache even at maximum window size:
// small dial max ~105px, large dial minimum ~148px.
inline int vintageGoldCacheSlot (float side) noexcept {
    return side >= 125.0f ? 1 : 0;
}
inline float vintageLedRadius (float side) noexcept {
    return juce::jlimit (4.0f, 6.3f, side * 0.055f);
}

// Preserve JUCE's native wheel/host gesture semantics. Ctrl only scales
// the incoming delta; standard scroll is unchanged and remains familiar.
inline juce::MouseWheelDetails refinedWheel (
    const juce::MouseWheelDetails& original, bool controlHeld) noexcept {
    auto result = original;
    if (controlHeld) {
        result.deltaX *= 0.125f;
        result.deltaY *= 0.125f;
    }
    return result;
}

class PrecisionRotarySlider final : public juce::Slider {
public:
    void mouseWheelMove (const juce::MouseEvent& e,
                         const juce::MouseWheelDetails& wheel) override {
        if (!isEnabled() || !isScrollWheelEnabled()) return;
        juce::Slider::mouseWheelMove (e,
            refinedWheel (wheel, e.mods.isCtrlDown()));
    }
};

// Parse only complete, finite numeric inputs in each parameter's DISPLAY
// units; out-of-range values are rejected, never silently written to APVTS.
bool parseDisplayValue (arverb::Parameter parameter, const juce::String& text,
                        float& nativeValue) noexcept;

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

class ReverbEditor final : public juce::AudioProcessorEditor,
                           private juce::ComboBox::Listener {
public:
    explicit ReverbEditor (ReverbProcessor&);
    ~ReverbEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void comboBoxChanged (juce::ComboBox* combo) override;
    void synchronisePresetSelection (int editedSlider = -1);
    void refreshDecayReadout();
    void refreshParameterReadout (std::size_t index);
    void commitTypedParameterValue();
    void rebuildFaceplateCache();
    juce::Rectangle<int> decayDisplayBounds() const;
    ReverbProcessor& processor;
    HardwareLookAndFeel theme;
    juce::ComboBox presetBox;
    juce::ComboBox modeBox;
    juce::ToggleButton bypassButton;
    juce::Label decayReadout;
    juce::Label parameterReadout;
    juce::Label parameterValueLabel;
    juce::Image faceplateCache;
    int activeParameter = 0; // Persistent header displays last touched control.
    bool applyingPreset = false;
    std::array<PrecisionRotarySlider, arverb::parameterCount> sliders;
    std::array<juce::Label, arverb::parameterCount> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,
               arverb::parameterCount> sliderAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbEditor)
};
} // namespace arplugin
