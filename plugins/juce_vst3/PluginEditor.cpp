#include "PluginEditor.hpp"
#include <BinaryData.h>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace arplugin {
namespace {
const juce::Colour ink (0xff26191d);
const juce::Colour mutedInk (0xff57323a);
const juce::Colour highlight (0xfff3afb7);
const juce::Colour displayGreen (0xffb3f1b9);
const juce::Colour ice (0xffd9faff);
const juce::Colour amber (0xffffac52);
const juce::Colour ledAmber (0xffffd086);
juce::Typeface::Ptr interTypeface() {
    static const auto face = juce::Typeface::createSystemTypefaceFor (
        BinaryData::InterVariable_ttf, BinaryData::InterVariable_ttfSize);
    return face;
}
juce::Font uiFont (float size) {
    return juce::Font (juce::FontOptions (size).withTypeface (interTypeface()));
}
juce::Font uiHeavy (float size) { return uiFont (size).boldened(); }
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
    {arverb::Parameter::Mix,          92, 220, 74, "MIX"},
    {arverb::Parameter::PreDelayMs,   92, 375, 74, "PRE-DELAY"},
    {arverb::Parameter::DecaySeconds, 266, 248, 172, "DECAY"},
    {arverb::Parameter::Size,         768, 214, 72, "SIZE"},
    {arverb::Parameter::Diffusion,    768, 369, 72, "DIFFUSION"},
    {arverb::Parameter::DampingHz,    514, 214, 72, "HF DAMP"},
    {arverb::Parameter::LowCutHz,     514, 369, 72, "LOW CUT"},
    {arverb::Parameter::HighCutHz,    622, 214, 72, "HIGH CUT"},
    {arverb::Parameter::ModDepthMs,  1016, 369, 72, "DEPTH"},
    {arverb::Parameter::ModRateHz,    904, 369, 72, "RATE"},
    {arverb::Parameter::Width,       1016, 214, 72, "WIDTH"},
    {arverb::Parameter::EarlyLevel,   904, 214, 72, "EARLY"},
    {arverb::Parameter::DuckAmount,   622, 369, 72, "DUCK"}
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
     int x, int y, int width, int height, float position, float, float,
     juce::Slider& slider) {
    const float side = float (std::min (width,height));
    const float cx=x+width*0.5f,cy=y+height*0.5f;
    const float ringR=side*0.465f,faceR=side*0.356f;
    const auto ring=juce::Rectangle<float>(cx-ringR,cy-ringR,2*ringR,2*ringR);
    g.setColour(juce::Colour(0xff916e65));g.fillEllipse(ring);
    g.setColour(juce::Colour(0xff211a1e));g.fillEllipse(ring.reduced(1.3f));
    g.setColour(juce::Colours::white.withAlpha(0.23f));
    g.drawEllipse(ring.reduced(0.7f),std::max(0.9f,side*0.012f));
    g.setColour(juce::Colours::black.withAlpha(0.78f));
    g.drawEllipse(ring.reduced(2.5f),std::max(1.2f,side*0.024f));
    // Discrete warm lamps reproduce the instrument's level-indicating bezel.
    // Unlike a moving cyan LED, a single fixed marker stays at 12 o'clock.
    const int total=side>=110.0f?29:19;
    const int litCount=juce::jlimit(0,total,
        int(std::round(juce::jlimit(0.0f,1.0f,position)*total)));
    const float r0=side*0.411f,r1=side*0.441f;
    for(int i=0;i<total;++i) {
        const float theta=-2.38f+4.76f*(float(i)+0.5f)/float(total);
        const float si=std::sin(theta),co=std::cos(theta);
        const bool lit=slider.isEnabled()&&i<litCount;
        g.setColour(lit?amber.withAlpha(0.37f):juce::Colour(0xff514449));
        g.drawLine(cx+si*(r0-0.7f),cy-co*(r0-0.7f),
                   cx+si*(r1+0.7f),cy-co*(r1+0.7f),
                   std::max(1.1f,side*0.023f));
        if(lit) {
            g.setColour(ledAmber);
            g.drawLine(cx+si*r0,cy-co*r0,cx+si*r1,cy-co*r1,
                       std::max(0.8f,side*0.014f));
        }
    }
    const auto disk=juce::Rectangle<float>(cx-faceR,cy-faceR,2*faceR,2*faceR);
    g.setColour(juce::Colours::black.withAlpha(0.53f));
    g.fillEllipse(disk.translated(0,side*0.025f).expanded(1.3f));
    g.setColour(juce::Colour(0xff676063));g.fillEllipse(disk.expanded(1.4f));
    g.setColour(juce::Colour(0xff151416));g.fillEllipse(disk.expanded(0.5f));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff333235),
       cx-faceR*0.65f,cy-faceR*0.83f,
       juce::Colour(0xff151416),cx+faceR*0.8f,cy+faceR*0.91f,false));
    g.fillEllipse(disk.reduced(1.4f));
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawEllipse(disk.reduced(2.3f),0.8f);
    const float length=std::max(4.8f,side*0.075f);
    const float thick=std::max(2.0f,side*0.022f);
    const float top=cy-faceR*0.73f;
    if(slider.isEnabled()) {
        g.setColour(amber.withAlpha(0.18f));
        g.fillRoundedRectangle(cx-thick*1.25f,top-1.5f,
                                thick*2.5f,length+3.0f,thick);
    }
    g.setColour(slider.isEnabled()?juce::Colour(0xffffe6bd):juce::Colour(0xff80787a));
    g.fillRoundedRectangle(cx-thick*0.5f,top,thick,length,thick*0.37f);
}

void HardwareLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h,
                                         bool, int, int, int, int, juce::ComboBox&) {
    const auto rect = juce::Rectangle<float>(0,0,float(w),float(h));
    g.setColour (juce::Colour(0xff04090d)); g.fillRoundedRectangle(rect,4.0f);
    g.setGradientFill (juce::ColourGradient(juce::Colour(0xff233039),0,0,
                                            juce::Colour(0xff090e13),0,float(h),false));
    g.fillRoundedRectangle(rect.reduced(1.1f),3.5f);
    g.setColour(amber.withAlpha(0.35f)); g.drawRoundedRectangle(rect.reduced(0.6f),3.6f,1.0f);
    g.setColour(ice);
    const float cx=w-15.0f,cy=h*0.5f;
    g.drawLine(cx-4,cy-2,cx,cy+2,1.3f);g.drawLine(cx,cy+2,cx+4,cy-2,1.3f);
}
juce::Font HardwareLookAndFeel::getComboBoxFont (juce::ComboBox&) {
    return uiHeavy(14.0f);
}
juce::Font HardwareLookAndFeel::getPopupMenuFont() {
    return uiHeavy(14.0f);
}
void HardwareLookAndFeel::drawPopupMenuBackground(juce::Graphics& g,int w,int h) {
    g.fillAll(juce::Colour(0xff080e13));
    g.setColour(amber.withAlpha(0.50f));g.drawRect(0,0,w,h,1);
}
void HardwareLookAndFeel::drawTooltip(juce::Graphics& g,const juce::String& value,
                                     int w,int h) {
    g.fillAll(juce::Colour(0xff090f14));
    g.setColour(amber);g.drawRect(0,0,w,h,1);
    g.setColour(ice);g.setFont(uiHeavy(13.0f));
    g.drawFittedText(value,7,4,w-14,h-8,juce::Justification::centredLeft,2);
}
void HardwareLookAndFeel::drawToggleButton (
    juce::Graphics& g,juce::ToggleButton& button,bool hovered,bool pressed) {
    const bool on=!button.getToggleState(); // APVTS bypass=true -> power off
    const auto shell=button.getLocalBounds().toFloat().reduced(1.0f);
    const auto cap=shell.reduced(1.5f).translated(0.0f,pressed?1.0f:0.0f);
    g.setColour(juce::Colour(0xff181619));g.fillRoundedRectangle(shell,5.0f);
    g.setColour(juce::Colour(0xffb09d90).withAlpha(0.70f));
    g.drawRoundedRectangle(shell.reduced(0.6f),4.7f,1.0f);
    g.setGradientFill(juce::ColourGradient(
         juce::Colour(0xff53484a),cap.getX(),cap.getY(),
         juce::Colour(0xff1b1b1d),cap.getRight(),cap.getBottom(),false));
    g.fillRoundedRectangle(cap,3.5f);
    g.setColour(juce::Colours::white.withAlpha(hovered?0.24f:0.13f));
    g.drawLine(cap.getX()+4,cap.getY()+1.3f,cap.getRight()-4,cap.getY()+1.3f,0.85f);
    // Classic illuminated physical power key, not a generic light-green switch.
    const float cx=cap.getX()+18.0f,cy=cap.getCentreY();
    const float radius=std::min(8.7f,cap.getHeight()*0.29f);
    juce::Path power;
    power.addCentredArc(cx,cy,radius,radius,0.0f,0.62f,5.66f,true);
    power.startNewSubPath(cx,cy-radius*1.12f);
    power.lineTo(cx,cy-radius*0.10f);
    if(on) {
        g.setColour(amber.withAlpha(0.25f));
        g.strokePath(power,juce::PathStrokeType(4.5f));
    }
    g.setColour(on?juce::Colour(0xffffd8a2):juce::Colour(0xff877c79));
    g.strokePath(power,juce::PathStrokeType(1.9f));
    g.setColour(on?juce::Colour(0xffffe7cb):juce::Colour(0xffc7bbba));
    g.setFont(uiHeavy(13.0f));
    g.drawFittedText(on?"ACTIVE":"BYPASSED",
        button.getLocalBounds().withTrimmedLeft(36).withTrimmedRight(5),
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
                     juce::Colour (0xff55412e));
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
        label.setColour (juce::Label::textColourId, juce::Colour(0xff1e1518));
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

    // Framed modular sections, mirroring the clean grouped hardware grammar
    // of the reference rather than the previous awkward vertical dividers.
    // Static geometry is rendered into the faceplate cache only upon resize.
    const float sx=w/referenceWidth, sy=h/referenceHeight;
    for (const auto& m : groupPanels) {
        const auto box=scaled(m.x,m.y,m.w,m.h,sx,sy).toFloat();
        g.setColour (juce::Colour (0xff38242b).withAlpha (0.26f));
        g.fillRoundedRectangle(box.translated(0.0f,1.5f),4.0f);
        g.setColour (juce::Colour (0xffffe2d4).withAlpha (0.52f));
        g.fillRoundedRectangle(box.reduced(1.2f),3.5f);
        g.setColour (juce::Colour (0xff422029).withAlpha (0.84f));
        g.drawRoundedRectangle(box.reduced(0.5f),4.0f,1.2f);
        g.setColour (juce::Colour (0xffffdfca).withAlpha (0.42f));
        g.drawRoundedRectangle(box.reduced(1.7f),3.2f,0.8f);
        const float header=(m.y+39.0f)*sy;
        g.setColour (juce::Colour (0xff482c34).withAlpha (0.40f));
        g.drawLine(box.getX()+3.8f,header,box.getRight()-3.8f,header,0.95f);
    }
}

void ReverbEditor::paint (juce::Graphics& g) {
    const float sx = getWidth() / referenceWidth;
    const float sy = getHeight() / referenceHeight;
    const float fs = juce::jlimit (0.74f, 1.35f, std::min (sx, sy));
    g.fillAll (juce::Colour (0xff241d20));
    if (faceplateCache.isValid()) g.drawImageAt (faceplateCache, 0, 0);

    // Inter typography with deliberate weight and contrast at the compact size.
    // A single wordmark prevents accidental spacing between ArSon / Verb.
    g.setColour (juce::Colour (0xff26191d));
    g.setFont (uiHeavy (33.0f * fs));
    g.drawText ("ArSonVerb", scaled (25, 20, 270, 38, sx, sy),
                juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xff47303a));
    g.setFont (uiHeavy (13.6f * fs));
    g.drawText ("MUSICAL VINTAGE REVERB",
        scaled (26, 63, 310, 20, sx, sy),
        juce::Justification::centredLeft);

    // Printed machine-engraved group captions inside the framed modules.
    // None of these are extra JUCE components or click targets.
    for (const auto& m : groupPanels) {
        g.setColour (juce::Colour (0xff301d22));
        g.setFont (uiHeavy (15.7f * fs));
        g.drawText (m.title,
            scaled (m.x+7, m.y+8, m.w-14, 24, sx, sy),
            juce::Justification::centred);
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

    // The reference uses framed modules without ornament at their base;
    // omit the previous decorative waveform/footer that collided with frames.

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
        const float captionTop = dial.centreY * sy + side * 0.5f + 7.0f * s;
        labels[i].setBounds (juce::roundToInt (dial.centreX * sx - 55.0f * sx),
                             juce::roundToInt (captionTop),
                             juce::roundToInt (110.0f * sx), juce::roundToInt (21.0f * s));
        labels[i].setFont (uiHeavy (17.6f * s));
    }
    decayReadout.setBounds (scaled (207, 424, 120, 44, sx, sy));
    decayReadout.setFont (uiHeavy (23.0f * s));
    parameterReadout.setBounds (scaled (370, 25, 325, 34, sx, sy));
    parameterReadout.setFont (uiHeavy (14.5f * s));
    rebuildFaceplateCache();
}
} // namespace arplugin
