#include "PluginProcessor.h"
#include "PluginEditor.h"

// ==============================================================================
LogDrumVoice::LogDrumVoice() 
{
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

bool LogDrumVoice::canPlaySound (juce::SynthesiserSound* sound) { return dynamic_cast<LogDrumSound*>(sound) != nullptr; }

void LogDrumVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    currentSampleRate = getSampleRate();
    currentVelocity = velocity;
    
    float newTargetFreq = (float)juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    if (glideRate <= 0.0001f || !ampAdsr.isActive()) {
        currentFrequency = newTargetFreq;
    }
    targetFrequency = newTargetFreq;

    ampAdsr.noteOn();
    fmAdsr.noteOn();
    pitchAdsr.noteOn();
    
    carrierPhase = 0.0f;
    modulatorPhase = 0.0f;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = currentSampleRate;
    spec.maximumBlockSize = 1;
    spec.numChannels = 1;
    filter.prepare(spec);
    filter.reset();
}

void LogDrumVoice::stopNote (float /*velocity*/, bool allowTailOff)
{
    ampAdsr.noteOff();
    fmAdsr.noteOff();
    pitchAdsr.noteOff();
    if (! allowTailOff) {
        clearCurrentNote();
        carrierPhase = 0.0f;
        modulatorPhase = 0.0f;
    }
}

void LogDrumVoice::pitchWheelMoved (int) {}
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
    
    filter.setCutoffFrequency(cutoff);
    filter.setResonance(res);

    if (glideMs > 1.0f)
        glideRate = 1.0f - std::exp(-1.0f / (glideMs * 0.001f * currentSampleRate));
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
    
    ampAdsr.setSampleRate(currentSampleRate);
    fmAdsr.setSampleRate(currentSampleRate);
    pitchAdsr.setSampleRate(currentSampleRate);
}

float LogDrumVoice::processSample()
{
    currentFrequency += glideRate * (targetFrequency - currentFrequency);
    
    float fmEnvVal = fmAdsr.getNextSample();
    float ampEnvVal = ampAdsr.getNextSample();
    float pitchEnvVal = pitchAdsr.getNextSample();
    
    float effectiveVel = juce::jmap(velSens, 1.0f, currentVelocity);
    float currentFmIndex = baseFmAmount * effectiveVel * fmEnvVal * 10.0f; 

    // Apply Pitch Envelope
    float actualFreq = currentFrequency * std::pow(2.0f, (pitchModAmount * pitchEnvVal) / 12.0f);

    float carInc = actualFreq * juce::MathConstants<float>::twoPi / currentSampleRate;
    float modInc = actualFreq * currentFmRatio * juce::MathConstants<float>::twoPi / currentSampleRate;

    float modVal = std::sin(modulatorPhase);
    float carVal = std::sin(carrierPhase + (currentFmIndex * modVal));
    float subVal = std::sin(carrierPhase * 0.5f); 
    
    modulatorPhase += modInc;
    if (modulatorPhase >= juce::MathConstants<float>::twoPi) modulatorPhase -= juce::MathConstants<float>::twoPi;
    
    carrierPhase += carInc;
    if (carrierPhase >= juce::MathConstants<float>::twoPi) carrierPhase -= juce::MathConstants<float>::twoPi;
    
    float mixed = (carVal * (1.0f - subMix)) + (subVal * subMix);
    float out = mixed * ampEnvVal * currentVelocity;
    
    // Filter
    out = filter.processSample(1, out);
    
    // Saturation
    out = std::tanh(out * drive);
    
    return out;
}

void LogDrumVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive()) return;
    for (int i = 0; i < numSamples; ++i) {
        float val = processSample();
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
            outputBuffer.addSample (channel, startSample + i, val);
        }
        if (! ampAdsr.isActive()) {
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
#endif
{
    synth.addSound (new LogDrumSound());
    for (int i = 0; i < 4; ++i) synth.addVoice (new LogDrumVoice());
}
ExtasisLogDrumAudioProcessor::~ExtasisLogDrumAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ExtasisLogDrumAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    // MODULE 1: FM TRANSIENT
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("FmAmount", "FM Amount", 0.0f, 1.0f, 0.46f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("FmRatio", "FM Ratio", 1.0f, 8.0f, 4.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("FmDecay", "FM Decay", juce::NormalisableRange<float>(1.0f, 150.0f, 0.1f, 0.5f), 15.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VelSens", "Vel Sens", 0.0f, 1.0f, 0.8f));
    
    // MODULE 2: BODY & PITCH
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("AmpDecay", "Body Decay", juce::NormalisableRange<float>(50.0f, 3000.0f, 1.0f, 0.3f), 400.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SubMix", "Sub Mix", 0.0f, 1.0f, 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("PitchEnv", "Pitch Drop", 0.0f, 24.0f, 7.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("PitchDecay", "Drop Time", juce::NormalisableRange<float>(10.0f, 200.0f, 0.1f, 0.5f), 45.0f));

    // MODULE 3: FILTER & AMP
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("Cutoff", "Cutoff", juce::NormalisableRange<float>(50.0f, 10000.0f, 1.0f, 0.3f), 3000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("Resonance", "Resonance", 0.1f, 2.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("Drive", "Drive", 1.0f, 10.0f, 1.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("Glide", "Glide", 0.0f, 500.0f, 0.0f));

    return { params.begin(), params.end() };
}

void ExtasisLogDrumAudioProcessor::prepareToPlay (double sampleRate, int) { synth.setCurrentPlaybackSampleRate (sampleRate); }
void ExtasisLogDrumAudioProcessor::releaseResources() {}
bool ExtasisLogDrumAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const { return true; }

void ExtasisLogDrumAudioProcessor::triggerAuditionNote(int note, float vel) {
    midiCollector.addMessageToQueue(juce::MidiMessage::noteOn(1, note, vel));
}

void ExtasisLogDrumAudioProcessor::releaseAuditionNote(int note) {
    midiCollector.addMessageToQueue(juce::MidiMessage::noteOff(1, note));
}

void ExtasisLogDrumAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    
    // Inject UI triggered notes thread-safely
    midiCollector.removeNextBlockOfMessages(midiMessages, buffer.getNumSamples());
    
    float fmAmount = apvts.getRawParameterValue ("FmAmount")->load();
    float fmRatio = apvts.getRawParameterValue ("FmRatio")->load();
    float fmDecay = apvts.getRawParameterValue ("FmDecay")->load();
    float velSens = apvts.getRawParameterValue ("VelSens")->load();
    
    float ampDecay = apvts.getRawParameterValue ("AmpDecay")->load();
    float subMix = apvts.getRawParameterValue ("SubMix")->load();
    float pitchEnv = apvts.getRawParameterValue ("PitchEnv")->load();
    float pitchDecay = apvts.getRawParameterValue ("PitchDecay")->load();
    
    float cutoff = apvts.getRawParameterValue ("Cutoff")->load();
    float res = apvts.getRawParameterValue ("Resonance")->load();
    float drive = apvts.getRawParameterValue ("Drive")->load();
    float glide = apvts.getRawParameterValue ("Glide")->load();

    for (int i = 0; i < synth.getNumVoices(); i++) {
        if (auto voice = dynamic_cast<LogDrumVoice*>(synth.getVoice(i))) {
            voice->updateParams (fmAmount, fmRatio, fmDecay, velSens,
                                 ampDecay, subMix, pitchEnv, pitchDecay,
                                 cutoff, res, drive, glide);
        }
    }

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
    if (onAudioBlockProcessed) {
        onAudioBlockProcessed(buffer.getReadPointer(0), buffer.getNumSamples());
    }
}

void ExtasisLogDrumAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}
void ExtasisLogDrumAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ExtasisLogDrumAudioProcessor(); }

struct Preset {
    const char* name;
    float fmAmount, fmRatio, fmDecay, velSens, ampDecay, subMix, pitchEnv, pitchDecay, cutoff, res, drive, glide;
};

static const Preset factoryPresets[10] = {
    {"01. AMAPIANO CLASSIC", 0.46f, 4.0f,  15.0f, 0.8f, 400.0f, 0.2f, 7.0f, 45.0f,  3000.0f, 0.5f, 1.5f, 0.0f},
    {"02. DEEP SUB TRUNK",   0.20f, 2.0f,  30.0f, 0.6f, 600.0f, 0.8f, 3.0f, 60.0f,  1000.0f, 0.2f, 2.0f, 0.0f},
    {"03. METALLIC DONK",    0.80f, 7.0f,  8.0f,  1.0f, 200.0f, 0.0f, 12.0f, 20.0f, 8000.0f, 0.8f, 3.0f, 0.0f},
    {"04. SHORT PLUCK",      0.50f, 4.0f,  10.0f, 0.9f, 150.0f, 0.1f, 5.0f, 15.0f,  4000.0f, 0.4f, 1.0f, 0.0f},
    {"05. OVERDRIVE LOG",    0.60f, 4.0f,  25.0f, 0.7f, 500.0f, 0.4f, 7.0f, 40.0f,  5000.0f, 1.2f, 5.0f, 0.0f},
    {"06. GLIDING BASS",     0.30f, 2.0f,  20.0f, 0.5f, 800.0f, 0.6f, 4.0f, 50.0f,  2000.0f, 0.5f, 2.5f, 120.0f},
    {"07. MUFFLED THUMP",    0.35f, 4.0f,  12.0f, 0.6f, 350.0f, 0.5f, 12.0f, 80.0f, 800.0f,  0.1f, 1.2f, 0.0f},
    {"08. HYPER FM",         1.0f,  8.0f,  50.0f, 1.0f, 400.0f, 0.1f, 24.0f, 30.0f, 9000.0f, 1.5f, 4.0f, 0.0f},
    {"09. CHORDAL 5THS",     0.50f, 3.0f,  40.0f, 0.8f, 500.0f, 0.3f, 0.0f, 10.0f,  6000.0f, 0.4f, 1.5f, 0.0f},
    {"10. SOFT MALLET",      0.15f, 2.0f,  50.0f, 0.4f, 600.0f, 0.2f, 2.0f, 30.0f,  1500.0f, 0.2f, 1.0f, 0.0f}
};

juce::StringArray ExtasisLogDrumAudioProcessor::getPresetNames() const {
    juce::StringArray arr;
    for (int i = 0; i < 10; ++i) arr.add(factoryPresets[i].name);
    return arr;
}

void ExtasisLogDrumAudioProcessor::loadPreset(int index) {
    if (index < 0 || index >= 10) return;
    const auto& p = factoryPresets[index];
    apvts.getParameter("FmAmount")->setValueNotifyingHost(apvts.getParameterRange("FmAmount").convertTo0to1(p.fmAmount));
    apvts.getParameter("FmRatio")->setValueNotifyingHost(apvts.getParameterRange("FmRatio").convertTo0to1(p.fmRatio));
    apvts.getParameter("FmDecay")->setValueNotifyingHost(apvts.getParameterRange("FmDecay").convertTo0to1(p.fmDecay));
    apvts.getParameter("VelSens")->setValueNotifyingHost(apvts.getParameterRange("VelSens").convertTo0to1(p.velSens));
    apvts.getParameter("AmpDecay")->setValueNotifyingHost(apvts.getParameterRange("AmpDecay").convertTo0to1(p.ampDecay));
    apvts.getParameter("SubMix")->setValueNotifyingHost(apvts.getParameterRange("SubMix").convertTo0to1(p.subMix));
    apvts.getParameter("PitchEnv")->setValueNotifyingHost(apvts.getParameterRange("PitchEnv").convertTo0to1(p.pitchEnv));
    apvts.getParameter("PitchDecay")->setValueNotifyingHost(apvts.getParameterRange("PitchDecay").convertTo0to1(p.pitchDecay));
    apvts.getParameter("Cutoff")->setValueNotifyingHost(apvts.getParameterRange("Cutoff").convertTo0to1(p.cutoff));
    apvts.getParameter("Resonance")->setValueNotifyingHost(apvts.getParameterRange("Resonance").convertTo0to1(p.res));
    apvts.getParameter("Drive")->setValueNotifyingHost(apvts.getParameterRange("Drive").convertTo0to1(p.drive));
    apvts.getParameter("Glide")->setValueNotifyingHost(apvts.getParameterRange("Glide").convertTo0to1(p.glide));
}
