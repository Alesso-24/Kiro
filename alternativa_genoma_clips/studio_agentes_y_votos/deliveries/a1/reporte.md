# A1 - Animador alegria / amor / carino

**Entrega**: 24 clips TAG 1 + 6 extras TAG 0 (idle) en `a1_alegria.clip.txt` (compilado: `a1_alegria.h`, prefijo a1; 244 keyframes, todos <=12 por clip, duracion 400-2450 ms, todos terminan en 0). check.py: OK sin errores graves (normal y espejo).

**TAG 1**: saltito_felicidad, bailecito, temblor_emocion, risa_contenida, abrazo, derretirse_ternura, latido_enamorado (heart x2), mirada_sonadora, guino_coqueto (wink), beso_volado (heart), ojos_ilusion (ps+, star), verguenza, orgullo_contento, celebracion_confeti (sparks x2), cosquillas, ronroneo, acurrucarse, me_quieres, aplauso (spark x2), pop_alegria, meneo_cola, suspiro_feliz, carcajada, timido_contento.
**TAG 0**: respiro_suave, mirada_flotante, parpadeo_carino, ladeo_dulce, estiron_chico, tarareo_feliz (note).
`mirror` en todos los asimetricos (bailecito, risa, mirada, guino, beso, verguenza, cosquillas, acurrucarse, me_quieres, meneo, timido, ladeo, mirada_flotante, tarareo).

**Criterios**: anticipacion (contra-squash antes de saltos/abrazo/beso), squash&stretch con volumen (sy+ => sx-), overshoot y asentamiento, arcos (ox/oy fuera de fase), follow-through (parpados/mirada llegan despues del cuerpo). Diferenciacion: saltito=1 salto, celebracion=2 brincos+tilt, pop=400 ms, carcajada=rebotes rapidos con ojos-arco, bailecito=vaiven lento, meneo_cola=rapido y decreciente.

**Hallazgos / riesgos**
- `clips_tagN.png` de check.py sale contaminado: el director/autonomia cambia expresion y look durante la tira (ojos crema, dormir), asi que no sirve para juzgar clips. Use mi arnes `mysheet.py` (expresion 1 fija, `sim_set_expr(1,0,0)`) -> `a1_tag1_m0/m1.png`, `a1_tag0_m0.png`. `--quick` de check.py no acelera nada (~100 s).
- Parpados: suma lt+lb >= ~0.85 hace desaparecer el ojo (guino_coqueto deja el ojo guinado invisible en vez de arco; sugerencia al motor: parpado con curva ^). Limite practico para ojos sonrientes: lt<=0.4, lb<=0.25.
- Descartado: lt/lb altos en cosquillas/ronroneo/carcajada (ojos = barras planas), reducidos.
- El directorio scratchpad es compartido entre agentes (otro agente sobrescribio mi script); mi arnes vive en el sandbox.
