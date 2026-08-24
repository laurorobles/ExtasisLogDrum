#pragma once
#include <JuceHeader.h>

class LogDrumVoice : public juce::SynthesiserVoice
{
public:
    LogDrumVoice();
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
    
    // DSP State
    float currentFrequency = 0.0f;
    float targetFrequency = 0.0f;
    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;

    // Envelopes
    juce::ADSR ampAdsr;
    juce::ADSR::Parameters ampParams;
    juce::ADSR fmAdsr;
    juce::ADSR::Parameters fmParams;
    juce::ADSR pitchAdsr;
    juce::ADSR::Parameters pitchParams;

    // Filter
    juce::dsp::StateVariableTPTFilter<float> filter;

    // Parameters
    float baseFmAmount = 1.0f;
    float currentFmRatio = 4.0f;
    float currentVelocity = 1.0f;
    float drive = 1.0f;
    float velSens = 1.0f;
    float glideRate = 0.0f;
    float subMix = 0.5f;
    float pitchModAmount = 0.0f;

    // Helper
    float processSample();
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
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void loadPreset(int index);
    juce::StringArray getPresetNames() const;

    juce::AudioProcessorValueTreeState apvts;
    
    // Live UI callbacks
    std::function<void(const float*, int)> onAudioBlockProcessed;

    // For UI triggering
    juce::MidiMessageCollector midiCollector;
    void triggerAuditionNote(int note, float velocity);
    void releaseAuditionNote(int note);

private:
    juce::Synthesiser synth;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExtasisLogDrumAudioProcessor)
};
