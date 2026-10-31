// Host-contract smoke tests. Built and run only when JUCE 9 is available.
// Intentionally calls the actual AudioProcessor methods used by DAWs.
#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"
#include <BinaryData.h>
#include <array>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <utility>

namespace {
void check(bool success, const char* detail) {
    if (!success) { std::fprintf(stderr, "PLUGIN HOST TEST FAILURE: %s\n", detail); std::abort(); }
}
void setParameter(arplugin::ReverbProcessor& processor, const char* id, float actual) {
    auto* p = processor.parameters.getParameter(id);
    check(p != nullptr, "expected stable host parameter ID");
    p->setValueNotifyingHost(p->convertTo0to1(actual));
}
void render(arplugin::ReverbProcessor& processor, int samples, bool bypassHost = false) {
    juce::AudioBuffer<float> audio(2, samples);
    juce::MidiBuffer midi;
    audio.clear();
    if (samples > 0) {
        audio.setSample(0, 0, 0.42f);
        audio.setSample(1, 0, -0.21f);
    }
    if (bypassHost) processor.processBlockBypassed(audio, midi);
    else processor.processBlock(audio, midi);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < samples; ++i)
            check(std::isfinite(audio.getSample(ch, i)), "non-finite output");
}
} // namespace

int main() {
    // JUCE editor/value attachments require a valid message manager on the
    // test thread; no actual DAW window is necessary.
    juce::ScopedJuceInitialiser_GUI juceGui;
    // The actual approved transparent PNG is embedded in both plugin + test.
    const auto logo = juce::ImageFileFormat::loadFrom (
        BinaryData::ArSonVerbLogo_png,
        static_cast<std::size_t> (BinaryData::ArSonVerbLogo_pngSize));
    check (logo.isValid() && logo.hasAlphaChannel(),
           "branded transparent PNG must decode at runtime");
    check (logo.getWidth() == 225 && logo.getHeight() == 39,
           "use optimized branded PNG, not a costly full-resolution artwork");
    // Gold knob source: strictly one compact transparent physical asset.
    // The vector pointer and jade progress arc must NOT be baked into the PNG.
    const auto gold = juce::ImageFileFormat::loadFrom (
        BinaryData::ArSonVerbGoldKnob_png,
        static_cast<std::size_t> (BinaryData::ArSonVerbGoldKnob_pngSize));
    check (gold.isValid() && gold.hasAlphaChannel(),
           "gold knob resource must decode with transparency");
    check (gold.getWidth() == 160 && gold.getHeight() == 160,
           "use compact optimized gold knob resource");
    const auto sourceCentre = gold.getPixelAt (80, 80);
    check (gold.getPixelAt (0, 0).getAlpha() == 0 &&
           sourceCentre.getAlpha() >= 225 &&
           sourceCentre.getRed() > sourceCentre.getBlue() + 15 &&
           sourceCentre.getGreen() > sourceCentre.getBlue(),
           "gold PNG source must be transparent outside and warm metal inside");
    // Headless renderer exercises new small + hero large sizes, pointer ends
    // and both extremes without unreliable pixel-alpha sampling.
    arplugin::HardwareLookAndFeel goldLook;
    for (int side : {72, 76, 105, 148, 155, 214}) {
        juce::Image surface (juce::Image::ARGB, side, side, true);
        juce::Graphics canvas (surface);
        juce::Slider control;
        goldLook.drawRotarySlider (canvas, 0, 0, side, side, 0.50f,
            juce::MathConstants<float>::pi * 1.25f,
            juce::MathConstants<float>::pi * 2.75f, control);
        check (surface.isValid(), "gold knob JUCE drawing smoke must succeed");
    }
    // Direct geometry regression: the original bug ignored the slider
    // position and left the pointer fixed at 12 o'clock for every value.
    const float start = juce::MathConstants<float>::pi * 1.25f;
    const float stop = juce::MathConstants<float>::pi * 2.75f;
    const auto angle = [=] (float v) { return arplugin::vintageRotaryAngle (v, start, stop); };
    check (std::abs (angle (0.0f) - start) < 1e-5f, "pointer minimum angle");
    check (std::abs (angle (0.5f) - (start + stop) * 0.5f) < 1e-5f,
           "pointer follows midpoint");
    check (std::abs (angle (1.0f) - stop) < 1e-5f, "pointer maximum angle");
    check (angle (0.25f) < angle (0.75f), "pointer must rotate as value changes");
    check (std::abs (angle (-1.0f) - start) < 1e-5f &&
           std::abs (angle (2.0f) - stop) < 1e-5f,
           "invalid rotary proportions clamp safely");

    check (arplugin::vintageFaceRatio (76.0f) > 0.40f &&
           arplugin::vintageFaceRatio (155.0f) > 0.40f,
           "gold discs must fill the premium reference bezel");
    check (arplugin::vintageGrooveRatio (76.0f) >
           arplugin::vintageArcRatio (76.0f) &&
           arplugin::vintageArcRatio (155.0f) >
           arplugin::vintageGoldVisibleRatio (155.0f),
           "jade arc stays outside the actual gold alpha edge");
    // Real alpha bounds: the 160px source has metal at roughly 85% of
    // the nominal image radius. Compare physical clearances, NOT PNG boxes.
    const auto clearance = [] (float side) {
        return side * (arplugin::vintageArcRatio (side) -
                       arplugin::vintageGoldVisibleRatio (side));
    };
    check (std::abs (clearance (98.0f) - clearance (200.0f)) < 1.25f &&
           std::abs (clearance (73.0f) - clearance (148.0f)) < 1.25f,
           "small and hero gold-to-emerald arc gaps must match at both scales");
    check (arplugin::vintageGoldVisibleRatio (200.0f) >
           arplugin::vintageGoldVisibleRatio (98.0f),
           "Decay must have proportionally larger visible gold face");
    for (float side : {73.0f, 98.0f, 148.0f, 200.0f}) {
        const float visibleGold = side * arplugin::vintageGoldVisibleRatio (side);
        const float ledCentre = side * arplugin::vintageLedPositionRatio (side);
        check (ledCentre < visibleGold * 0.80f &&
               ledCentre + arplugin::vintageLedRadius (side) + 1.4f < visibleGold,
               "LED dot and its dark bezel must remain inset from physical gold rim");
        check (arplugin::vintageLedPositionRatio (side) < 0.344f,
               "LED dot must move inward versus previous rim-mounted design");
    }
    check (arplugin::vintageGoldCacheSlot (105.0f) == 0 &&
           arplugin::vintageGoldCacheSlot (148.0f) == 1,
           "small/hero metal caches must remain separate at all host sizes");
    check (arplugin::vintageLedRadius (155.0f) >
           arplugin::vintageLedRadius (76.0f),
           "hero Decay must use a larger luminous LED dot");


    // Fidelity contracts: normal scroll unchanged; Ctrl+scroll scales both
    // smooth trackpads and stepped wheels without changing orientation.
    juce::MouseWheelDetails wheel {};
    wheel.deltaY = 0.5f;
    wheel.deltaX = -0.125f;
    wheel.isSmooth = true;
    const auto normalWheel = arplugin::refinedWheel (wheel, false);
    const auto preciseWheel = arplugin::refinedWheel (wheel, true);
    check (std::abs (normalWheel.deltaY - 0.5f) < 1e-6f
            && std::abs (preciseWheel.deltaY - 0.0625f) < 1e-6f
            && std::abs (preciseWheel.deltaX + 0.015625f) < 1e-6f
            && preciseWheel.isSmooth == wheel.isSmooth,
           "Ctrl wheel scaling must preserve wheel metadata and normal mode");

    // Exact-value parser: physical display units, frequency shorthand, and
    // strict full-string/range validation (no accidental 0 on invalid text).
    float parsed = -99.0f;
    using Param = arverb::Parameter;
    check (arplugin::parseDisplayValue (Param::Mix, "35%", parsed)
           && std::abs (parsed - 0.35f) < 1e-6f, "percent text entry");
    check (arplugin::parseDisplayValue (Param::DampingHz, "6.3k", parsed)
           && std::abs (parsed - 6300.0f) < 0.001f, "kHz text entry");
    check (arplugin::parseDisplayValue (Param::DecaySeconds, "3.75 s", parsed)
           && std::abs (parsed - 3.75f) < 1e-6f, "seconds text entry");
    for (const auto bad : {"not a number", "nan", "1e999", "-99",
                           "35%garbage", "999999", "1.5.5"}) {
        check (!arplugin::parseDisplayValue (Param::Mix, bad, parsed),
               "invalid/out-of-range percent entries must be rejected");
    }
    check (!arplugin::parseDisplayValue (Param::DampingHz, "2 kittens", parsed)
            && !arplugin::parseDisplayValue (Param::DecaySeconds, "30", parsed),
           "invalid suffix and out-of-range decay must be rejected");

    arplugin::ReverbProcessor p;
    check (p.getLatencySamples() == 0,
           "zero reported plugin latency; device/host buffer latency separate");
    check(p.getNumInputChannels() == 2 && p.getNumOutputChannels() == 2,
          "stereo bus layout");
    check(p.getBypassParameter() != nullptr, "explicit bypass parameter");
    check(p.parameters.getParameter("algorithm") != nullptr,
          "new algorithm parameter appended after legacy IDs");
    check(p.parameters.getRawParameterValue("algorithm")->load() == 0.0f,
          "Classic must be default for pre-P4 project compatibility");
    check(p.getParameters().size() == static_cast<int>(arverb::parameterCount) + 2,
          "number of automatable parameters");

    {
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        check(editor != nullptr && editor->getWidth() == 842 && editor->getHeight() == 394,
              "plugin editor initial bounds");
        editor->setSize(860, 460);
        check(editor->getWidth() == 860 && editor->getHeight() == 460,
              "editor resize without host window");
        // Optional *off-audio-thread* GUI profiling in the existing real-JUCE
        // host test. Exercises the complete editor/13 children at two window
        // sizes, not just the static gold PNG. No assertion on elapsed time:
        // CI runner scheduling and software rasterization are not controlled.
        if (const char* csvPath = std::getenv("ARVERB_GUI_PROFILE_CSV")) {
            std::FILE* report = std::fopen(csvPath, "w");
            check(report != nullptr, "GUI performance report must be writable");
            std::fprintf(report, "width,height,resize_us,paint_mean_us,"
                                 "paint_p95_us,paint_max_us,iterations\n");
            using Clock = std::chrono::steady_clock;
            const auto us = [](Clock::time_point a, Clock::time_point b) {
                return std::chrono::duration<double, std::micro>(b - a).count();
            };
            for (const auto [w, h] : {std::pair<int,int>{842, 394},
                                      {1180, 545}}) {
                const auto resizeStart = Clock::now();
                editor->setSize(w, h);
                const double resizeTime = us(resizeStart, Clock::now());
                juce::Image surface (juce::Image::ARGB, w, h, true);
                juce::Graphics canvas(surface);
                editor->paintEntireComponent(canvas, true); // warm the caches
                std::vector<double> paints;
                paints.reserve(16);
                for (int pass = 0; pass < 16; ++pass) {
                    const auto start = Clock::now();
                    editor->paintEntireComponent(canvas, true);
                    paints.push_back(us(start, Clock::now()));
                }
                const double sum = std::accumulate(paints.begin(), paints.end(), 0.0);
                std::sort(paints.begin(), paints.end());
                std::fprintf(report, "%d,%d,%.3f,%.3f,%.3f,%.3f,%zu\n",
                    w, h, resizeTime, sum / paints.size(),
                    paints[15], paints.back(), paints.size());
            }
            std::fclose(report);
            editor->setSize(860, 460);
        }

        // GUI regression: all interactive controls stay on the visible panel
        // at minimum, default, and DAW-resized dimensions.
        for (const auto [width, height] : {std::pair<int,int>{800, 370},
                                           {842, 394}, {1000, 470}, {1180, 545}}) {
            editor->setSize(width, height);
            int toggleCount = 0;
            int sliderCount = 0;
            for (int index = 0; index < editor->getNumChildComponents(); ++index) {
                const auto* child = editor->getChildComponent(index);
                if (child->isVisible())
                    check(editor->getLocalBounds().contains(child->getBounds()),
                          "visible GUI control outside editor bounds");
                if (const auto* toggle = dynamic_cast<const juce::ToggleButton*>(child)) {
                    ++toggleCount;
                    check(toggle->getWidth() >= 95,
                          "BTN180-style BYPASSED text must fit at minimum editor size");
                }
                if (const auto* slider = dynamic_cast<const juce::Slider*>(child)) {
                    ++sliderCount;
                    check(slider->isScrollWheelEnabled()
                            && dynamic_cast<const arplugin::PrecisionRotarySlider*>(slider) != nullptr
                            && slider->getMouseDragSensitivity() == 280,
                          "all 13 knobs support fine Ctrl-wheel and predictable drag");
                }
            }
            check(toggleCount == 1 && sliderCount == static_cast<int>(arverb::parameterCount),
                  "editor must retain bypass and all 13 DSP sliders");
            // Independent GUI geometry, including small 800x370 plugin host:
            // the active header name must have a reproducible inset.
            bool headerNameFound = false;
            for (int child = 0; child < editor->getNumChildComponents(); ++child)
                if (auto* label = dynamic_cast<juce::Label*> (
                        editor->getChildComponent (child)))
                    if (label->getText() == "Mix") {
                        check (label->getX() >= juce::roundToInt (
                            371.0f * width / 1100.0f) - 1,
                            "active Mix name padded from dark LCD left edge");
                        headerNameFound = true;
                    }
            check (headerNameFound, "active parameter header name exists");
            auto* exactDisplay = dynamic_cast<juce::Label*> (
                editor->findChildWithID ("active-parameter-value"));
            check (exactDisplay && exactDisplay->isEditableOnDoubleClick()
                   && editor->getLocalBounds().contains (exactDisplay->getBounds()),
                   "active parameter LCD value must be directly editable at all sizes");
            // A focused visual geometry contract: hero dial dominant, small
            // dials comfortably sized, and fully inside their hardware modules.
            const juce::Slider* hero = nullptr;
            int fullSizedSmall = 0;
            for (int n=0; n<editor->getNumChildComponents(); ++n)
                if (const auto* slider = dynamic_cast<const juce::Slider*> (
                        editor->getChildComponent(n))) {
                    if (hero == nullptr || slider->getWidth() > hero->getWidth())
                        hero = slider;
                    if (slider->getWidth() >= static_cast<int>(width * 0.065))
                        ++fullSizedSmall;
                }
            check (hero != nullptr &&
                   hero->getWidth() >= static_cast<int> (width * 0.12f) &&
                   fullSizedSmall == static_cast<int> (arverb::parameterCount),
                   "all twelve small knobs and hero Decay must scale with window");
            // LCD lower border and PRE-DELAY caption lower border must be
            // optically co-linear at each supported host/editor size.
            const juce::Label* preDelayLabel = nullptr;
            const juce::Label* decayValueLabel = nullptr;
            for (int n = 0; n < editor->getNumChildComponents(); ++n) {
                if (auto* label = dynamic_cast<const juce::Label*> (
                    editor->getChildComponent(n))) {
                    if (label->getText() == "PRE-DELAY") preDelayLabel = label;
                    if (label->isVisible() && label->getText().endsWith(" s"))
                        decayValueLabel = label;
                }
            }
            check (preDelayLabel && decayValueLabel,
                   "PRE-DELAY caption and Decay LCD label must exist");
            check (std::abs (preDelayLabel->getBottom() - decayValueLabel->getBottom()) <= 1,
                   "Decay LCD bottom must track PRE-DELAY caption at all sizes");
            int factoryCount = 0;
            for (int n = 0; n < editor->getNumChildComponents(); ++n)
                if (auto* combo = dynamic_cast<juce::ComboBox*> (
                        editor->getChildComponent (n))) {
                    if (combo->getComponentID() == "algorithm-mode") {
                        check (combo->getNumItems() == 4, "four real mode choices visible");
                        combo->setSelectedId (2, juce::sendNotificationSync); // Plate
                        check (juce::roundToInt (
                            p.parameters.getRawParameterValue ("algorithm")->load()) == 1,
                            "native mode selector must automate algorithm parameter");
                        // Restore Classic before the user chooses a factory
                        // program. The preset must subsequently choose Hall.
                        combo->setSelectedId (1, juce::sendNotificationSync);
                        continue;
                    }
                    factoryCount += combo->getNumItems();
                    // Test actual menu wiring, not merely Processor::applyPreset.
                    combo->setSelectedId (static_cast<int> (arverb::presetCount),
                                          juce::sendNotificationSync);
                    check (combo->getSelectedId() == static_cast<int> (arverb::presetCount),
                           "last factory program must be selectable in GUI");
                    const auto actualDecay =
                        p.parameters.getRawParameterValue ("decay")->load();
                    check (std::fabs (actualDecay - 10.8f) < 0.03f,
                           "dropdown selection must apply actual parameter values");
                    // A real edit via the GUI (including wheel/double-click)
                    // must turn the preset display to CUSTOM, then correctly
                    // re-select the factory program if all 13 values match.
                    juce::Slider* mixControl = nullptr;
                    for (int childIndex = 0; childIndex < editor->getNumChildComponents(); ++childIndex)
                        if (auto* s = dynamic_cast<juce::Slider*> (
                                editor->getChildComponent (childIndex)))
                            if (s->getTooltip() == arverb::parameterInfo (
                                    arverb::Parameter::Mix).name)
                                mixControl = s;
                    check (mixControl != nullptr, "mix GUI control remains addressable");
                    const double originalMix = mixControl->getValue();
                    mixControl->setValue (originalMix < 0.75 ? originalMix + 0.14 :
                                                               originalMix - 0.14,
                                          juce::sendNotificationSync);
                    check (combo->getSelectedId() == 0,
                           "knob changes must not leave a stale factory preset selected");
                    mixControl->setValue (originalMix, juce::sendNotificationSync);
                    check (combo->getSelectedId() == static_cast<int> (arverb::presetCount),
                           "preset recognition must recover after value restoration");
                    // Simulate JUCE's native double-click TextEditor Enter
                    // commit: valid typed display percentage is applied to
                    // APVTS; invalid text cannot corrupt the current value.
                    exactDisplay->showEditor();
                    check (exactDisplay->getCurrentTextEditor() != nullptr,
                           "double-click LCD must open native TextEditor");
                    exactDisplay->getCurrentTextEditor()->setText ("35", false);
                    exactDisplay->hideEditor (false);
                    check (std::abs (mixControl->getValue() - 0.35) < 0.0001
                           && combo->getSelectedId() == 0,
                           "exact typed value updates the knob and invalidates factory preset");
                    exactDisplay->showEditor();
                    exactDisplay->getCurrentTextEditor()->setText ("garbage", false);
                    exactDisplay->hideEditor (false);
                    check (std::abs (mixControl->getValue() - 0.35) < 0.0001,
                           "invalid typed value must not alter host parameter");
                    combo->setSelectedId (static_cast<int> (arverb::presetCount),
                                          juce::sendNotificationSync);
                    check (combo->getSelectedId() == static_cast<int> (arverb::presetCount),
                           "factory preset remains selectable after inline text edits");
                }
            check (factoryCount == static_cast<int> (arverb::presetCount),
                   "all 32 real factory programs must be selectable");
            // Render every native JUCE control in a headless offscreen image.
            juce::Image image (juce::Image::ARGB, width, height, true);
            juce::Graphics graphics (image);
            // Standalone JUCE host tests do not attach this editor to a
            // desktop peer. paintEntireComponent() may skip hidden children;
            // explicitly paint the parent to validate its cached faceplate.
            editor->paint (graphics);
            editor->paintEntireComponent (graphics, true);
            // Headless Windows runners do not guarantee stable alpha contents
            // for unattached native components. The paint calls above are smoke
            // tests (no crash); geometry and automation remain strict assertions.
            check(image.isValid(), "offscreen editor image allocation");

        }
    }
    for (std::uint32_t i = 0; i < arverb::parameterCount; ++i)
        check(p.parameters.getParameter(arverb::parameterInfo(
            static_cast<arverb::Parameter>(i)).id) != nullptr,
            "stable DSP ID missing after GUI redesign");
    p.prepareToPlay(48000.0, 256);
    // Verify UI/processor use the same catalog for every host-facing parameter.
    p.applyPreset (arverb::Preset::SlowCinematicWash);
    const auto lastPreset = arverb::presetValues (arverb::Preset::SlowCinematicWash);
    for (std::size_t i = 0; i < arverb::parameterCount; ++i) {
        const auto& meta = arverb::parameterInfo (static_cast<arverb::Parameter> (i));
        const auto* apvts = p.parameters.getParameter (meta.id);
        check (apvts != nullptr, "new program host parameter missing");
        check (std::fabs (apvts->convertFrom0to1 (apvts->getValue()) - lastPreset[i])
                <= (meta.maximum - meta.minimum) * 0.002f,
               "32-program selection must update each of 13 DAW parameters");
    }
    p.applyPreset(arverb::Preset::VocalPlate);
    setParameter(p, "decay", 3.4f);
    setParameter(p, "lowcut", 320.0f);
    setParameter(p, "mix", 0.7f);
    render(p, 256);
    render(p, 17);
    render(p, 1);
    render(p, 256, true); // DAW host bypass path must remain finite.
    // Repeated non-power-of-two and zero-frame host blocks after automation.
    for (int frames : {0, 1, 17, 127, 256, 511, 1024}) render(p, frames);
    setParameter(p, "bypass", 1.0f);
    render(p, 256); // Plugin automatable bypass path.

    // DAW automation of topology must survive state save/restore.
    setParameter(p, "algorithm", 2.0f); // Chamber
    render(p, 256);
    juce::MemoryBlock saved;
    p.getStateInformation(saved);
    check(saved.getSize() > 0, "serialized session must not be empty");
    arplugin::ReverbProcessor restored;
    restored.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    for (const char* id : {"decay", "lowcut", "mix", "bypass", "algorithm"}) {
        auto* before = p.parameters.getParameter(id);
        auto* after  = restored.parameters.getParameter(id);
        check(before && after, "state parameter missing");
        check(std::fabs(before->getValue() - after->getValue()) < 1e-5f,
              "DAW state roundtrip mismatch");
    }
    // Actual pre-P4 XML chunks had no algorithm child. Rebuild an old chunk
    // from a modern one after removing only the new mode row. A previously
    // selected modern mode must not leak into a restored legacy project.
    auto legacyXml = p.getXmlFromBinary(saved.getData(), static_cast<int>(saved.getSize()));
    check(legacyXml != nullptr, "state XML readable");
    for (int i=legacyXml->getNumChildElements()-1; i>=0; --i) {
        auto* child=legacyXml->getChildElement(i);
        if (child->getStringAttribute("id") == "algorithm")
            legacyXml->removeChildElement(child, true);
    }
    juce::MemoryBlock legacyChunk;
    p.copyXmlToBinary(*legacyXml, legacyChunk);
    restored.setStateInformation(legacyChunk.getData(), static_cast<int>(legacyChunk.getSize()));
    check(restored.parameters.getRawParameterValue("algorithm")->load() == 0.0f,
          "legacy state explicitly restores Classic mode");
        // Loading corrupt chunks must not crash or corrupt valid parameters.
    constexpr std::array<char, 6> corrupted{{'b','a','d','\0','\x7f','z'}};
    restored.setStateInformation(corrupted.data(), static_cast<int>(corrupted.size()));
    check(std::fabs(restored.parameters.getParameter("decay")->getValue() -
                    p.parameters.getParameter("decay")->getValue()) < 1e-5f,
          "corrupted state should be ignored");
    restored.prepareToPlay(96000.0, 512);
    render(restored, 512);
    restored.releaseResources();
    p.releaseResources();
    std::puts("PASS: JUCE host contract, audio, automation, bypass, state roundtrip, corrupt chunk");
}
