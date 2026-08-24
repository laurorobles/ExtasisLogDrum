#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace ExtasisGUI
{
class BeeperDisplay : public juce::Component, public juce::Timer
{
public:
    BeeperDisplay();
    ~BeeperDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void setParameterReadout(const juce::String& paramName, const juce::String& valueText);
    void setNoteReadout(const juce::String& noteText);
    void pushAudioSamples(const float* samples, int numSamples);
    void triggerMessageIcon();

private:
    juce::String currentParamName = "READY";
    juce::String currentValueText = "WAITING FOR MSG";
    juce::String currentNoteText = "NOTE: NONE";
    int readoutTimeoutCounter = 0;
    int msgBlinkCounter = 0;

    static constexpr int scopeBufferSize = 256;
    std::vector<float> scopeBuffer;
    std::vector<float> incomingFifo;
    juce::CriticalSection fifoLock;
    
    void drawDotMatrixText(juce::Graphics& g, juce::String text, int x, int y, int size);
};
}
