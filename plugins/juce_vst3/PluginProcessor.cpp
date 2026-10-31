#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"
#include <cmath>
#include <stdexcept>

namespace arplugin {
namespace {
constexpr const char* kBypassId = "bypass";
constexpr const char* kAlgorithmId = "algorithm";
constexpr const char* kStateType = "ARStudioReverbState";
constexpr int kStateVersion = 1;

juce::NormalisableRange<float> rangeFor (arverb::Parameter parameter) {
    const auto& p = arverb::parameterInfo (parameter);
    juce::NormalisableRange<float> range {p.minimum, p.maximum};
    switch (parameter) {
        case arverb::Parameter::DampingHz:
        case arverb::Parameter::LowCutHz:
        case arverb::Parameter::HighCutHz:
            range.setSkewForCentre (std::sqrt (p.minimum * p.maximum));
            break;
        case arverb::Parameter::DecaySeconds:
            range.setSkewForCentre (2.2f);
            break;
        default: break;
    }
    return range;
}
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout ReverbProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout result;
    for (std::uint32_t i = 0; i < arverb::parameterCount; ++i) {
        const auto p = static_cast<arverb::Parameter> (i);
        const auto& info = arverb::parameterInfo (p);
        result.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID {info.id, 1}, info.name,
            rangeFor (p), info.defaultValue,
            juce::AudioParameterFloatAttributes().withLabel (info.unit)));
    }
    result.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID {kBypassId, 1}, "Bypass", false));
    // Append only: all previous 13 IDs and bypass order remain immutable.
    result.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID {kAlgorithmId, 1}, "Algorithm",
        juce::StringArray {"Classic", "Plate", "Chamber", "Hall"}, 0));
    return result;
}

ReverbProcessor::ReverbProcessor()
    : juce::AudioProcessor (BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, kStateType, createParameterLayout()) {
    for (std::uint32_t i = 0; i < arverb::parameterCount; ++i)
        targets[i] = parameters.getRawParameterValue (
            arverb::parameterInfo (static_cast<arverb::Parameter> (i)).id);
    bypassTarget = parameters.getRawParameterValue (kBypassId);
    algorithmTarget = parameters.getRawParameterValue (kAlgorithmId);
    // APVTS owns all parameter objects for the lifetime of this processor.
    jassert (bypassTarget != nullptr && algorithmTarget != nullptr);
}

bool ReverbProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void ReverbProcessor::prepareToPlay (double sampleRate, int) {
    ready = false;
    try {
        engine.prepare (sampleRate);  // May allocate, only in prepareToPlay.
        ready = true;
    } catch (const std::exception&) {
        // Unsupported sample rate: safe dry pass-through until re-prepared.
        jassertfalse;
    }
}

void ReverbProcessor::releaseResources() {
    ready = false;
}

void ReverbProcessor::render (juce::AudioBuffer<float>& audio, bool hostBypass) noexcept {
    const auto frames = audio.getNumSamples();
    if (frames <= 0 || audio.getNumChannels() != 2 || !ready) return;
    const juce::ScopedNoDenormals noDenormals;
    const bool bypass = hostBypass || (bypassTarget != nullptr
                          && bypassTarget->load (std::memory_order_relaxed) >= 0.5f);
    for (std::uint32_t i = 0; i < arverb::parameterCount; ++i) {
        auto value = targets[i] != nullptr
            ? targets[i]->load (std::memory_order_relaxed)
            : arverb::parameterInfo (static_cast<arverb::Parameter> (i)).defaultValue;
        if (i == static_cast<std::uint32_t> (arverb::Parameter::Mix) && bypass)
            value = 0.0f; // Smooth wet ramp; keep processing the tail and input history.
        engine.setParameter (static_cast<arverb::Parameter> (i), value);
    }
    auto* l = audio.getWritePointer (0);
    auto* r = audio.getWritePointer (1);
    if (algorithmTarget != nullptr) {
        const auto index = juce::jlimit (0, 3,
            juce::roundToInt (algorithmTarget->load (std::memory_order_relaxed)));
        engine.setAlgorithm (static_cast<arverb::Algorithm> (index));
    }
    engine.process (l, r, l, r, static_cast<std::size_t> (frames));
}

void ReverbProcessor::processBlock (juce::AudioBuffer<float>& audio, juce::MidiBuffer&) {
    render (audio, false);
}

void ReverbProcessor::processBlockBypassed (juce::AudioBuffer<float>& audio, juce::MidiBuffer&) {
    // JUCE may route hosts' bypass through this callback: preserve reverb state.
    render (audio, true);
}

juce::AudioProcessorParameter* ReverbProcessor::getBypassParameter() const {
    return parameters.getParameter (kBypassId);
}

void ReverbProcessor::getStateInformation (juce::MemoryBlock& destination) {
    // Never called from our processBlock; state serialization may allocate.
    auto state = parameters.copyState();
    state.setProperty ("schemaVersion", kStateVersion, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destination);
}

void ReverbProcessor::setStateInformation (const void* data, int size) {
    if (data == nullptr || size <= 0) return;
    auto xml = getXmlFromBinary (data, size);
    if (xml == nullptr || !xml->hasTagName (kStateType)) return;
    auto tree = juce::ValueTree::fromXml (*xml);
    if (!tree.isValid()) return;
    const auto version = static_cast<int> (tree.getProperty ("schemaVersion", 0));
    if (version > kStateVersion) return; // Never silently misread unknown future states.
    // Sessions saved before P4 do not have an Algorithm parameter.
    // Always restore them as Classic, even if this instance previously
    // had a newly-selected mode; the old reverb must sound unchanged.
    bool hasAlgorithm = false;
    for (int i = 0; i < tree.getNumChildren(); ++i)
        if (tree.getChild (i).getProperty ("id").toString() == kAlgorithmId)
            hasAlgorithm = true;
    parameters.replaceState (tree);
    if (!hasAlgorithm)
        if (auto* mode = parameters.getParameter (kAlgorithmId))
            mode->setValueNotifyingHost (0.0f);
}

void ReverbProcessor::applyPreset (arverb::Preset preset) {
    // Native factory presets now select genuinely distinct feedback topology,
    // while their original 13 numeric values remain unchanged.
    const auto values = arverb::presetValues (preset);
    if (auto* mode = parameters.getParameter (kAlgorithmId)) {
        mode->beginChangeGesture();
        mode->setValueNotifyingHost (mode->convertTo0to1 (
            static_cast<float>(static_cast<unsigned>(arverb::presetAlgorithm(preset)))));
        mode->endChangeGesture();
    }
    for (std::uint32_t i = 0; i < arverb::parameterCount; ++i) {
        const auto p = static_cast<arverb::Parameter> (i);
        if (auto* parameter = parameters.getParameter (arverb::parameterInfo (p).id)) {
            const float target = values[i];
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (target));
            parameter->endChangeGesture();
        }
    }
}

juce::AudioProcessorEditor* ReverbProcessor::createEditor() {
    return new ReverbEditor (*this);
}

} // namespace arplugin

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new arplugin::ReverbProcessor();
}
