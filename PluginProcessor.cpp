#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FairlightChorusAudioProcessor::FairlightChorusAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, "Parameters", createParameterLayout()),
    oversampler(2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false)
{
}

FairlightChorusAudioProcessor::~FairlightChorusAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout FairlightChorusAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mix", 1), "Mix",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 128.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("depth", 1), "Depth",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 40.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("centerDelay", 1), "Center Delay",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 80.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoRate", 1), "LFO Rate",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 4.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("phaseSpread", 1), "Phase Spread",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("feedback", 1), "Feedback",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gain", 1), "Gain",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 255.0f));

    return layout;
}

//==============================================================================
const juce::String FairlightChorusAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FairlightChorusAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool FairlightChorusAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool FairlightChorusAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double FairlightChorusAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FairlightChorusAudioProcessor::getNumPrograms()
{
    return 1;
}

int FairlightChorusAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FairlightChorusAudioProcessor::setCurrentProgram(int)
{
}

const juce::String FairlightChorusAudioProcessor::getProgramName(int)
{
    return {};
}

void FairlightChorusAudioProcessor::changeProgramName(int, const juce::String&)
{
}

//==============================================================================
void FairlightChorusAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    hostSampleRate = sampleRate;

    // Prepare oversampler for 2x (simulates 32kHz processing on modern systems)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<uint32_t>(samplesPerBlock);
    spec.numChannels = static_cast<uint32_t>(getTotalNumInputChannels());

    oversampler.initProcessing(static_cast<size_t>(samplesPerBlock));

    // Prepare chorus at effective 32kHz (2x oversampling from typical 44.1/48kHz)
    chorus.prepare(sampleRate * 0.75); // Approximate CMI's 32kHz
    chorus.reset();

    silentBlockCount = 0;
}

void FairlightChorusAudioProcessor::releaseResources()
{
    oversampler.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FairlightChorusAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void FairlightChorusAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    if (totalNumInputChannels == 0 || numSamples == 0)
        return;

    // CPU optimization: Silence detection (Melda-style)
    float rmsLeft = buffer.getRMSLevel(0, 0, numSamples);
    float rmsRight = totalNumInputChannels > 1 ? buffer.getRMSLevel(1, 0, numSamples) : 0.0f;

    constexpr float silenceThreshold = 0.0001f; // -80 dB

    if (rmsLeft < silenceThreshold && rmsRight < silenceThreshold)
    {
        silentBlockCount++;
        if (silentBlockCount > kMaxSilentBlocks)
        {
            // Skip processing during extended silence
            return;
        }
    }
    else
    {
        silentBlockCount = 0;
    }

    // Update parameters (once per block, not per sample)
    chorus.setParameter(0, static_cast<uint8_t>(parameters.getRawParameterValue("mix")->load()));
    chorus.setParameter(1, static_cast<uint8_t>(parameters.getRawParameterValue("depth")->load()));
    chorus.setParameter(2, static_cast<uint8_t>(parameters.getRawParameterValue("centerDelay")->load()));
    chorus.setParameter(3, static_cast<uint8_t>(parameters.getRawParameterValue("lfoRate")->load()));
    chorus.setParameter(4, static_cast<uint8_t>(parameters.getRawParameterValue("phaseSpread")->load()));
    chorus.setParameter(5, static_cast<uint8_t>(parameters.getRawParameterValue("feedback")->load()));
    chorus.setParameter(6, static_cast<uint8_t>(parameters.getRawParameterValue("gain")->load()));

    // Create AudioBlock from buffer for oversampling
    juce::dsp::AudioBlock<float> block(buffer);

    // Upsample to simulate 32kHz CMI processing
    auto oversampledBlock = oversampler.processSamplesUp(block);

    // Process each channel at upsampled rate
    for (size_t channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        auto* channelData = oversampledBlock.getChannelPointer(channel);
        auto channelSamples = static_cast<int>(oversampledBlock.getNumSamples());

        // SIMD-optimized block processing
        chorus.processBlock(channelData, channelSamples);
    }

    // Downsample back to host rate
    oversampler.processSamplesDown(block);
}

//==============================================================================
bool FairlightChorusAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FairlightChorusAudioProcessor::createEditor()
{
    return new FairlightChorusAudioProcessorEditor(*this);
}

//==============================================================================
void FairlightChorusAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void FairlightChorusAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FairlightChorusAudioProcessor();
}