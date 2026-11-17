# 1. Concepto

## Qué es Kiro
Un dispositivo tipo Tamagotchi: una pantalla redonda que muestra **dos ojos de color sobre fondo negro**. Los ojos se deforman como gelatina, miran alrededor, parpadean y expresan emociones. Cada Kiro es **único**: al nacer se sortea una semilla de 32 bits que decide colores, forma de los ojos, pupila, espejo, escala, separación y personalidad. La semilla se guarda en la memoria flash (EEPROM emulada), así que al apagar y encender es la **misma criatura**.

Inspiración: [@lilguynet](https://www.instagram.com/lilguynet/) (Lil Guy / Starboy). El nombre **Kiro** es corto, memorable y "brandeable".

## Objetivos
1. Una **PCB propia** (circular) que integre el microcontrolador, el display, un acelerómetro, la batería y dos botones.
2. Animaciones de calidad "de personaje": anticipación, squash & stretch, sin parpadeos feos ni cortes bruscos.
3. **Bajo consumo**: el display se apaga por completo en reposo (Q1) y el RP2350 entra en modo dormant.
4. Que se sienta vivo: reacciona al movimiento, a la hora del día, a la temperatura y a las vibraciones.

## Reglas de diseño visual (gustos del equipo)
- Colores planos sobre negro, sin degradados; ojos de un solo color con pupila simple.
- Nada de estrellitas ni destellos de 4 puntas; nada "recargado" encima de los ojos.
- Cambios de golpe o deformando formas; **sin desvanecidos** (cross-fades).
- Corazones solo rojos; ojo cerrado = trazo de pincel con extremos redondos.

## Requisitos de firmware con la PCB
Estos requisitos dependen del hardware de la placa (ver [pendientes](10-pendientes.md)):

| ID | Requisito | Hardware que lo permite |
|---|---|---|
| REQ-1 | Auto-sueño por quietud y **despertar por movimiento** | LIS3DHTR INT2 → GPIO1 (D7) |
| REQ-2 | **RTC interno** y ciclos día/noche; hora preservada en dormant | RP2350 + batería |
| REQ-3 | Reacción a **temperatura** ambiente (COLD / HOT) | Registro OUT_TEMP del LIS3DHTR |
| REQ-4 | Reacción a **vibraciones / música** fuerte (baile) | Acelerómetro (sin micrófono; v2: MEMS I2S/PDM) |

## Equipo y contexto
Proyecto de **Producción Electrónica** en la **Universidad Iberoamericana Puebla**. Diseño de hardware: Alexa y Alessandro. Firmware y animaciones: Alessandro con asistencia de Claude Code.
