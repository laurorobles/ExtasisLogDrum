/*
  ExtasisLogDrum - Free & Open Source Amapiano Log Drum Synthesizer
  Copyright (C) 2026 ExtasisRecords / @laurorobles

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
*/

#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>

// Lock-free single-producer / single-consumer ring buffer for real-time oscilloscope
class AudioScopeFifo
{
public:
    static constexpr int capacity = 4096;

    AudioScopeFifo() : abstractFifo(capacity)
    {
        buffer.fill(0.0f);
    }

    void pushSamples(const float* samples, int numSamples) noexcept
    {
        if (samples == nullptr || numSamples <= 0)
            return;

        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite(numSamples, start1, size1, start2, size2);

        if (size1 > 0)
            std::copy(samples, samples + size1, buffer.begin() + start1);
        if (size2 > 0)
            std::copy(samples + size1, samples + size1 + size2, buffer.begin() + start2);

        abstractFifo.finishedWrite(size1 + size2);
    }

    int popSamples(float* dest, int maxSamples) noexcept
    {
        const int available = abstractFifo.getNumReady();
        const int toRead = juce::jmin(available, maxSamples);
        if (toRead <= 0)
            return 0;

        int start1, size1, start2, size2;
        abstractFifo.prepareToRead(toRead, start1, size1, start2, size2);

        if (size1 > 0)
            std::copy(buffer.begin() + start1, buffer.begin() + start1 + size1, dest);
        if (size2 > 0)
            std::copy(buffer.begin() + start2, buffer.begin() + start2 + size2, dest + size1);

        abstractFifo.finishedRead(size1 + size2);
        return size1 + size2;
    }

private:
    juce::AbstractFifo abstractFifo;
    std::array<float, capacity> buffer;
};

class LogDrumVoice : public juce::SynthesiserVoice
{
public:
    explicit LogDrumVoice(float* sharedLastFreqPtr);

    void prepare(double sampleRate, int maxBlockSize);

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    // Updated params (12 params)
    void updateParams (float fmAmount, float fmRatio, float fmDecayMs, float velSens,
                       float ampDecayMs, float subLevel, float pitchEnv, float pitchDecayMs,
                       float cutoff, float res, float driveAmount, float glideTimeMs);

private:
    double currentSampleRate = 44100.0;
    float* lastPlayedFreqPtr = nullptr;

    // DSP State
    float currentFrequency = 0.0f;
    float targetFrequency = 0.0f;
    float pitchBendSemitones = 0.0f;
    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float subPhase = 0.0f;

    // Envelopes
    juce::ADSR ampAdsr;
    juce::ADSR::Parameters ampParams;
    juce::ADSR fmAdsr;
    juce::ADSR::Parameters fmParams;
    juce::ADSR pitchAdsr;
    juce::ADSR::Parameters pitchParams;

    // Filter
    juce::dsp::StateVariableTPTFilter<float> filter;
    bool filterPrepared = false;

    // Parameters
    float baseFmAmount = 0.46f;
    float currentFmRatio = 4.0f;
    float currentVelocity = 1.0f;
    float drive = 1.5f;
    float velSens = 0.8f;
    float glideRate = 1.0f;
    float subMix = 0.2f;
    float pitchModAmount = 7.0f;

    // Helper
    float processSample() noexcept;
};

class LogDrumSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class ExtasisLogDrumAudioProcessor  : public juce::AudioProcessor
{
public:
    ExtasisLogDrumAudioProcessor();
    ~ExtasisLogDrumAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.2; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void loadPreset(int index);
    juce::StringArray getPresetNames() const;

    juce::AudioProcessorValueTreeState apvts;

    // Lock-free oscilloscope feed & live MIDI readout
    AudioScopeFifo scopeFifo;
    int getActiveMidiNote() const noexcept { return activeMidiNote.load(std::memory_order_relaxed); }
    uint32_t getTriggerCounter() const noexcept { return triggerCounter.load(std::memory_order_relaxed); }

    // For UI triggering (thread-safe via MidiMessageCollector)
    void triggerAuditionNote(int note, float velocity);
    void releaseAuditionNote(int note);

private:
    juce::Synthesiser synth;
    juce::MidiMessageCollector midiCollector;
    float sharedLastPlayedFreq = 0.0f;
    std::atomic<int> currentPresetIndex { 0 };
    std::atomic<int> activeMidiNote { -1 };
    std::atomic<uint32_t> triggerCounter { 0 };

    // Cached atomic parameter pointers for zero-lookup processBlock
    std::atomic<float>* fmAmountParam = nullptr;
    std::atomic<float>* fmRatioParam = nullptr;
    std::atomic<float>* fmDecayParam = nullptr;
    std::atomic<float>* velSensParam = nullptr;
    std::atomic<float>* ampDecayParam = nullptr;
    std::atomic<float>* subMixParam = nullptr;
    std::atomic<float>* pitchEnvParam = nullptr;
    std::atomic<float>* pitchDecayParam = nullptr;
    std::atomic<float>* cutoffParam = nullptr;
    std::atomic<float>* resParam = nullptr;
    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* glideParam = nullptr;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExtasisLogDrumAudioProcessor)
};
