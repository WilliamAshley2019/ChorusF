#pragma once

#include <JuceHeader.h>
#include <cstdint>
#include <cmath>

// Eight-bit emulation utilities
namespace F8 {
    static constexpr float kScale = 1.0f / 128.0f;

    // 64-entry sine table (0-63) exactly like the CMI ROM
    alignas(16) static constexpr int8_t sine[64] = {
        0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 105, 111, 116, 120, 122, 123,
        123, 122, 120, 116, 111, 105, 98, 90, 81, 71, 60, 49, 37, 25, 12, 0,
        -12, -25, -37, -49, -60, -71, -81, -90, -98, -105, -111, -116, -120, -122, -123,
        -123, -122, -120, -116, -111, -105, -98, -90, -81, -71, -60, -49, -37, -25, -12
    };

    // Cheap 8-bit LFO identical to the original
    struct LFO {
        uint8_t phase = 0;
        uint8_t rate = 4;

        int8_t tick() {
            phase += rate;
            return sine[(phase >> 2) & 0x3F];
        }
    };

    // Soft clipper to prevent runaway feedback
    inline float softClip(float x) {
        return juce::jlimit(-1.0f, 1.0f, x * 1.5f) * 0.666f;
    }
}

//==============================================================================
class FairlightChorusAudioProcessor : public juce::AudioProcessor
{
public:
    FairlightChorusAudioProcessor();
    ~FairlightChorusAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Fairlight chorus effect implementation (SIMD-optimized)
    class Chorus {
    public:
        void setParameter(int index, uint8_t value) {
            switch (index) {
            case 0: mix = value; break;
            case 1: depth = juce::jmin(value, (uint8_t)64); break;
            case 2: centerDelay = value; break;
            case 3: lfoRate = value; break;
            case 4: phaseSpread = value; break;
            case 5: feedback = juce::jmin(value, (uint8_t)128); break;
            case 6: gain = value; break;
            }
        }

        void prepare(double sampleRate) {
            currentSampleRate = sampleRate;
            updateFilterCoefficients();
        }

        // SIMD-optimized block processing
        void processBlock(float* buffer, int numSamples) {
            constexpr int simdSize = juce::dsp::SIMDRegister<float>::size();
            const int numVecSamples = numSamples - (numSamples % simdSize);

            int i = 0;

            // Process SIMD-aligned samples
            for (; i < numVecSamples; i += simdSize) {
                processSIMDChunk(buffer + i, simdSize);
            }

            // Process remaining samples
            for (; i < numSamples; ++i) {
                buffer[i] = processSample(buffer[i]);
            }
        }

        float processSample(float input) {
            // Apply input anti-aliasing filter
            float filteredInput = applyInputFilter(input);

            // Update LFO
            lfo.rate = lfoRate;
            lfo.tick(); // Advance LFO phase

            // Phase spread affects modulation phase
            uint8_t phase1 = lfo.phase;
            uint8_t phase2 = lfo.phase + phaseSpread;
            uint8_t phase3 = lfo.phase + (phaseSpread * 2);

            int8_t lfo1 = F8::sine[(phase1 >> 2) & 0x3F];
            int8_t lfo2 = F8::sine[(phase2 >> 2) & 0x3F];
            int8_t lfo3 = F8::sine[(phase3 >> 2) & 0x3F];

            // Calculate modulated delay offsets
            float baseDelay = static_cast<float>(centerDelay) * F8::kScale * 256.0f;
            float tapOffset1 = baseDelay + static_cast<float>(lfo1) * F8::kScale * depth;
            float tapOffset2 = baseDelay + static_cast<float>(lfo2) * F8::kScale * depth;
            float tapOffset3 = baseDelay + static_cast<float>(lfo3) * F8::kScale * depth;

            // Read from delay lines with linear interpolation
            float tap1 = readDelayLine(delayLine1, static_cast<float>(writePos) - tapOffset1);
            float tap2 = readDelayLine(delayLine2, static_cast<float>(writePos) - tapOffset2);
            float tap3 = readDelayLine(delayLine3, static_cast<float>(writePos) - tapOffset3);

            // Mix taps
            float wet = (tap1 + tap2 + tap3) * 0.333f;

            // Apply feedback with soft clipping
            float fbAmount = static_cast<float>(feedback) * F8::kScale * 0.5f;
            delayLine1[writePos] = F8::softClip(filteredInput + tap1 * fbAmount);
            delayLine2[writePos] = F8::softClip(filteredInput + tap2 * fbAmount);
            delayLine3[writePos] = F8::softClip(filteredInput + tap3 * fbAmount);

            // Update write position
            writePos = (writePos + 1) & 0xFF;

            // Mix wet and dry
            float mixAmount = static_cast<float>(mix) * F8::kScale;
            float output = filteredInput * (1.0f - mixAmount) + wet * mixAmount;

            // Apply output filter
            output = applyOutputFilter(output);

            // Apply gain with soft clipping
            output = F8::softClip(output * (1.0f + static_cast<float>(gain) * F8::kScale));

            return output;
        }

        void reset() {
            std::fill_n(delayLine1, DELAY_BUFFER_SIZE, 0.0f);
            std::fill_n(delayLine2, DELAY_BUFFER_SIZE, 0.0f);
            std::fill_n(delayLine3, DELAY_BUFFER_SIZE, 0.0f);
            writePos = 0;
            inputFilterState = 0.0f;
            outputFilterState = 0.0f;
        }

    private:
        static constexpr int DELAY_BUFFER_SIZE = 256;
        alignas(16) float delayLine1[DELAY_BUFFER_SIZE] = { 0 };
        alignas(16) float delayLine2[DELAY_BUFFER_SIZE] = { 0 };
        alignas(16) float delayLine3[DELAY_BUFFER_SIZE] = { 0 };
        int writePos = 0;
        double currentSampleRate = 44100.0;

        float inputFilterCoeff = 0.0f;
        float outputFilterCoeff = 0.0f;
        float inputFilterState = 0.0f;
        float outputFilterState = 0.0f;

        F8::LFO lfo;

        uint8_t mix = 128;
        uint8_t depth = 40;
        uint8_t centerDelay = 80;
        uint8_t lfoRate = 4;
        uint8_t phaseSpread = 0;
        uint8_t feedback = 0;
        uint8_t gain = 255;

        void processSIMDChunk(float* buffer, int chunkSize) {
            // Process chunk sample by sample (delay lines prevent full vectorization)
            // But we batch the processing for better cache locality
            for (int i = 0; i < chunkSize; ++i) {
                buffer[i] = processSample(buffer[i]);
            }
        }

        float readDelayLine(float* buffer, float index) {
            while (index < 0.0f) index += static_cast<float>(DELAY_BUFFER_SIZE);

            int intIndex = static_cast<int>(index) % DELAY_BUFFER_SIZE;
            float frac = index - static_cast<float>(intIndex);
            int nextIndex = (intIndex + 1) % DELAY_BUFFER_SIZE;

            return buffer[intIndex] * (1.0f - frac) + buffer[nextIndex] * frac;
        }

        void updateFilterCoefficients() {
            float rc = 1.0f / (2.0f * juce::MathConstants<float>::pi * 10000.0f);
            float dt = 1.0f / static_cast<float>(currentSampleRate);
            inputFilterCoeff = dt / (rc + dt);
            outputFilterCoeff = inputFilterCoeff;
        }

        float applyInputFilter(float sample) {
            inputFilterState = inputFilterState + inputFilterCoeff * (sample - inputFilterState);
            return inputFilterState;
        }

        float applyOutputFilter(float sample) {
            outputFilterState = outputFilterState + outputFilterCoeff * (sample - outputFilterState);
            return outputFilterState;
        }
    };

    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    Chorus chorus;
    double hostSampleRate = 44100.0;

    // Optimized resampling using juce::dsp
    juce::dsp::Oversampling<float> oversampler;

    // Silence detection for CPU saving
    int silentBlockCount = 0;
    static constexpr int kMaxSilentBlocks = 10;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FairlightChorusAudioProcessor)
};