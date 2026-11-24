/*
  ExtasisLogDrum - Free & Open Source Amapiano Log Drum Synthesizer
  Copyright (C) 2026 ExtasisRecords / @laurorobles

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
*/

#include "BeeperLookAndFeel.h"
#include <cmath>

namespace ExtasisGUI
{
BeeperLookAndFeel::BeeperLookAndFeel()
{
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff1e1e1e));
    setColour(juce::PopupMenu::textColourId, juce::Colour(0xffdddddd));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, getLcdBacklight());
    setColour(juce::PopupMenu::highlightedTextColourId, getLcdPixel());
}

void BeeperLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPosProportional, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider& slider)
{
    const float outerRadius = static_cast<float>(juce::jmin(width, height)) * 0.5f - 2.0f;
    const float radius = outerRadius - 4.0f;
    const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // 0. Subtle KORG-style radial tick marks around the perimeter
    constexpr int numTicks = 11;
    for (int i = 0; i < numTicks; ++i)
    {
        const float prop = static_cast<float>(i) / static_cast<float>(numTicks - 1);
        const float tickAngle = rotaryStartAngle + prop * (rotaryEndAngle - rotaryStartAngle) - juce::MathConstants<float>::halfPi;
        const bool isActive = (prop <= sliderPosProportional + 0.01f);

        const float innerR = radius + 1.8f;
        const float outerR = radius + (i == 0 || i == numTicks - 1 || i == 5 ? 4.2f : 3.2f);
        const float x1 = centreX + std::cos(tickAngle) * innerR;
        const float y1 = centreY + std::sin(tickAngle) * innerR;
        const float x2 = centreX + std::cos(tickAngle) * outerR;
        const float y2 = centreY + std::sin(tickAngle) * outerR;

        g.setColour(isActive ? getLcdBacklight().withAlpha(0.85f) : juce::Colour(0xff444444));
        g.drawLine(x1, y1, x2, y2, isActive ? 1.3f : 1.0f);
    }

    // 1. Shadow / Base Ring
    g.setColour(juce::Colours::black.withAlpha(0.65f));
    g.fillEllipse(rx + 1.0f, ry + 2.0f, rw, rw);

    // 2. Main Body (Dark rubbery/plastic KORG MS-20 cap)
    juce::ColourGradient grad(juce::Colour(0xff383838), centreX, ry,
                              juce::Colour(0xff101010), centreX, ry + rw, false);
    g.setGradientFill(grad);
    g.fillEllipse(rx, ry, rw, rw);

    // 3. Subtle edge highlight (brighter on mouse hover / drag)
    const bool isHighlighted = slider.isMouseOverOrDragging();
    g.setColour(isHighlighted ? getLcdBacklight().withAlpha(0.65f) : juce::Colour(0xff484848));
    g.drawEllipse(rx, ry, rw, rw, isHighlighted ? 1.3f : 1.0f);

    // 4. Inner cap recess
    const float innerCap = radius * 0.72f;
    g.setColour(juce::Colour(0xff1d1d1d));
    g.fillEllipse(centreX - innerCap, centreY - innerCap, innerCap * 2.0f, innerCap * 2.0f);

    // 5. White Indicator Line (Korg MS-20 style)
    juce::Path p;
    const float pointerLength = radius * 0.82f;
    const float pointerThickness = 2.4f;
    p.addRoundedRectangle(-pointerThickness * 0.5f, -radius * 0.92f, pointerThickness, pointerLength, 0.8f);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(isHighlighted ? juce::Colours::white : juce::Colour(0xffeaeaea));
    g.fillPath(p);
}

void BeeperLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);

    juce::Colour base = backgroundColour;
    if (shouldDrawButtonAsDown) base = base.brighter(0.2f);
    else if (shouldDrawButtonAsHighlighted) base = base.brighter(0.1f);

    juce::ColourGradient grad (base.brighter(0.1f), bounds.getX(), bounds.getY(),
                               base.darker(0.2f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.5f);

    g.setColour(base.darker(0.4f));
    g.drawRoundedRectangle(bounds, bounds.getHeight() * 0.5f, 1.5f);
}

void BeeperLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                        bool /*highlighted*/, bool isDown)
{
    g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    g.setColour(juce::Colours::white.withAlpha(isDown ? 0.7f : 0.9f));

    auto textBounds = button.getLocalBounds();
    if (isDown) textBounds.translate(0, 1);
    g.drawText(button.getButtonText(), textBounds, juce::Justification::centred, true);
}

void BeeperLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                      int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                      juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0.5f, 0.5f, static_cast<float>(width) - 1.0f, static_cast<float>(height) - 1.0f);
    const bool hover = box.isMouseOver() || isButtonDown;

    g.setColour(juce::Colour(0xff1e1e1e));
    g.fillRoundedRectangle(bounds, 3.0f);

    g.setColour(hover ? getLcdBacklight().withAlpha(0.8f) : juce::Colour(0xff444444));
    g.drawRoundedRectangle(bounds, 3.0f, 1.2f);

    // Crisp chevron arrow
    const float arrowX = static_cast<float>(width) - 14.0f;
    const float arrowY = static_cast<float>(height) * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath(arrowX - 3.5f, arrowY - 1.5f);
    arrow.lineTo(arrowX, arrowY + 2.5f);
    arrow.lineTo(arrowX + 3.5f, arrowY - 1.5f);

    g.setColour(hover ? getLcdBacklight() : juce::Colour(0xff999999));
    g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void BeeperLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
    g.setColour(juce::Colour(0xff444444));
    g.drawRect(0, 0, width, height, 1);
}

void BeeperLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                                           bool isSeparator, bool isActive, bool isHighlighted,
                                           bool isTicked, bool /*hasSubMenu*/,
                                           const juce::String& text, const juce::String& /*shortcutKeyText*/,
                                           const juce::Drawable* /*icon*/, const juce::Colour* /*textColour*/)
{
    if (isSeparator)
    {
        g.setColour(juce::Colour(0xff333333));
        g.fillRect(area.reduced(4, 0).withHeight(1).withY(area.getCentreY()));
        return;
    }

    auto r = area.reduced(2, 1);
    if (isHighlighted && isActive)
    {
        g.setColour(getLcdBacklight());
        g.fillRoundedRectangle(r.toFloat(), 2.0f);
        g.setColour(getLcdPixel());
    }
    else
    {
        g.setColour(isTicked ? getLcdBacklight() : juce::Colour(0xffdddddd));
    }

    g.setFont(getPopupMenuFont());
    auto textArea = r.reduced(8, 0);
    g.drawText(text, textArea, juce::Justification::centredLeft, true);
}

juce::Font BeeperLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold));
}

juce::Font BeeperLookAndFeel::getPopupMenuFont()
{
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold));
}
} // namespace ExtasisGUI
