/*
  ExtasisLogDrum - Free & Open Source Amapiano Log Drum Synthesizer
  Copyright (C) 2026 ExtasisRecords / @laurorobles

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
*/

#include "BeeperTriggerButton.h"
#include "BeeperLookAndFeel.h"
#include "BinaryData.h"

namespace ExtasisGUI
{
BeeperTriggerButton::BeeperTriggerButton()
{
    setRepaintsOnMouseActivity(true);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    if (BinaryData::logo_pngSize > 0)
        logoImage = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
}

juce::String BeeperTriggerButton::getNoteNameString() const
{
    const int currentNote = getCurrentMidiNote();
    const int octave = (currentNote / 12) - 2;
    static const char* const noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return juce::String(noteNames[currentNote % 12]) + juce::String(octave);
}

void BeeperTriggerButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(3.0f, 3.0f);

    // Subtle recessed rubber socket shadow
    g.setColour(juce::Colours::black.withAlpha(0.75f));
    g.fillRoundedRectangle(bounds.translated(0.0f, 2.5f), bounds.getHeight() * 0.42f);

    if (isTriggerActive)
        bounds.translate(0.0f, 2.0f);

    juce::Colour baseColor = juce::Colour(0xff2c2c2c); // Matte rubber grey
    if (isTriggerActive)
        baseColor = BeeperLookAndFeel::getLedRed().darker(0.15f);
    else if (isMouseOver())
        baseColor = baseColor.brighter(0.12f);

    // Outer bevel
    juce::ColourGradient outerGrad(baseColor.brighter(0.18f), bounds.getX(), bounds.getY(),
                                   baseColor.darker(0.35f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(outerGrad);
    g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.42f);

    // Inner rubber pad face
    auto inner = bounds.reduced(2.5f);
    juce::ColourGradient innerGrad(baseColor.brighter(0.06f), inner.getX(), inner.getY(),
                                   baseColor.darker(0.18f), inner.getX(), inner.getBottom(), false);
    g.setGradientFill(innerGrad);
    g.fillRoundedRectangle(inner, inner.getHeight() * 0.40f);

    // Subtle rim highlight
    g.setColour(isTriggerActive ? BeeperLookAndFeel::getLedRed().brighter(0.3f)
                                : (isMouseOver() ? BeeperLookAndFeel::getLcdBacklight().withAlpha(0.45f)
                                                 : juce::Colour(0xff464646)));
    g.drawRoundedRectangle(bounds, bounds.getHeight() * 0.42f, 1.1f);

    if (logoImage.isValid())
    {
        auto imgBounds = inner.reduced(5.0f);
        const float opacity = isTriggerActive ? 1.0f : (isMouseOver() ? 0.96f : 0.85f);
        g.setOpacity(opacity);
        g.drawImage(logoImage, imgBounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        g.setOpacity(1.0f);
    }
    else
    {
        g.setColour(isTriggerActive ? juce::Colours::white : juce::Colour(0xffdddddd));
        g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
        g.drawText("TRIGGER", inner, juce::Justification::centred, false);
    }

    // Subtle pitch offset pill badge when shifted away from C1
    if (semitoneOffset != 0)
    {
        const juce::String badge = (semitoneOffset > 0 ? "+" : "") + juce::String(semitoneOffset);
        auto badgeRect = juce::Rectangle<float>(bounds.getRight() - 28.0f, bounds.getY() + 4.0f, 24.0f, 14.0f);
        g.setColour(juce::Colour(0xcc111111));
        g.fillRoundedRectangle(badgeRect, 3.0f);
        g.setColour(BeeperLookAndFeel::getLcdBacklight());
        g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold)));
        g.drawText(badge, badgeRect, juce::Justification::centred, false);
    }
}

void BeeperTriggerButton::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        semitoneOffset = 0;
        if (onNoteChanged) onNoteChanged("NOTE: " + getNoteNameString());
        repaint();
        return;
    }

    isTriggerActive = true;
    dragStartY = static_cast<float>(e.getScreenY());
    dragStartOffset = semitoneOffset;

    if (onNoteOn) onNoteOn(getCurrentMidiNote(), 1.0f);
    if (onTriggerEvent) onTriggerEvent();
    if (onNoteChanged) onNoteChanged("NOTE: " + getNoteNameString());
    repaint();
}

void BeeperTriggerButton::mouseDrag(const juce::MouseEvent& e)
{
    if (!isTriggerActive)
        return;

    const float deltaY = dragStartY - static_cast<float>(e.getScreenY());
    const int newOffset = juce::jlimit(-24, 24, dragStartOffset + static_cast<int>(deltaY / 8.0f));
    if (newOffset != semitoneOffset)
    {
        const int oldNote = getCurrentMidiNote();
        semitoneOffset = newOffset;
        if (onNoteOff) onNoteOff(oldNote);
        if (onNoteOn) onNoteOn(getCurrentMidiNote(), 1.0f);
        if (onNoteChanged) onNoteChanged("NOTE: " + getNoteNameString());
        repaint();
    }
}

void BeeperTriggerButton::mouseUp(const juce::MouseEvent&)
{
    if (isTriggerActive)
    {
        isTriggerActive = false;
        if (onNoteOff) onNoteOff(getCurrentMidiNote());
        if (onNoteChanged) onNoteChanged("NOTE: NONE");
        repaint();
    }
}

void BeeperTriggerButton::mouseDoubleClick(const juce::MouseEvent&)
{
    semitoneOffset = 0;
    repaint();
}

void BeeperTriggerButton::mouseEnter(const juce::MouseEvent&) { repaint(); }
void BeeperTriggerButton::mouseExit(const juce::MouseEvent&) { repaint(); }
} // namespace ExtasisGUI
