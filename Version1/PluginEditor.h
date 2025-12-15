#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FairlightChorusAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    FairlightChorusAudioProcessorEditor(FairlightChorusAudioProcessor&);
    ~FairlightChorusAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FairlightChorusAudioProcessor& processor;

    // Parameter sliders
    juce::Slider mixSlider;
    juce::Slider depthSlider;
    juce::Slider centerDelaySlider;
    juce::Slider lfoRateSlider;
    juce::Slider phaseSpreadSlider;
    juce::Slider feedbackSlider;
    juce::Slider gainSlider;

    // Slider attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> depthAttachment;
    std::unique_ptr<SliderAttachment> centerDelayAttachment;
    std::unique_ptr<SliderAttachment> lfoRateAttachment;
    std::unique_ptr<SliderAttachment> phaseSpreadAttachment;
    std::unique_ptr<SliderAttachment> feedbackAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;

    // Labels
    juce::Label mixLabel;
    juce::Label depthLabel;
    juce::Label centerDelayLabel;
    juce::Label lfoRateLabel;
    juce::Label phaseSpreadLabel;
    juce::Label feedbackLabel;
    juce::Label gainLabel;
    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FairlightChorusAudioProcessorEditor)
};