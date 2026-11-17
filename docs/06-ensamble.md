# 6. Ensamble

Todo el ensamble se hace **a mano** (estación de aire caliente 858D ~260–270 °C, pasta de soldar y cautín).

## Orden recomendado

| # | Paso | Método | Notas |
|---|---|---|---|
| 1 | Preparar el XIAO: puente de soldadura en **BAT+/BAT−** | Cautín | Pads en la **parte de abajo** del módulo; hacerlo **antes** de montarlo en la PCB |
| 2 | SMD 0402: R1–R5, C1–C5 | Aire caliente + pasta | R1, R2, R3, R5, C1, C2, C4 van en **Bottom**; R4, C3, C5 en Top (según el Pick & Place) |
| 3 | Q1 (SOT-23) | Aire caliente | Verificar orientación del pin 1 (gate) |
| 4 | **U2 LIS3DHTR (LGA-16)** | Aire caliente + flux generoso | Pads **bajo** el chip. Requiere ENIG |
| 5 | Verificar U2 | Multímetro | Continuidad de SDA/SCL/INT2 y que **INT1 y pines NC no estén a GND**; aislamiento 3V3–GND |
| 6 | XIAO | Cautín | Castellated holes accesibles desde los lados |
| 7 | J1–J4 (through-hole) | Cautín | J1 = header del display; J2 batería; J3/J4 botones |

## Pruebas después de soldar
1. **Sin batería ni display**: conectar USB-C y medir 3V3 en el pad `3V3`.
2. Escanear I2C (debe aparecer **0x18**).
3. Probar `DISP_PWR`: GPIO en LOW debe dar 3.3 V en `VCC_DISP`; en HIGH, ~0 V.
4. Leer `VBAT_DIV` con la batería conectada (≈ Vbat/2).
5. Conectar el display y correr el firmware de [pruebas](07-firmware.md).

## Referencias de la práctica
La metodología de diseño en Altium está documentada en la guía "Creación de PCBs → Altium" del portafolio (alesso-24.github.io/portfolio).
