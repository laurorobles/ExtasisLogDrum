/*
  ExtasisLogDrum - Free & Open Source Amapiano Log Drum Synthesizer
  Copyright (C) 2026 ExtasisRecords / @laurorobles

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
*/

#include "BeeperDisplay.h"
#include "BeeperLookAndFeel.h"
#include "../PluginProcessor.h"
#include <algorithm>

namespace ExtasisGUI
{
BeeperDisplay::BeeperDisplay(ExtasisLogDrumAudioProcessor& processor)
    : processorRef(processor)
{
    scopeBuffer.fill(0.0f);
    rollingCapture.assign(captureWindowSize, 0.0f);
    lastSeenTriggerCount = processorRef.getTriggerCounter();
    startTimerHz(30);
}

BeeperDisplay::~BeeperDisplay()
{
    stopTimer();
}

juce::String BeeperDisplay::midiNoteToName(int midiNote)
{
    if (midiNote < 0 || midiNote > 127)
        return "NONE";

    static const char* const noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const int octave = (midiNote / 12) - 2; // C1 = MIDI 36
    return juce::String(noteNames[midiNote % 12]) + juce::String(octave);
}

void BeeperDisplay::setParameterReadout(const juce::String& paramName, const juce::String& valueText)
{
    currentParamName = paramName.substring(0, 12).toUpperCase();
    currentValueText = valueText.substring(0, 12).toUpperCase();
    readoutTimeoutCounter = 45; // 1.5s at 30 Hz
    repaint();
}

void BeeperDisplay::setNoteReadout(const juce::String& noteText)
{
    if (currentNoteText != noteText)
    {
        currentNoteText = noteText;
        repaint();
    }
}

void BeeperDisplay::triggerMessageIcon()
{
    msgBlinkCounter = 15;
}

void BeeperDisplay::timerCallback()
{
    bool needsRepaint = false;

    if (readoutTimeoutCounter > 0)
    {
        readoutTimeoutCounter--;
        needsRepaint = true;
    }

    if (msgBlinkCounter > 0)
    {
        msgBlinkCounter--;
        needsRepaint = true;
    }

    // Poll lock-free MIDI trigger state (works for both DAW MIDI and UI Trigger Button)
    const uint32_t currentTrigCount = processorRef.getTriggerCounter();
    if (currentTrigCount != lastSeenTriggerCount)
    {
        lastSeenTriggerCount = currentTrigCount;
        triggerMessageIcon();
        needsRepaint = true;
    }

    const int currentNote = processorRef.getActiveMidiNote();
    if (currentNote != lastSeenMidiNote)
    {
        lastSeenMidiNote = currentNote;
        currentNoteText = "NOTE: " + midiNoteToName(currentNote);
        needsRepaint = true;
    }

    // Drain lock-free audio FIFO
    float tempChunk[1024];
    int totalRead = 0;
    while (true)
    {
        const int n = processorRef.scopeFifo.popSamples(tempChunk, 1024);
        if (n <= 0)
            break;

        totalRead += n;
        if (n >= captureWindowSize)
        {
            std::copy(tempChunk + (n - captureWindowSize), tempChunk + n, rollingCapture.begin());
        }
        else
        {
            const size_t shift = static_cast<size_t>(n);
            std::move(rollingCapture.begin() + static_cast<std::ptrdiff_t>(shift),
                      rollingCapture.end(),
                      rollingCapture.begin());
            std::copy(tempChunk, tempChunk + n,
                      rollingCapture.end() - static_cast<std::ptrdiff_t>(shift));
        }
    }

    if (totalRead > 0)
    {
        // Rising-edge zero-crossing trigger stabilization for clean hardware scope display
        int triggerIndex = captureWindowSize - scopeBufferSize;
        const int searchStart = 1;
        const int searchEnd = captureWindowSize - scopeBufferSize;
        for (int i = searchEnd; i >= searchStart; --i)
        {
            const float prev = rollingCapture[static_cast<size_t>(i - 1)];
            const float curr = rollingCapture[static_cast<size_t>(i)];
            if (prev <= 0.0f && curr > 0.005f)
            {
                triggerIndex = i;
                break;
            }
        }

        for (int i = 0; i < scopeBufferSize; ++i)
            scopeBuffer[static_cast<size_t>(i)] = rollingCapture[static_cast<size_t>(triggerIndex + i)];

        needsRepaint = true;
    }

    if (needsRepaint)
        repaint();
}

void BeeperDisplay::drawDotMatrixText(juce::Graphics& g, const juce::String& text,
                                      int startX, int startY, int maxWidth, int size)
{
    g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(),
                                           static_cast<float>(size),
                                           juce::Font::bold)));
    g.drawText(text, startX, startY, maxWidth, size + 4, juce::Justification::left, true);
}

void BeeperDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 1. Classic Green LCD Backlight with subtle radial illumination gradient
    juce::ColourGradient lcdGrad(BeeperLookAndFeel::getLcdBacklight().brighter(0.06f),
                                 static_cast<float>(bounds.getCentreX()),
                                 static_cast<float>(bounds.getCentreY()),
                                 BeeperLookAndFeel::getLcdBacklight().darker(0.08f),
                                 0.0f, 0.0f, true);
    g.setGradientFill(lcdGrad);
    g.fillRoundedRectangle(bounds.toFloat(), 3.0f);

    // 2. Subtle dot-matrix pixel grid texture
    g.setColour(BeeperLookAndFeel::getLcdPixel().withAlpha(0.045f));
    for (int gy = 4; gy < bounds.getHeight() - 4; gy += 3)
        g.drawHorizontalLine(gy, 4.0f, static_cast<float>(bounds.getWidth() - 4));

    // 3. Recessed LCD Inner Bezel Shadow
    g.setColour(juce::Colours::black.withAlpha(0.32f));
    g.drawRoundedRectangle(bounds.toFloat().reduced(1.5f), 2.5f, 3.0f);

    g.setColour(BeeperLookAndFeel::getLcdPixel());

    // 4. Right-Side Status Icons (Battery & Pager Message Envelope)
    const int iconX = bounds.getRight() - 32;
    g.drawRect(iconX, 11, 20, 10, 2);
    g.fillRect(iconX + 20, 14, 2, 4);
    g.fillRect(iconX + 3, 14, 14, 4);

    if (msgBlinkCounter == 0 || (msgBlinkCounter % 4) < 2)
    {
        g.drawRect(iconX - 30, 11, 20, 12, 2);
        g.drawLine(static_cast<float>(iconX - 29), 12.0f, static_cast<float>(iconX - 20), 17.5f, 1.8f);
        g.drawLine(static_cast<float>(iconX - 11), 12.0f, static_cast<float>(iconX - 20), 17.5f, 1.8f);
    }

    // 5. Left-Side Text Overlay (strictly constrained so it never touches the oscilloscope)
    const int scopeStartX = 164;
    const int scopeWidth = 230;
    const int leftTextMaxWidth = scopeStartX - 18;

    if (readoutTimeoutCounter > 0)
    {
        drawDotMatrixText(g, currentParamName, 10, 10, leftTextMaxWidth, 14);
        drawDotMatrixText(g, "VAL: " + currentValueText, 10, 30, leftTextMaxWidth, 13);
    }
    else
    {
        drawDotMatrixText(g, "EXTASIS LOG DRUM", 10, 10, leftTextMaxWidth, 14);
        drawDotMatrixText(g, "STATUS: OK", 10, 30, leftTextMaxWidth, 13);
    }

    // Note Readout (bottom left)
    drawDotMatrixText(g, currentNoteText, 10, bounds.getBottom() - 25, leftTextMaxWidth, 14);

    // 6. Dot-Matrix Oscilloscope (clipped cleanly inside border — zero overflow!)
    const juce::Rectangle<int> scopeBounds(scopeStartX, 10, scopeWidth, bounds.getHeight() - 20);
    g.drawRect(scopeBounds, 2);

    const auto innerScope = scopeBounds.reduced(3);
    g.saveState();
    g.reduceClipRegion(innerScope);

    const int pixelSize = 3;
    const float midY = static_cast<float>(innerScope.getCentreY());
    const float heightHalf = static_cast<float>(innerScope.getHeight()) * 0.45f;
    const int zeroRow = (static_cast<int>(midY) - innerScope.getY()) / pixelSize;
    const int zeroY = innerScope.getY() + zeroRow * pixelSize;

    // Subtle dotted zero-baseline inside oscilloscope
    g.setColour(BeeperLookAndFeel::getLcdPixel().withAlpha(0.25f));
    for (int bx = innerScope.getX(); bx < innerScope.getRight(); bx += pixelSize * 2)
        g.fillRect(bx, zeroY, pixelSize - 1, pixelSize - 1);

    g.setColour(BeeperLookAndFeel::getLcdPixel());

    const int numCols = innerScope.getWidth() / pixelSize;
    for (int col = 0; col < numCols; ++col)
    {
        const int sampleIdx = (col * scopeBufferSize) / juce::jmax(1, numCols);
        const float val = juce::jlimit(-1.0f, 1.0f, scopeBuffer[static_cast<size_t>(sampleIdx)]);
        const float y = midY - val * heightHalf;

        const int snapX = innerScope.getX() + col * pixelSize;
        const int row = juce::jlimit(0, (innerScope.getHeight() / pixelSize) - 1,
                                     (static_cast<int>(y) - innerScope.getY()) / pixelSize);
        const int snapY = innerScope.getY() + row * pixelSize;

        g.fillRect(snapX, snapY, pixelSize - 1, pixelSize - 1);

        if (snapY != zeroY)
        {
            const int step = (snapY < zeroY) ? pixelSize : -pixelSize;
            for (int fy = snapY + step; fy != zeroY; fy += step)
                g.fillRect(snapX, fy, pixelSize - 1, pixelSize - 1);
        }
    }

    g.restoreState();
}

void BeeperDisplay::resized() {}
} // namespace ExtasisGUI
