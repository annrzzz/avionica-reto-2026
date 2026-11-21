# avionica-reto-2026

Firmware de la máquina de estados del sistema de recuperación (paracaídas) de un cohete, para el Reto de Reclutamiento de la División de Aviónica AAIAQ.

**Autora:** Andrea Ruiz Olvera
**Carrera / semestre:** Ingenieria Aeroespacial · 1er semestre
**Ruta:** 3 - Firmware y sistemas embebidos
**GitHub:** [@annrzzz](https://github.com/annrzzz)


# Qué hace el proyecto

El sistema de recuperación debe cumplir tres reglas: **nunca disparar el paracaídas en tierra**, **nunca dispararlo dos veces** y **desplegarlo aunque falle la detección de apogeo**. Este firmware lo resuelve con una máquina de estados de 5 estados controlada por un botón, con registro serial de cada transición y cada evento ignorado.

### Estados y transiciones

| Estado actual | Evento | Siguiente estado |

| `IDLE` | Pulsación larga (2 s o más) | `ARMED` |
| `ARMED` | Pulsación larga | `IDLE` |
| `ARMED` | Pulsación corta (menos de 1 s) | `ASCENT` |
| `ASCENT` | Pulsación corta | `DEPLOYMENT` |
| `ASCENT` | 5 bajadas seguidas del altímetro (bonus) | `DEPLOYMENT` |
| `ASCENT` | Botón de estación terrena (extra) | `DEPLOYMENT` |
| `ASCENT` | 8 s sin que ocurra nada (respaldo) | `DEPLOYMENT` |
| `DEPLOYMENT` | Al entrar | Pulso de ignición de 1000 ms, una sola vez |
| `DEPLOYMENT` | Pulsación corta | `LANDED` |
| `LANDED` | Ninguno | Estado final |

Cualquier evento que no esté en la tabla (incluida una pulsación de entre 1 y 2 s) se ignora y se registra por serial.

# Características

- Sin `delay()`: antirrebote, parpadeo, temporizadores y muestreo se hacen con `millis()`.
- Antirrebote por software (30 ms) y distinción entre pulsación corta y larga.
- Patrón de LED distinto por estado.
- LED de ignición encendido solo durante el pulso de 1000 ms, una sola vez.
- Registro serial con marca de tiempo, por ejemplo: `[4100 ms] ARMED -> ASCENT (short press)`.
- **Bonus 2:** altímetro simulado con potenciómetro: el apogeo se detecta con 5 muestras consecutivas a la baja (una cada 200 ms).
- **Bonus de Git:** el altímetro se desarrolló en la rama `feature/altimetro` y se integró con Pull Request.
- El botón manual, el botón de estación terrena y el temporizador de 8 s se conservan como respaldo si el altímetro falla. Los tres solo actúan en `ASCENT`.

# Patrones de LED

| Estado | LED | Patrón |

| `IDLE` | Morado (pin 9) | Fijo |
| `ARMED` | Verde (pin 10) | Parpadeo lento |
| `ASCENT` | Verde (pin 10) | Parpadeo rápido |
| `DEPLOYMENT` | Azul (pin 11) | Fijo |
| `LANDED` | Naranja (pin 12) | Fijo |
| Ignición | Rojo (pin 13) | Encendido 1000 ms, una vez |

# Herramientas y versiones

| Herramienta | Uso | Versión |

| Raspberry Pi Pico (simulada) | Microcontrolador | RP2040 |
| Wokwi | Simulador del circuito | Versión web (https://wokwi.com) |
| C++ con framework Arduino | Lenguaje del firmware | [versión del framework] |
| PlatformIO | Compilación (`platform = raspberrypi`, `board = pico`) 
| Visual Studio Code | Editor | 
| Git y GitHub | Control de versiones | 
| Overleaf (pdfLaTeX) | Reporte | 

# Estructura del repositorio

avionica-reto-2026/
├── platformio.ini     # configuración de PlatformIO (Pico + Arduino)
├── src/
│   └── main.cpp       # firmware
├── docs/              # reporte (PDF y .tex)
├── media/             # capturas y gráficas ligeras
├── .gitignore
└── README.md
```

# Cómo reproducirlo

Opción A: en Wokwi (la más rápida)

1. Abre la simulación: **https://wokwi.com/projects/481073306276356097**
2. Verifica que el archivo `sketch.ino` tenga el mismo contenido que `src/main.cpp` de este repositorio.
3. Presiona **Start the simulation** (botón verde de play).
4. Abre el monitor serial (panel de abajo a la derecha) para ver el registro.
5. Prueba este recorrido:
   1. Pulsación corta en `IDLE`: se ignora y se registra.
   2. Mantén el botón principal 2 s o más y suéltalo: `IDLE -> ARMED` (verde parpadea lento).
   3. Pulsación corta: `ARMED -> ASCENT` (verde parpadea rápido).
   4. Opciones desde `ASCENT`:
      - Baja la perilla del potenciómetro de forma continua durante aproximadamente 1 s: `apogeo detectado`.
      - Pulsación corta del botón principal.
      - Presiona el botón de estación terrena.
      - No toques nada durante 8 s: `8 s backup timer`.
   5. En `DEPLOYMENT` el LED azul queda fijo y el rojo se enciende 1 s.
   6. Pulsación corta: `DEPLOYMENT -> LANDED` (naranja fijo).

Opción B: armar el circuito desde cero en Wokwi

1. Crea un proyecto nuevo con **Raspberry Pi Pico** y **Arduino** como framework.
2. Agrega 5 LEDs, 5 resistencias, 2 pulsadores, 2 resistencias de 10 kΩ y 1 potenciómetro.
3. Conecta según la tabla:

| Componente | Conexión |
|---|---|
| LED morado | Pin 9 (GP9), con resistencia a GND |
| LED verde | Pin 10 (GP10), con resistencia a GND |
| LED azul | Pin 11 (GP11), con resistencia a GND |
| LED naranja | Pin 12 (GP12), con resistencia a GND |
| LED rojo (ignición) | Pin 13 (GP13), con resistencia a GND |
| Botón principal | 3V3 a un lado; el otro lado a GP15 y, del mismo punto, resistencia de 10 kΩ a GND (pull-down) |
| Botón de estación terrena | 3V3 a un lado; el otro lado a GP14 y resistencia de 10 kΩ a GND (pull-down) |
| Potenciómetro | VCC a 3V3, GND a GND, señal a GP26 |

4. Copia el contenido de `src/main.cpp` en `sketch.ino` y ejecuta la simulación como en la opción A.


Ejemplo de registro serial

[0 ms] IDLE (reset)
[3250 ms] Ignorado en IDLE
[5100 ms] IDLE -> ARMED (long press)
[6400 ms] ARMED -> ASCENT (short press)
[14400 ms] ASCENT -> DEPLOYMENT (8 s backup timer)
[16000 ms] DEPLOYMENT -> LANDED (short press)

Entregables

- Código: `src/main.cpp`
- Reporte: carpeta `docs/`
- Video (máximo 60 s): [pega aquí el enlace]
- Simulación: https://wokwi.com/projects/481073306276356097

Autora

**Andrea Ruiz Olvera**, aspirante a la División de Aviónica AAIAQ. :)