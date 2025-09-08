#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
FairlightChorusAudioProcessorEditor::FairlightChorusAudioProcessorEditor(FairlightChorusAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(600, 350); // Increased size to fit all controls

    // Title
    titleLabel.setText("Chorus F", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // Mix parameter
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    mixSlider.setRange(0.0, 255.0, 1.0);
    mixSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    mixSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(mixSlider);

    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(mixLabel);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "mix", mixSlider);

    // Depth parameter
    depthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    depthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    depthSlider.setRange(0.0, 255.0, 1.0);
    depthSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    depthSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(depthSlider);

    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.setJustificationType(juce::Justification::centred);
    depthLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(depthLabel);

    depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "depth", depthSlider);

    // Center Delay parameter
    centerDelaySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    centerDelaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    centerDelaySlider.setRange(0.0, 255.0, 1.0);
    centerDelaySlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    centerDelaySlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(centerDelaySlider);

    centerDelayLabel.setText("Delay", juce::dontSendNotification);
    centerDelayLabel.setJustificationType(juce::Justification::centred);
    centerDelayLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(centerDelayLabel);

    centerDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "centerDelay", centerDelaySlider);

    // LFO Rate parameter
    lfoRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lfoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    lfoRateSlider.setRange(0.0, 255.0, 1.0);
    lfoRateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    lfoRateSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(lfoRateSlider);

    lfoRateLabel.setText("LFO Rate", juce::dontSendNotification);
    lfoRateLabel.setJustificationType(juce::Justification::centred);
    lfoRateLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(lfoRateLabel);

    lfoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "lfoRate", lfoRateSlider);

    // Phase Spread parameter
    phaseSpreadSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    phaseSpreadSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    phaseSpreadSlider.setRange(0.0, 255.0, 1.0);
    phaseSpreadSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    phaseSpreadSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(phaseSpreadSlider);

    phaseSpreadLabel.setText("Phase", juce::dontSendNotification);
    phaseSpreadLabel.setJustificationType(juce::Justification::centred);
    phaseSpreadLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(phaseSpreadLabel);

    phaseSpreadAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "phaseSpread", phaseSpreadSlider);

    // Feedback parameter
    feedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    feedbackSlider.setRange(0.0, 255.0, 1.0);
    feedbackSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    feedbackSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(feedbackSlider);

    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.setJustificationType(juce::Justification::centred);
    feedbackLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(feedbackLabel);

    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "feedback", feedbackSlider);

    // Gain parameter
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    gainSlider.setRange(0.0, 255.0, 1.0);
    gainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);
    gainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(gainSlider);

    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(gainLabel);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, "gain", gainSlider);
}

FairlightChorusAudioProcessorEditor::~FairlightChorusAudioProcessorEditor()
{
}

//==============================================================================
void FairlightChorusAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fairlight cream background
    g.fillAll(juce::Colour(0xff404040));

    // Draw monitor bezel
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(20, 20, getWidth() - 40, getHeight() - 40, 10);

    // Draw screen area
    g.setColour(juce::Colour(0xff224422)); // Fairlight green phosphor
    g.fillRect(30, 30, getWidth() - 60, getHeight() - 100);

    // Draw parameter area background
    g.setColour(juce::Colour(0xff333333));
    g.fillRect(30, getHeight() - 70, getWidth() - 60, 50);
}

void FairlightChorusAudioProcessorEditor::resized()
{
    // Title
    titleLabel.setBounds(30, 40, getWidth() - 60, 30);

    // Calculate slider positions
    int sliderWidth = 60;
    int sliderHeight = 80;
    int margin = 15;
    int topY = 80;
    int startX = 40;

    // Position sliders in a row with proper spacing
    mixSlider.setBounds(startX, topY, sliderWidth, sliderHeight);
    mixLabel.setBounds(startX, topY + sliderHeight, sliderWidth, 20);

    depthSlider.setBounds(startX + sliderWidth + margin, topY, sliderWidth, sliderHeight);
    depthLabel.setBounds(startX + sliderWidth + margin, topY + sliderHeight, sliderWidth, 20);

    centerDelaySlider.setBounds(startX + 2 * (sliderWidth + margin), topY, sliderWidth, sliderHeight);
    centerDelayLabel.setBounds(startX + 2 * (sliderWidth + margin), topY + sliderHeight, sliderWidth, 20);

    lfoRateSlider.setBounds(startX + 3 * (sliderWidth + margin), topY, sliderWidth, sliderHeight);
    lfoRateLabel.setBounds(startX + 3 * (sliderWidth + margin), topY + sliderHeight, sliderWidth, 20);

    phaseSpreadSlider.setBounds(startX + 4 * (sliderWidth + margin), topY, sliderWidth, sliderHeight);
    phaseSpreadLabel.setBounds(startX + 4 * (sliderWidth + margin), topY + sliderHeight, sliderWidth, 20);

    feedbackSlider.setBounds(startX + 5 * (sliderWidth + margin), topY, sliderWidth, sliderHeight);
    feedbackLabel.setBounds(startX + 5 * (sliderWidth + margin), topY + sliderHeight, sliderWidth, 20);

    gainSlider.setBounds(startX + 6 * (sliderWidth + margin), topY, sliderWidth, sliderHeight);
    gainLabel.setBounds(startX + 6 * (sliderWidth + margin), topY + sliderHeight, sliderWidth, 20);
}