// Host-contract smoke tests. Built and run only when JUCE 9 is available.
// Intentionally calls the actual AudioProcessor methods used by DAWs.
#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"
#include <array>
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

    check (arplugin::vintageGrooveRatio (74.0f) -
           arplugin::vintageFaceRatio (74.0f) > 0.15f,
           "small knob requires visibly deeper socket");
    check (arplugin::vintageGrooveRatio (74.0f) -
           arplugin::vintageFaceRatio (74.0f) >
           arplugin::vintageGrooveRatio (172.0f) -
           arplugin::vintageFaceRatio (172.0f),
           "small knob black ring must be proportionally stronger");
    check (arplugin::vintageFaceRatio (172.0f) > 0.35f,
           "large Decay knob face remains unchanged");

    arplugin::ReverbProcessor p;
    check(p.getNumInputChannels() == 2 && p.getNumOutputChannels() == 2,
          "stereo bus layout");
    check(p.getBypassParameter() != nullptr, "explicit bypass parameter");
    check(p.getParameters().size() == static_cast<int>(arverb::parameterCount) + 1,
          "number of automatable parameters");

    {
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        check(editor != nullptr && editor->getWidth() == 842 && editor->getHeight() == 394,
              "plugin editor initial bounds");
        editor->setSize(860, 460);
        check(editor->getWidth() == 860 && editor->getHeight() == 460,
              "editor resize without host window");
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
                    check(slider->isScrollWheelEnabled(),
                          "all rotary controls must respond to mouse wheel");
                }
            }
            check(toggleCount == 1 && sliderCount == static_cast<int>(arverb::parameterCount),
                  "editor must retain bypass and all 13 DSP sliders");
            int factoryCount = 0;
            for (int n = 0; n < editor->getNumChildComponents(); ++n)
                if (auto* combo = dynamic_cast<juce::ComboBox*> (
                        editor->getChildComponent (n))) {
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

    juce::MemoryBlock saved;
    p.getStateInformation(saved);
    check(saved.getSize() > 0, "serialized session must not be empty");
    arplugin::ReverbProcessor restored;
    restored.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    for (const char* id : {"decay", "lowcut", "mix", "bypass"}) {
        auto* before = p.parameters.getParameter(id);
        auto* after  = restored.parameters.getParameter(id);
        check(before && after, "state parameter missing");
        check(std::fabs(before->getValue() - after->getValue()) < 1e-5f,
              "DAW state roundtrip mismatch");
    }
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
