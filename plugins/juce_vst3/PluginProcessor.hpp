#pragma once

#include <arverb/Reverb.hpp>
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>

namespace arplugin {

class ReverbProcessor final : public juce::AudioProcessor {
public:
    ReverbProcessor();
    ~ReverbProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "ArSonVerb"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.3; } // Conservative max RT60 + reflections
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;
    juce::AudioProcessorParameter* getBypassParameter() const override;

    // UI-thread operation only. Not called from processBlock.
    void applyPreset (arverb::Preset);
    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void syncEngineParameters (bool hostBypass) noexcept;
    void render (juce::AudioBuffer<float>&, bool hostBypass) noexcept;

    arverb::Engine engine;
    std::array<std::atomic<float>*, arverb::parameterCount> targets{};
    std::atomic<float>* bypassTarget = nullptr;
    std::atomic<float>* algorithmTarget = nullptr;
    bool ready = false; // Audio thread owns after prepareToPlay, no GUI reads

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbProcessor)
};

} // namespace arplugin
