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

ExtasisLogDrumAudioProcessorEditor::ExtasisLogDrumAudioProcessorEditor (ExtasisLogDrumAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), display (p)
{
    setLookAndFeel(&customLookAndFeel);

    addAndMakeVisible(display);

    triggerBtn.onNoteOn = [this](int midiNote, float velocity) { processorRef.triggerAuditionNote(midiNote, velocity); };
    triggerBtn.onNoteOff = [this](int midiNote) { processorRef.releaseAuditionNote(midiNote); };
    triggerBtn.onNoteChanged = [this](const juce::String& str) { display.setNoteReadout(str); };
    triggerBtn.onTriggerEvent = [this]() { display.triggerMessageIcon(); };
    addAndMakeVisible(triggerBtn);

    // Presets
    presetBox.setLookAndFeel(&customLookAndFeel);
    presetBox.addItemList(processorRef.getPresetNames(), 1);
    presetBox.setSelectedId(processorRef.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this]()
    {
        const int idx = presetBox.getSelectedId() - 1;
        if (idx >= 0)
        {
            processorRef.loadPreset(idx);
            display.setParameterReadout("PRESET", presetBox.getText());
        }
    };
    addAndMakeVisible(presetBox);

    presetBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1e1e1e));
    presetBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xffeeeeee));
    presetBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff444444));
    presetBox.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff999999));

    // Module 1: FM TRANSIENT
    createKnob("FmAmount", "DONK AMT");
    createKnob("FmRatio", "FM RATIO");
    createKnob("FmDecay", "DONK DEC");
    createKnob("VelSens", "VEL SENS");

    // Module 2: BODY & PITCH
    createKnob("AmpDecay", "BODY DEC");
    createKnob("SubMix", "SUB BASS");
    createKnob("PitchEnv", "PITCH DROP");
    createKnob("PitchDecay", "DROP TIME");

    // Module 3: FILTER & AMP
    createKnob("Cutoff", "LPF CUT");
    createKnob("Resonance", "RESONANCE");
    createKnob("Drive", "SATURATE");
    createKnob("Glide", "GLIDE");

    setSize (520, 440);
    resized();
}

ExtasisLogDrumAudioProcessorEditor::~ExtasisLogDrumAudioProcessorEditor()
{
    presetBox.setLookAndFeel(nullptr);
    for (auto& k : knobs)
        k->slider.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void ExtasisLogDrumAudioProcessorEditor::createKnob(const juce::String& paramId, const juce::String& labelText)
{
    auto knob = std::make_unique<Knob>();
    knob->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob->slider.setLookAndFeel(&customLookAndFeel);
    knob->slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    knob->slider.setMouseDragSensitivity(135);
    addAndMakeVisible(knob->slider);

    knob->label.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::bold)));
    knob->label.setText(labelText, juce::dontSendNotification);
    knob->label.setJustificationType(juce::Justification::centred);
    knob->label.setColour(juce::Label::textColourId, juce::Colour(0xff9a9a9a));
    addAndMakeVisible(knob->label);

    knob->attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, paramId, knob->slider);

    if (auto* param = processorRef.apvts.getParameter(paramId))
    {
        const double defaultVal = static_cast<double>(param->convertFrom0to1(param->getDefaultValue()));
        knob->slider.setDoubleClickReturnValue(true, defaultVal);
    }

    knob->slider.onValueChange = [this, paramId, labelText]()
    {
        if (!isShowing())
            return;

        if (auto* param = processorRef.apvts.getParameter(paramId))
            display.setParameterReadout(labelText, param->getCurrentValueAsText());
    };

    knobs.push_back(std::move(knob));
}

void ExtasisLogDrumAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (ExtasisGUI::BeeperLookAndFeel::getChassisColor());

    // Top plastic ridge with subtle specular edge
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(0, 0, getWidth(), 14);
    g.setColour(juce::Colour(0xff2f2f2f));
    g.drawHorizontalLine(14, 0.0f, static_cast<float>(getWidth()));

    // Subtle chassis corner screws
    auto drawScrew = [&](float cx, float cy)
    {
        g.setColour(juce::Colour(0xff0d0d0d));
        g.fillEllipse(cx - 3.5f, cy - 3.5f, 7.0f, 7.0f);
        g.setColour(juce::Colour(0xff2a2a2a));
        g.drawEllipse(cx - 3.5f, cy - 3.5f, 7.0f, 7.0f, 0.8f);
        g.setColour(juce::Colour(0xff141414));
        g.drawLine(cx - 2.0f, cy - 1.5f, cx + 2.0f, cy + 1.5f, 1.0f);
    };
    drawScrew(9.0f, 7.0f);
    drawScrew(static_cast<float>(getWidth()) - 9.0f, 7.0f);
    drawScrew(9.0f, static_cast<float>(getHeight()) - 9.0f);
    drawScrew(static_cast<float>(getWidth()) - 9.0f, static_cast<float>(getHeight()) - 9.0f);

    // Pager branding header
    g.setColour(juce::Colour(0xff525252));
    g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 15.0f, juce::Font::bold | juce::Font::italic)));
    g.drawText("EXTASIS ADVISOR", 0, 15, getWidth(), 20, juce::Justification::centred, false);

    // Draw 3 Module Boxes (recessed dark panels)
    const int modWidth = 150;
    const int modHeight = 166;
    const int startX = 20;
    const int startY = 146;

    auto drawModule = [&](int x, int y, const juce::String& name)
    {
        juce::Rectangle<float> r(static_cast<float>(x), static_cast<float>(y),
                                 static_cast<float>(modWidth), static_cast<float>(modHeight));
        g.setColour(ExtasisGUI::BeeperLookAndFeel::getModuleFill());
        g.fillRoundedRectangle(r, 4.5f);

        g.setColour(juce::Colour(0xff303030));
        g.drawRoundedRectangle(r, 4.5f, 1.2f);

        g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::bold)));
        g.setColour(juce::Colour(0xff777777));
        g.drawText(name, x, y - 17, modWidth, 16, juce::Justification::centred, false);
    };

    drawModule(startX, startY, "FM TRANSIENT");
    drawModule(startX + 165, startY, "BODY & PITCH");
    drawModule(startX + 330, startY, "FILTER & AMP");

    // Subtle micro-labels in bottom row
    g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold)));
    g.setColour(juce::Colour(0xff5a5a5a));
    g.drawText("PRESET", 22, 344, 135, 12, juce::Justification::left, false);

    g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::plain)));
    g.setColour(juce::Colour(0xff4e4e4e));
    g.drawText("HOLD + DRAG PITCH", (getWidth() - 140) / 2, 421, 140, 12, juce::Justification::centred, false);

    // Open-Source Version & Developer Credit
    g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain)));
    g.setColour(juce::Colour(0xff555555));
    g.drawText("v1.0.0  GPLv3", getWidth() - 155, getHeight() - 34, 135, 14, juce::Justification::right, false);
    g.setColour(juce::Colour(0xff666666));
    g.drawText("coded by @laurorobles", getWidth() - 155, getHeight() - 21, 135, 14, juce::Justification::right, false);
}

void ExtasisLogDrumAudioProcessorEditor::resized()
{
    // Screen at the top
    display.setBounds(20, 35, getWidth() - 40, 94);

    // 3 Modules layout (4 knobs per module, 2x2 grid)
    const int knobWidth = 50;
    const int knobHeight = 50;
    const int startX = 20;
    const int startY = 146;

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const int idx = static_cast<int>(i);
        const int moduleIdx = idx / 4;
        const int knobIdxInModule = idx % 4;

        const int row = knobIdxInModule / 2;
        const int col = knobIdxInModule % 2;

        const int mX = startX + moduleIdx * 165;
        const int x = mX + 13 + col * 74;
        const int y = startY + 12 + row * 78;

        knobs[i]->slider.setBounds(x, y, knobWidth, knobHeight);
        knobs[i]->label.setBounds(x - 12, y + knobHeight - 1, knobWidth + 24, 16);
    }

    // Trigger button at the bottom center
    triggerBtn.setBounds((getWidth() - 120) / 2, 328, 120, 90);

    // Preset box bottom left
    presetBox.setBounds(20, 358, 148, 26);
}

juce::AudioProcessorEditor* ExtasisLogDrumAudioProcessor::createEditor()
{
    return new ExtasisLogDrumAudioProcessorEditor (*this);
}
