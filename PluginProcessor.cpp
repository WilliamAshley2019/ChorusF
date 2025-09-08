#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_dsp/juce_dsp.h>

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
    parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

FairlightChorusAudioProcessor::~FairlightChorusAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout FairlightChorusAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Original Fairlight parameters (0-255 range)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 128.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "depth", "Depth",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 40.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "centerDelay", "Center Delay",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 80.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfoRate", "LFO Rate",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 4.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "phaseSpread", "Phase Spread",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "feedback", "Feedback",
        juce::NormalisableRange<float>(0.0f, 255.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "gain", "Gain",
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

void FairlightChorusAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String FairlightChorusAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void FairlightChorusAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void FairlightChorusAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Initialize oversampler for internal 32kHz processing
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumInputChannels(),
        4, // Increased to 4x oversampling for better anti-aliasing
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true); // Use steep filters for better anti-aliasing

    oversampler->initProcessing(static_cast<size_t> (samplesPerBlock));

    // Prepare chorus with the oversampled sample rate
    double oversampledRate = sampleRate * oversampler->getOversamplingFactor();
    chorus.prepare(oversampledRate);

    // Reset chorus effect
    chorus.reset();
}

void FairlightChorusAudioProcessor::releaseResources()
{
    oversampler->reset();
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

void FairlightChorusAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update parameters
    chorus.setParameter(0, static_cast<uint8_t>(*parameters.getRawParameterValue("mix")));
    chorus.setParameter(1, static_cast<uint8_t>(*parameters.getRawParameterValue("depth")));
    chorus.setParameter(2, static_cast<uint8_t>(*parameters.getRawParameterValue("centerDelay")));
    chorus.setParameter(3, static_cast<uint8_t>(*parameters.getRawParameterValue("lfoRate")));
    chorus.setParameter(4, static_cast<uint8_t>(*parameters.getRawParameterValue("phaseSpread")));
    chorus.setParameter(5, static_cast<uint8_t>(*parameters.getRawParameterValue("feedback")));
    chorus.setParameter(6, static_cast<uint8_t>(*parameters.getRawParameterValue("gain")));

    // Upsample to higher rate for better anti-aliasing
    juce::dsp::AudioBlock<float> inputBlock(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(inputBlock);

    // Process each channel
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = oversampledBlock.getChannelPointer(channel);

        for (int sample = 0; sample < oversampledBlock.getNumSamples(); ++sample)
        {
            channelData[sample] = chorus.processSample(channelData[sample]);
        }
    }

    // Downsample back to original sample rate with anti-aliasing filters
    oversampler->processSamplesDown(inputBlock);
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

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FairlightChorusAudioProcessor();
}