# Reporte M1 — reacciones de movimiento

**Entregado**: `guion_reacciones.md` (26 gestos con umbrales, duración, enfriamiento, reacción y anti-falsos-positivos), `Motion.h` v2, 20 clips TAG 7 (`m1_reacciones.clip.txt/.h`, prefijo m1, 158 keyframes, 300-2400 ms, todos terminan en 0, 19 con `mirror`), `clips_tag7.png`, `tests/mtest_gestos.cpp` (arnés g++ con gestos sintéticos).
Clips: sacudida_suave, sacudida_fuerte, caida, impacto, boca_abajo, m1_asiente (el base `asiente` sigue; Motion.h usa m1_asiente y cae a asiente), arrullo, mecido, caminata, giro, tap, doble_tap, recogido, puesto_en_mesa, mareo_de_giro, alivio, susto_de_caida, aterrizaje, vibracion, cosquillas_de_mov.

## Cambios en Motion.h (API intacta: `motionFeed(ax,ay,az,now,dt)`, `mActive`, `motionReact(what,strength,now)`; códigos 0-7 conservados, nuevos 8-26)
- Entrada saneada (NaN/inf/dt) — probado con 200 cuadros de NaN/±inf/±1e30: estado finito.
- Sacudida exige inversiones de signo + energía sostenida; impacto sin caída se confirma 0.12 s y no se dispara dentro de una sacudida (antes lo hacía cada 0.4 s).
- Ventana de aterrizaje 0.22 s: impacto / aterrizaje / susto / alivio; "lanzado" vs "se cayó".
- Nuevos detectores: tap + doble tap, recogido (quietud previa), puesto en mesa, mecer/arrullar (osciladores por eje), marcha/correr, giro + mareo (giro neto con signo), vibración, cosquillas, inclinado largo, aburrido plano, sueño por quietud.
- `mCool` crece a 24 entradas; un gancho `#ifdef MOTION_TRACE` (inexistente en firmware) permite probar.

## Validación
`check.py studio/work/m1`: OK (24 clips normal+espejo, 600 cuadros aleatorios, 120 semillas x 20 expresiones). Arnés de gestos: caída/golpe/alivio/aterrizaje/susto, tap/doble tap/3 taps, marcha/correr/llegada, giro/mareo, mecer/arrullo, vibración, cosquillas, de cabeza, boca abajo, plano 30 s: todos disparan lo esperado; 20 000 cuadros de ruido gaussiano sin NaN.

## Riesgos / pendientes
- Umbrales calibrados con señales sintéticas; hay que afinarlos con un MPU real (ruido, montaje, 200 Hz). Alternancia de vibración depende del muestreo.
- Ambigüedades conocidas: llevarlo en la mano con balanceo lento puede leerse como "mecido"; poner en mesa justo tras caminar (< 1.4 s) se reporta como "llegada" y no como "puesto en mesa"; el giro plano sobre la mesa requiere giroscopio.
- Los clips que cierran mucho los párpados (boca_abajo, arrullo) se ven casi negros en la hoja; es intencional pero conviene revisarlo en el simulador.
- Descartado: clip propio para "inclinado" y "aburrido" (bastan expresión + mirada) para no pasar de 20 clips.
