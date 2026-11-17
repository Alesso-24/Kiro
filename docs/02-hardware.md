# 2. Hardware y BOM

## Diagrama de bloques

```
 LiPo 400 mAh ──J2──► BAT_P ──► XIAO RP2350 (cargador MCP73831 + LDO 3.3 V integrados, USB-C)
                        │                         │
                        └─ R3/R4 divisor ──► D0 (ADC0)         3V3 ──┬─► LIS3DHTR (I2C1: D4/D5, INT2→D7)
                                                                     ├─► R1/R2 pull-ups I2C
 BTN1 ─J3─► D6      BTN2 ─J4─► D9                                    ├─► Q1 (P-MOSFET) ──► VCC_DISP ──► J1 ──► GC9A01
                                                                     └─► bypass C2/C4          ▲
 D3 (GPIO5) ───────────────────────────────────────── gate de Q1 (LOW = display ON) ─────────┘
 SPI0: D8 (SCK) y D10 (MOSI) · CS D1 · DC D2   ──► J1 (display)
```

## Componentes

| Ref | Componente | Detalle | LCSC | ¿Dónde se consigue? |
|---|---|---|---|---|
| U1 | **XIAO RP2350** (Seeed) | Cortex-M33 dual-core, USB-C, cargador LiPo, LDO 3.3 V. Sin WiFi/BT. Módulo con castellated holes | — | Seeed Studio / Amazon (**no** está en LCSC) |
| U2 | **LIS3DHTR** (ST) | Acelerómetro 3 ejes ±2/4/8/16 g, I2C/SPI, **LGA-16 3×3 mm, pitch 0.5 mm** | C15134 | LCSC (~$1.93) |
| — | **GC9A01 1.28"** | Display TFT circular 240×240, SPI, 7 pines (RST CS DC SDA SCL GND VCC) | — | AliExpress |
| Q1 | **BSS84LT1G** | P-MOSFET SOT-23, −50 V; interruptor de energía del display | C82079 | LCSC |
| R1, R2 | 4.7 kΩ 0402 | Pull-ups I2C (SDA/SCL) | C25900 | LCSC |
| R3, R4 | 100 kΩ 0402 | Divisor de voltaje de batería | C25741 | LCSC |
| R5 | 100 kΩ 0402 | Mantiene Q1 apagado mientras arranca (gate = source) | C25741 | LCSC |
| C1, C2, C3, C5 | 100 nF 0402 | Bypass (VCC_DISP, 3V3, VDD y VDD_IO del LIS3DH) | C1525 | LCSC |
| C4 | 10 µF 0402 **10 V X5R** | Bulk 3V3. **No usar C15525 (6.3 V)**: a 3.3 V pierde 50–70 % de capacidad por DC bias | C19702 | LCSC |
| J1 | Header 1×7, 2.54 mm, THT (DS1021-1X7SF162-B) | Conecta el display | — | genérico |
| J2 | JST **PH** 2.0 mm, 2 pines (S2B-PH-K-S) | Batería LiPo | C157932 | LCSC |
| J3, J4 | 2 pines (en el esquemático: S2B-PH-K-S) | Botón 1 y botón 2 | ver [inconsistencia 8](09-inconsistencias.md) | — |
| BT | LiPo "Unit 60-20-35" | 400 mAh @ 3.7 V = 1.48 Wh | — | — |
| SW | Botones táctiles 6×6 mm | Con cables a J3/J4 | — | — |

> ⚠️ Los valores de la tabla vienen del esquemático (captura en [03](03-esquematico.md)) y de las notas del equipo. El `BOM_ALUX.csv` que viaja dentro del zip de Gerbers (31-oct) es **anterior** y todavía lista C4 = C15525; ver [inconsistencias](09-inconsistencias.md).

## Por qué existe cada componente

**Q1 + R5 — apagar el display.** Un display encendido consume ~20 mA; apagarlo con un P-MOSFET lo deja en ~0 µA durante el sueño profundo. R5 (100 kΩ entre gate y source) mantiene el MOSFET cerrado mientras el RP2350 arranca y los GPIO flotan. Lógica: **GPIO en alto = display OFF**, **GPIO en bajo = display ON**.

**R3 + R4 — leer la batería.** La LiPo va de 3.0 a 4.2 V y el ADC del RP2350 llega a 3.3 V. Dos resistencias iguales dividen entre 2 (1.5–2.1 V); el firmware multiplica por 2. *Costo:* el divisor drena ≈ 21 µA a 4.2 V de forma permanente (relevante para el sueño largo, ver [inconsistencia 12](09-inconsistencias.md)).

**R1 + R2 — pull-ups I2C.** Los pull-ups internos (~50 kΩ) son demasiado lentos para 100 kHz (subida ~2100 ns contra ~1000 ns permitidos). Con 4.7 kΩ baja a ~200 ns.

**C1–C5 — bypass.** Reservorios locales de carga para los picos de corriente; sin ellos el voltaje cae y hay resets. C4 (10 µF) es el bulk de 3V3.

**LIS3DHTR.** Detecta sacudidas, toques, quietud y vibraciones, y su pin INT2 despierta al microcontrolador del modo dormant. También trae un sensor de temperatura interno.

## Presupuesto estimado (notas del equipo)

| Concepto | USD |
|---|---|
| PCB JLCPCB (5 unidades) | ~21 |
| Componentes LCSC | ~18 |
| Envío consolidado | 15–25 |
| XIAO RP2350 | 8–10 |
| Display GC9A01 | 5–8 |
| Batería LiPo | 3–5 |
| **Total** | **~70–87** |
