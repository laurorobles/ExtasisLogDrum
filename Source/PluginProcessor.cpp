/*
  ExtasisLogDrum - Free & Open Source Amapiano Log Drum Synthesizer
  Copyright (C) 2026 ExtasisRecords / @laurorobles

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
struct Preset
{
    const char* name;
    float fmAmount, fmRatio, fmDecay, velSens;
    float ampDecay, subMix, pitchEnv, pitchDecay;
    float cutoff, res, drive, glide;
};

constexpr int kNumFactoryPresets = 10;

static const Preset factoryPresets[kNumFactoryPresets] = {
    { "01. AMAPIANO CLASSIC", 0.46f, 4.0f,  15.0f, 0.8f, 400.0f, 0.20f,  7.0f, 45.0f, 3000.0f, 0.5f, 1.5f,   0.0f },
    { "02. DEEP SUB TRUNK",   0.20f, 2.0f,  30.0f, 0.6f, 600.0f, 0.80f,  3.0f, 60.0f, 1000.0f, 0.2f, 2.0f,   0.0f },
    { "03. METALLIC DONK",    0.80f, 7.0f,   8.0f, 1.0f, 200.0f, 0.00f, 12.0f, 20.0f, 8000.0f, 0.8f, 3.0f,   0.0f },
    { "04. SHORT PLUCK",      0.50f, 4.0f,  10.0f, 0.9f, 150.0f, 0.10f,  5.0f, 15.0f, 4000.0f, 0.4f, 1.0f,   0.0f },
    { "05. OVERDRIVE LOG",    0.60f, 4.0f,  25.0f, 0.7f, 500.0f, 0.40f,  7.0f, 40.0f, 5000.0f, 1.2f, 5.0f,   0.0f },
    { "06. GLIDING BASS",     0.30f, 2.0f,  20.0f, 0.5f, 800.0f, 0.60f,  4.0f, 50.0f, 2000.0f, 0.5f, 2.5f, 120.0f },
    { "07. MUFFLED THUMP",    0.35f, 4.0f,  12.0f, 0.6f, 350.0f, 0.50f, 12.0f, 80.0f,  800.0f, 0.1f, 1.2f,   0.0f },
    { "08. HYPER FM",         1.00f, 8.0f,  50.0f, 1.0f, 400.0f, 0.10f, 24.0f, 30.0f, 9000.0f, 1.5f, 4.0f,   0.0f },
    { "09. CHORDAL 5THS",     0.50f, 3.0f,  40.0f, 0.8f, 500.0f, 0.30f,  0.0f, 10.0f, 6000.0f, 0.4f, 1.5f,   0.0f },
    { "10. SOFT MALLET",      0.15f, 2.0f,  50.0f, 0.4f, 600.0f, 0.20f,  2.0f, 30.0f, 1500.0f, 0.2f, 1.0f,   0.0f }
};

float pitchWheelToSemitones(int wheelPos) noexcept
{
    // Standard +/- 2 semitones pitch wheel range
    return ((static_cast<float>(wheelPos) - 8192.0f) / 8192.0f) * 2.0f;
}
} // namespace

// ==============================================================================
LogDrumVoice::LogDrumVoice(float* sharedLastFreqPtr)
    : lastPlayedFreqPtr(sharedLastFreqPtr)
{
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void LogDrumVoice::prepare(double sampleRate, int /*maxBlockSize*/)
{
    currentSampleRate = (sampleRate > 0.0) ? sampleRate : 44100.0;

    ampAdsr.setSampleRate(currentSampleRate);
    fmAdsr.setSampleRate(currentSampleRate);
    pitchAdsr.setSampleRate(currentSampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = currentSampleRate;
    spec.maximumBlockSize = 1;
    spec.numChannels = 1;
    filter.prepare(spec);
    filter.reset();
    filterPrepared = true;
}

bool LogDrumVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<LogDrumSound*>(sound) != nullptr;
}

void LogDrumVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    const double sr = getSampleRate();
    if (sr > 0.0 && std::abs(sr - currentSampleRate) > 0.5)
        prepare(sr, 1);

    currentVelocity = juce::jlimit(0.01f, 1.0f, velocity);
    pitchBendSemitones = pitchWheelToSemitones(currentPitchWheelPosition);

    const float newTargetFreq = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));

    if (glideRate < 0.9999f && lastPlayedFreqPtr != nullptr && *lastPlayedFreqPtr > 0.0f)
        currentFrequency = *lastPlayedFreqPtr;
    else
        currentFrequency = newTargetFreq;

    targetFrequency = newTargetFreq;
    if (lastPlayedFreqPtr != nullptr)
        *lastPlayedFreqPtr = newTargetFreq;

    // Reset phases for clean, click-free percussive transient consistency
    carrierPhase = 0.0f;
    modulatorPhase = 0.0f;
    subPhase = 0.0f;

    ampAdsr.reset();
    fmAdsr.reset();
    pitchAdsr.reset();

    ampAdsr.noteOn();
    fmAdsr.noteOn();
    pitchAdsr.noteOn();

    if (filterPrepared)
        filter.reset();
}

void LogDrumVoice::stopNote (float /*velocity*/, bool allowTailOff)
{
    ampAdsr.noteOff();
    fmAdsr.noteOff();
    pitchAdsr.noteOff();

    if (! allowTailOff)
    {
        clearCurrentNote();
        carrierPhase = 0.0f;
        modulatorPhase = 0.0f;
        subPhase = 0.0f;
    }
}

void LogDrumVoice::pitchWheelMoved (int newPitchWheelValue)
{
    pitchBendSemitones = pitchWheelToSemitones(newPitchWheelValue);
}

void LogDrumVoice::controllerMoved (int, int) {}

void LogDrumVoice::updateParams(float fmAmount, float fmRatio, float fmDecayMs, float vsens,
                                float ampDecayMs, float sub, float pitchEnv, float pitchDecayMs,
                                float cutoff, float res, float driveAmount, float glideMs)
{
    baseFmAmount = fmAmount;
    currentFmRatio = fmRatio;
    velSens = vsens;
    subMix = sub;
    pitchModAmount = pitchEnv;
    drive = driveAmount;

    const float nyquistSafeCutoff = juce::jlimit(20.0f, static_cast<float>(currentSampleRate * 0.45), cutoff);
    filter.setCutoffFrequency(nyquistSafeCutoff);
    filter.setResonance(juce::jlimit(0.1f, 4.0f, res));

    if (glideMs > 1.0f && currentSampleRate > 0.0)
        glideRate = static_cast<float>(1.0 - std::exp(-1.0 / (static_cast<double>(glideMs) * 0.001 * currentSampleRate)));
    else
        glideRate = 1.0f;

    ampParams.attack = 0.004f;
    ampParams.decay = ampDecayMs / 1000.0f;
    ampParams.sustain = 0.0f;
    ampParams.release = 0.1f;
    ampAdsr.setParameters(ampParams);

    fmParams.attack = 0.002f;
    fmParams.decay = fmDecayMs / 1000.0f;
    fmParams.sustain = 0.0f;
    fmParams.release = 0.1f;
    fmAdsr.setParameters(fmParams);

    pitchParams.attack = 0.002f;
    pitchParams.decay = pitchDecayMs / 1000.0f;
    pitchParams.sustain = 0.0f;
    pitchParams.release = 0.1f;
    pitchAdsr.setParameters(pitchParams);
}

float LogDrumVoice::processSample() noexcept
{
    currentFrequency += glideRate * (targetFrequency - currentFrequency);

    const float fmEnvVal = fmAdsr.getNextSample();
    const float ampEnvVal = ampAdsr.getNextSample();
    const float pitchEnvVal = pitchAdsr.getNextSample();

    const float effectiveVel = juce::jmap(velSens, 1.0f, currentVelocity);
    const float currentFmIndex = baseFmAmount * effectiveVel * fmEnvVal * 10.0f;

    // Apply Pitch Envelope + MIDI Pitch Wheel
    const float totalSemitones = (pitchModAmount * pitchEnvVal) + pitchBendSemitones;
    const float actualFreq = currentFrequency * std::pow(2.0f, totalSemitones / 12.0f);

    const float invSampleRate = 1.0f / static_cast<float>(currentSampleRate);
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float carInc = actualFreq * twoPi * invSampleRate;
    const float modInc = carInc * currentFmRatio;
    const float subInc = carInc * 0.5f;

    const float modVal = std::sin(modulatorPhase);
    const float carVal = std::sin(carrierPhase + (currentFmIndex * modVal));
    const float subVal = std::sin(subPhase);

    modulatorPhase += modInc;
    while (modulatorPhase >= twoPi) modulatorPhase -= twoPi;

    carrierPhase += carInc;
    while (carrierPhase >= twoPi) carrierPhase -= twoPi;

    subPhase += subInc;
    while (subPhase >= twoPi) subPhase -= twoPi;

    const float mixed = (carVal * (1.0f - subMix)) + (subVal * subMix);
    float out = mixed * ampEnvVal * currentVelocity;

    // StateVariableTPTFilter channel 0 (mono voice)
    if (filterPrepared)
        out = filter.processSample(0, out);

    // Analog-style soft saturation
    out = std::tanh(out * drive);

    return out;
}

void LogDrumVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    const int numChannels = outputBuffer.getNumChannels();
    for (int i = 0; i < numSamples; ++i)
    {
        const float val = processSample();
        for (int channel = 0; channel < numChannels; ++channel)
            outputBuffer.addSample (channel, startSample + i, val);

        if (! ampAdsr.isActive())
        {
            clearCurrentNote();
            break;
        }
    }
}

// ==============================================================================
ExtasisLogDrumAudioProcessor::ExtasisLogDrumAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts(*this, nullptr, "Parameters", createParameters())
#else
     : apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
    synth.addSound (new LogDrumSound());
    for (int i = 0; i < 4; ++i)
        synth.addVoice (new LogDrumVoice(&sharedLastPlayedFreq));

    // Cache parameter pointers for lock-free, lookup-free audio thread access
    fmAmountParam   = apvts.getRawParameterValue ("FmAmount");
    fmRatioParam    = apvts.getRawParameterValue ("FmRatio");
    fmDecayParam    = apvts.getRawParameterValue ("FmDecay");
    velSensParam    = apvts.getRawParameterValue ("VelSens");
    ampDecayParam   = apvts.getRawParameterValue ("AmpDecay");
    subMixParam     = apvts.getRawParameterValue ("SubMix");
    pitchEnvParam   = apvts.getRawParameterValue ("PitchEnv");
    pitchDecayParam = apvts.getRawParameterValue ("PitchDecay");
    cutoffParam     = apvts.getRawParameterValue ("Cutoff");
    resParam        = apvts.getRawParameterValue ("Resonance");
    driveParam      = apvts.getRawParameterValue ("Drive");
    glideParam      = apvts.getRawParameterValue ("Glide");
}

ExtasisLogDrumAudioProcessor::~ExtasisLogDrumAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout ExtasisLogDrumAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto percentAttrs = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float v, int) { return juce::String(juce::roundToInt(v * 100.0f)) + "%"; });
    auto msAttrs = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float v, int) { return juce::String(v, v < 100.0f ? 1 : 0) + " ms"; });
    auto ratioAttrs = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float v, int) { return juce::String(v, 2) + "x"; });
    auto stAttrs = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float v, int) { return "+" + juce::String(v, 1) + " st"; });
    auto hzAttrs = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float v, int) {
            return (v >= 1000.0f) ? (juce::String(v / 1000.0f, 2) + " kHz")
                                  : (juce::String(juce::roundToInt(v)) + " Hz");
        });

    // MODULE 1: FM TRANSIENT
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("FmAmount", 1), "FM Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.46f, percentAttrs));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("FmRatio", 1), "FM Ratio",
        juce::NormalisableRange<float>(1.0f, 8.0f, 0.01f), 4.0f, ratioAttrs));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("FmDecay", 1), "FM Decay",
        juce::NormalisableRange<float>(1.0f, 150.0f, 0.1f, 0.5f), 15.0f, msAttrs));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("VelSens", 1), "Vel Sens",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.8f, percentAttrs));

    // MODULE 2: BODY & PITCH
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("AmpDecay", 1), "Body Decay",
        juce::NormalisableRange<float>(50.0f, 3000.0f, 1.0f, 0.3f), 400.0f, msAttrs));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("SubMix", 1), "Sub Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.2f, percentAttrs));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("PitchEnv", 1), "Pitch Drop",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 7.0f, stAttrs));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("PitchDecay", 1), "Drop Time",
        juce::NormalisableRange<float>(10.0f, 200.0f, 0.1f, 0.5f), 45.0f, msAttrs));

    // MODULE 3: FILTER & AMP
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("Cutoff", 1), "Cutoff",
        juce::NormalisableRange<float>(50.0f, 10000.0f, 1.0f, 0.3f), 3000.0f, hzAttrs));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("Resonance", 1), "Resonance",
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f), 0.5f, ratioAttrs));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("Drive", 1), "Drive",
        juce::NormalisableRange<float>(1.0f, 10.0f, 0.01f, 0.5f), 1.5f, ratioAttrs));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID("Glide", 1), "Glide",
        juce::NormalisableRange<float>(0.0f, 500.0f, 1.0f, 0.5f), 0.0f, msAttrs));

    return { params.begin(), params.end() };
}

void ExtasisLogDrumAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    midiCollector.reset (sampleRate);
    sharedLastPlayedFreq = 0.0f;

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<LogDrumVoice*>(synth.getVoice(i)))
            voice->prepare(sampleRate, samplesPerBlock);
    }
}

void ExtasisLogDrumAudioProcessor::releaseResources() {}

bool ExtasisLogDrumAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void ExtasisLogDrumAudioProcessor::triggerAuditionNote(int note, float vel)
{
    auto msg = juce::MidiMessage::noteOn(1, juce::jlimit(0, 127, note), juce::jlimit(0.0f, 1.0f, vel));
    msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    midiCollector.addMessageToQueue(msg);
}

void ExtasisLogDrumAudioProcessor::releaseAuditionNote(int note)
{
    auto msg = juce::MidiMessage::noteOff(1, juce::jlimit(0, 127, note));
    msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    midiCollector.addMessageToQueue(msg);
}

void ExtasisLogDrumAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Inject UI triggered notes thread-safely
    midiCollector.removeNextBlockOfMessages(midiMessages, buffer.getNumSamples());

    // Track incoming MIDI notes (from DAW or UI trigger) for the LCD readout
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            activeMidiNote.store(msg.getNoteNumber(), std::memory_order_relaxed);
            triggerCounter.fetch_add(1, std::memory_order_relaxed);
        }
        else if (msg.isNoteOff())
        {
            if (activeMidiNote.load(std::memory_order_relaxed) == msg.getNoteNumber())
                activeMidiNote.store(-1, std::memory_order_relaxed);
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            activeMidiNote.store(-1, std::memory_order_relaxed);
        }
    }

    const float fmAmount   = fmAmountParam->load(std::memory_order_relaxed);
    const float fmRatio    = fmRatioParam->load(std::memory_order_relaxed);
    const float fmDecay    = fmDecayParam->load(std::memory_order_relaxed);
    const float velSens    = velSensParam->load(std::memory_order_relaxed);
    const float ampDecay   = ampDecayParam->load(std::memory_order_relaxed);
    const float subMix     = subMixParam->load(std::memory_order_relaxed);
    const float pitchEnv   = pitchEnvParam->load(std::memory_order_relaxed);
    const float pitchDecay = pitchDecayParam->load(std::memory_order_relaxed);
    const float cutoff     = cutoffParam->load(std::memory_order_relaxed);
    const float res        = resParam->load(std::memory_order_relaxed);
    const float drive      = driveParam->load(std::memory_order_relaxed);
    const float glide      = glideParam->load(std::memory_order_relaxed);

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<LogDrumVoice*>(synth.getVoice(i)))
        {
            voice->updateParams (fmAmount, fmRatio, fmDecay, velSens,
                                 ampDecay, subMix, pitchEnv, pitchDecay,
                                 cutoff, res, drive, glide);
        }
    }

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Push channel 0 to lock-free oscilloscope FIFO (zero locks, zero allocations)
    if (buffer.getNumChannels() > 0)
        scopeFifo.pushSamples(buffer.getReadPointer(0), buffer.getNumSamples());
}

int ExtasisLogDrumAudioProcessor::getNumPrograms()
{
    return kNumFactoryPresets;
}

int ExtasisLogDrumAudioProcessor::getCurrentProgram()
{
    return currentPresetIndex.load(std::memory_order_relaxed);
}

void ExtasisLogDrumAudioProcessor::setCurrentProgram (int index)
{
    loadPreset(index);
}

const juce::String ExtasisLogDrumAudioProcessor::getProgramName (int index)
{
    if (index >= 0 && index < kNumFactoryPresets)
        return factoryPresets[index].name;
    return {};
}

void ExtasisLogDrumAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("currentPresetIndex", currentPresetIndex.load(std::memory_order_relaxed), nullptr);
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    if (xml != nullptr)
        copyXmlToBinary (*xml, destData);
}

void ExtasisLogDrumAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
    {
        auto tree = juce::ValueTree::fromXml (*xmlState);
        const int savedPreset = static_cast<int>(tree.getProperty("currentPresetIndex", 0));
        currentPresetIndex.store(juce::jlimit(0, kNumFactoryPresets - 1, savedPreset), std::memory_order_relaxed);
        apvts.replaceState (tree);
    }
}

juce::StringArray ExtasisLogDrumAudioProcessor::getPresetNames() const
{
    juce::StringArray arr;
    for (int i = 0; i < kNumFactoryPresets; ++i)
        arr.add(factoryPresets[i].name);
    return arr;
}

void ExtasisLogDrumAudioProcessor::loadPreset(int index)
{
    if (index < 0 || index >= kNumFactoryPresets)
        return;

    currentPresetIndex.store(index, std::memory_order_relaxed);
    const auto& p = factoryPresets[index];

    auto setParam = [this](const juce::String& id, float value)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(apvts.getParameterRange(id).convertTo0to1(value));
    };

    setParam("FmAmount",   p.fmAmount);
    setParam("FmRatio",    p.fmRatio);
    setParam("FmDecay",    p.fmDecay);
    setParam("VelSens",    p.velSens);
    setParam("AmpDecay",   p.ampDecay);
    setParam("SubMix",     p.subMix);
    setParam("PitchEnv",   p.pitchEnv);
    setParam("PitchDecay", p.pitchDecay);
    setParam("Cutoff",     p.cutoff);
    setParam("Resonance",  p.res);
    setParam("Drive",      p.drive);
    setParam("Glide",      p.glide);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ExtasisLogDrumAudioProcessor();
}
