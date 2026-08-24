#include "PluginProcessor.h"
#include "PluginEditor.h"

ExtasisLogDrumAudioProcessorEditor::ExtasisLogDrumAudioProcessorEditor (ExtasisLogDrumAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel(&customLookAndFeel);

    addAndMakeVisible(display);
    processorRef.onAudioBlockProcessed = [this](const float* samples, int numSamples) {
        display.pushAudioSamples(samples, numSamples);
    };

    triggerBtn.onNoteOn = [this](int midiNote, float velocity) { processorRef.triggerAuditionNote(midiNote, velocity); };
    triggerBtn.onNoteOff = [this](int midiNote) { processorRef.releaseAuditionNote(midiNote); };
    triggerBtn.onNoteChanged = [this](juce::String str) { display.setNoteReadout(str); };
    triggerBtn.onTriggerEvent = [this]() { display.triggerMessageIcon(); };
    addAndMakeVisible(triggerBtn);

    // Presets
    presetBox.addItemList(processorRef.getPresetNames(), 1);
    presetBox.setSelectedId(1, juce::dontSendNotification);
    presetBox.onChange = [this]() {
        processorRef.loadPreset(presetBox.getSelectedId() - 1);
    };
    addAndMakeVisible(presetBox);

    presetBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff222222));
    presetBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    presetBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff555555));
    presetBox.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff888888));
    
    // Module 1
    createKnob("FmAmount", "DONK AMT");
    createKnob("FmRatio", "FM RATIO");
    createKnob("FmDecay", "DONK DEC");
    createKnob("VelSens", "VEL SENS");
    
    // Module 2
    createKnob("AmpDecay", "BODY DEC");
    createKnob("SubMix", "SUB BASS");
    createKnob("PitchEnv", "PITCH DROP");
    createKnob("PitchDecay", "DROP TIME");
    
    // Module 3
    createKnob("Cutoff", "LPF CUT");
    createKnob("Resonance", "RESONANCE");
    createKnob("Drive", "SATURATE");
    createKnob("Glide", "GLIDE");
    
    setSize (520, 440);
    resized(); // Force layout update in case setSize is a no-op
}

ExtasisLogDrumAudioProcessorEditor::~ExtasisLogDrumAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

void ExtasisLogDrumAudioProcessorEditor::createKnob(const juce::String& paramId, const juce::String& labelText)
{
    auto knob = std::make_unique<Knob>();
    knob->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob->slider.setLookAndFeel(&customLookAndFeel);
    knob->slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(knob->slider);
    
    juce::Font f (juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold);
    knob->label.setFont(f);
    knob->label.setText(labelText, juce::dontSendNotification);
    knob->label.setJustificationType(juce::Justification::centred);
    knob->label.setColour(juce::Label::textColourId, juce::Colour(0xff888888));
    addAndMakeVisible(knob->label);
    
    knob->attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, paramId, knob->slider);
    
    knob->slider.onValueChange = [this, paramId, labelText, k = knob.get()]() {
        juce::String valStr = k->slider.getTextFromValue(k->slider.getValue());
        display.setParameterReadout(labelText, valStr);
    };
    knobs.push_back(std::move(knob));
}

void ExtasisLogDrumAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (ExtasisGUI::BeeperLookAndFeel::getChassisColor());
    
    // Top plastic ridge
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(0, 0, getWidth(), 15);
    
    g.setColour(juce::Colour(0xff444444));
    juce::Font f (juce::Font::getDefaultMonospacedFontName(), 16.0f, juce::Font::bold | juce::Font::italic);
    g.setFont(f);
    g.drawText("EXTASIS ADVISOR", 0, 15, getWidth(), 20, juce::Justification::centred, false);
    
    // Draw Module Boxes
    g.setColour(juce::Colour(0xff333333));
    int modWidth = 150;
    int modHeight = 170;
    int startX = 20;
    int startY = 145;
    
    auto drawModule = [&](int x, int y, const juce::String& name) {
        juce::Rectangle<int> r(x, y, modWidth, modHeight);
        g.drawRoundedRectangle(r.toFloat(), 4.0f, 1.5f);
        
        juce::Font modFont (juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold);
        g.setFont(modFont);
        g.setColour(juce::Colour(0xff666666));
        g.drawText(name, x, y - 18, modWidth, 20, juce::Justification::centred, false);
        g.setColour(juce::Colour(0xff333333));
    };
    
    drawModule(startX, startY, "FM TRANSIENT");
    drawModule(startX + 165, startY, "BODY & PITCH");
    drawModule(startX + 330, startY, "FILTER & AMP");
    
    // Developer Credit
    juce::Font creditFont (juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain);
    g.setFont(creditFont);
    g.setColour(juce::Colour(0xff555555));
    g.drawText("coded by @laurorobles", getWidth() - 140, getHeight() - 20, 130, 20, juce::Justification::bottomRight, false);
}

void ExtasisLogDrumAudioProcessorEditor::resized()
{
    // Screen at the top
    display.setBounds(20, 35, getWidth() - 40, 95);
    
    // 3 Modules layout
    int knobWidth = 45;
    int knobHeight = 45;
    int modWidth = 150;
    int startX = 20;
    int startY = 145;
    
    for (int i = 0; i < knobs.size(); ++i) {
        int moduleIdx = i / 4;
        int knobIdxInModule = i % 4;
        
        int row = knobIdxInModule / 2;
        int col = knobIdxInModule % 2;
        
        int mX = startX + moduleIdx * 165;
        int x = mX + 15 + col * 75;
        int y = startY + 15 + row * 80;
        
        knobs[i]->slider.setBounds(x, y, knobWidth, knobHeight);
        knobs[i]->label.setBounds(x - 15, y + knobHeight, knobWidth + 30, 20);
    }

    // Trigger button at the bottom
    triggerBtn.setBounds((getWidth() - 120) / 2, 330, 120, 90);
    
    // Preset box bottom left
    presetBox.setBounds(20, 360, 135, 24);
}

juce::AudioProcessorEditor* ExtasisLogDrumAudioProcessor::createEditor() {
    return new ExtasisLogDrumAudioProcessorEditor (*this);
}
