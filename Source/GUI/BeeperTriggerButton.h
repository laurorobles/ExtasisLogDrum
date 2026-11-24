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
#include <functional>

namespace ExtasisGUI
{
class BeeperTriggerButton : public juce::Component
{
public:
    BeeperTriggerButton();
    ~BeeperTriggerButton() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    int getCurrentMidiNote() const noexcept { return baseMidiNote + semitoneOffset; }
    juce::String getNoteNameString() const;

    std::function<void(int midiNote, float velocity)> onNoteOn;
    std::function<void(int midiNote)> onNoteOff;
    std::function<void()> onTriggerEvent;
    std::function<void(const juce::String&)> onNoteChanged;

private:
    juce::Image logoImage;
    int baseMidiNote = 36; // C1
    int semitoneOffset = 0;
    bool isTriggerActive = false;
    float dragStartY = 0.0f;
    int dragStartOffset = 0;
};
} // namespace ExtasisGUI
