# A5 Animador - curiosidad / sorpresa / juego / micro-acciones (prefijo a5)

Entregado: 29 clips en `a5_curiosidad.clip.txt` (+ `a5_curiosidad.h`, 235 keyframes, todos <=12 keys, 300-2200 ms, acaban en reposo).
Se piden 28: el listado del brief tenia 29 items (sorpresa_pop y susto_risa por separado); se dejaron todos.

- TAG 5 (19): ladeo_curioso, asomarse, mira_que_pasa, sigue_mosca, eh_doble, sorpresa_pop, susto_risa, olfatear, estornudo (ev=dots), mira_estrella, descubrir, guino_amigo, escondidas, rebote_pelota, perseguir_cola, timida_acerca, mareo_jugueton, ta_da, pensar.
- TAG 0 (10): hipo, asentir_dos, negar, encoger_hombros, parpadeo_doble, parpadeo_confianza, estirar_ojos, mirada_reojo, mirada_vacio, bostezo_contagio.
- `mirror` en todos los asimetricos (ladeo, mira_que_pasa, mosca, eh, estrella, guino, reojo, escondidas, pelota, cola, timida, mareo, ta_da, pensar, negar, hombros, vacio).

Criterios: anticipacion (contra-movimiento antes de cada accion), squash&stretch con volumen (sy sube / sx baja; en acercamientos sube ambos), overshoot por Catmull-Rom, arcos en ox/oy (cola, pelota), mirada que se adelanta al cuerpo (gx/gy antes que ox), reposo largo. Pupila: se encoge en susto, se dilata en descubrir/estrella. Cada uno tiene firma propia (p.ej. descubrir = mirada abajo + pupilas grandes; sorpresa = pupilas chicas + lineas).

Descartado: espirales de pupila (no hay canal), cierre total de parpado en guino/parpadeo_confianza (dejaba el ojo invisible: ahora queda una rendija).

Hallazgos / riesgos:
1. `clips_tagN.png` de check.py queda contaminado: a ~0.5 s la criatura corre su autonomia (idle act, cambio de look, hype de arranque) y las tiras se ven con otro color/expresion. Para revisar use `a5_hoja1/2.png` (script propio: set_expr(0) tras 10 cuadros, 90 de asentamiento, clip, 16 muestras; sin autonomia).
2. `sim/viewer.py build()` sella la DLL solo con mtime de EyesCore.h y eyes_sim.cpp: si solo cambian los datos (.h de clips) se reutiliza una DLL vieja. Hay que `touch EyesCore.h` en el sandbox.
3. Clips con parpado a ~1 (escondidas, estirar_ojos, estornudo) dejan la pantalla casi negra 0.3-0.6 s: intencional, pero el director deberia espaciarlos.
4. Nombres que chocan en el repo son solo los base y de otros agentes (acurrucarse, arrullo, verguenza); ninguno de a5.
