# ExtasisLogDrum — Technical Specifications (v1.0.0)

| Specification | Details |
| :--- | :--- |
| **Version** | `1.0.0` |
| **License** | GNU General Public License v3.0 (`GPL-3.0`) |
| **Language / Framework** | C++20 / JUCE 8 |
| **Plugin Formats** | VST3, Audio Unit (AU), Standalone |
| **Platforms** | macOS Universal (`arm64` + `x86_64`), Windows (`x64`) |
| **Plugin Type** | Virtual Instrument / Synthesizer (`IS_SYNTH TRUE`) |
| **Audio I/O** | Mono Out or Stereo Out (0 inputs, 1–2 outputs) |
| **MIDI I/O** | MIDI Input (Notes, Velocity, Pitch Wheel, All Notes Off) |
| **Sample Rates** | Sample-rate independent (`44.1 kHz` – `192 kHz+`) |
| **Synthesis Engine** | 2-Operator Phase Modulation (FM/PM) + Dedicated Octave Sub-Oscillator |
| **Polyphony** | 4 Voices with cross-voice monophonic Glide memory |
| **Filter Topology** | Zero-Delay Feedback `StateVariableTPTFilter` (Lowpass, Nyquist-clamped) |
| **Waveshaper** | Soft-clipping hyperbolic tangent (`std::tanh(x * drive)`) |
| **Real-Time Safety** | Lock-free `juce::AbstractFifo` oscilloscope + cached atomic APVTS pointers + `juce::MidiMessageCollector` |
| **GUI Dimensions** | `520 × 440 px` (Compact fixed-footprint hardware layout) |
| **Manufacturer / Plugin Code** | `Exta` / `LgDr` |

---

## Parameter Table (APVTS)

| Parameter ID | Display Name | Range | Default | Unit / Skew |
| :--- | :--- | :--- | :--- | :--- |
| `FmAmount` | FM Amount | `0.0 – 1.0` | `0.46` | `%` (Linear) |
| `FmRatio` | FM Ratio | `1.0 – 8.0` | `4.0` | `x` (Linear) |
| `FmDecay` | FM Decay | `1.0 – 150.0` | `15.0` | `ms` (Skew `0.5`) |
| `VelSens` | Vel Sens | `0.0 – 1.0` | `0.8` | `%` (Linear) |
| `AmpDecay` | Body Decay | `50.0 – 3000.0` | `400.0` | `ms` (Skew `0.3`) |
| `SubMix` | Sub Mix | `0.0 – 1.0` | `0.2` | `%` (Linear) |
| `PitchEnv` | Pitch Drop | `0.0 – 24.0` | `7.0` | `st` (Linear) |
| `PitchDecay` | Drop Time | `10.0 – 200.0` | `45.0` | `ms` (Skew `0.5`) |
| `Cutoff` | Cutoff | `50.0 – 10000.0` | `3000.0` | `Hz/kHz` (Skew `0.3`) |
| `Resonance` | Resonance | `0.1 – 2.0` | `0.5` | `x` (Linear) |
| `Drive` | Drive | `1.0 – 10.0` | `1.5` | `x` (Skew `0.5`) |
| `Glide` | Glide | `0.0 – 500.0` | `0.0` | `ms` (Skew `0.5`) |
