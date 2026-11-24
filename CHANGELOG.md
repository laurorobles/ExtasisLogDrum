# Changelog

All notable changes to **ExtasisLogDrum** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-11-23

### Added
- **Free & Open Source Release (GPL-3.0)** — ExtasisLogDrum is now 100% free and open source for all producers and developers.
- **FM/PM Log Drum Synthesis Engine**: Pure sine carrier + modulator with adjustable FM Amount, FM Ratio (1x–8x), FM Decay, and Velocity Sensitivity.
- **Body & Pitch Module**: Dedicated octave-below pure sine Sub-Bass oscillator, Body Decay, Pitch Drop (up to +24 semitones), and Drop Time.
- **Filter & Amp Module**: Zero-delay feedback Lowpass Filter (`StateVariableTPTFilter`), Resonance, analog-style `tanh` Saturation (Drive), and monophonic-style cross-voice Glide.
- **90s Pager GUI ("Extasis Advisor")**:
  - Backlit dot-matrix green LCD display with zero-crossing stabilized real-time oscilloscope, parameter readout with units, live MIDI note monitor (DAW + UI trigger), and blinking pager message icon.
  - KORG MS-20 inspired rotary knobs with radial tick marks and double-click-to-reset.
  - Tactile rubber Trigger Pad with vertical drag pitch-shifting (±24 semitones) and right-click/double-click reset to C1.
  - 10 factory Amapiano & Log Drum presets accessible from both the plugin UI and DAW native preset selector.
- **Cross-Platform Formats**: VST3, Audio Unit (AU on macOS), and Standalone App (macOS Universal arm64/x86_64 & Windows x64).

### Fixed
- Lock-free `AbstractFifo` oscilloscope pipeline (eliminates mutex locks and heap allocations on the audio thread and prevents dangling editor callbacks).
- Fixed `StateVariableTPTFilter` channel index (`0`) and moved filter preparation to `prepareToPlay`.
- Fixed sub-bass oscillator phase continuity with an independent phase accumulator.
- Fixed `MidiMessageCollector` sample-rate initialization and timestamps for responsive UI auditioning.
- Fixed oscilloscope dot-matrix pixel clipping inside the LCD border.
- Fixed macOS & Windows automated installer scripts to resolve relative bundle paths and clear macOS quarantine attributes.
