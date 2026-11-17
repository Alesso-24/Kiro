# A3 - Animador: tristeza / miedo / ansiedad / frio / verguenza

**Entrega**: 22 clips `a3_tristeza.clip.txt` (+ `a3_tristeza.h`, `--prefix a3`, TAG `expr=3`), 500-2600 ms, <=12 keyframes, todos terminan en reposo. `check.py`: OK sin errores graves (normal + espejo).
Lista: suspiro_triste, sollozo, mirar_al_suelo, ojitos_llorosos, temblor_de_miedo, esconderse, susto, mirada_nerviosa*, sudor_frio, tiritar_de_frio, soledad*, decepcion, abandono*, ojos_de_perrito*, verguenza*, tembleque_ansiedad, hipo_de_llanto, secarse_lagrima*, mareo_suave*, no_me_dejes, resignacion, consuelo_recibido (* = mirror).

**Criterios**: anticipacion (inhalar/alzar antes de hundirse), squash&stretch con volumen (sy y sx opuestos), overshoot en caidas (esconderse, susto), follow-through (gx/gy y parpados llegan despues del cuerpo), arcos (mareo = circulo ox/oy + mirada), vibracion rapida a 45 ms para miedo/frio. Cada clip tiene un gesto distinto: abandono sigue algo que se va y acaba en "..." (ev=dots); consuelo termina en rebote con ev=spark; no_me_dejes "avanza" con pulsos; perrito es estatico con cabeza ladeada.
**Eventos**: drop (sollozo, ojitos, hipo, no_me_dejes, consuelo), lines (susto), sweat, snow, dots, blink, spark.

**Hallazgos / descartado**:
- Parpados lt/lb=0.3-0.5 tapaban casi todo el ojo (rendija); los reduje a ~0.55x (max ~0.28).
- `clips_tag3.png` de check.py esta contaminado: el "director" autonomo cambia de expresion/look a mitad del clip. Hice `preview_clips.py` (arranca el clip a 1.3 s en expr=1 y vigila que no cambie) -> `a3_prev.png` es la hoja fiable.
- TRAMPA de herramientas: `sim/viewer.py build()` solo recompila el DLL si cambian EyesCore.h/eyes_sim.cpp, NO los datos `.h`; tras editar clips hay que borrar `sim/build/*.dll` o se ven clips viejos.
**Riesgos**: vibraciones de 45 ms (miedo, frio, ansiedad) se submuestrean en la hoja; hay que verlas en el simulador/placa a 60 fps. Todos los clips usan solo ox/oy/tilt/parpados/ps/mirada, sin color, valen para cualquier criatura.
