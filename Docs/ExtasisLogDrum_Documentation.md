# ExtasisLogDrum — Documentación Oficial (v1.0.0)

Bienvenido a **ExtasisLogDrum**, el sintetizador gratuito y de código abierto (GPL-3.0) de **ExtasisRecords (`@laurorobles`)** diseñado para capturar la topología y el comportamiento acústico del icónico bajo percusivo del Amapiano.

---

## 1. Introducción y Concepto

**ExtasisLogDrum** no es un reproductor de samples de madera, sino un sintetizador puro de **Modulación de Fase (PM/FM)** inspirado en la genealogía del *Log Drum* del mda DX10 / Fruity DX10, expandido con control completo de diseño sonoro.

El sonido del log drum del Amapiano nace de la interacción entre una excitación espectral ultracorta (transitorio FM controlado por Velocity), un golpe inicial de tono (*Pitch Drop*) y un cuerpo grave tonal reforzado por un sub-oscilador dedicado a una octava inferior.

---

## 2. Arquitectura DSP (v1.0.0)

- **Núcleo FM/PM**: Portadora senoidal pura + modulador senoidal con **FM Ratio** ajustable (`1.0x – 8.0x`, siendo `4.0x` el estándar clásico del DX10).
- **Sub-Oscilador Dedicado**: Senoidal pura independiente a `0.5x` de la frecuencia fundamental con fase coherente, mezclable mediante `SUB BASS`.
- **Tres Envolventes Independientes**:
  1. **FM Envelope** (`DONK DEC`): Controla el índice de modulación transitorio (`1–150 ms`).
  2. **Pitch Drop Envelope** (`PITCH DROP` / `DROP TIME`): Caída rápida de tono de hasta `+24 semitonos` para impacto en el pecho.
  3. **Amp Envelope** (`BODY DEC`): Controla la duración del cuerpo grave (`50–3000 ms`) con ataque suave anti-clicks de `4 ms` y reinicio de fase por nota.
- **Filtro y Saturación**: Filtro pasa-bajos Zero-Delay Feedback (`StateVariableTPTFilter`) + saturación analógica suave `std::tanh(x * drive)` + portamento **Glide** entre notas.
- **Seguridad en Tiempo Real**: Osciloscopio alimentado por `AudioScopeFifo` (`juce::AbstractFifo` sin locks ni asignaciones de memoria) y disparo MIDI desde UI mediante `juce::MidiMessageCollector`.

---

## 3. Documentación Complementaria

- [README.md](../README.md) — Resumen, instalación y compilación desde código fuente.
- [MANUAL.md](../MANUAL.md) — Manual completo de parámetros, presets y MIDI.
- [TECHNICAL.md](../TECHNICAL.md) — Especificaciones técnicas y tabla APVTS.
- [ARCHITECTURE.md](../ARCHITECTURE.md) — Diagramas de flujo DSP y arquitectura de hilos.
- [LICENSE](../LICENSE) — Licencia GNU GPL v3.0.

*— Desarrollado por `@laurorobles` en ExtasisRecords —*
