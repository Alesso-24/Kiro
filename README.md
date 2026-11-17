# Kiro

> Tamagotchi minimalista con PCB circular propia: dos ojos planos de color que sienten, se mueven y reaccionan.
> **Producción Electrónica · IBERO Puebla** · Autores: **Alexa y Alessandro**

![Kiro en Altium, vista 3D](docs/img/pcb_3d_firma_alexa_alessandro.webp)

Kiro está inspirado en [@lilguynet](https://www.instagram.com/lilguynet/). Es un dispositivo de bolsillo con una **placa circular diseñada en Altium**, una **XIAO RP2350**, una pantalla redonda **GC9A01**, un **acelerómetro LIS3DHTR** y una batería LiPo. En la pantalla vive una criatura única (generada a partir de una semilla guardada en flash) que parpadea, se ríe, se enoja, se duerme y reacciona a lo que le haces.

| | |
|---|---|
| MCU | Seeed XIAO RP2350 (Cortex-M33, sin WiFi/BT), corriendo a 250 MHz |
| Pantalla | GC9A01 1.28" redonda, 240×240, SPI |
| Sensor | LIS3DHTR (acelerómetro 3 ejes, I2C, LGA-16) |
| Energía | LiPo 400 mAh · cargador integrado del XIAO · Q1 (P-MOSFET) apaga el display |
| PCB | 2 capas, circular ~55 mm, JLCPCB, hecha en Altium Designer 26 |
| Firmware | Arduino (core Earle Philhower), DMA SPI, 50–60 fps, 23 expresiones + intro + nacimiento |

## Demo en vivo (GitHub Pages)

**https://alesso-24.github.io/Kiro/**

| | |
|---|---|
| [Estudio de animaciones](https://alesso-24.github.io/Kiro/estudio/kiro_v4.html) | Las 23 expresiones, el nacimiento y un tour completo, interactivo |
| [Intros del logo KIRO](https://alesso-24.github.io/Kiro/estudio/intro_kiro_10.html) | 12 propuestas animadas; la elegida es la 12 |
| [Intro + nacimiento completo](https://alesso-24.github.io/Kiro/estudio/intro_completa.html) | Render exacto del firmware, ~46 s |
| [Documentación web](https://alesso-24.github.io/Kiro/capitulos/01-concepto.html) | Los capítulos de abajo, con navegación |

[![Estudio de animaciones](docs/img/web_estudio_amor.png)](https://alesso-24.github.io/Kiro/estudio/kiro_v4.html)

## Documentación

Todo el proyecto está documentado por capítulos, con imágenes:

| # | Capítulo | Qué contiene |
|---|---|---|
| 1 | [Concepto](docs/01-concepto.md) | Qué es Kiro, objetivos, requisitos |
| 2 | [Hardware y BOM](docs/02-hardware.md) | Componentes, números LCSC, arquitectura de energía |
| 3 | [Esquemático](docs/03-esquematico.md) | Redes, mapa de pines verificado, por qué existe cada componente |
| 4 | [PCB](docs/04-pcb.md) | Forma, capas, taladros, DRC, renders de los Gerbers |
| 5 | [Fabricación](docs/05-fabricacion.md) | Paquete de Gerbers, parámetros de JLCPCB/LCSC, pedidos |
| 6 | [Ensamble](docs/06-ensamble.md) | Orden de soldado, verificaciones |
| 7 | [Firmware](docs/07-firmware.md) | Arquitectura, pinout de producción vs protoboard, compilar/subir |
| 8 | [Animaciones](docs/08-animaciones.md) | Las 23 expresiones, intro KIRO, nacimiento |
| 9 | [**Inconsistencias**](docs/09-inconsistencias.md) | Lo que no cuadra entre notas, archivos de Altium y firmware |
| 10 | [Pendientes y bitácora](docs/10-pendientes.md) | Qué falta y qué se hizo cuándo |

## Estructura del repositorio

```
Kiro/
├─ README.md                  este documento
├─ CLAUDE.md                  contexto técnico vivo (para trabajar con Claude Code)
├─ docs/                      documentación por capítulos + docs/img/
├─ hardware/
│  ├─ pcb_altium/             COPIA del proyecto Altium PCB_TAMAGOCHI_FINAL (ver hardware/README.md)
│  └─ fabricacion/            Gerbers_Alux.zip (Oct 31), BOM de JLCPCB/LCSC
├─ firmware/
│  ├─ Kiro_Eyes/              sketch Arduino: Kiro_Eyes.ino + EyesCore.h (motor sin hardware)
│  └─ XIAO_GC9A01_Test/       prueba original pantalla + botones
├─ estudio/                   estudio web de animaciones (kiro_v4.html), intros, calificador
├─ herramientas/              arnés en PC (compila el mismo EyesCore.h) y pruebas por serial
├─ respaldos/                 puntos de restauración del firmware (v37, v38)
└─ alternativa_genoma_clips/  sistema de animación anterior, documentado y NO usado
```

## Compilar y subir el firmware

```
cd firmware
arduino-cli compile --fqbn rp2040:rp2040:seeed_xiao_rp2350:freq=250,opt=Optimize2 Kiro_Eyes
arduino-cli upload  -p COM6 --fqbn rp2040:rp2040:seeed_xiao_rp2350:freq=250,opt=Optimize2 Kiro_Eyes
```

Más detalles (pinout, serial de pruebas, rendimiento) en [docs/07-firmware.md](docs/07-firmware.md).

## Estado (2026-11-17)

- Animaciones y firmware probados en la XIAO + pantalla real (protoboard).
- PCB diseñada, ruteada (46/46 conexiones) y enviada a fabricar; componentes SMD pedidos a LCSC (según las notas del equipo).
- Siguiente: ensamblar la PCB, adaptar el firmware al pinout de producción y agregar sueño por movimiento, RTC, temperatura y vibraciones. Ver [pendientes](docs/10-pendientes.md) y las [inconsistencias detectadas](docs/09-inconsistencias.md) antes de ensamblar.
