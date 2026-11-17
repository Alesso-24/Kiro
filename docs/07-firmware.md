# 7. Firmware

Carpeta: [`firmware/Kiro_Eyes/`](../firmware/Kiro_Eyes/) — `Kiro_Eyes.ino` (pantalla, botones, serial, EEPROM) y `EyesCore.h` (todo el motor de ojos, **sin dependencias de hardware**; lo comparte la placa y el arnés de PC).

## Arquitectura

- **Render**: framebuffer RGB565 doble (en RAM), antialiasing por píxel, ojos como polígonos de "boceto" con picos, pupilas con resorte propio, párpados, mejillas, corazones, medidor de batería.
- **Pantalla**: SPI0 a 41.7 MHz con DMA a 16 bits; solo se envían las filas que cambiaron entre cuadros (menos datos, más fps).
- **Identidad**: `creatureFromSeed()`; la semilla vive en la EEPROM emulada (`0xA110C0DF` + seed). Verde 5 s = renacer.
- **CPU**: 250 MHz (overclock sobre los 150 MHz de fábrica del RP2350).
- **Rendimiento medido en la placa**: 50–60 fps, render 3–10 ms; durante la intro/nacimiento, mínimo puntual de 43 fps.

### Lección crítica del DMA
Tras la DMA el SPI queda en 16 bits y el core Philhower **no restaura** el formato; sin `spi_set_format(spi0, 8, …)` en `pushWait()` los comandos del siguiente cuadro se corrompen y la pantalla se congela o queda negra. También hay que pasar `clk_peri` a `clk_sys` (el core lo deja a 48 MHz, techo de ~24 MHz de SPI).

## Pinout: protoboard (actual) vs PCB (producción)

El firmware actual está cableado para la **protoboard**. Con la PCB hay que cambiar:

| Función | Protoboard (firmware hoy) | **PCB (producción)** | Cambio |
|---|---|---|---|
| SCK | D8 / GPIO2 | D8 / GPIO2 | = |
| MOSI | D10 / GPIO3 | D10 / GPIO3 | = |
| CS | D1 / GPIO27 | D1 / GPIO27 | = |
| **DC** | D0 / GPIO26 | **D2 / GPIO28** | **cambia** |
| **RST** | D2 / GPIO28 | **no existe** (RST del display a 3V3) | **cambia** (`RST = -1`) |
| **Energía display** | — (siempre ON) | **D3 / GPIO5** (LOW = ON) | **nuevo** |
| **Batería (ADC)** | — (simulada) | **D0 / GPIO26 (ADC0)** | **nuevo** |
| Botón azul | D6 / GPIO0 | D6 / GPIO0 (`BTN1`) | = |
| **Botón verde** | **D7 / GPIO1** | **D9 / GPIO4** (`BTN2`) | **cambia** |
| **INT2 del LIS3DH** | — | **D7 / GPIO1** | **nuevo (choca con el botón verde actual)** |
| I2C | — | **I2C1**: SDA D4/GPIO6, SCL D5/GPIO7 (`Wire1`) | nuevo |

Código de referencia para la PCB (usar los alias `D*` del core en vez de números crudos):

```cpp
#define DISP_PWR   D3   // GPIO5  — gate de Q1: LOW = display ON
#define SPI_SCK    D8   // GPIO2  [SPI0]
#define SPI_MOSI   D10  // GPIO3  [SPI0]
#define DISP_CS    D1   // GPIO27
#define DISP_DC    D2   // GPIO28
#define I2C_SDA    D4   // GPIO6  [I2C1 / Wire1]
#define I2C_SCL    D5   // GPIO7
#define LIS_INT2   D7   // GPIO1
#define BTN1       D6   // GPIO0
#define BTN2       D9   // GPIO4
#define VBAT_ADC   D0   // GPIO26 / ADC0

// batería: divisor 1:2
analogReadResolution(12);
float vbat = (analogRead(VBAT_ADC) * 3.3f / 4095.0f) * 2.0f;   // <3.0 V apagar · <3.5 V aviso
```

Con `RST = -1` en la librería de la pantalla, comprobar que se haga un reset por software y que el display arranque tras conmutar Q1 (ver [inconsistencia 4](09-inconsistencias.md)).

## Controles actuales

| Acción | Resultado |
|---|---|
| Azul corto | Siguiente emoción (~3.4 s): curioso, feliz, amor, sorprendido, orgulloso, hype, cara llena, pantalla de color, enojado, triste, ansioso, frío, mareado, bostezo, batería baja, guiño, eléctrico |
| Azul largo (0.8 s) | Ver batería |
| Verde (mantener) | Amor (corazones) |
| Verde 5 s | Renacer: criatura nueva + intro + nacimiento, se guarda |
| 35 s sin tocar | Bosteza |
| 100 s sin tocar | Se duerme (arcos que respiran) |

## Serial de pruebas (115200)
`1..9` expresión · espacio siguiente · `t` otro color (sin guardar) · `n` criatura nueva sin guardar · `R` renacer y guardar · `?` imprime `[ID]` · `w` guiño · `v` batería · `j` ruido · `k`/`K` sacudida · `x` desprecio · `b` batería simulada · `f` velocidad SPI (25/31/42/62 MHz) · `d` línea de rendimiento. **Evitar `u` (USB) e `i` (frío)**: dejan banderas activas hasta reiniciar.

## Compilar y subir

```
cd firmware
arduino-cli compile --fqbn rp2040:rp2040:seeed_xiao_rp2350:freq=250,opt=Optimize2 Kiro_Eyes
arduino-cli upload  -p COM6 --fqbn rp2040:rp2040:seeed_xiao_rp2350:freq=250,opt=Optimize2 Kiro_Eyes
```
Cerrar cualquier monitor serie antes de subir. La compilación tarda más de 2 minutos la primera vez. Uso: ~116 KB de flash (5 %), ~253 KB de RAM (48 %).

## Probar sin placa
`herramientas/arnes/` compila el mismo `EyesCore.h` en el PC (`harness.cpp`, `ident.cpp`, `soak.cpp`) y vuelca cuadros RGB565 para revisar animaciones. `herramientas/serial/` tiene las pruebas de estrés y de identidad contra la placa.

## Requisitos pendientes de implementar con la PCB
Ver [10. Pendientes](10-pendientes.md): REQ-1 (auto-sueño + despertar por INT2), REQ-2 (RTC y ciclos día/noche), REQ-3 (temperatura), REQ-4 (vibraciones).
