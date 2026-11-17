# 10. Pendientes y bitácora

## Lista de pendientes (en orden)

### Antes de ensamblar
- [ ] Resolver las inconsistencias 🟠/🔴 de [9. Inconsistencias](09-inconsistencias.md): contorno (#7), BOM/JST (#8), reset del display (#4).
- [ ] Esperar llegada de PCBs (JLCPCB) y SMD (LCSC).
- [ ] Verificar piezas: LIS3DHTR (7), BSS84 (10), 0402 (100 c/u), JST PH.

### Ensamble ([capítulo 6](06-ensamble.md))
- [ ] SMT 0402 → Q1 → LIS3DHTR (LGA-16, aire caliente + flux) → XIAO → conectores.
- [ ] Verificar I2C del LIS3DH (dirección 0x18) y continuidad con multímetro.

### Firmware de producción
- [ ] Adaptar el pinout ([tabla en 07](07-firmware.md)): DC → D2, sin RST, `DISP_PWR` en D3 (GPIO5), botón verde → D9 (GPIO4), `LIS_INT2` en D7, `VBAT_ADC` en D0.
- [ ] Inicializar Q1 (GPIO en LOW al arrancar el display) y reset por software del GC9A01.
- [ ] **REQ-1** auto-sueño por quietud (10–20 min) + modo DORMANT + despertar por `LIS_INT2` → CURIOSO.
- [ ] **REQ-2** RTC interno, ciclos día/noche, hora preservada en dormant (medir consumo real, ver #12).
- [ ] **REQ-3** temperatura con `OUT_TEMP` + expresiones COLD/HOT (calibrar con la carcasa PLA cerrada).
- [ ] **REQ-4** vibraciones/música con el acelerómetro → HYPE / baile.
- [ ] Confirmar en pantalla real: sin parpadeo, sin tearing, colores correctos, intro y transición fluidas.
- [ ] Lectura de batería real (reemplazar la simulada) y aviso <3.5 V / apagado <3.0 V.

### Diseño / v2
- [ ] Ground pour en la v2.
- [ ] Micrófono MEMS (ICS-43434 / MP34DT01) para detección real de ritmo.
- [ ] Cortar el divisor de batería con un MOSFET para bajar el consumo en dormant.
- [ ] En la próxima revisión de la PCB: serigrafía «Kiro» en lugar de «Alux» (la placa actual se conserva como **Kiro v1**).
- [ ] Calificar las 50 criaturas base (`estudio/calificar_50.html`) para afinar el generador.

### Carcasa
- [ ] **Diseñar la carcasa** (PLA impreso en 3D). Ver [capítulo 11](11-carcasa.md).
- [ ] Calibrar el sensor de temperatura **con la carcasa cerrada** (el PLA agrega un offset).

### Documentación
- [ ] Fotos reales de la PCB ensamblada y del dispositivo funcionando.
- [ ] Capturas de Altium de la PCB1 final (3D top/bottom) para sustituir las de la placa de práctica en el capítulo 4.
- [ ] Fotos de cada paso de soldadura y de las pruebas con multímetro.

## Bitácora

| Fecha | Hito |
|---|---|
| 2026-10-02 | Primeros proyectos en Altium (`PCB_ALTIUM`, práctica de la guía) |
| 2026-10-23 | Carrito de partes de JLCPCB/LCSC exportado |
| 2026-10-31 | `PCB1` ruteada (DRC: 0 errores eléctricos, 26 estéticos), Gerbers `Gerbers_Alux.zip`, Pick & Place, BOM |
| 2026-11-10 | Placa de práctica `PCB_ALTIUM_PRUEBA` (guía del portafolio, 22 violaciones estéticas); firma "designed by Alexa and Alessandro" |
| 2026-11-14 | Cableado en protoboard (XIAO + GC9A01 + 2 botones), bug SPI/DMA resuelto, primera versión aprobada de animaciones |
| 2026-11-14…16 | Sistema genoma + clips, votaciones y nacimiento aprobado (ahora en `alternativa_genoma_clips/`) |
| 2026-11-17 | Estudio v4 definitivo: motor nuevo portado al firmware (23 expresiones), identidad guardada en EEPROM, proyecto renombrado a **Kiro** |
| 2026-11-17 | **V38**: intro KIRO (propuesta 12) + transición de hilos desde los bordes, cargado en la XIAO (50–60 fps) |
| 2026-11-17 | Repositorio y documentación por capítulos; auditoría de inconsistencias |
| 2026-11-17 | Se define la nomenclatura: PCB = **Kiro v1** (ordenada como «Alux V1»); **falta la carcasa** |
| (según notas) | Pedidos JLCPCB y LCSC realizados; display, batería y botones en mano |
