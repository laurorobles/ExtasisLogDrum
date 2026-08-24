#include "BeeperTriggerButton.h"
#include "BeeperLookAndFeel.h"
#include "BinaryData.h"

namespace ExtasisGUI
{
BeeperTriggerButton::BeeperTriggerButton() 
{ 
    setRepaintsOnMouseActivity(true); 
    if (BinaryData::logo_pngSize > 0)
    {
        logoImage = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
    }
}

juce::String BeeperTriggerButton::getNoteNameString() const
{
    int currentNote = getCurrentMidiNote();
    int octave = (currentNote / 12) - 2;
    static const char* const noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return juce::String(noteNames[currentNote % 12]) + juce::String(octave);
}

void BeeperTriggerButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    if (isTriggerActive) bounds.translate(0, 2.0f);

    juce::Colour baseColor = juce::Colour(0xff2d2d2d); // Rubber grey
    if (isTriggerActive) baseColor = BeeperLookAndFeel::getLedRed();
    else if (isMouseOver()) baseColor = baseColor.brighter(0.1f);

    g.setColour(baseColor.darker(0.3f));
    g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.4f);
    
    g.setColour(baseColor);
    g.fillRoundedRectangle(bounds.reduced(2.0f), bounds.getHeight() * 0.4f);

    if (logoImage.isValid()) {
        auto imgBounds = bounds.reduced(6.0f);
        float opacity = isTriggerActive ? 1.0f : (isMouseOver() ? 0.95f : 0.8f);
        g.setOpacity(opacity);
        g.drawImage(logoImage, imgBounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        g.setOpacity(1.0f);
    } else {
        g.setColour(isTriggerActive ? juce::Colours::white : juce::Colour(0xffdddddd));
        juce::Font f (16.0f, juce::Font::bold);
        g.setFont(f);
        g.drawText("TRIGGER LOG DRUM", bounds, juce::Justification::centred, false);
    }
}

void BeeperTriggerButton::mouseDown(const juce::MouseEvent& e)
{
    isTriggerActive = true;
    dragStartY = (float)e.getScreenY();
    dragStartOffset = semitoneOffset;
    if (onNoteOn) onNoteOn(getCurrentMidiNote(), 1.0f);
    if (onTriggerEvent) onTriggerEvent();
    if (onNoteChanged) onNoteChanged("NOTE: " + getNoteNameString());
    repaint();
}

void BeeperTriggerButton::mouseDrag(const juce::MouseEvent& e)
{
    if (!isTriggerActive) return;
    float deltaY = dragStartY - (float)e.getScreenY();
    int newOffset = juce::jlimit(-24, 24, dragStartOffset + (int)(deltaY / 8.0f));
    if (newOffset != semitoneOffset) {
        int oldNote = getCurrentMidiNote();
        semitoneOffset = newOffset;
        if (onNoteOff) onNoteOff(oldNote);
        if (onNoteOn) onNoteOn(getCurrentMidiNote(), 1.0f);
        if (onNoteChanged) onNoteChanged("NOTE: " + getNoteNameString());
        repaint();
    }
}

void BeeperTriggerButton::mouseUp(const juce::MouseEvent&)
{
    if (isTriggerActive) {
        isTriggerActive = false;
        if (onNoteOff) onNoteOff(getCurrentMidiNote());
        if (onNoteChanged) onNoteChanged("NOTE: NONE");
        repaint();
    }
}
void BeeperTriggerButton::mouseEnter(const juce::MouseEvent&) { repaint(); }
void BeeperTriggerButton::mouseExit(const juce::MouseEvent&) { repaint(); }
}
