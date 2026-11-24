/*
  ExtasisLogDrum - Free & Open Source Amapiano Log Drum Synthesizer
  Copyright (C) 2026 ExtasisRecords / @laurorobles

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <vector>

class ExtasisLogDrumAudioProcessor;

namespace ExtasisGUI
{
class BeeperDisplay : public juce::Component, public juce::Timer
{
public:
    explicit BeeperDisplay(ExtasisLogDrumAudioProcessor& processor);
    ~BeeperDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void setParameterReadout(const juce::String& paramName, const juce::String& valueText);
    void setNoteReadout(const juce::String& noteText);
    void triggerMessageIcon();
    bool isTriggerLedActive() const noexcept { return msgBlinkCounter > 8; }

    static juce::String midiNoteToName(int midiNote);

private:
    ExtasisLogDrumAudioProcessor& processorRef;

    juce::String currentParamName = "READY";
    juce::String currentValueText = "WAITING";
    juce::String currentNoteText = "NOTE: NONE";
    int readoutTimeoutCounter = 0;
    int msgBlinkCounter = 0;
    uint32_t lastSeenTriggerCount = 0;
    int lastSeenMidiNote = -1;

    static constexpr int scopeBufferSize = 256;
    static constexpr int captureWindowSize = 1024;
    std::array<float, scopeBufferSize> scopeBuffer {};
    std::vector<float> rollingCapture;

    void drawDotMatrixText(juce::Graphics& g, const juce::String& text, int x, int y, int maxWidth, int size);
};
} // namespace ExtasisGUI
