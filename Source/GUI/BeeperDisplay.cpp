#include "BeeperDisplay.h"
#include "BeeperLookAndFeel.h"

namespace ExtasisGUI
{
BeeperDisplay::BeeperDisplay()
{
    scopeBuffer.resize(scopeBufferSize, 0.0f);
    incomingFifo.reserve(4096);
    startTimerHz(30);
}

BeeperDisplay::~BeeperDisplay() { stopTimer(); }

void BeeperDisplay::setParameterReadout(const juce::String& paramName, const juce::String& valueText)
{
    currentParamName = paramName.substring(0, 12).toUpperCase();
    currentValueText = valueText.substring(0, 10).toUpperCase();
    readoutTimeoutCounter = 45; // 1.5s
}

void BeeperDisplay::setNoteReadout(const juce::String& noteText)
{
    currentNoteText = noteText;
    repaint();
}

void BeeperDisplay::triggerMessageIcon()
{
    msgBlinkCounter = 15;
}

void BeeperDisplay::pushAudioSamples(const float* samples, int numSamples)
{
    juce::ScopedLock sl (fifoLock);
    for (int i = 0; i < numSamples; ++i)
        incomingFifo.push_back(samples[i]);
}

void BeeperDisplay::timerCallback()
{
    if (readoutTimeoutCounter > 0) readoutTimeoutCounter--;
    if (msgBlinkCounter > 0) msgBlinkCounter--;

    bool needsRepaint = false;
    {
        juce::ScopedLock sl (fifoLock);
        if (incomingFifo.size() >= scopeBufferSize) {
            int itemsToRemove = incomingFifo.size() - scopeBufferSize;
            incomingFifo.erase(incomingFifo.begin(), incomingFifo.begin() + itemsToRemove);
            for (int i = 0; i < scopeBufferSize; ++i) scopeBuffer[i] = incomingFifo[i];
            incomingFifo.clear();
            needsRepaint = true;
        }
    }

    if (needsRepaint || readoutTimeoutCounter > 0 || msgBlinkCounter > 0)
        repaint();
}

void BeeperDisplay::drawDotMatrixText(juce::Graphics& g, juce::String text, int startX, int startY, int size)
{
    juce::Font f (juce::Font::getDefaultMonospacedFontName(), (float)size, juce::Font::bold);
    g.setFont(f);
    g.drawText(text, startX, startY, 400, size + 4, juce::Justification::left, false);
}

void BeeperDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Background
    g.fillAll(BeeperLookAndFeel::getLcdBacklight());
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.drawRect(bounds, 3);
    
    g.setColour(BeeperLookAndFeel::getLcdPixel());

    // Icons
    int iconX = bounds.getRight() - 30;
    g.drawRect(iconX, 10, 20, 10, 2);
    g.fillRect(iconX + 20, 13, 2, 4);
    g.fillRect(iconX + 2, 12, 16, 6);

    if (msgBlinkCounter == 0 || (msgBlinkCounter % 4) < 2) {
        g.drawRect(iconX - 30, 10, 20, 12, 2);
        g.drawLine(iconX - 30, 10, iconX - 20, 16, 2.0f);
        g.drawLine(iconX - 10, 10, iconX - 20, 16, 2.0f);
    }

    // Text Overlay
    if (readoutTimeoutCounter > 0) {
        drawDotMatrixText(g, "PARAM: " + currentParamName, 10, 10, 14);
        drawDotMatrixText(g, "VAL:   " + currentValueText, 10, 30, 14);
    } else {
        drawDotMatrixText(g, "EXTASIS LOG DRUM", 10, 10, 14);
        drawDotMatrixText(g, "STATUS: OK", 10, 30, 14);
    }
    
    // Note Readout (bottom left)
    drawDotMatrixText(g, currentNoteText, 10, bounds.getBottom() - 25, 14);

    // Oscilloscope (Perfectly centered in the available space for a 480px display)
    int scopeStartX = 164;
    int scopeWidth = 230;
    
    juce::Rectangle<int> scopeBounds(scopeStartX, 10, scopeWidth, bounds.getHeight() - 20);
    g.drawRect(scopeBounds, 2);
    
    int pixelSize = 3;
    float midY = scopeBounds.getCentreY();
    float heightHalf = scopeBounds.getHeight() * 0.45f;
    
    int numPoints = scopeBuffer.size();
    if (numPoints > 0) {
        for (int i = 0; i < numPoints; i += 2) { 
            float x = scopeBounds.getX() + (i * scopeBounds.getWidth()) / (float)numPoints;
            float val = juce::jlimit(-1.0f, 1.0f, scopeBuffer[i]);
            float y = midY - val * heightHalf;
            
            int snapX = ((int)x / pixelSize) * pixelSize;
            int snapY = ((int)y / pixelSize) * pixelSize;
            
            g.fillRect(snapX, snapY, pixelSize - 1, pixelSize - 1);
            
            int zeroY = ((int)midY / pixelSize) * pixelSize;
            int step = (snapY < zeroY) ? pixelSize : -pixelSize;
            for (int fy = snapY; fy != zeroY; fy += step) {
                g.fillRect(snapX, fy, pixelSize - 1, pixelSize - 1);
            }
        }
    }
}
void BeeperDisplay::resized() {}
}
