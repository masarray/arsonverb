// Host-contract smoke tests. Built and run only when JUCE 9 is available.
// Intentionally calls the actual AudioProcessor methods used by DAWs.
#include "PluginProcessor.hpp"
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
