#include "PluginEditor.hpp"
#include <algorithm>
#include <cmath>

namespace arplugin {
namespace {
const juce::Colour ink (0xff292225);
const juce::Colour mutedInk (0xff563b40);
const juce::Colour highlight (0xfff0a1a3);
const juce::Colour displayGreen (0xffb3f1b9);
constexpr float referenceWidth = 1100.0f;
constexpr float referenceHeight = 510.0f;

struct Dial {
    arverb::Parameter parameter;
    float centreX;
    float centreY;
    float diameter;
    const char* caption;
};
constexpr std::array<Dial, arverb::parameterCount> dials {{
    {arverb::Parameter::Mix,          86, 187, 76, "MIX"},
    {arverb::Parameter::PreDelayMs,   86, 354, 76, "PRE-DELAY"},
    {arverb::Parameter::DecaySeconds, 266, 236, 174, "DECAY"},
    {arverb::Parameter::Size,         772, 190, 72, "SIZE"},
    {arverb::Parameter::Diffusion,    772, 354, 72, "DIFFUSION"},
    {arverb::Parameter::DampingHz,    514, 190, 70, "HF DAMP"},
    {arverb::Parameter::LowCutHz,     514, 354, 70, "LOW CUT"},
    {arverb::Parameter::HighCutHz,    620, 190, 70, "HIGH CUT"},
    {arverb::Parameter::ModDepthMs,   996, 354, 70, "DEPTH"},
    {arverb::Parameter::ModRateHz,    898, 354, 70, "RATE"},
    {arverb::Parameter::Width,        996, 190, 70, "WIDTH"},
    {arverb::Parameter::EarlyLevel,   898, 190, 70, "EARLY"},
    {arverb::Parameter::DuckAmount,   620, 354, 70, "DUCK"}
}};
static_assert (dials.size() == arverb::parameterCount, "All DSP parameters need controls");

juce::Rectangle<int> scaled (float x, float y, float w, float h,
                              float sx, float sy) {
    return {juce::roundToInt (x * sx), juce::roundToInt (y * sy),
            juce::roundToInt (w * sx), juce::roundToInt (h * sy)};
}
} // namespace

void HardwareLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                             int x, int y, int width, int height,
                                             float position, float start, float end,
                                             juce::Slider& slider) {
    const float side = static_cast<float> (std::min (width, height));
    const float radius = side * 0.465f;
    const float cx = x + width * 0.5f;
    const float cy = y + height * 0.5f;
    const juce::Rectangle<float> disk (cx - radius, cy - radius,
                                       radius * 2.0f, radius * 2.0f);

    // Edge shadow, machined rim, then restrained radial charcoal highlight.
    g.setColour (juce::Colours::black.withAlpha (0.24f));
    g.fillEllipse (disk.translated (0.0f, side * 0.032f));
    g.setColour (juce::Colour (0xff241e21));
    g.fillEllipse (disk.expanded (1.6f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff454044),
        cx - radius * 0.5f, cy - radius * 0.8f,
        juce::Colour (0xff171719), cx + radius * 0.7f,
        cy + radius * 0.9f, false));
    g.fillEllipse (disk.reduced (1.3f));
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawEllipse (disk.reduced (2.0f), 1.1f);

    // One concise pointer instead of an illuminated ring (hardware reference).
    const float angle = start + position * (end - start);
    const float si = std::sin (angle), co = std::cos (angle);
    g.setColour (slider.isEnabled() ? juce::Colour (0xffefa6a7)
                                    : juce::Colour (0xff786668));
    g.drawLine (cx + si * radius * 0.58f, cy - co * radius * 0.58f,
                cx + si * radius * 0.82f, cy - co * radius * 0.82f,
                std::max (1.6f, side * 0.027f));
}

void HardwareLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height,
                                         bool, int, int, int, int,
                                         juce::ComboBox&) {
    auto box = juce::Rectangle<float> (0, 0, (float) width, (float) height);
    g.setColour (juce::Colour (0xff392a30).withAlpha (0.85f));
    g.fillRoundedRectangle (box, 5.0f);
    g.setColour (juce::Colour (0xffefbbbb).withAlpha (0.30f));
    g.drawRoundedRectangle (box.reduced (0.5f), 5.0f, 0.8f);
    const float cx = width - 16.0f, cy = height * 0.5f;
    g.setColour (juce::Colour (0xffffddde));
    g.drawLine (cx - 4.0f, cy - 1.8f, cx, cy + 2.5f, 1.4f);
    g.drawLine (cx, cy + 2.5f, cx + 4.0f, cy - 1.8f, 1.4f);
}

void HardwareLookAndFeel::drawToggleButton (juce::Graphics& g,
                                              juce::ToggleButton& button,
                                              bool hovered, bool pressed) {
    const auto r = button.getLocalBounds().toFloat().reduced (1.0f);
    const bool bypass = button.getToggleState();
    g.setColour (bypass ? juce::Colour (0xff453033)
                        : juce::Colour (0xff34372f));
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (bypass ? juce::Colour (0xfff4b0ae)
                        : juce::Colour (0xffb7e5ad));
    g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, hovered ? 1.4f : 0.8f);
    const auto size = std::min (r.getHeight() * 0.32f, 10.0f);
    g.fillEllipse (r.getX() + 9.0f, r.getCentreY() - size * 0.5f, size, size);
    g.setColour (juce::Colour (0xffffe3e1));
    g.setFont (juce::Font (juce::FontOptions (12.0f)));
    g.drawFittedText (bypass ? "BYPASSED" : "ACTIVE", button.getLocalBounds()
        .withTrimmedLeft (25), juce::Justification::centredLeft, 1);
    juce::ignoreUnused (pressed);
}

ReverbEditor::ReverbEditor (ReverbProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p) {
    setLookAndFeel (&theme);
    theme.setColour (juce::ComboBox::textColourId, juce::Colour (0xffffe1df));
    theme.setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff30272c));
    theme.setColour (juce::PopupMenu::textColourId, juce::Colour (0xffffe6e0));
    theme.setColour (juce::PopupMenu::highlightedBackgroundColourId,
                     juce::Colour (0xff87444e));
    theme.setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

    presetBox.setTextWhenNothingSelected ("PRESETS");
    presetBox.addItem ("Vocal Plate", 1);
    presetBox.addItem ("Vocal Chamber", 2);
    presetBox.addItem ("Music Hall", 3);
    presetBox.addItem ("Airy Ballad", 4);
    presetBox.setJustificationType (juce::Justification::centredLeft);
    presetBox.onChange = [this] {
        const int id = presetBox.getSelectedId();
        if (id >= 1 && id <= 4)
            processor.applyPreset (static_cast<arverb::Preset> (id - 1));
    };
    addAndMakeVisible (presetBox);

    bypassButton.setButtonText ("Active");
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.parameters, "bypass", bypassButton);
    addAndMakeVisible (bypassButton);

    for (std::size_t i = 0; i < arverb::parameterCount; ++i) {
        const auto& dial = dials[i];
        const auto& meta = arverb::parameterInfo (dial.parameter);
        // The visual table is explicitly indexed by the stable Parameter enum.
        jassert (static_cast<std::size_t> (dial.parameter) == i);
        auto& slider = sliders[i];
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f, true);
        slider.setScrollWheelEnabled (false);
        slider.setPopupDisplayEnabled (true, true, this);
        slider.setDoubleClickReturnValue (true, meta.defaultValue);
        slider.setTooltip (juce::String (meta.name) + "  (" + meta.unit + ")");
        addAndMakeVisible (slider);
        auto& label = labels[i];
        label.setText (dial.caption, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, ink);
        addAndMakeVisible (label);
        sliderAttachments[i] = std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment> (
                processor.parameters, meta.id, slider);
        slider.onDragStart = [this] {
            // A tweaked preset is custom; do not display a misleading preset name.
            presetBox.setSelectedId (0, juce::dontSendNotification);
        };
    }

    decayReadout.setJustificationType (juce::Justification::centred);
    decayReadout.setColour (juce::Label::textColourId, displayGreen);
    decayReadout.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (decayReadout);
    sliders[static_cast<std::size_t> (arverb::Parameter::DecaySeconds)].onValueChange =
        [this] { refreshDecayReadout(); };
    refreshDecayReadout();

    setResizable (true, true);
    setResizeLimits (830, 390, 1440, 680);
    setSize (1100, 510);
}

ReverbEditor::~ReverbEditor() {
    // APVTS attachments must be destroyed before the controls and theme.
    bypassAttachment.reset();
    for (auto& attachment : sliderAttachments) attachment.reset();
    setLookAndFeel (nullptr);
}

void ReverbEditor::refreshDecayReadout() {
    const float seconds = static_cast<float> (
        sliders[static_cast<std::size_t> (arverb::Parameter::DecaySeconds)].getValue());
    decayReadout.setText (juce::String (seconds, 2) + " s", juce::dontSendNotification);
}

void ReverbEditor::paint (juce::Graphics& g) {
    const float sx = getWidth() / referenceWidth;
    const float sy = getHeight() / referenceHeight;
    const float fs = juce::jlimit (0.74f, 1.35f, std::min (sx, sy));
    const auto w = static_cast<float> (getWidth());
    const auto h = static_cast<float> (getHeight());
    g.fillAll (juce::Colour (0xff241f22));

    auto face = juce::Rectangle<float> (5, 5, w - 10, h - 10);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffd87977),
                      0, 5, juce::Colour (0xffba585e), 0, h, false));
    g.fillRoundedRectangle (face, 7.0f);

    // Very light deterministic horizontal brushing. No image assets or animation.
    for (int y = 12; y < getHeight() - 12; y += 5) {
        const auto alpha = 0.018f + 0.014f * static_cast<float> ((y * 17) % 7) / 6.0f;
        g.setColour (juce::Colours::white.withAlpha (alpha));
        g.drawHorizontalLine (y, 10.0f, w - 10.0f);
    }
    g.setColour (juce::Colour (0xfff1adad).withAlpha (0.53f));
    g.drawRoundedRectangle (face.reduced (0.5f), 7.0f, 1.0f);

    // Original ArSonVerb faceplate identity (no reference-brand logos or marks).
    g.setColour (ink);
    g.setFont (juce::Font (juce::FontOptions (29.0f * fs, juce::Font::plain)));
    g.drawText ("ArSon", scaled (25, 20, 110, 36, sx, sy),
                juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xffffe1d8));
    g.drawText ("Verb", scaled (133, 20, 86, 36, sx, sy),
                juce::Justification::centredLeft);
    g.setColour (mutedInk);
    g.setFont (juce::Font (juce::FontOptions (10.5f * fs)));
    g.drawText ("MUSICAL REVERB  /  STUDIO EDITION",
                scaled (26, 59, 310, 19, sx, sy),
                juce::Justification::centredLeft);
    g.setColour (ink.withAlpha (0.25f));
    g.drawLine (24.0f * sx, 97.0f * sy,
                (referenceWidth - 24.0f) * sx, 97.0f * sy, 1.0f);

    // Optical groups, intentionally separated instead of 7x2 debug grid.
    const auto group = [&] (float left, float right, const juce::String& name) {
        g.setColour (mutedInk.withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (10.5f * fs)));
        g.drawText (name, scaled (left, 114, right - left, 19, sx, sy),
                    juce::Justification::centred);
        g.setColour (ink.withAlpha (0.22f));
        g.drawLine ((left + 6.0f) * sx, 138.0f * sy,
                    (right - 6.0f) * sx, 138.0f * sy, 1.0f);
    };
    group (450, 689, "TONE / CONTROL");
    group (725, 821, "SHAPE");
    group (850, 1052, "SPACE / MOTION");

    // Thin spatial dividers echo studio hardware panel printing.
    g.setColour (ink.withAlpha (0.28f));
    for (const float x : {436.0f, 710.0f, 838.0f})
        g.drawLine (x * sx, 159.0f * sy, x * sx, 432.0f * sy, 1.0f);

    // Green hardware-style decay readout, not a general-purpose text box.
    const auto display = scaled (206, 425, 122, 44, sx, sy).toFloat();
    g.setColour (juce::Colour (0xff263529));
    g.fillRoundedRectangle (display, 5.0f * fs);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1e6943),
            display.getX(), display.getY(),
            juce::Colour (0xff17442e), display.getRight(),
            display.getBottom(), false));
    g.fillRoundedRectangle (display.reduced (2.0f * fs), 4.0f * fs);
    g.setColour (juce::Colour (0xffa1d5a8).withAlpha (0.42f));
    g.drawRoundedRectangle (display.reduced (2.0f * fs), 4.0f * fs, 0.9f);

    // Original accent: five quiet sound-wave slots, no copied logo.
    g.setColour (ink.withAlpha (0.68f));
    for (int i = 0; i < 5; ++i)
        g.fillRoundedRectangle (scaled (25.0f + i * 12.0f, 469.0f - (i % 2) * 3.0f,
                                         7.0f, 15.0f + (i % 2) * 6.0f, sx, sy).toFloat(),
                                2.0f * fs);
    g.setColour (juce::Colour (0xffffdcce).withAlpha (0.37f));
    g.setFont (juce::Font (juce::FontOptions (9.0f * fs)));
    g.drawText ("ORIGINAL DSP  •  C++ CORE", scaled (838, 469, 218, 22, sx, sy),
                juce::Justification::centredRight);
}

void ReverbEditor::resized() {
    const float sx = getWidth() / referenceWidth;
    const float sy = getHeight() / referenceHeight;
    const float s = juce::jlimit (0.74f, 1.35f, std::min (sx, sy));
    presetBox.setBounds (scaled (785, 26, 188, 33, sx, sy));
    bypassButton.setBounds (scaled (982, 26, 94, 33, sx, sy));

    for (const auto& dial : dials) {
        const auto i = static_cast<std::size_t> (dial.parameter);
        const float side = dial.diameter * s;
        sliders[i].setBounds (juce::roundToInt (dial.centreX * sx - side * 0.5f),
                              juce::roundToInt (dial.centreY * sy - side * 0.5f),
                              juce::roundToInt (side), juce::roundToInt (side));
        const float captionTop = dial.centreY * sy + side * 0.5f + 5.0f * s;
        labels[i].setBounds (juce::roundToInt (dial.centreX * sx - 55.0f * sx),
                             juce::roundToInt (captionTop),
                             juce::roundToInt (110.0f * sx), juce::roundToInt (21.0f * s));
        labels[i].setFont (juce::Font (juce::FontOptions (11.5f * s)));
    }
    decayReadout.setBounds (scaled (207, 424, 120, 44, sx, sy));
    decayReadout.setFont (juce::Font (juce::FontOptions (23.0f * s)));
}
} // namespace arplugin
