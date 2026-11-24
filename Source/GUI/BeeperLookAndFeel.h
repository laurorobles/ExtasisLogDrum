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

namespace ExtasisGUI
{
class BeeperLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BeeperLookAndFeel();
    ~BeeperLookAndFeel() override = default;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu,
                            const juce::String& text, const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* textColour) override;

    juce::Font getComboBoxFont (juce::ComboBox& box) override;
    juce::Font getPopupMenuFont() override;

    static juce::Colour getChassisColor()  { return juce::Colour(0xff161616); } // Matte Black
    static juce::Colour getModuleFill()    { return juce::Colour(0xff1c1c1c); } // Recessed Panel
    static juce::Colour getLcdBacklight()  { return juce::Colour(0xff7db552); } // Classic Green LCD
    static juce::Colour getLcdPixel()      { return juce::Colour(0xff15250f); } // Dark Green/Black pixel
    static juce::Colour getLedRed()        { return juce::Colour(0xffff1515); }
};
} // namespace ExtasisGUI
