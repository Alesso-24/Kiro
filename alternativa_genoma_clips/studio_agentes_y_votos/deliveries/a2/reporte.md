# A2 - Enojo / molestia / orgullo / celos / travesura

**Entrega**: 22 clips en `a2_enojo.clip.txt` (compilado: `a2_enojo.h`, `--prefix a2`; 173 keyframes, max 12 por clip, 400-2400 ms, todos terminan en reposo). check.py: 0 errores (normal y espejo). TAG 2: puchero, inflarse_coraje, patada_al_aire, mirada_de_lado, pisoton, sacudida_furia, ojos_entrecerrados, hmph, irritacion_lenta, rabieta_berrinche, enfado_se_deshace, me_ignoras, mirada_fulminante, tic_nervioso, cruzar_brazos, explosion_enojo, arrepentimiento. TAG 0: celos, travesura, orgullo_cara_alta, burla_alternada, desconfianza. `mirror` en todos los asimetricos.

**Criterios**: enojado adorable = anticipacion (inhalar/subir antes de apretar), squash&stretch con volumen (sx sube cuando sy baja), overshoot y asentamiento, pupila que se retrasa/encoge (ps-) en vez de rendijas amenazantes, y finales que "se deshacen" (suspiro, gota, corazones). Cada clip tiene un gesto propio: inflar lento, rebote seco, vibracion, hold inmovil (brazos cruzados), ojo alterno (burla), un solo ojo entrecerrado (desconfianza), ida y vuelta de mirada (celos / me_ignoras).

**Revision visual**: hojas con la criatura en reposo fija (`a2_prev0/1/2.png`, `a2_prev1m.png` = espejo). `clips_tag2.png` de check.py NO sirve para juzgar: la autonomia del simulador cambia expresion/color/duerme a mitad de la tira.

**Descartado**: "ceno que sube y baja" como clip aparte (fusionado en irritacion_lenta para llegar a 22 sin duplicar); ojos_entrecerrados mas abierto que la 1.a version (rendija demasiado siniestra); inflarse_coraje y cruzar_brazos exagerados un poco mas para que se lean.

**Riesgos / hallazgos de herramientas**
- `sim/viewer.py build()` estampa la DLL solo con la mtime de EyesCore.h/eyes_sim.cpp: al editar datos NO recompila y check.py/vistas usan datos viejos. Hubo que borrar `sim/build/*.dll`. Deberia incluir mtime de `src/datos/*.h`.
- No hay canal de parpado inclinado (ceno en V): el enojo se lee por parpado plano + aplastado + ps-, es "molesto/ofendido" tierno, no furia. Un canal `lid angle` (o ltl/ltr con pendiente) daria enojo mas legible.
- Directorio scratchpad compartido: otro agente sobrescribio mi script; usar nombres con prefijo.
