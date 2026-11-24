# ExtasisLogDrum — Signal & System Architecture (v1.0.0)

## 1. Per-Voice DSP Signal Flow

```mermaid
flowchart LR
    MIDI["MIDI Note + Velocity + Pitch Wheel"] --> GLIDE["Exponential Glide Smoother"]
    GLIDE --> PENV["Pitch Drop ADSR (+0..24 st)"]
    PENV --> FREQ["Instantaneous Frequency (Hz)"]

    FREQ --> MOD["Modulator Sine (Freq * FM Ratio)"]
    FMENV["FM Transient ADSR * VelSens"] --> FMIDX["FM Index"]
    MOD --> FMIDX
    FMIDX --> CAR["Carrier Sine (Phase Modulated)"]
    FREQ --> CAR

    FREQ --> SUB["Dedicated Sub Sine (Freq * 0.5)"]

    CAR --> MIX["Carrier / Sub Mix"]
    SUB --> MIX
    MIX --> AMP["Body Amp ADSR * Velocity"]
    AMP --> LPF["ZDF StateVariableTPT Lowpass Filter"]
    LPF --> SAT["tanh(x * Drive) Soft Saturator"]
    SAT --> OUT["Stereo Output Bus"]
```

---

## 2. Thread-Safe Audio / UI Architecture

ExtasisLogDrum enforces strict real-time safety on the audio thread (zero mutex locks, zero heap allocations, zero string lookups):

```mermaid
flowchart TD
    UI_TRIG["BeeperTriggerButton (UI Thread)"] -->|"addMessageToQueue()"| COLLECTOR["juce::MidiMessageCollector"]
    DAW_MIDI["DAW MIDI Buffer"] --> PROC["processBlock() (Audio Thread)"]
    COLLECTOR -->|"removeNextBlockOfMessages()"| PROC

    APVTS["APVTS Cached std::atomic<float>*"] -->|"relaxed load"| PROC
    PROC --> SYNTH["4-Voice LogDrumVoice Synthesiser"]
    SYNTH --> OUT["Audio Output Buffer"]

    PROC -->|"lock-free pushSamples()"| FIFO["AudioScopeFifo (juce::AbstractFifo)"]
    PROC -->|"atomic store"| NOTE["activeMidiNote & triggerCounter"]

    FIFO -->|"popSamples() @ 30Hz"| DISP["BeeperDisplay (UI Thread)"]
    NOTE -->|"atomic load @ 30Hz"| DISP
```

---

## 3. Source Code Layout

- `Source/PluginProcessor.h` / `.cpp`: `AudioScopeFifo`, `LogDrumVoice`, `LogDrumSound`, `ExtasisLogDrumAudioProcessor`, APVTS parameters, and 10 factory presets.
- `Source/PluginEditor.h` / `.cpp`: Compact `520×440` hardware chassis layout, 3-module knob grid, double-click reset, and preset selector.
- `Source/GUI/BeeperDisplay.h` / `.cpp`: Dot-matrix backlit LCD screen, zero-crossing stabilized oscilloscope, parameter readout, and live MIDI note display.
- `Source/GUI/BeeperLookAndFeel.h` / `.cpp`: KORG MS-20 rotary knob renderer and dark/green-LCD ComboBox & PopupMenu styling.
- `Source/GUI/BeeperTriggerButton.h` / `.cpp`: Tactile rubber pad with embedded logo, vertical drag pitch-shifting (`±24 st`), and right-click/double-click reset.
