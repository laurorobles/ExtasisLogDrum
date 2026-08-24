# 🥁 ExtasisLogDrum - Documentación Oficial, Manual y Especificaciones Técnicas

Bienvenido a **ExtasisLogDrum**, el sintetizador especializado de ExtasisRecords diseñado para emular la topología y el comportamiento acústico del icónico bajo percusivo del Amapiano. 

Esta documentación detalla el uso, la arquitectura DSP interna y las raíces históricas del desarrollo del plugin.

---

## 1. Introducción y Concepto

**ExtasisLogDrum** no es un reproductor de samples de madera (wood samples), sino un sintetizador puro de modulación de fase (PM/FM). Está basado en la genealogía del preset original *Log Drum* del mda DX10 / Fruity DX10, un instrumento que definió el sonido del Amapiano en las manos de productores como MDU aka TRP. 

El sonido del "log drum" del amapiano surge de la interacción temporal entre una excitación espectral extremadamente corta (transitorio FM) y un cuerpo de baja frecuencia (subgrave sinusoide), controlado todo mediante parámetros dependientes del "Velocity".

---

## 2. Arquitectura DSP (Digital Signal Processing)

### 2.1. El Núcleo de Modulación (FM/PM)
El motor de sonido se compone de dos osciladores trabajando en modulación de fase (comúnmente conocida en la síntesis comercial como FM):
- **Oscilador 1 (Carrier)**: Onda Senoidal pura. Responsable del "Cuerpo" y el subgrave.
- **Oscilador 2 (Modulador)**: Onda Senoidal pura.
- **Ratio Fijo (4:1)**: La frecuencia del modulador se ajusta matemáticamente a `Carrier_Freq * 4.0`. Este ratio específico fue extraído de las matemáticas del preset original del DX10 (Ratio = 4) y es el responsable de la expansión armónica metálica, creando la sensación psicoacústica de un objeto sólido/ahuecado que está siendo percutido.

### 2.2. Envolventes Independientes
Existen dos rutas de tiempo (envelopes) exponenciales AR (Attack-Release):
1. **FM Envelope**: Controla el índice de modulación a lo largo del tiempo. Posee un ataque de ~0 ms y un *Decay* que típicamente ronda los **4.3 ms**. Este decaimiento hiper-rápido colapsa el espectro metálico de regreso a una senoidal, emulando la energía transitoria de golpear madera ("el donk").
2. **Amp Envelope**: Controla la amplitud de salida del Carrier. Ataque instantáneo (0 ms), con un decaimiento más generoso (~70 a 120 ms) y sin nivel de *sustain*.

### 2.3. Dinámica (Velocity Mapping)
A diferencia de sintetizadores comunes, **el Velocity MIDI de entrada modula directamente el Índice de FM**. 
- **Bajo Velocity**: Modulación cercana a 0%. Resultado: Una senoide redonda, suave (un "OOM").
- **Alto Velocity**: Modulación alta. Resultado: Ruptura armónica profunda, ataque súper metálico y presente (un "DOONK" masivo).

### 2.4. Waveshaping y Saturación (Dirt)
Para asegurar que las notas por debajo de los 60 Hz sean audibles en sistemas pequeños y teléfonos móviles, la señal pasa por un soft clipper matemático `std::tanh()` que inyecta armónicos de orden impar cuando el control de *Drive* supera el valor unitario.

---

## 3. Especificaciones Técnicas

| Módulo | Característica / Rango |
| :--- | :--- |
| **Framework** | C++ 20 / JUCE 7.0+ |
| **Arquitectura** | Phase Modulation (Carrier + Modulator) |
| **Ratio FM** | 4.0 (Constante) |
| **Polifonía** | 4 Voces (Optimizadas para superposición de colas/releases) |
| **Formatos soportados** | VST3, AU, Standalone |
| **Frecuencia de Muestreo** | Independiente (44.1kHz a 192kHz) |
| **Consumo CPU** | Extremadamente Bajo (< 0.1% por voz) |

---

## 4. Manual de Usuario (GUI y Parámetros)

La interfaz gráfica del plugin es minimalista, centrada en la rápida obtención del groove correcto en el secuenciador.

### Controles de la Interfaz:
1. **[TRIGGER LOG DRUM] (Botón)**: Un botón de audición instantánea que dispara la nota MIDI 36 (C1) a Velocity alta. Utilízalo para escuchar y ajustar rápidamente los parámetros sin necesidad de un controlador MIDI o dibujar notas en el Piano Roll. 
2. **FM Amount**: Define la profundidad máxima de la modulación de fase. Valores más altos dan un tono inicial más brillante y metálico ("plástico / madera").
3. **FM Decay**: Establece qué tan rápido desaparece ese tono metálico. 
   - *Valores bajos (1-10 ms)*: El clásico Log Drum del Amapiano, ataque súper corto.
   - *Valores altos (20-50 ms)*: Se convierte más en un "Synth Bass" o "Donk Bass" de la vieja escuela.
4. **Amp Decay**: Controla la longitud total de la nota. 
   - *Valores cortos (30-80 ms)*: Notas súper percusivas que dejan gran espacio rítmico (clave en el amapiano).
   - *Valores largos (200-500 ms)*: El subgrave se sostiene y funciona más como un 808.
5. **Drive**: Aplica saturación analógica suave a la salida. Ideal para engrosar el rango de 100-300 Hz.

---

## 5. Notas de Desarrollo e Historia

El desarrollo del **ExtasisLogDrum** respeta profundamente la arqueología musical sudafricana:
> *"La conexión cultural y nominal con los instrumentos tradicionales existe, pero la arquitectura sonora contemporánea parece estar mucho más directamente ligada al sintetizador FM digital y a las prácticas de producción de FL Studio."*

Al recrear este instrumento, en lugar de encadenar un sample, EQ y distorsión, ExtasisRecords optó por reconstruir la relación espectral de un `Índice de FM transitorio variable + Modulación 4:1`. Al tocar este plugin, no estás disparando un parche de batería; estás inyectando voltaje digital y excitando sidebands de frecuencia que caen drásticamente en los primeros 10 milisegundos de vida de la onda, resultando en ese histórico y adictivo "*Knock*".

*— Desarrollado en ExtasisRecords —*
