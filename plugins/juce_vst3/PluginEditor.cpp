#include "PluginEditor.hpp"
#include <BinaryData.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace arplugin {
namespace {
// Single premium palette: satin bronze chassis, champagne typography,
// subdued jade metering, and dark Bakelite hardware. No pink/neon mix.
namespace palette {
const juce::Colour chassisTop (0xff59473c);
const juce::Colour chassisBottom (0xff271f1f);
const juce::Colour moduleTop (0xff675043);
const juce::Colour moduleBottom (0xff43352f);
const juce::Colour moduleBorder (0xffc4a077);
const juce::Colour mainText (0xfffaeee1);
const juce::Colour secondaryText (0xffdcc7ae);
const juce::Colour lcd (0xff10191a);
const juce::Colour lcdBorder (0xffbd956c);
const juce::Colour jade (0xff60e5af);
const juce::Colour amber (0xffeac48e);
const juce::Colour meterText (0xff7df3c0);
}
const juce::Colour ice = palette::mainText;
const juce::Colour amber = palette::amber;
const juce::Colour volumeGreen = palette::jade;
const juce::Colour displayGreen = palette::meterText;
juce::Typeface::Ptr interTypeface() {
    static const auto face = juce::Typeface::createSystemTypefaceFor (
        BinaryData::InterVariable_ttf, BinaryData::InterVariable_ttfSize);
    return face;
}
// Clone one true Semibold (wght=600) Inter outline instead of synthetic
// boldening. The generated typeface is cached once on the GUI thread.
juce::Typeface::Ptr interSemiboldTypeface() {
    static const auto face = []() -> juce::Typeface::Ptr {
        const std::array<juce::FontVariableSetting,1> settings {{
            {juce::FontFeatureTag {"wght"}, 600.0f}
        }};
        if (auto configured = interTypeface()->cloneWithVariableSettings (settings))
            return configured;
        return interTypeface();
    }();
    return face;
}
juce::Font uiFont (float size) {
    return juce::Font (juce::FontOptions (std::round (size))
                          .withTypeface (interTypeface()));
}
juce::Font uiHeavy (float size) {
    return juce::Font (juce::FontOptions (std::round (size))
                          .withTypeface (interSemiboldTypeface()));
}
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
    {arverb::Parameter::Mix,           96, 214, 98, "MIX"},
    {arverb::Parameter::PreDelayMs,    96, 360, 98, "PRE-DELAY"},
    {arverb::Parameter::DecaySeconds, 295, 253, 200, "DECAY"},
    {arverb::Parameter::Size,         767, 214, 98, "SIZE"},
    {arverb::Parameter::Diffusion,    767, 360, 98, "DIFFUSION"},
    {arverb::Parameter::DampingHz,    511, 214, 98, "HF DAMP"},
    {arverb::Parameter::LowCutHz,     511, 360, 98, "LOW CUT"},
    {arverb::Parameter::HighCutHz,    621, 214, 98, "HIGH CUT"},
    {arverb::Parameter::ModDepthMs,  1014, 360, 98, "DEPTH"},
    {arverb::Parameter::ModRateHz,    902, 360, 98, "RATE"},
    {arverb::Parameter::Width,       1014, 214, 98, "WIDTH"},
    {arverb::Parameter::EarlyLevel,   902, 214, 98, "EARLY"},
    {arverb::Parameter::DuckAmount,   621, 360, 98, "DUCK"}
}};
struct GroupPanel { float x, y, w, h; const char* title; };
constexpr std::array<GroupPanel, 4> groupPanels {{
    {17, 107, 414, 370, "MIX  /  DECAY"},
    {439, 107, 254, 370, "TONE  /  CONTROL"},
    {701, 107, 132, 370, "SHAPE"},
    {841, 107, 242, 370, "SPACE  /  MOTION"}
}};
static_assert (dials.size() == arverb::parameterCount, "All DSP parameters need controls");
constexpr bool dialOrderIsStable() noexcept {
    for (std::size_t i = 0; i < dials.size(); ++i)
        if (static_cast<std::size_t> (dials[i].parameter) != i) return false;
    return true;
}
static_assert (dialOrderIsStable(), "Faceplate controls must match stable DSP IDs");

// One alpha PNG decoded on first use, on JUCE's message thread. All plugin
// windows share the same decoded logo without asset I/O or audio-thread work.
const juce::Image& productWordmark() {
    static const juce::Image logo = juce::ImageFileFormat::loadFrom (
        BinaryData::ArSonVerbLogo_png,
        static_cast<std::size_t> (BinaryData::ArSonVerbLogo_pngSize));
    return logo;
}

// One embedded original gold-metal knob asset shared by all 13 controls.
// Decode ONCE on GUI thread; use two cached scaled surfaces for large/small.
const juce::Image& goldKnobMaster() {
    static const juce::Image gold = juce::ImageFileFormat::loadFrom (
        BinaryData::ArSonVerbGoldKnob_png,
        static_cast<std::size_t> (BinaryData::ArSonVerbGoldKnob_pngSize));
    return gold;
}

juce::String humanValue (arverb::Parameter param, float value) {
    using P = arverb::Parameter;
    switch (param) {
        case P::Mix: case P::Diffusion: case P::Width:
        case P::EarlyLevel: case P::DuckAmount:
            return juce::String (value * 100.0f, 0) + " %";
        case P::DecaySeconds: return juce::String (value, 2) + " s";
        case P::PreDelayMs: return juce::String (value, 0) + " ms";
        case P::ModDepthMs: return juce::String (value, 2) + " ms";
        case P::ModRateHz: return juce::String (value, 2) + " Hz";
        case P::DampingHz: case P::LowCutHz: case P::HighCutHz:
            return value >= 1000.0f ? juce::String (value / 1000.0f, 1) + " kHz"
                                    : juce::String (value, 0) + " Hz";
        case P::Size: return juce::String (value, 2) + " x";
        default: return juce::String (value, 2);
    }
}

juce::Rectangle<int> scaled (float x, float y, float w, float h,
                              float sx, float sy) {
    return {juce::roundToInt (x * sx), juce::roundToInt (y * sy),
            juce::roundToInt (w * sx), juce::roundToInt (h * sy)};
}
} // namespace

void HardwareLookAndFeel::drawRotarySlider (juce::Graphics& g,
     int x, int y, int width, int height, float position,
     float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) {
    const float side = static_cast<float> (std::min (width, height));
    if (side <= 1.0f) return;
    const float cx = x + width * 0.5f;
    const float cy = y + height * 0.5f;
    const float angle = vintageRotaryAngle (position, rotaryStartAngle, rotaryEndAngle);
    const float grooveR = side * vintageGrooveRatio (side);
    const float faceR = side * vintageFaceRatio (side);
    const float progressR = side * vintageArcRatio (side);
    const auto shell = juce::Rectangle<float>(
        cx - grooveR, cy - grooveR, 2.0f * grooveR, 2.0f * grooveR);

    // Dark, physically recessed Bakelite outer ring with a champagne alloy rim.
    // Drawing is confined to the slider's own bounds.
    g.setColour (juce::Colours::black.withAlpha (0.29f));
    g.fillEllipse (shell.translated (0.0f, side * 0.017f));
    g.setGradientFill (juce::ColourGradient (
        juce::Colour (0xffc6a77d), cx - grooveR, cy - grooveR,
        juce::Colour (0xff4d423a), cx + grooveR, cy + grooveR, false));
    g.fillEllipse (shell);
    g.setColour (juce::Colour (0xff0e1213));
    g.fillEllipse (shell.reduced (1.25f));
    g.setColour (juce::Colour (0xff383737));
    g.drawEllipse (shell.reduced (2.1f), 0.8f);

    // One continuous live meter with a warm jade light pipe and subtle bloom.
    // The same rotary angle feeds BOTH the arc end and the moving LED.
    const float arcWidth = juce::jlimit (3.2f, 5.2f, side * 0.040f);
    juce::Path track;
    track.addCentredArc (cx, cy, progressR, progressR, 0.0f,
                         rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xff303737));
    g.strokePath (track, juce::PathStrokeType (
        arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    if (slider.isEnabled() && position > 0.0001f) {
        juce::Path progress;
        progress.addCentredArc (cx, cy, progressR, progressR, 0.0f,
                                rotaryStartAngle, angle, true);
        g.setColour (palette::jade.withAlpha (0.16f));
        g.strokePath (progress, juce::PathStrokeType (
            arcWidth + 2.5f, juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));
        g.setColour (palette::jade);
        g.strokePath (progress, juce::PathStrokeType (
            arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Gold PNG is a stationary hardware surface, cached only in two sizes.
    // No per-automation resampling, rotating bitmaps or audio-thread work.
    const int imageSide = juce::roundToInt (side);
    const int cacheSlot = vintageGoldCacheSlot (side);
    if (goldFaceSizes[cacheSlot] != imageSide || !goldFaceCache[cacheSlot].isValid()) {
        juce::Image image (juce::Image::ARGB, imageSide, imageSide, true);
        juce::Graphics metal (image);
        const float c = imageSide * 0.5f;
        const float radius = imageSide * vintageFaceRatio (float (imageSide));
        if (const auto& asset = goldKnobMaster(); asset.isValid()) {
            const auto disk = juce::Rectangle<float> (
                c - radius, c - radius, 2.0f * radius, 2.0f * radius)
                .getSmallestIntegerContainer();
            metal.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            metal.drawImageWithin (asset, disk.getX(), disk.getY(),
                                   disk.getWidth(), disk.getHeight(),
                                   juce::RectanglePlacement::stretchToFit);
        } else {
            // Preserve the arc clearance if a damaged PNG triggers fallback.
            const float visibleR = imageSide * vintageGoldVisibleRatio (float (imageSide));
            const auto disk = juce::Rectangle<float> (
                c - visibleR, c - visibleR, 2.0f * visibleR, 2.0f * visibleR);
            metal.setGradientFill (juce::ColourGradient (
                juce::Colour (0xfff2d4a1), disk.getX(), disk.getY(),
                juce::Colour (0xff6c4e33), disk.getRight(), disk.getBottom(), false));
            metal.fillEllipse (disk);
        }
        goldFaceCache[cacheSlot] = image;
        goldFaceSizes[cacheSlot] = imageSide;
    }
    g.drawImageAt (goldFaceCache[cacheSlot],
        juce::roundToInt (cx - imageSide * 0.5f),
        juce::roundToInt (cy - imageSide * 0.5f));

    // Emerald LED sits INSIDE the visible gold-metal rim, not on its edge.
    // The dot and progress arc still share exactly the same automation angle.
    const float radius = side * vintageLedPositionRatio (side);
    const float dotX = cx + std::sin (angle) * radius;
    const float dotY = cy - std::cos (angle) * radius;
    const float dotR = vintageLedRadius (side);
    const auto disk = juce::Rectangle<float> (
        dotX - dotR, dotY - dotR, 2.0f * dotR, 2.0f * dotR);
    g.setColour (juce::Colour (0xff101d1a));
    g.fillEllipse (disk.expanded (1.45f));
    if (slider.isEnabled()) {
        g.setColour (palette::jade.withAlpha (0.22f));
        g.fillEllipse (disk.expanded (2.75f));
    }
    g.setColour (slider.isEnabled() ? juce::Colour (0xff40d58c)
                                    : juce::Colour (0xff777a73));
    g.fillEllipse (disk);
    g.setColour (slider.isEnabled() ? juce::Colour (0xffe7fff0)
                                    : juce::Colour (0xffc4c4ba));
    g.fillEllipse (disk.reduced (dotR * 0.48f));
}

void HardwareLookAndFeel::drawComboBox (juce::Graphics& g,
                                         int width, int height, bool,
                                         int, int, int, int, juce::ComboBox&) {
    // Solid, pixel-aligned LCD, with no translucent gradient behind text.
    // The native ComboBox Label is positioned explicitly below.
    const auto frame = juce::Rectangle<float> (
        0.5f, 0.5f, float (width - 1), float (height - 1));
    g.setColour (palette::lcd);
    g.fillRoundedRectangle (frame, 3.5f);
    g.setColour (palette::lcdBorder);
    g.drawRoundedRectangle (frame, 3.5f, 1.0f);
    g.setColour (juce::Colour (0xffd8c1a3).withAlpha (0.24f));
    g.drawHorizontalLine (2, 4.0f, float (width - 5));

    const float x = float (width - 15);
    const float y = float (height / 2);
    g.setColour (palette::mainText);
    juce::Path chevron;
    chevron.startNewSubPath (x - 3.5f, y - 1.5f);
    chevron.lineTo (x, y + 2.0f);
    chevron.lineTo (x + 3.5f, y - 1.5f);
    g.strokePath (chevron, juce::PathStrokeType (
        1.35f, juce::PathStrokeType::curved,
        juce::PathStrokeType::rounded));
}
juce::Font HardwareLookAndFeel::getComboBoxFont (juce::ComboBox&) {
    return uiHeavy (13.2f);
}
void HardwareLookAndFeel::positionComboBoxText (
    juce::ComboBox& box, juce::Label& label) {
    label.setBounds (9, 1, juce::jmax (0, box.getWidth() - 35),
                     juce::jmax (0, box.getHeight() - 2));
    label.setFont (getComboBoxFont (box));
    label.setJustificationType (juce::Justification::centredLeft);
    label.setColour (juce::Label::textColourId, palette::mainText);
}
juce::Font HardwareLookAndFeel::getPopupMenuFont() {
    return uiHeavy (13.2f);
}
void HardwareLookAndFeel::drawPopupMenuBackground (
    juce::Graphics& g, int width, int height) {
    g.fillAll (palette::lcd);
    g.setColour (palette::lcdBorder);
    g.drawRect (0, 0, width, height, 1);
}
void HardwareLookAndFeel::drawTooltip (juce::Graphics& g,
                                       const juce::String& text,
                                       int width, int height) {
    g.fillAll (palette::lcd);
    g.setColour (palette::lcdBorder);
    g.drawRect (0, 0, width, height, 1);
    g.setColour (palette::mainText);
    g.setFont (uiHeavy (13.0f));
    g.drawText (text, juce::Rectangle<int> (8, 4, width - 16, height - 8),
                juce::Justification::centredLeft, false);
}
void HardwareLookAndFeel::drawToggleButton (
    juce::Graphics& g, juce::ToggleButton& button,
    bool hovered, bool pressed) {
    const bool active = ! button.getToggleState(); // APVTS bypass=true -> off
    const auto outer = button.getLocalBounds().toFloat().reduced (0.5f);
    const auto cap = outer.reduced (1.8f).translated (0.0f, pressed ? 1.0f : 0.0f);
    // One crisp classic switch with a machined rim and solid dark cap.
    g.setColour (juce::Colour (0xff131515));
    g.fillRoundedRectangle (outer, 4.0f);
    g.setColour (palette::lcdBorder);
    g.drawRoundedRectangle (outer, 4.0f, 1.0f);
    g.setColour (pressed ? juce::Colour (0xff2a2725)
                         : juce::Colour (0xff33302d));
    g.fillRoundedRectangle (cap, 3.0f);
    g.setColour (juce::Colours::white.withAlpha (hovered ? 0.27f : 0.13f));
    g.drawLine (cap.getX() + 4.0f, cap.getY() + 1.5f,
                cap.getRight() - 4.0f, cap.getY() + 1.5f, 0.85f);

    const float cx = cap.getX() + 17.5f, cy = cap.getCentreY();
    const float radius = std::min (8.2f, cap.getHeight() * 0.30f);
    juce::Path powerIcon;
    powerIcon.addCentredArc (cx, cy, radius, radius, 0.0f,
                              0.62f, 5.66f, true);
    powerIcon.startNewSubPath (cx, cy - radius * 1.13f);
    powerIcon.lineTo (cx, cy - radius * 0.11f);
    if (active) {
        g.setColour (palette::amber.withAlpha (0.22f));
        g.strokePath (powerIcon, juce::PathStrokeType (4.1f));
    }
    g.setColour (active ? palette::amber : juce::Colour (0xff938b83));
    g.strokePath (powerIcon, juce::PathStrokeType (1.9f));

    // No drawFittedText auto-compression: it caused fuzzy, undersized labels.
    // Fixed-size Inter Bold, integer text bounds, full-opacity ivory.
    const juce::String caption = active ? "ACTIVE" : "BYPASSED";
    const auto textBounds = button.getLocalBounds()
        .withTrimmedLeft (33).withTrimmedRight (4);
    g.setFont (uiHeavy (11.7f));
    g.setColour (active ? palette::mainText : palette::secondaryText);
    g.drawText (caption, textBounds, juce::Justification::centred, false);
}

ReverbEditor::ReverbEditor (ReverbProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p) {
    setLookAndFeel (&theme);
    theme.setDefaultSansSerifTypeface (interTypeface());
    theme.setColour (juce::ComboBox::textColourId, ice);
    theme.setColour (juce::PopupMenu::backgroundColourId, palette::lcd);
    theme.setColour (juce::PopupMenu::textColourId, ice);
    theme.setColour (juce::PopupMenu::highlightedBackgroundColourId,
                     juce::Colour (0xff514637));
    theme.setColour (juce::PopupMenu::highlightedTextColourId, palette::mainText);

    presetBox.setTextWhenNothingSelected ("PRESETS");
    // No placeholder rows: the immutable core catalog supplies every label
    // and all 13 parameters. Menu section order does not renumber Preset IDs.
    for (const auto* category : {"Vocal", "Music", "Vintage", "Ambient"}) {
        presetBox.addSectionHeading (category);
        for (std::size_t i = 0; i < arverb::presetCount; ++i) {
            const auto& info = arverb::presetInfo (static_cast<arverb::Preset> (i));
            if (std::strcmp (info.category, category) == 0)
                presetBox.addItem (info.name, static_cast<int> (i) + 1);
        }
    }
    presetBox.setJustificationType (juce::Justification::centredLeft);
    presetBox.onChange = [this] {
        const int id = presetBox.getSelectedId();
        if (id > 0 && id <= static_cast<int> (arverb::presetCount))
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
        slider.setScrollWheelEnabled (true);
        // Avoid JUCE's full-precision floating bubble (e.g. 10000.000000).
        slider.setPopupDisplayEnabled (false, false, this);
        slider.setDoubleClickReturnValue (true, meta.defaultValue);
        slider.setTooltip (juce::String (meta.name));
        addAndMakeVisible (slider);
        auto& label = labels[i];
        label.setText (dial.caption, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, palette::mainText);
        addAndMakeVisible (label);
        sliderAttachments[i] = std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment> (
                processor.parameters, meta.id, slider);
        slider.onDragStart = [this, i] {
            presetBox.setSelectedId (0, juce::dontSendNotification);
            activeParameter = static_cast<int> (i);
            refreshParameterReadout (i);
            parameterReadout.setVisible (true);
        };
        slider.onDragEnd = [this] {
            activeParameter = -1;
            parameterReadout.setVisible (false);
        };
        slider.onValueChange = [this, i] {
            if (i == static_cast<std::size_t> (arverb::Parameter::DecaySeconds))
                refreshDecayReadout();
            if (activeParameter == static_cast<int> (i))
                refreshParameterReadout (i);
        };
    }

    decayReadout.setJustificationType (juce::Justification::centred);
    decayReadout.setColour (juce::Label::textColourId, displayGreen);
    decayReadout.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (decayReadout);
    refreshDecayReadout();
    parameterReadout.setColour (juce::Label::backgroundColourId, palette::lcd);
    parameterReadout.setColour (juce::Label::textColourId, palette::mainText);
    parameterReadout.setJustificationType (juce::Justification::centred);
    parameterReadout.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (parameterReadout);
    parameterReadout.setVisible (false);

    setResizable (true, true);
    setResizeLimits (800, 370, 1180, 545);
    setSize (842, 394);
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

void ReverbEditor::refreshParameterReadout (std::size_t index) {
    const auto parameter = static_cast<arverb::Parameter> (index);
    parameterReadout.setText (
        juce::String (arverb::parameterInfo (parameter).name) + "   " +
        humanValue (parameter, static_cast<float> (sliders[index].getValue())),
        juce::dontSendNotification);
}

juce::Rectangle<int> ReverbEditor::decayDisplayBounds() const {
    const float sx = getWidth() / referenceWidth;
    const float sy = getHeight() / referenceHeight;
    // Anchor LCD to the exact bottom of the PRE-DELAY caption. The aligned
    // baseline survives min/default/max editor sizes and font rounding.
    const auto nominal = scaled (231.0f, 0.0f, 128.0f, 42.0f, sx, sy);
    const auto captionBottom = labels[static_cast<std::size_t> (
        arverb::Parameter::PreDelayMs)].getBottom();
    return {nominal.getX(), captionBottom - nominal.getHeight(),
            nominal.getWidth(), nominal.getHeight()};
}

void ReverbEditor::rebuildFaceplateCache() {
    const int w = getWidth(), h = getHeight();
    if (w <= 0 || h <= 0) return;

    // One cached background for all four milled modules; never repaint its
    // brushed texture on parameter automation or the audio callback.
    faceplateCache = juce::Image (juce::Image::ARGB, w, h, true);
    juce::Graphics g (faceplateCache);
    g.fillAll (juce::Colour (0xff1c1b1c));
    const auto chassis = juce::Rectangle<float> (
        5.0f, 5.0f, float (w - 10), float (h - 10));
    juce::ColourGradient bronze (
        palette::chassisTop, 0.0f, 5.0f,
        palette::chassisBottom, 0.0f, float (h - 5), false);
    bronze.addColour (0.38, juce::Colour (0xff4f3d34));
    bronze.addColour (0.74, juce::Colour (0xff332824));
    g.setGradientFill (bronze);
    g.fillRoundedRectangle (chassis, 7.0f);

    // Subtle deterministic satin metal brushing, not coarse horizontal bands.
    std::uint32_t seed = 0x61e3a597U;
    const auto next = [&seed]() noexcept {
        seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
        return seed;
    };
    for (int y = 9; y < h - 9; ++y) {
        const float n = float (next() & 255U) / 255.0f;
        g.setColour (n > 0.5f
            ? juce::Colours::white.withAlpha (0.003f + (n - 0.5f) * 0.015f)
            : juce::Colours::black.withAlpha (0.002f + (0.5f - n) * 0.015f));
        g.drawHorizontalLine (y, 9.0f, float (w - 9));
    }
    g.setColour (palette::moduleBorder.withAlpha (0.79f));
    g.drawRoundedRectangle (chassis.reduced (0.5f), 7.0f, 0.95f);

    const float sx = w / referenceWidth, sy = h / referenceHeight;
    for (const auto& module : groupPanels) {
        const auto rect = scaled (module.x, module.y, module.w, module.h,
                                  sx, sy).toFloat();
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (rect.translated (0.0f, 1.3f), 4.4f);
        juce::ColourGradient satin (
            palette::moduleTop, rect.getX(), rect.getY(),
            palette::moduleBottom, rect.getX(), rect.getBottom(), false);
        satin.addColour (0.50, juce::Colour (0xff554238));
        g.setGradientFill (satin);
        g.fillRoundedRectangle (rect.reduced (1.2f), 3.7f);
        g.setColour (palette::moduleBorder.withAlpha (0.96f));
        g.drawRoundedRectangle (rect.reduced (0.6f), 4.0f, 1.0f);
        g.setColour (juce::Colour (0xffecd3b4).withAlpha (0.22f));
        g.drawRoundedRectangle (rect.reduced (1.9f), 3.2f, 0.75f);
        const float titleRuleY = (module.y + 39.0f) * sy;
        g.setColour (palette::moduleBorder.withAlpha (0.56f));
        g.drawLine (rect.getX() + 6, titleRuleY,
                    rect.getRight() - 6, titleRuleY, 0.9f);
    }
}

void ReverbEditor::paint (juce::Graphics& g) {
    const float sx = getWidth() / referenceWidth;
    const float sy = getHeight() / referenceHeight;
    const float fs = juce::jlimit (0.74f, 1.35f, std::min (sx, sy));
    g.fillAll (juce::Colour (0xff1c1b1c));
    if (faceplateCache.isValid()) g.drawImageAt (faceplateCache, 0, 0);

    // Transparent champagne-gold product asset. Aspect ratio remains exact,
    // and the image is decoded once, never during mouse/automation updates.
    const auto logoBox = scaled (25, 12, 315, 61, sx, sy);
    const auto& logo = productWordmark();
    if (logo.isValid()) {
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        g.drawImageWithin (logo, logoBox.getX(), logoBox.getY(),
                           logoBox.getWidth(), logoBox.getHeight(),
                           juce::RectanglePlacement::centred);
    } else {
        // Defensive fallback when a damaged resource is encountered.
        g.setColour (palette::mainText);
        g.setFont (uiHeavy (26.0f * fs));
        g.drawText ("ArSon-Verb", logoBox, juce::Justification::centredLeft);
    }
    g.setColour (palette::secondaryText);
    g.setFont (uiHeavy (std::max (11.8f, 13.6f * fs)));
    g.drawText ("MUSICAL VINTAGE REVERB",
        scaled (27, 72, 315, 20, sx, sy),
        juce::Justification::centredLeft);

    // Printed machine-engraved group captions inside the framed modules.
    // None of these are extra JUCE components or click targets.
    for (const auto& m : groupPanels) {
        g.setColour (palette::mainText);
        g.setFont (uiHeavy (std::max (13.2f, 16.2f * fs)));
        g.drawText (m.title,
            scaled (m.x+7, m.y+8, m.w-14, 24, sx, sy),
            juce::Justification::centred);
    }

    // Green hardware-style decay readout, not a general-purpose text box.
    const auto display = decayDisplayBounds().toFloat();
    g.setColour (juce::Colour (0xff1e2622));
    g.fillRoundedRectangle (display, 5.0f * fs);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff173e31),
            display.getX(), display.getY(),
            juce::Colour (0xff09221b), display.getRight(),
            display.getBottom(), false));
    g.fillRoundedRectangle (display.reduced (2.0f * fs), 4.0f * fs);
    g.setColour (palette::jade.withAlpha (0.56f));
    g.drawRoundedRectangle (display.reduced (2.0f * fs), 4.0f * fs, 0.9f);

    // The reference uses framed modules without ornament at their base;
    // omit the previous decorative waveform/footer that collided with frames.

}

void ReverbEditor::resized() {
    const float sx = getWidth() / referenceWidth;
    const float sy = getHeight() / referenceHeight;
    const float s = juce::jlimit (0.74f, 1.35f, std::min (sx, sy));
    // One shared optical baseline for LCD preset and physical power button.
    // Integer-pixel snapping prevents blurred half-pixel text and frames.
    presetBox.setBounds (scaled (672, 27, 253, 34, sx, sy));
    bypassButton.setBounds (scaled (935, 27, 141, 34, sx, sy));

    for (const auto& dial : dials) {
        const auto i = static_cast<std::size_t> (dial.parameter);
        const float side = dial.diameter * s;
        sliders[i].setBounds (juce::roundToInt (dial.centreX * sx - side * 0.5f),
                              juce::roundToInt (dial.centreY * sy - side * 0.5f),
                              juce::roundToInt (side), juce::roundToInt (side));
        const float captionTop = dial.centreY * sy + side * 0.5f + 7.0f * s;
        labels[i].setBounds (juce::roundToInt (dial.centreX * sx - 55.0f * sx),
                             juce::roundToInt (captionTop),
                             juce::roundToInt (110.0f * sx), juce::roundToInt (21.0f * s));
        labels[i].setFont (uiHeavy (std::max (12.5f, 16.8f * s)));
    }
    decayReadout.setBounds (decayDisplayBounds().withTrimmedLeft (2)
                                                .withTrimmedRight (2));
    decayReadout.setFont (uiHeavy (23.8f * s));
    parameterReadout.setBounds (scaled (355, 27, 305, 34, sx, sy));
    parameterReadout.setFont (uiHeavy (std::max (12.0f, 14.5f * s)));
    rebuildFaceplateCache();
}
} // namespace arplugin
