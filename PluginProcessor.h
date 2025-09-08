#pragma once

#include <JuceHeader.h>
#include <cstdint>

// Eight-bit emulation utilities
namespace F8 {
    static constexpr float kScale = 1.0f / 128.0f;

    // 64-entry sine table (0-63) exactly like the CMI ROM
    static constexpr int8_t sine[64] = {
        0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 105, 111, 116, 120, 122, 123,
        123, 122, 120, 116, 111, 105, 98, 90, 81, 71, 60, 49, 37, 25, 12, 0,
        -12, -25, -37, -49, -60, -71, -81, -90, -98, -105, -111, -116, -120, -122, -123,
        -123, -122, -120, -116, -111, -105, -98, -90, -81, -71, -60, -49, -37, -25, -12, 0
    };

    // Cheap 8-bit LFO identical to the original
    struct LFO {
        uint8_t phase = 0;
        uint8_t rate = 4;   // 0-255 → 0-6 Hz at 32 kHz

        int8_t tick() {
            phase += rate;
            return sine[phase >> 2];   // use top 6 bits
        }
    };

    // Soft clipper to prevent runaway feedback
    inline float softClip(float x) {
        // Fast tanh approximation to prevent clipping
        x = juce::jlimit(-2.0f, 2.0f, x);
        return x - (x * x * x) / 3.0f;
    }
}

//==============================================================================
/**
*/
class FairlightChorusAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    FairlightChorusAudioProcessor();
    ~FairlightChorusAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Fairlight chorus effect implementation
    class Chorus {
    public:
        void setParameter(int index, uint8_t value) {
            switch (index) {
            case 0: mix = value; break;
            case 1: depth = value; break;
            case 2: centerDelay = value; break;
            case 3: lfoRate = value; break;
            case 4: phaseSpread = value; break;
            case 5: feedback = value; break;
            case 6: gain = value; break;
                // Parameter 7 was unused in the original
            }
        }

        float processSample(float input) {
            // Update LFO
            lfo.rate = lfoRate;
            int8_t lfoValue = lfo.tick();

            // Calculate modulated delay offsets
            float modulation = static_cast<float>(lfoValue) * F8::kScale * depth;
            float tapOffset1 = centerDelay * F8::kScale * 256.0f + modulation;
            float tapOffset2 = centerDelay * F8::kScale * 256.0f;
            float tapOffset3 = centerDelay * F8::kScale * 256.0f - modulation;

            // Read from delay lines with linear interpolation
            float tap1 = readDelayLine(delayLine1, writePos - tapOffset1);
            float tap2 = readDelayLine(delayLine2, writePos - tapOffset2);
            float tap3 = readDelayLine(delayLine3, writePos - tapOffset3);

            // Mix taps
            float wet = (tap1 + tap2 + tap3) * 0.333f;

            // Apply feedback with soft clipping to prevent runaway
            float fbAmount = feedback * F8::kScale * 0.5f; // Reduced feedback range
            delayLine1[writePos] = F8::softClip(input + tap1 * fbAmount);
            delayLine2[writePos] = F8::softClip(input + tap2 * fbAmount);
            delayLine3[writePos] = F8::softClip(input + tap3 * fbAmount);

            // Update write position
            writePos = (writePos + 1) % DELAY_BUFFER_SIZE;

            // Mix wet and dry
            float output = input * (1.0f - mix * F8::kScale) + wet * (mix * F8::kScale);

            // Apply gain with soft clipping
            output = F8::softClip(output * (gain * F8::kScale));

            return output;
        }

        void reset() {
            std::fill_n(delayLine1, DELAY_BUFFER_SIZE, 0.0f);
            std::fill_n(delayLine2, DELAY_BUFFER_SIZE, 0.0f);
            std::fill_n(delayLine3, DELAY_BUFFER_SIZE, 0.0f);
            writePos = 0;
        }

    private:
        static constexpr int DELAY_BUFFER_SIZE = 256; // 256-sample circular buffer
        float delayLine1[DELAY_BUFFER_SIZE] = { 0 };
        float delayLine2[DELAY_BUFFER_SIZE] = { 0 };
        float delayLine3[DELAY_BUFFER_SIZE] = { 0 };
        int writePos = 0;

        F8::LFO lfo;

        // Parameters with defaults
        uint8_t mix = 128;        // 50% mix
        uint8_t depth = 40;       // Moderate modulation
        uint8_t centerDelay = 80; // ~12ms at 32kHz
        uint8_t lfoRate = 4;      // ~0.3Hz
        uint8_t phaseSpread = 0;  // No phase spread
        uint8_t feedback = 0;     // No feedback
        uint8_t gain = 255;       // Full gain

        float readDelayLine(float* buffer, float index) {
            // Handle negative indices
            while (index < 0) index += DELAY_BUFFER_SIZE;

            // Get integer and fractional parts
            int intIndex = static_cast<int>(index) % DELAY_BUFFER_SIZE;
            float frac = index - intIndex;

            // Get next index for interpolation
            int nextIndex = (intIndex + 1) % DELAY_BUFFER_SIZE;

            // Linear interpolation
            return buffer[intIndex] * (1.0f - frac) + buffer[nextIndex] * frac;
        }
    };

    // Audio processor value tree state for parameters
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    //==============================================================================
    Chorus chorus;
    double internalSampleRate = 32000.0; // Fairlight's internal processing rate

    // Oversampling for rate conversion
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FairlightChorusAudioProcessor)
};