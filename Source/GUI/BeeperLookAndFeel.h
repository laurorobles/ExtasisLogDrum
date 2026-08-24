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

    static juce::Colour getChassisColor()  { return juce::Colour(0xff161616); } // Matte Black
    static juce::Colour getLcdBacklight()  { return juce::Colour(0xff7db552); } // Classic Green LCD
    static juce::Colour getLcdPixel()      { return juce::Colour(0xff15250f); } // Dark Green/Black pixel
    static juce::Colour getLedRed()        { return juce::Colour(0xffff1515); }
};
} // namespace ExtasisGUI
