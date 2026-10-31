#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"
#include <algorithm>
#include <array>
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

void ReverbProcessor::syncEngineParameters (bool hostBypass) noexcept {
    // All 14 host preset fields should reach one coherent block boundary.
    // If GUI is currently applying a factory preset, keep the previous
    // completed Engine snapshot. No locks, spin, allocation, or waiting.
    const auto epoch = presetEpoch.load (std::memory_order_acquire);
    if ((epoch & 1u) != 0u) return;
    std::array<float, arverb::parameterCount> snapshot{};
    for (std::uint32_t i = 0; i < arverb::parameterCount; ++i)
        snapshot[i] = targets[i] != nullptr
            ? targets[i]->load (std::memory_order_relaxed)
            : arverb::parameterInfo (static_cast<arverb::Parameter> (i)).defaultValue;
    const bool bypass = hostBypass || (bypassTarget != nullptr
                      && bypassTarget->load (std::memory_order_relaxed) >= 0.5f);
    const auto algorithm = algorithmTarget != nullptr
        ? juce::jlimit (0, 3,
            juce::roundToInt (algorithmTarget->load (std::memory_order_relaxed)))
        : 0;
    if (presetEpoch.load (std::memory_order_acquire) != epoch) return;
    if (bypass) snapshot[static_cast<std::size_t> (arverb::Parameter::Mix)] = 0.0f;
    for (std::uint32_t i = 0; i < arverb::parameterCount; ++i)
        engine.setParameter (static_cast<arverb::Parameter> (i), snapshot[i]);
    engine.setAlgorithm (static_cast<arverb::Algorithm> (algorithm));
}

bool ReverbProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    // Vocal mono -> stereo wet is a standard creative-reverb workflow.
    // Hosts that cannot expand a mono track can use mono->mono instead.
    return (in == juce::AudioChannelSet::stereo() &&
            out == juce::AudioChannelSet::stereo()) ||
           (in == juce::AudioChannelSet::mono() &&
            (out == juce::AudioChannelSet::mono() ||
             out == juce::AudioChannelSet::stereo()));
}

void ReverbProcessor::prepareToPlay (double sampleRate, int) {
    ready = false;
    try {
        // Prime from the actual DAW/APVTS state, not internal defaults.
        syncEngineParameters (false);
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
    if (frames <= 0 || !ready ||
        audio.getNumChannels() < getTotalNumOutputChannels()) return;
    const juce::ScopedNoDenormals noDenormals;
    syncEngineParameters (hostBypass);
    const int inputChannels = getTotalNumInputChannels();
    const int outputChannels = getTotalNumOutputChannels();
    if (outputChannels == 2) {
        if (inputChannels == 1) {
            // Mono vocal -> true stereo reverb: duplicate dry input ONCE to
            // the host-provided second output channel, then existing stereo
            // FDN builds independently diffused L/R tails. No heap or locks.
            audio.copyFrom (1, 0, audio, 0, 0, frames);
        }
        auto* l = audio.getWritePointer (0);
        auto* r = audio.getWritePointer (1);
        engine.process (l, r, l, r, static_cast<std::size_t> (frames));
    } else {
        // Mono input -> mono output: dual-mono excitation through the
        // same FDN, then return mid = (left + right)/2. This deliberately discards stereo side
        // energy for mono hosts, avoiding phase-unsafe fake stereo mixing.
        // Fixed bounded stack buffers: no audio callback heap or locks.
        constexpr int chunkSize = 128;
        std::array<float, chunkSize> wetL {};
        std::array<float, chunkSize> wetR {};
        auto* mono = audio.getWritePointer (0);
        for (int offset = 0; offset < frames; offset += chunkSize) {
            const int count = std::min (chunkSize, frames - offset);
            const float* input = mono + offset;
            engine.process (input, input, wetL.data(), wetR.data(),
                            static_cast<std::size_t> (count));
            for (int i = 0; i < count; ++i)
                mono[offset + i] = 0.5f * (wetL[static_cast<std::size_t> (i)]
                                           + wetR[static_cast<std::size_t> (i)]);
        }
    }
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
    // GUI-thread factory transaction. Audio skips partial state without
    // retrying; no work or locks added to the time-critical callback.
    const auto values = arverb::presetValues (preset);
    presetEpoch.fetch_add (1u, std::memory_order_acq_rel);
    for (std::uint32_t i = 0; i < arverb::parameterCount; ++i) {
        const auto p = static_cast<arverb::Parameter> (i);
        if (auto* parameter = parameters.getParameter (arverb::parameterInfo (p).id)) {
            const float normalized = parameter->convertTo0to1 (values[i]);
            // Common preset values should never create superfluous DAW
            // messages, APVTS listeners, or UI invalidations.
            if (std::abs (parameter->getValue() - normalized) < 1.0e-7f)
                continue;
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (normalized);
            parameter->endChangeGesture();
        }
    }
    // Topology last reduces the number of intermediate state combinations.
    if (auto* mode = parameters.getParameter (kAlgorithmId)) {
        const float normalized = mode->convertTo0to1 (
            static_cast<float> (static_cast<unsigned> (arverb::presetAlgorithm (preset))));
        if (std::abs (mode->getValue() - normalized) >= 1.0e-7f) {
            mode->beginChangeGesture();
            mode->setValueNotifyingHost (normalized);
            mode->endChangeGesture();
        }
    }
    presetEpoch.fetch_add (1u, std::memory_order_release);
}

juce::AudioProcessorEditor* ReverbProcessor::createEditor() {
    return new ReverbEditor (*this);
}

} // namespace arplugin

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new arplugin::ReverbProcessor();
}
