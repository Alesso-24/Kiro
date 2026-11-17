# NACIMIENTO aprobado por Alessandro (2026-11-16)
Punto de restauración completo (EyesCore.h, Alloy_Eyes.ino, Data.h, Motion.h, src/datos). Equivale a la versión V29 (`firmware/old/v29_alimenta/`).
Las animaciones (webp, 50 fps) están en `animaciones/nacimiento/`.

## Secuencia (~33 s, se dispara en cada encendido; nada se guarda; botones/eventos ignorados mientras nace)
1. **Preludio (~11.5 s)**, fondo NEGRO:
   - 12 líneas claras (arcos curvos de 26 segmentos, guiones que fluyen hacia dentro) giran siguiendo la silueta circular, aceleran sin parar (w = 1+33x² rad/s, flujo 0.5+13x²) y se contraen en espiral hacia el centro dejando los bordes vacíos (7 s).
   - El centro es una partícula-nube blanca de un solo color: crece (3 -> ~26 px) alimentándose; cada guion que le llega la empuja hacia ese lado y la agranda un poco (bocado); espasmos cada vez más seguidos.
   - Sin frenar, EXPLOTA (0.38 s): aro de onda de choque blanco algo irregular + 20 rayos desiguales; la nube se encoge.
   - Nace el "feto": nube blanca/crema que crece con rebote (latido lub-dub), se parte en dos ojos de nube.
2. **Ruleta (~20 s)**: esos mismos ojos de nube aceleran (cambio de combinación cada 650 -> 33 ms, intervalos algo irregulares). Con cada cambio brusco dan un pasito: otra forma, menos bultos, más color; en un cambio (x >= 0.30) cortan a los ojos de verdad. Siguen cambiando de forma y color (de pastel a muy colorido, elige las paletas más saturadas) con rayas de velocidad, frenan como rodillo, cierran ~2 s (arcos), abren con rayitas y miran alrededor según su personalidad.

## Parámetros clave (EyesCore.h)
PR_W 7000, PR_D 0, PR_R 380, PR_B 3000, PR_S 1300, PR_H 800 (PRE ≈ 12.5 s con el tiempo de cierre), RU_T1 15000, RU_C 550, RU_H 2000, RU_O 750; NTH 12; `thr[]`, `blobAA`, `birthFx/birthAdd/birthTgt`, `evBirth`.

## Pendiente conocido (para después)
Entrecortes / rasgado (tearing): es cosa de la pantalla (GC9A01 sin pin TE; barrido libre). Mitigaciones hechas: SPI 41.7 MHz, envío solo de filas cambiadas (fb vs shown), velocidades laterales limitadas, relleno rápido. Probar serial `f` (62.5 MHz) y, a futuro, otra pantalla/TE.
