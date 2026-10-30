#include "PluginEditor.hpp"
#include <BinaryData.h>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace arplugin {
namespace {
const juce::Colour ink (0xff292225);
const juce::Colour mutedInk (0xff57323a);
const juce::Colour highlight (0xfff3afb7);
const juce::Colour displayGreen (0xffb3f1b9);
const juce::Colour ice (0xffd9faff);
const juce::Colour cyan (0xff83d2e7);
juce::Typeface::Ptr interTypeface() {
    static const auto face = juce::Typeface::createSystemTypefaceFor (
        BinaryData::InterVariable_ttf, BinaryData::InterVariable_ttfSize);
    return face;
}
juce::Font uiFont (float size) {
    return juce::Font (juce::FontOptions (size).withTypeface (interTypeface()));
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
constexpr bool dialOrderIsStable() noexcept {
    for (std::size_t i = 0; i < dials.size(); ++i)
        if (static_cast<std::size_t> (dials[i].parameter) != i) return false;
    return true;
}
static_assert (dialOrderIsStable(), "Faceplate controls must match stable DSP IDs");

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
     int x, int y, int width, int height, float position, float start,
     float end, juce::Slider& slider) {
    const float side = float (std::min (width, height));
    const float cx = x + width * 0.5f, cy = y + height * 0.5f;
    const float recessR = side * 0.468f, knobR = side * 0.392f;
    const float angle = start + position * (end - start);
    const auto groove = juce::Rectangle<float> (cx-recessR, cy-recessR,
                                                 2*recessR, 2*recessR);

    // Shadowed metal recess: an actual gap, not a glow drawn over the knob.
    g.setColour (juce::Colour (0xff69323d).withAlpha (0.60f));
    g.fillEllipse (groove);
    g.setColour (juce::Colour (0xff140e14));
    g.fillEllipse (groove.reduced (1.5f));
    g.setColour (juce::Colour (0xffffd1c3).withAlpha (0.23f));
    g.drawEllipse (groove.reduced (0.7f), 1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.62f));
    g.drawEllipse (groove.reduced (2.4f), 1.4f);

    // Position-tracking cool cyan in the groove. Two small arcs, no timer.
    juce::Path arc;
    const float lightR = side * 0.436f;
    arc.addCentredArc (cx, cy, lightR, lightR, 0.0f,
                       angle - 0.20f, angle + 0.20f, true);
    g.setColour (cyan.withAlpha (slider.isEnabled() ? 0.20f : 0.06f));
    g.strokePath (arc, juce::PathStrokeType (std::max (3.2f, side*0.07f)));
    g.setColour (cyan.withAlpha (slider.isEnabled() ? 0.75f : 0.15f));
    g.strokePath (arc, juce::PathStrokeType (std::max (1.4f, side*0.02f)));

    const auto disk = juce::Rectangle<float> (cx-knobR, cy-knobR, 2*knobR, 2*knobR);
    g.setColour (juce::Colours::black.withAlpha (0.50f));
    g.fillEllipse (disk.translated (0, side*0.032f).expanded (1.0f));
    g.setColour (juce::Colour (0xff17171b));
    g.fillEllipse (disk.expanded (0.8f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff554b50),
       cx-knobR*0.54f, cy-knobR*0.83f, juce::Colour (0xff151619),
       cx+knobR*0.77f, cy+knobR*0.9f, false));
    g.fillEllipse (disk.reduced (1.0f));
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawEllipse (disk.reduced (1.8f), 1.0f);

    // Tangential rectangular LED with restrained bloom.
    const float px = cx + std::sin(angle)*knobR*0.7f;
    const float py = cy - std::cos(angle)*knobR*0.7f;
    const float ledW = std::max (5.0f, side*0.102f);
    const float ledH = std::max (2.6f, side*0.03f);
    const auto marker = [&] (float inflate) {
        juce::Path p;
        p.addRoundedRectangle(-ledW*0.5f-inflate,-ledH*0.5f-inflate,
                              ledW+2*inflate,ledH+2*inflate,1.4f);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(px,py));
        return p;
    };
    if (slider.isEnabled()) {
        g.setColour (cyan.withAlpha (0.19f));
        g.fillPath (marker (2.6f));
        g.setColour (cyan.withAlpha (0.49f));
        g.fillPath (marker (1.0f));
    }
    g.setColour (slider.isEnabled() ? ice : juce::Colour (0xff75787f));
    g.fillPath (marker(0.0f));
}

void HardwareLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h,
                                         bool, int, int, int, int, juce::ComboBox&) {
    const auto rect = juce::Rectangle<float>(0,0,float(w),float(h));
    g.setColour (juce::Colour(0xff04090d)); g.fillRoundedRectangle(rect,4.0f);
    g.setGradientFill (juce::ColourGradient(juce::Colour(0xff233039),0,0,
                                            juce::Colour(0xff090e13),0,float(h),false));
    g.fillRoundedRectangle(rect.reduced(1.1f),3.5f);
    g.setColour(cyan.withAlpha(0.43f)); g.drawRoundedRectangle(rect.reduced(0.6f),3.6f,1.0f);
    g.setColour(ice);
    const float cx=w-15.0f,cy=h*0.5f;
    g.drawLine(cx-4,cy-2,cx,cy+2,1.3f);g.drawLine(cx,cy+2,cx+4,cy-2,1.3f);
}
juce::Font HardwareLookAndFeel::getComboBoxFont (juce::ComboBox&) {
    return uiFont(12.5f);
}
juce::Font HardwareLookAndFeel::getPopupMenuFont() {
    return uiFont(13.0f);
}
void HardwareLookAndFeel::drawPopupMenuBackground(juce::Graphics& g,int w,int h) {
    g.fillAll(juce::Colour(0xff080e13));
    g.setColour(cyan.withAlpha(0.52f));g.drawRect(0,0,w,h,1);
}
void HardwareLookAndFeel::drawTooltip(juce::Graphics& g,const juce::String& value,
                                     int w,int h) {
    g.fillAll(juce::Colour(0xff090f14));
    g.setColour(cyan);g.drawRect(0,0,w,h,1);
    g.setColour(ice);g.setFont(uiFont(13.0f));
    g.drawFittedText(value,7,4,w-14,h-8,juce::Justification::centredLeft,2);
}
void HardwareLookAndFeel::drawToggleButton(juce::Graphics& g,juce::ToggleButton& button,
                                           bool hovered,bool pressed) {
    const bool bypassed=button.getToggleState();
    const auto outer=button.getLocalBounds().toFloat().reduced(1.0f);
    const auto rect=outer.reduced(1.0f).translated(0.0f,pressed?1.0f:0.0f);
    const auto cap=bypassed?juce::Colour(0xffcbd1d1):juce::Colour(0xff91edac);
    const auto edge=bypassed?juce::Colour(0xff748081):juce::Colour(0xff4b9b6a);
    // ArSonMiX BTN-180 physical recipe translated from CSS to native JUCE.
    g.setColour(juce::Colours::black.withAlpha(0.34f));
    g.fillRoundedRectangle(outer.translated(0,1.2f),3.7f);
    g.setColour(edge);g.fillRoundedRectangle(rect,3.4f);
    g.setGradientFill(juce::ColourGradient(cap.brighter(0.17f),rect.getX(),rect.getY(),
                             cap.darker(0.08f),rect.getRight(),rect.getBottom(),false));
    g.fillRoundedRectangle(rect.reduced(1.0f),2.9f);
    g.setColour(juce::Colours::white.withAlpha(pressed?0.11f:0.34f));
    g.drawLine(rect.getX()+3,rect.getY()+1.6f,rect.getRight()-3,rect.getY()+1.6f,1);
    g.setColour(juce::Colours::black.withAlpha(pressed?0.31f:0.17f));
    g.drawLine(rect.getX()+3,rect.getBottom()-2,rect.getRight()-3,rect.getBottom()-2,1);
    if(hovered) {g.setColour(juce::Colours::white.withAlpha(0.22f));
       g.drawRoundedRectangle(rect.reduced(0.8f),3.4f,1.0f);}
    const float led=std::min(8.0f,rect.getHeight()*0.27f);
    g.setColour(bypassed?juce::Colour(0xff74525a):juce::Colour(0xff267449));
    g.fillEllipse(rect.getX()+8.5f,rect.getCentreY()-led*0.5f,led,led);
    g.setColour(juce::Colour(0xff18252b));
    g.setFont(uiFont(11.7f));
    g.drawFittedText(bypassed?"BYPASSED":"ACTIVE",
         button.getLocalBounds().withTrimmedLeft(20).withTrimmedRight(4),
         juce::Justification::centred,1);
}

ReverbEditor::ReverbEditor (ReverbProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p) {
    setLookAndFeel (&theme);
    theme.setDefaultSansSerifTypeface (interTypeface());
    theme.setColour (juce::ComboBox::textColourId, ice);
    theme.setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff080e13));
    theme.setColour (juce::PopupMenu::textColourId, ice);
    theme.setColour (juce::PopupMenu::highlightedBackgroundColourId,
                     juce::Colour (0xff194654));
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
        slider.setScrollWheelEnabled (true);
        // Avoid JUCE's full-precision floating bubble (e.g. 10000.000000).
        slider.setPopupDisplayEnabled (false, false, this);
        slider.setDoubleClickReturnValue (true, meta.defaultValue);
        slider.setTooltip (juce::String (meta.name));
        addAndMakeVisible (slider);
        auto& label = labels[i];
        label.setText (dial.caption, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, ink);
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
    parameterReadout.setColour (juce::Label::backgroundColourId, juce::Colour (0xff091319));
    parameterReadout.setColour (juce::Label::textColourId, ice);
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

void ReverbEditor::rebuildFaceplateCache() {
    const int w = getWidth(), h = getHeight();
    if (w <= 0 || h <= 0) return;
    faceplateCache = juce::Image (juce::Image::ARGB, w, h, true);
    juce::Graphics g (faceplateCache);
    g.fillAll (juce::Colour (0xff241d20));
    const auto panel = juce::Rectangle<float> (5.0f, 5.0f, float (w-10), float (h-10));
    juce::ColourGradient alloy (juce::Colour (0xffec8985), 0.0f, 5.0f,
                                juce::Colour (0xffa44953), 0.0f, float (h-5), false);
    alloy.addColour (0.22, juce::Colour (0xffde7777));
    alloy.addColour (0.57, juce::Colour (0xffcd686b));
    alloy.addColour (0.82, juce::Colour (0xffba5861));
    g.setGradientFill (alloy);
    g.fillRoundedRectangle (panel, 7.0f);

    // Micro-brushing is deterministic and cached only on resize; no bitmap asset
    // or audio-callback work. Irregular strokes break up the striped appearance.
    std::uint32_t seed = 0x61e3a597U;
    const auto next = [&seed]() noexcept {
        seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
        return seed;
    };
    for (int y = 9; y < h-9; ++y) {
        const float n = float (next() & 255U) / 255.0f;
        g.setColour (n > 0.50f
          ? juce::Colours::white.withAlpha (0.005f + (n-0.5f)*0.027f)
          : juce::Colours::black.withAlpha (0.004f + (0.5f-n)*0.025f));
        g.drawHorizontalLine (y, 9.0f, float (w-9));
        if ((y % 4) == 0) for (int k = 0; k < 2; ++k) {
            const float x = 9.0f + float (next() % std::uint32_t (std::max (1, w-75)));
            const float len = 15.0f + float (next() % 65U);
            g.setColour (juce::Colours::white.withAlpha (0.010f));
            g.drawLine (x, float (y), std::min (x+len,float (w-9)), float (y), 0.5f);
        }
    }
    juce::ColourGradient darkFoot (juce::Colours::transparentBlack, 0.0f, float (h)*0.58f,
                                    juce::Colour (0xff4e222e).withAlpha (0.40f),
                                    0.0f, float (h-6), false);
    g.setGradientFill (darkFoot);
    g.fillRoundedRectangle (panel, 7.0f);
    g.setColour (juce::Colour (0xffffd0c7).withAlpha (0.55f));
    g.drawRoundedRectangle (panel.reduced (0.5f), 7.0f, 1.0f);
}

void ReverbEditor::paint (juce::Graphics& g) {
    const float sx = getWidth() / referenceWidth;
    const float sy = getHeight() / referenceHeight;
    const float fs = juce::jlimit (0.74f, 1.35f, std::min (sx, sy));
    g.fillAll (juce::Colour (0xff241d20));
    if (faceplateCache.isValid()) g.drawImageAt (faceplateCache, 0, 0);

    // Original ArSonVerb faceplate identity (no reference-brand logos or marks).
    g.setColour (ink);
    g.setFont (uiFont(29.0f * fs));
    g.drawText ("ArSon", scaled (25, 20, 100, 36, sx, sy),
                juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xffffe1d8));
    g.drawText ("Verb", scaled (106, 20, 86, 36, sx, sy),
                juce::Justification::centredLeft);
    g.setColour (mutedInk);
    g.setFont (uiFont(11.8f * fs));
    g.drawText ("MUSICAL REVERB  /  STUDIO EDITION",
                scaled (26, 59, 310, 19, sx, sy),
                juce::Justification::centredLeft);
    g.setColour (ink.withAlpha (0.25f));
    g.drawLine (24.0f * sx, 97.0f * sy,
                (referenceWidth - 24.0f) * sx, 97.0f * sy, 1.0f);

    // Optical groups, intentionally separated instead of 7x2 debug grid.
    const auto group = [&] (float left, float right, const juce::String& name) {
        g.setColour (mutedInk.withAlpha (0.85f));
        g.setFont (uiFont(11.8f * fs));
        g.drawText (name, scaled (left, 114, right - left, 19, sx, sy),
                    juce::Justification::centred);
        g.setColour (ink.withAlpha (0.42f));
        g.drawLine ((left + 6.0f) * sx, 138.0f * sy,
                    (right - 6.0f) * sx, 138.0f * sy, 1.0f);
    };
    group (450, 689, "TONE / CONTROL");
    group (725, 821, "SHAPE");
    group (850, 1052, "SPACE / MOTION");

    // Thin spatial dividers echo studio hardware panel printing.
    for (const float x : {436.0f, 710.0f, 838.0f}) {
        g.setColour (juce::Colour(0xff46242d).withAlpha(0.63f));
        g.drawLine(x*sx,159*sy,x*sx,432*sy,1.3f);
        g.setColour(juce::Colour(0xffffcebc).withAlpha(0.40f));
        g.drawLine(x*sx+1.5f,159*sy,x*sx+1.5f,432*sy,0.9f);
    }

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
    g.setFont (uiFont(10.0f * fs));
    g.drawText ("VINTAGE REVERB  /  ORIGINAL DSP", scaled (838, 469, 218, 22, sx, sy),
                juce::Justification::centredRight);
}

void ReverbEditor::resized() {
    const float sx = getWidth() / referenceWidth;
    const float sy = getHeight() / referenceHeight;
    const float s = juce::jlimit (0.74f, 1.35f, std::min (sx, sy));
    presetBox.setBounds (scaled (754, 26, 174, 33, sx, sy));
    bypassButton.setBounds (scaled (938, 26, 138, 33, sx, sy));

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
        labels[i].setFont (uiFont (15.0f * s));
    }
    decayReadout.setBounds (scaled (207, 424, 120, 44, sx, sy));
    decayReadout.setFont (uiFont (23.0f * s));
    parameterReadout.setBounds (scaled (370, 25, 325, 34, sx, sy));
    parameterReadout.setFont (uiFont (14.5f * s));
    rebuildFaceplateCache();
}
} // namespace arplugin
