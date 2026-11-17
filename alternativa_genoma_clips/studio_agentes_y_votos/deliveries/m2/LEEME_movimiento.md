# Acelerómetro virtual del simulador (M2)

Copiar `viewer.py` y `eyes_sim.cpp` a `sim/`. Corre `python viewer.py`. Panel a la derecha: vista 3D del dispositivo, barras ax/ay/az (g), energía de sacudida (marcas 0.85 = suave, 2.0 = fuerte), último evento, expresión y clips activos. `sim_motion` se llama a 60 Hz fijos.

## Teclas
Modo sensor = **mantener TAB** o **`** (fijo). En ese modo las letras son gestos; al soltar, todas las teclas de siempre vuelven (S foto, F batería llena, B/N/O expresiones, U USB, W guiño, R pupilas).

| Tecla (modo sensor) | Gesto |
|---|---|
| TAB + arrastrar clic izq | inclinar (horizontal = balanceo, vertical = cabeceo; arriba = pantalla mira al techo); al soltar vuelve suave |
| S / Shift+S | sacudida suave / fuerte (~1 s, ruidosa) |
| F | caída libre (0.55 s a ~0 g) + impacto |
| V / B / N | de cabeza / boca abajo / plano boca arriba (se mantienen) |
| U | recogerlo (si no está plano, lo deja plano ~3 s y lo levanta) |
| W (mantener) | caminar: rebote 1.9 Hz + vaivén 0.95 Hz |
| R | de pie (reset) |
| O | sensor OFF (`sim_motion_off`); cualquier gesto/TAB lo reactiva |

Sin TAB, clic izq sigue siendo "tocar". El sensor arranca apagado (comportamiento previo intacto).

## Exports nuevos en eyes_sim.cpp (solo lectura, no tocan firmware)
- `sim_motion_state(float o[16])`: mG[3], mShake, mDown[2], mRoll, mFalling, mWasUpside, mActive, última lectura[3], mUpsideT, mFlatT, mFaceDownT.
- `sim_motion_cool(int k)`: marca de tiempo del último evento tipo k (cambia al dispararse; así se detecta el evento).
- `sim_clip_active(int ids[3])`: id de clip por ranura (-1 libre).

## Prueba sin ventana
`python viewer.py --motion-test [DIR]` ejecuta cada gesto contra la DLL, imprime los eventos detectados y guarda `panel.png`. Resultados: suave -> sacudida suave; caída -> caída libre + impacto; V -> de cabeza (1.9 s) y al volver "posición normal"; B -> boca abajo; caminar/arrastre/reposo -> sin falsos eventos.

## Hallazgos sobre Motion.h (no editado)
1. **Sacudida fuerte nunca sale como fuerte**: el filtro de mShake pasa por 0.85 antes que por 2.0, dispara "suave" y el enfriamiento (1500 ms) bloquea la fuerte; además dispara impactos (dev>2.4). Sugerencia: evaluar la fuerte primero con un umbral sostenido, o un enfriamiento por separado.
2. **"Recogido" (MC_PICKUP) es prácticamente inalcanzable**: la rama else se evalúa en el primer cuadro en que mG[2]<=0.85 o mShake>=0.15 y exige mG[2]<0.5 en ese mismo cuadro, pero el filtro (tau 0.25 s) no baja tanto en un cuadro; mFlatT se reinicia antes. Sugerencia: una bandera `mWasFlat` y disparar cuando mG[2]<0.6.
