# ExtasisLogDrum — User Manual (v1.0.0)

Welcome to **ExtasisLogDrum**, a free and open-source Amapiano Log Drum synthesizer by **ExtasisRecords (`@laurorobles`)**.

---

## 1. Quick Start

1. Load **ExtasisLogDrum** on an instrument / MIDI track in your DAW (VST3 or AU), or open the **Standalone** application.
2. Click the central **Extasis Logo Trigger Pad** to audition the current patch immediately.
3. **Hold & drag vertically** on the Trigger Pad to sweep through notes (`±24 semitones` around `C1`). **Right-click** or **double-click** the pad to reset back to `C1`.
4. Choose one of the **10 Factory Presets** from the bottom-left dropdown menu, then shape the sound with the 3 hardware modules.
5. **Double-click any knob** to reset it to its default value.

---

## 2. The Beeper LCD Display

The backlit dot-matrix green LCD provides real-time feedback:

- **Top Left**: Displays `EXTASIS LOG DRUM` / `STATUS: OK` at rest. Whenever you turn a knob or change a preset, it temporarily displays the parameter name and formatted value (e.g., `LPF CUT` / `VAL: 3.00 KHZ`).
- **Bottom Left (`NOTE:`)**: Shows the currently playing MIDI note (e.g., `NOTE: C1`, `NOTE: F#1`), responding to both your DAW Piano Roll / MIDI controller and the built-in Trigger Pad.
- **Center (Oscilloscope)**: A real-time dot-matrix waveform monitor with zero-crossing trigger stabilization so you can visually verify transient punch, pitch drop, and sub-bass symmetry.
- **Top Right Icons**: Pager battery indicator and a **message envelope icon** that blinks whenever a note is triggered.

---

## 3. Sound Design Modules

### Module 1: `FM TRANSIENT` (The "Knock")
- **DONK AMT (`0% – 100%`)**: Controls the phase modulation index. Lower values yield a soft sub thump; higher values introduce the signature hollow wooden/metallic Amapiano knock.
- **FM RATIO (`1.00x – 8.00x`)**: Sets the harmonic multiplier of the modulator oscillator.
  - `2.0x`: Warm octave/hollow tone.
  - `3.0x`: Woody, slightly chordal 5th harmonic.
  - `4.0x`: Classic DX10 Amapiano Log Drum ratio.
  - `7.0x – 8.0x`: Aggressive, metallic donk.
- **DONK DEC (`1.0 – 150 ms`)**: How quickly the FM transient collapses back into a pure sine wave. Keep between `8–20 ms` for tight Amapiano rhythms.
- **VEL SENS (`0% – 100%`)**: Scales FM intensity by incoming MIDI velocity. Program ghost notes at low velocity (`30–60`) and accented hits at high velocity (`100–127`) for authentic groove dynamics.

### Module 2: `BODY & PITCH` (Weight & Punch)
- **BODY DEC (`50 – 3000 ms`)**: Total decay time of the fundamental bass body. Short (`150–350 ms`) for syncopated percussion; long (`600–1200 ms`) for 808-style sustains.
- **SUB BASS (`0% – 100%`)**: Mixes in a dedicated pure sine oscillator tuned one octave below the carrier.
- **PITCH DROP (`0.0 – +24.0 st`)**: Amount of fast downward pitch sweep at note start. Adds chest-hitting transient impact.
- **DROP TIME (`10 – 200 ms`)**: Speed of the pitch drop envelope.

### Module 3: `FILTER & AMP` (Tone & Movement)
- **LPF CUT (`50 Hz – 10 kHz`)**: Controls the zero-delay feedback lowpass filter. Roll down to `800–2000 Hz` for warm, muffled sub trunks; open up to `4–9 kHz` for biting metallic transients.
- **RESONANCE (`0.10x – 2.00x`)**: Emphasizes frequencies near the cutoff.
- **SATURATE (`1.00x – 10.00x`)**: Soft-clipping analog saturation (`tanh`). Adds odd harmonics so sub-60 Hz notes translate clearly on phones and club systems.
- **GLIDE (`0 – 500 ms`)**: Smooth pitch portamento between consecutive notes.

---

## 4. Factory Presets

| # | Preset Name | Character |
| :-: | :--- | :--- |
| 01 | `01. AMAPIANO CLASSIC` | Authentic balanced 4:1 DX10-style Log Drum knock & sub body |
| 02 | `02. DEEP SUB TRUNK` | Heavy octave sub-bass with dark lowpass filtering |
| 03 | `03. METALLIC DONK` | High 7:1 FM ratio, snappy transient, aggressive drive |
| 04 | `04. SHORT PLUCK` | Ultra-tight rhythmic percussion hit |
| 05 | `05. OVERDRIVE LOG` | Saturated, resonant log drum that cuts through dense mixes |
| 06 | `06. GLIDING BASS` | Long body decay with 120 ms portamento glide |
| 07 | `07. MUFFLED THUMP` | Deep low-end thump with pronounced pitch drop |
| 08 | `08. HYPER FM` | Extreme 8:1 FM + 24 st pitch drop for experimental bass |
| 09 | `09. CHORDAL 5THS` | 3:1 harmonic ratio for melodic/chordal Amapiano hooks |
| 10 | `10. SOFT MALLET` | Gentle, rounded wooden mallet tone |

---

## 5. MIDI Support

- **MIDI Notes**: Full 0–127 range (`C1` = MIDI 36 is the sweet spot for Amapiano basslines).
- **MIDI Velocity**: Controls both output amplitude and FM transient brightness (scaled by `VEL SENS`).
- **MIDI Pitch Wheel**: Supported (`±2 semitones` standard pitch bend).
- **DAW Automation**: All 12 parameters are exposed to host automation with formatted unit readouts.
