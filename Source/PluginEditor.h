#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/BeeperLookAndFeel.h"
#include "GUI/BeeperDisplay.h"
#include "GUI/BeeperTriggerButton.h"

class ExtasisLogDrumAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ExtasisLogDrumAudioProcessorEditor (ExtasisLogDrumAudioProcessor&);
    ~ExtasisLogDrumAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    ExtasisLogDrumAudioProcessor& processorRef;
    ExtasisGUI::BeeperLookAndFeel customLookAndFeel;
    ExtasisGUI::BeeperDisplay display;
    ExtasisGUI::BeeperTriggerButton triggerBtn;
    juce::ComboBox presetBox;
    
    struct Knob {
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
        juce::Label label;
    };
    std::vector<std::unique_ptr<Knob>> knobs;

    void createKnob(const juce::String& paramId, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExtasisLogDrumAudioProcessorEditor)
};
