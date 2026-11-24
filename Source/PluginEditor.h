/*
  ExtasisLogDrum - Free & Open Source Amapiano Log Drum Synthesizer
  Copyright (C) 2026 ExtasisRecords / @laurorobles

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/BeeperLookAndFeel.h"
#include "GUI/BeeperDisplay.h"
#include "GUI/BeeperTriggerButton.h"

class ExtasisLogDrumAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit ExtasisLogDrumAudioProcessorEditor (ExtasisLogDrumAudioProcessor&);
    ~ExtasisLogDrumAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ExtasisLogDrumAudioProcessor& processorRef;
    ExtasisGUI::BeeperLookAndFeel customLookAndFeel;
    ExtasisGUI::BeeperDisplay display;
    ExtasisGUI::BeeperTriggerButton triggerBtn;
    juce::ComboBox presetBox;

    struct Knob
    {
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
        juce::Label label;
    };
    std::vector<std::unique_ptr<Knob>> knobs;

    void createKnob(const juce::String& paramId, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExtasisLogDrumAudioProcessorEditor)
};
