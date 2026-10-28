#include "PluginEditor.hpp"
#include <algorithm>

namespace arplugin {
namespace {
constexpr auto background = juce::Colour (0xff171b22);
constexpr auto accent = juce::Colour (0xff9dd6b1);
constexpr auto foreground = juce::Colour (0xffdae2eb);
constexpr auto muted = juce::Colour (0xff84919f);
}

ReverbEditor::ReverbEditor (ReverbProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p) {
    theme.setColour (juce::Slider::rotarySliderFillColourId, accent);
    theme.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff37414a));
    theme.setColour (juce::Slider::thumbColourId, foreground);
    theme.setColour (juce::Slider::textBoxTextColourId, foreground);
    theme.setColour (juce::Slider::textBoxBackgroundColourId, background);
    theme.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff313d48));
    theme.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff262d37));
    theme.setColour (juce::ComboBox::textColourId, foreground);
    theme.setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff39434c));
    theme.setColour (juce::ToggleButton::textColourId, foreground);
    setLookAndFeel (&theme);

    title.setText ("AR STUDIO REVERB", juce::dontSendNotification);
    title.setColour (juce::Label::textColourId, foreground);
    title.setFont (juce::Font (juce::FontOptions (17.0f, juce::Font::plain)));
    addAndMakeVisible (title);
    subtitle.setText ("Musical space for vocal & music", juce::dontSendNotification);
    subtitle.setColour (juce::Label::textColourId, muted);
    subtitle.setFont (juce::Font (juce::FontOptions (12.0f)));
    addAndMakeVisible (subtitle);

    presetBox.setTextWhenNothingSelected ("Select a preset...");
    presetBox.addItem ("Vocal Plate", 1);
    presetBox.addItem ("Vocal Chamber", 2);
    presetBox.addItem ("Music Hall", 3);
    presetBox.addItem ("Airy Ballad", 4);
    presetBox.onChange = [this] {
        const auto selected = presetBox.getSelectedId();
        if (selected >= 1 && selected <= 4)
            processor.applyPreset (static_cast<arverb::Preset> (selected - 1));
    };
    addAndMakeVisible (presetBox);
    bypassButton.setButtonText ("Bypass");
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.parameters, "bypass", bypassButton);
    addAndMakeVisible (bypassButton);

    for (std::uint32_t i = 0; i < arverb::parameterCount; ++i) {
        const auto& meta = arverb::parameterInfo (static_cast<arverb::Parameter> (i));
        auto& slider = sliders[i];
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 76, 21);
        slider.setPopupDisplayEnabled (true, true, this);
        slider.setScrollWheelEnabled (false); // Accidental DAW wheel changes are disruptive.
        slider.setDoubleClickReturnValue (true, meta.defaultValue);
        addAndMakeVisible (slider);
        labels[i].setText (meta.name, juce::dontSendNotification);
        labels[i].setJustificationType (juce::Justification::centred);
        labels[i].setColour (juce::Label::textColourId, foreground);
        labels[i].setFont (juce::Font (juce::FontOptions (12.0f)));
        addAndMakeVisible (labels[i]);
        sliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.parameters, meta.id, slider);
    }
    setResizable (true, true);
    setResizeLimits (660, 330, 1200, 800);
    setSize (770, 420);
}

ReverbEditor::~ReverbEditor() {
    bypassAttachment.reset();
    for (auto& attachment : sliderAttachments) attachment.reset();
    setLookAndFeel (nullptr);
}

void ReverbEditor::paint (juce::Graphics& g) {
    g.fillAll (background);
    const auto bounds = getLocalBounds().toFloat();
    const float pad = 14.0f;
    g.setColour (juce::Colour (0xff242c35));
    g.fillRoundedRectangle (juce::Rectangle<float> {pad, 82.0f,
        bounds.getWidth() - pad * 2.0f, bounds.getHeight() - 98.0f}, 8.0f);
    g.setColour (accent.withAlpha (0.9f));
    g.fillRect (pad, 75.0f, bounds.getWidth() - 2.0f * pad, 1.0f);
}

void ReverbEditor::resized() {
    const int w = getWidth();
    const int h = getHeight();
    title.setBounds (18, 11, 300, 27);
    subtitle.setBounds (18, 38, 300, 18);
    presetBox.setBounds (w - 305, 22, 180, 30);
    bypassButton.setBounds (w - 107, 23, 90, 28);

    const int left = 18, top = 92;
    const int areaW = w - 36;
    const int areaH = h - 115;
    constexpr int cols = 7, rows = 2;
    const int cellW = areaW / cols;
    const int cellH = areaH / rows;
    for (std::size_t i = 0; i < arverb::parameterCount; ++i) {
        const int x = left + static_cast<int> (i % cols) * cellW;
        const int y = top + static_cast<int> (i / cols) * cellH;
        labels[i].setBounds (x + 1, y + 3, cellW - 2, 19);
        sliders[i].setBounds (x + 2, y + 23, cellW - 4, cellH - 26);
    }
}

} // namespace arplugin
