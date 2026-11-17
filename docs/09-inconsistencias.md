# 9. Inconsistencias detectadas

Resultado de comparar tres fuentes: **(A)** las notas de contexto del equipo ("Kiro — Contexto Maestro", 2026-11-17), **(B)** los archivos reales de Altium en `hardware/pcb_altium/` (esquemático `Sheet1.SchDoc`, `Pick Place`, DRC, Gerbers del 31-oct) y **(C)** el firmware y la documentación de este repositorio (+ el archivo de variante del core del XIAO RP2350).

**Gravedad:** 🔴 puede romper el hardware o el firmware · 🟠 error de datos que conviene corregir antes de ensamblar/pedir · 🟡 documentación u observación.

| # | Gravedad | Tema | Resumen |
|---|---|---|---|
| 1 | 🔴 | D3 ≠ GPIO29 | En el XIAO RP2350, D3 = **GPIO5** |
| 2 | 🔴 | Botón verde vs INT2 | El firmware usa D7/GPIO1 para el botón verde; en la PCB D7 es `LIS_INT2` |
| 3 | 🟠 | Pines del LIS3DH | La numeración de pines de las notas no coincide con el esquemático ni con el datasheet |
| 4 | 🟠 | Reset del display | Las notas dicen dos cosas; el esquemático fija RST a 3V3; el firmware aún usa un RST |
| 5 | 🟡 | Q1 | Nombre ambiguo (AO3407 vs BSS84) |
| 6 | 🟡 | Lado de J1 | Notas: Bottom; Pick & Place y Gerbers: **Top** |
| 7 | 🟠 | Contorno de la placa | Notas: circular; Gerbers: círculo + pestaña rectangular |
| 8 | 🟠 | BOM y metadatos | Comentarios de biblioteca (953 kΩ, 470 pF, XH), BOM antiguo, cantidad de JST PH |
| 9 | 🟡 | Nombre en la placa | La serigrafía dice "Alux v1", no Kiro |
| 10 | 🟡 | Archivos "Kiro" no encontrados | `BOM_Kiro.csv`, `Gerbers_Kiro.zip` no están en este equipo |
| 11 | 🟡 | Firmware (frecuencia, expresiones, botones) | Notas describen el sistema de animación anterior |
| 12 | 🟠 | Autonomía en dormant | El estimado de 3–4 meses ignora el divisor de batería |
| 13 | 🟡 | Temperatura del LIS3DH | Es relativa; la precisión "±2 °C" necesita calibración |
| 14 | 🟡 | DRC | Cifras coinciden con `PCB1`; no con la placa de práctica |

---

## 1. 🔴 D3 = GPIO5, no GPIO29
- **Notas (A):** `D3 → GPIO29 → DISP_PWR`, y `#define DISP_PWR 29`.
- **Core del XIAO RP2350 (`pins_arduino.h`):** `D3 = 5`, `D4 = 6`, `D5 = 7`, `D6 = 0`, `D7 = 1`, `D8 = 2`, `D9 = 4`, `D10 = 3`. (En el XIAO **RP2040**, D3 sí era GPIO29.)
- **Impacto:** el *hardware* está bien (la red `DISP_PWR` va al pad D3 del módulo), pero un firmware con `DISP_PWR 29` **no encendería el display** (GPIO29 no sale del módulo).
- **Acción:** en el firmware usar siempre los alias `D3`, `D0`… en lugar de números de GPIO. Ya corregido en [07-firmware](07-firmware.md) y [03-esquematico](03-esquematico.md).

## 2. 🔴 Botón verde (D7/GPIO1) choca con `LIS_INT2`
- **Firmware actual (C):** botón azul D6/GPIO0, **botón verde D7/GPIO1**.
- **PCB (B):** `BTN1` = D6/GPIO0 (igual), **`BTN2` = D9/GPIO4**, y **D7/GPIO1 = `LIS_INT2`**.
- **Impacto:** si se sube el firmware actual a la PCB, la lectura del botón verde será en realidad la línea INT2 del acelerómetro (y el botón físico, en D9, no hará nada). Las notas ya mencionan que DC/RST/DISP_PWR/VBAT cambian, pero **no los botones**.
- **Acción:** cambiar `BTN_GREEN` a GPIO4 (D9) y reservar GPIO1 para INT2. Incluido en la tabla de [07-firmware](07-firmware.md).

## 3. 🟠 Numeración de pines del LIS3DHTR en las notas
Las notas listan: `VDD(pin8), VDD_IO(pin5), CS(pin14), RFU(pin6) a 3V3, SDI(pin3), SPC(pin2), INT2(pin4), SDO(pin7), INT1(pin9)`.

El esquemático (y el datasheet LGA-16 de ST) dicen:

| Señal | Pin real (esquemático) | Pin en las notas |
|---|---|---|
| VDD | **14** | 8 |
| VDD_IO | **1** | 5 |
| CS | **8** | 14 |
| SCL/SPC | **4** | 2 |
| SDA/SDI | **6** | 3 |
| SDO/SA0 | 7 | 7 ✓ |
| INT2 | **9** | 4 |
| INT1 | **11** (sin conectar) | 9 |
| RES | **10 → GND** | "RFU pin 6 → 3V3" ✗ |
| GND | 5 y 12 | — |

- **Impacto:** el *esquemático* está correcto (coincide con el datasheet); las **notas** no. Si alguien soldara/probara siguiendo las notas, mediría los pines equivocados. En especial `RES` debe ir a **GND**, no a 3V3.
- **Acción:** usar las tablas de [03-esquematico](03-esquematico.md) como referencia y corregir el documento `lil-guy-schematic-handoff.md`.

## 4. 🟠 Reset del display
- **Notas (A):** una vez dicen "RST ── 3V3 (siempre HIGH, sin reset por hardware)" y otra "en PCB: RC auto-reset".
- **Esquemático (B):** J1 pin 1 (RST) está en **+3V3**. No hay red RC.
- **Firmware (C):** hoy usa RST en GPIO28, que en la PCB es **`DISP_DC`**.
- **Acción:** fijar `RST = -1` en la librería de la pantalla y verificar en pruebas que el display inicia bien (idealmente con comando SWRESET por software) **cada vez que Q1 vuelve a encender el display**.

## 5. 🟡 Nombre de Q1
La nota titula "AO3407 (BSS84LT1G equivalente)". El esquemático, el BOM y el LCSC (C82079) son **BSS84LT1G** (P-MOSFET −50 V, ~10 Ω). El AO3407 es otro dispositivo (30 V, 4.3 A). Usar **BSS84LT1G**. Nota: con 10 Ω de Rds(on) a 20 mA el MOSFET cae ~0.2 V; el display aún recibe ~3.1 V.

## 6. 🟡 Lado de J1 (header del display)
- **Notas:** "J1 en Bottom Layer".
- **`Pick Place for PCB1.csv`:** J1 → **TopLayer**. El render de los Gerbers muestra `J1` en la serigrafía Top.
- **Acción:** confirmar en el 3D de `PCB1.PcbDoc` y reflejarlo en las notas.

## 7. 🟠 Contorno de la placa
- **Notas:** circular de 55 mm.
- **Gerbers (render):** el contorno (`PCB1.GM`) es un círculo con una **pestaña rectangular** hacia J1. El DRC de `PCB1` además marca 6 colisiones de serigrafía con el borde (el contorno del XIAO y de J1).
- **Acción:** revisar el contorno en el visor de JLCPCB; si la pestaña es intencional, actualizar la descripción de la forma.

## 8. 🟠 BOM y metadatos del esquemático
1. **Descripciones de biblioteca**: los símbolos de resistencias (todas) dicen "953 kΩ" y los capacitores "470 pF 50 V" (esto sale en el *Pick & Place* y en el *Comment*). Los **valores** que muestra la hoja son 4k7 / 100k / 100n / 10u. Los LCSC de la tabla de [hardware](02-hardware.md) son los correctos.
2. **`BOM_ALUX.csv`** (en el zip de Gerbers, 31-oct) lista C4 = **C15525 (6.3 V)** —justo el que las notas prohíben— y trae "4.7k?"/"100k?" con signos de interrogación (codificación).
3. **J2/J3/J4**: comentario `S2B-XH-A(LF)(SN)` pero huella `JST_S2B-PH-K-S` (PH 2.0 mm); en las notas J3/J4 son "JST 2-pin 2.54 mm genérico". En el esquemático los **tres** son S2B-PH-K-S.
4. **Cantidad pedida de C157932 (S2B-PH-K-S): 5.** Si J2, J3 y J4 son PH, 5 placas necesitan **15**. Alcanza para 5 baterías (J2) pero no para los botones.
- **Acción:** poner `Comment = valor` en los símbolos, regenerar el BOM, decidir el tipo de J3/J4 y completar el pedido de JST si hacen falta.

## 9. 🟡 La placa dice "Alux v1"
La serigrafía enviada a fabricar (Gerbers del 31-oct) lleva **"Alux v1"** y el logo IBERO Puebla; el proyecto ahora se llama **Kiro**. Es solo estético, pero la placa física no dirá "Kiro".

## 10. 🟡 Archivos citados que no se encontraron
Las notas mencionan `BOM_Kiro.csv`, `Gerbers_Kiro.zip` y `claude/*.md`. En este equipo solo hay `Gerbers_Alux.zip` y `BOM_ALUX.csv` (31-oct). Los documentos `lil-guy-*.md` no están en este repositorio.

## 11. 🟡 Firmware: las notas describen el sistema anterior
| Tema | Notas (A) | Firmware actual (C) |
|---|---|---|
| Expresiones | 12 (HYPE, CURIOUS, AWAKE…) con coordenadas de iris | **23**, motor nuevo (Estudio v4) |
| Arranque | HYPE → CURIOUS → AWAKE siempre | Intro KIRO + nacimiento **solo al nacer/renacer**; con criatura guardada abre los ojos contenta |
| Botones | BTN1 = wake/on-off, BTN2 = cambiar expresión; BTN1+BTN2 2 s = FULL_FACE | Azul corto = siguiente emoción, azul largo = ver batería, verde = amor, verde 5 s = renacer |
| Sueño | SLEEPING tras 30 s | Bostezo a los 35 s, dormido a los 100 s |
| Frecuencia | `freq=200`, "RP2350 150 MHz" | `freq=250` (overclock) |
| Documentos | `Kiro\docs\` y `claude/kiro-master-context.md` | `README.md` + `docs/` de este repo y `CLAUDE.md` |
- **Impacto de 250 MHz:** más consumo que a 150 MHz; considerar bajar la frecuencia cuando esté en reposo o con batería baja.
- **Acción:** reconciliar el mapa de triggers (`Shake`, `Tap`, `INT2`) con el motor actual al implementar REQ-1..4.

## 12. 🟠 Autonomía en reserva (REQ-2)
Las notas estiman ~10–20 µA en DORMANT y **3–4 meses** con la batería al 15 % (60 mAh). Pero el divisor R3+R4 (200 kΩ total) drena **≈ 21 µA a 4.2 V (≈ 15 µA a 3.0 V)** de forma permanente, y el LDO del XIAO también tiene corriente en reposo.
- Con ~40 µA totales: 60 mAh / 0.04 mA ≈ 1500 h ≈ **2 meses**.
- **Acción:** medir el consumo real en dormant; si hace falta, cortar el divisor con un MOSFET en la v2 (o leer la batería solo de vez en cuando con un pin a alta impedancia).

## 13. 🟡 Temperatura con el LIS3DH (REQ-3)
El sensor de temperatura del LIS3DH entrega una lectura **relativa** de 8 bits (≈ 1 LSB/°C) y requiere habilitar `TEMP_EN` (y `BDU`). No tiene exactitud absoluta garantizada. La notas ya piden calibrar con la carcasa de PLA cerrada; conviene hacerlo con un termómetro de referencia y guardar el offset.

## 14. 🟡 Cifras del DRC
"0 errores eléctricos, 26 cosméticos" **coincide con `PCB1`** (31-oct: 20 silk-máscara + 6 contorno). La placa de práctica `PCB_ALTIUM_PRUEBA` (10-nov) tiene 22. Los pedidos corresponden a `PCB1`.

---

## Lo que sí coincide ✔
- Mapa D0–D10 → redes `VBAT_DIV`, `DISP_CS`, `DISP_DC`, `DISP_PWR`, `I2C_SDA/SCL`, `BTN1`, `LIS_INT2`, `SPI_SCK`, `BTN2`, `SPI_MOSI` (esquemático = notas, salvo el GPIO de D3).
- Header J1: pin 1 = 3V3 (RST), 2 CS, 3 DC, 4 MOSI, 5 SCK, 6 GND, 7 VCC_DISP.
- LIS3DH: SDO a GND (dirección 0x18), INT1 sin conectar, INT2 a D7, CS a 3V3.
- Valores del esquemático: R1/R2 = 4.7 kΩ, R3/R4/R5 = 100 kΩ, C1/C2/C3/C5 = 100 nF, C4 = 10 µF.
- 46/46 conexiones ruteadas; 3 taladros NPTH de 2.2 mm (M2); acabado/espesor/color según las notas.
- Pines del firmware de producción de las notas (excepto D3 y los botones) coinciden con los alias del core.
