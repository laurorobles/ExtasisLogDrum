#include "BeeperLookAndFeel.h"

namespace ExtasisGUI
{
BeeperLookAndFeel::BeeperLookAndFeel() {}

void BeeperLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPosProportional, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider& /*slider*/)
{
    float radius = (float)juce::jmin(width / 2, height / 2) - 2.0f;
    float centreX = (float)x + (float)width * 0.5f;
    float centreY = (float)y + (float)height * 0.5f;
    float rx = centreX - radius;
    float ry = centreY - radius;
    float rw = radius * 2.0f;
    float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // KORG style
    // 1. Shadow / Base Ring
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillEllipse(rx + 1, ry + 2, rw, rw);

    // 2. Main Body (Dark rubbery/plastic)
    juce::ColourGradient grad(juce::Colour(0xff333333), centreX, ry, juce::Colour(0xff111111), centreX, ry + rw, false);
    g.setGradientFill(grad);
    g.fillEllipse(rx, ry, rw, rw);
    
    // 3. Subtle edge highlight
    g.setColour(juce::Colour(0xff444444));
    g.drawEllipse(rx, ry, rw, rw, 1.0f);
    
    // 4. White Indicator Line (Korg MS-20 style)
    juce::Path p;
    float pointerLength = radius * 0.85f;
    float pointerThickness = 2.5f;
    // Drawn vertically pointing up, then rotated
    p.addRectangle(-pointerThickness * 0.5f, -radius * 0.95f, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    
    g.setColour(juce::Colours::white);
    g.fillPath(p);
}

void BeeperLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    
    // Rubbery pill-shaped button
    juce::Colour base = backgroundColour;
    if (shouldDrawButtonAsDown) base = base.brighter(0.2f);
    else if (shouldDrawButtonAsHighlighted) base = base.brighter(0.1f);
    
    juce::ColourGradient grad (base.brighter(0.1f), bounds.getX(), bounds.getY(),
                               base.darker(0.2f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.5f); // pill shape
    
    g.setColour(base.darker(0.4f));
    g.drawRoundedRectangle(bounds, bounds.getHeight() * 0.5f, 1.5f);
}

void BeeperLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                        bool /*highlighted*/, bool isDown)
{
    juce::Font font (14.0f, juce::Font::bold);
    g.setFont (font);
    g.setColour (juce::Colours::white.withAlpha(isDown ? 0.7f : 0.9f));
    
    auto textBounds = button.getLocalBounds();
    if (isDown) textBounds.translate(0, 1);
    g.drawText (button.getButtonText(), textBounds, juce::Justification::centred, true);
}
}
