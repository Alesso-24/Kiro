# AUD_animacion — Supervisión de animación (169 clips)

Método y límites: leí contrato, biblia, motor (`EyesCore.h`, `Motion.h`) y parseé los 169 clips (duración, picos por canal, reposo, cierre de párpados, similitud de trayectorias). Visual: **las tiras oficiales `clips_tag0..7.png` están contaminadas**: a mitad de cada fila la criatura se transforma en DORMIDO naranja (no hay `sim_set_expr(1)` antes de `clip_play` en `tools/check.py`; arreglar). Por eso juzgué con las hojas limpias de los agentes (a1 completa, a2, a3 completa, a4 filas 4-13, a5 completa, a6) y con los números. Tag 7 (m1) solo con números. Los puntos 2.-5. del motor (abajo) son hallazgos reales.

## 0. Hallazgos críticos del motor/datos
1. **`clipFind("arrullo")` devuelve el de a4 (`loop`)**, no el de m1: la reacción de movimiento "arrullo" queda en bucle infinito. Además el director puede elegir `arrullo` y `respira_dormido` (loop, TAG 4): un clip `loop` no se apaga solo y se queda hasta que 3 clips nuevos lo desalojan. **Cortar ambos.**
2. Nombres duplicados: `verguenza` (a1/a3), `acurrucarse` (a1/a4), `arrullo`. Con los cortes de abajo quedan únicos.
3. Residuos de reposo: `timido_contento` (lt=0.05 queda), `celos` (sx=-0.03), `cruzar_brazos` (gy=0.4: la mirada se queda abajo). Nota: `lt`/`lb` sobrescriben ltl/ltr y lbl/lbr, así que no hay más fugas.
4. TAG 6 (22 clips) y TAG 7: el director nunca los elige. Los 18 de TAG 6 que quedan solo salen con la tecla `c`: **conectarlos a eventos** (USB, carga completa, batería baja, apagado) o serán peso muerto.
5. Párpados: el núcleo acota pose+clip a 0.84, pero 19 clips piden suma >0.9 (plateau plano = animación que "se atasca"): `escondidas` (2.2), `sueno_agitado`, `tallarse_ojos`, `guino_al_usuario`, `parpadeo_confianza`... Regla: en TAG 4 multiplicar todo lt/ltl/ltr por 0.85 (picos 0.75-0.90 pasan a ≤0.7).
6. Idle aleatorio con `negar`, `encoger_hombros`, `asentir_dos` (TAG 0) parece un tic sin causa: retag `expr=5` (negar, encoger_hombros) o cortar.

## 1. Evaluación por clip
Formato: `nombre (calificación)`. Solo se detalla en tabla lo que no es MANTENER 4-5. MANTENER 5 = joyas.

**MANTENER 5:** saltito_felicidad, abrazo, derretirse_ternura, latido_enamorado, guino_coqueto, beso_volado, ojos_ilusion, celebracion_confeti, pop_alegria, travesura, hipo, puchero, pisoton, rabieta_berrinche, explosion_enojo, sollozo, ojitos_llorosos, esconderse, susto, ojos_de_perrito, cabeceo, bostezo, estira_al_despertar, asomarse, sorpresa_pop, estornudo, rebote_pelota, ta_da, carga_completa, encendido_crt, cargado_de_poder, sacudida_fuerte, caida, impacto, puesto_en_mesa, susto_de_caida.

**MANTENER 4:** bailecito, temblor_emocion, risa_contenida, verguenza(a1), orgullo_contento, cosquillas, ronroneo, me_quieres, aplauso, suspiro_feliz, estiron_chico, desconfianza, encoger_hombros, parpadeo_doble, inflarse_coraje, mirada_de_lado, hmph, enfado_se_deshace, me_ignoras, mirada_fulminante, arrepentimiento, suspiro_triste, temblor_de_miedo, tiritar_de_frio, soledad, decepcion, secarse_lagrima, no_me_dejes, consuelo_recibido, parpados_pesados, parpadeo_lento, desperezarse, acurrucarse(a4), ronquido_rebote, sonar_bonito, cinco_minutos_mas, despertar_feliz, insomnio, ladeo_curioso, eh_doble, susto_risa, olfatear, descubrir, guino_amigo, perseguir_cola, timida_acerca, pensar, arranque_ojos, apagado_lento, conectar_cable, hambre_bateria, tambaleo_sin_energia, sorbo_de_energia, sobrecarga, glitch, reiniciando, flash_pantalla, mirar_a_camara, sacudida_suave, boca_abajo, m1_asiente, arrullo(m1), mecido, giro, doble_tap, recogido, mareo_de_giro, alivio, aterrizaje, cosquillas_de_mov.

**MANTENER 3** (aceptables, sin urgencia): meneo, ladeo_dulce, orgullo_cara_alta, mirada_reojo, mirada_vacio, mirada_sonadora, patada_al_aire, irritacion_lenta, mareo_suave, mirada_somnolienta, asiente (fallback de m1_asiente), carga_pulso, cortocircuito_tic, modo_ahorro, caminar_energia, caminata, tap.

| nombre | cal | veredicto | nota |
|---|---|---|---|
| mirada_flotante | 2 | CORTAR | solo mirada (<0.6 gaze, 1 px): ya lo hace la autonomía |
| respiro_suave | 2 | CORTAR | sy 0.05, oy 1.5: imperceptible |
| parpadeo_carino | 3 | CORTAR | fusionar en parpadeo_confianza |
| celos | 2 | CORTAR | dup de mirada_de_lado/desconfianza; residuo sx |
| burla_alternada | 2 | CORTAR | dup de travesura/guino_coqueto |
| asentir_dos | 2 | CORTAR | dup de m1_asiente/asiente |
| estirar_ojos | 3 | CORTAR | dup de estira_al_despertar; cierra a 1.0 |
| salto | 3 | CORTAR | base, dup de saltito_felicidad |
| guino | 3 | CORTAR | 5.º guiño (coqueto, amigo, travesura) |
| acurrucarse(a1) | 3 | CORTAR | dup de abrazo y de a4; nombre repetido |
| meneo_cola | 2 | CORTAR | sy 0.02, dup bailecito |
| carcajada | 3 | CORTAR | dup de cosquillas |
| sacudida_furia | 3 | CORTAR | dup de sacudida_fuerte/rabieta |
| ojos_entrecerrados | 2 | CORTAR | se lee como sueño/aburrimiento, no enojo |
| tic_nervioso | 2 | CORTAR | dup de vibracion/cortocircuito_tic |
| mirar_al_suelo | 3 | CORTAR | dup soledad/decepcion |
| resignacion | 3 | CORTAR | dup suspiro_triste |
| abandono | 3 | CORTAR | dup soledad/no_me_dejes |
| hipo_de_llanto | 2 | CORTAR | lt<0.1, casi no se ve; dup sollozo |
| sudor_frio | 3 | CORTAR | dup temblor_de_miedo |
| tembleque_ansiedad | 3 | CORTAR | dup temblor_de_miedo/tiritar |
| verguenza(a3) | 3 | CORTAR | dup de a1 y de timida_acerca; nombre repetido |
| arrullo(a4) | 1 | CORTAR | LOOP: rompe M1 y director |
| respira_dormido | 2 | CORTAR | LOOP, termina con lt 0.3 |
| sueno_agitado | 3 | CORTAR | ojos negros 2 s; dup sonar_bonito |
| sobresalto_despertar | 3 | CORTAR | dup de susto |
| caida_de_sueno | 3 | CORTAR | dup cabeceo/dormido_a_mitad, 3 s |
| mira_estrella | 3 | CORTAR | dup de ojos_ilusion |
| mareo_jugueton | 3 | CORTAR | dup de perseguir_cola |
| bostezo_apagado | 3 | CORTAR | dup bostezo |
| guino_al_usuario | 3 | CORTAR | guiño nº6; lt 1.15 |
| ojos_corazon_rapido | 3 | CORTAR | dup latido_enamorado |
| ojos_espiral_breve | 3 | CORTAR | dup mareo_de_giro |
| tarareo_feliz | 3 | AJUSTAR | tilt/oy tímidos; sin anticipación |
| negar | 3 | AJUSTAR | retag expr=5 |
| parpadeo_confianza | 3 | AJUSTAR | suma 1.08, plateau de 440 ms |
| bostezo_contagio | 3 | AJUSTAR | lt=1 mantenido |
| timido_contento | 3 | AJUSTAR | residuo lt=0.05 |
| cruzar_brazos | 4 | AJUSTAR | gy=0.4 no vuelve |
| mirada_nerviosa | 3 | AJUSTAR | casi solo pupilas; sin squash |
| tallarse_ojos | 4 | AJUSTAR | ltr 0.9: bajar a 0.7 |
| dormido_a_mitad | 3 | AJUSTAR | lt 0.85 → regla ×0.85 |
| mira_que_pasa | 3 | AJUSTAR | falta anticipación/pausa |
| sigue_mosca | 3 | AJUSTAR | casi imperceptible (ox 2 px) |
| escondidas | 3 | AJUSTAR | ojos negros 640 ms, suma 2.2 |
| vibracion | 3 | AJUSTAR | ox 3 px, SUBTLE; M1 lo usa |

Duplicados semánticos (fusiones): guiños → guino_coqueto + guino_amigo + travesura; bostezos → bostezo + bostezo_contagio; susto → susto + susto_risa + sorpresa_pop; triste-caída (soledad/decepcion/abandono/resignacion) → soledad + decepcion; temblor → temblor_de_miedo + tiritar_de_frio; sueño profundo → cabeceo + dormido_a_mitad.

## 2. (a) CORTES (33 → quedan 136, +5 nuevos = 141)
TAG0: mirada_flotante, respiro_suave, parpadeo_carino, celos, burla_alternada, asentir_dos, estirar_ojos · TAG1: salto, guino, acurrucarse(a1), meneo_cola, carcajada · TAG2: sacudida_furia, ojos_entrecerrados, tic_nervioso · TAG3: mirar_al_suelo, resignacion, abandono, hipo_de_llanto, sudor_frio, tembleque_ansiedad, verguenza(a3) · TAG4: arrullo(a4), respira_dormido, sueno_agitado, sobresalto_despertar, caida_de_sueno · TAG5: mira_estrella, mareo_jugueton · TAG6: bostezo_apagado, guino_al_usuario, ojos_corazon_rapido, ojos_espiral_breve.
Quedan por tag: T0 15, T1 21, T2 14, T3 15, T4 15, T5 18, T6 18, T7 20.

## 3. (b) AJUSTAR: keyframes exactos (reemplazan al clip homónimo)
Cambios de una línea: `timido_contento`: sustituir `2000: lb=0` por `2000: lb=0 lt=0`. `cruzar_brazos`: en `1800:` añadir `gy=0`. `tallarse_ojos`: `ltr=0.9`→`ltr=0.7`. `negar` y `encoger_hombros`: `expr=0`→`expr=5`. TAG 4 (`dormido_a_mitad`, `cabeceo`, `parpados_pesados`, `estira_al_despertar`, `cinco_minutos_mas`, `mirada_somnolienta`): multiplicar lt/ltl/ltr por 0.85.

```
clip parpadeo_confianza expr=0
  0:    lt=0
  160:  lt=0.1 sy=0.03 oy=-1
  420:  lt=0.6 lb=0.18 sy=-0.07 sx=0.05 oy=3
  760:  lt=0.62 lb=0.18 sy=-0.08 sx=0.06 oy=3
  1080: lt=0.1 lb=0 sy=0.04 sx=-0.02 oy=-1 ps=0.1
  1300: lt=0 lb=0 sy=0 sx=0 oy=0 ps=0
end
clip bostezo_contagio expr=0
  0:    sy=0
  200:  sy=0.1 sx=-0.04 oy=-4 ps=-0.1
  520:  sy=0.18 sx=-0.07 oy=-7 lt=0.35
  780:  sy=-0.05 sx=0.09 oy=-2 lt=0.72 lb=0.1 ev=drop
  1300: lt=0.3 lb=0 sy=0.02 sx=0 oy=0
  1650: lt=0 sy=0 ps=0
end
clip tarareo_feliz mirror expr=0
  0:    tilt=0
  220:  tilt=-0.04 oy=2 sy=-0.04 sx=0.03 lb=0.1
  480:  tilt=0.15 oy=-4 sy=0.05 sx=-0.03 lb=0.2 ev=note
  740:  tilt=-0.14 oy=3 sy=-0.05 sx=0.04 lb=0.15
  1000: tilt=0.15 oy=-4 sy=0.05 sx=-0.03 lb=0.2 ev=note
  1260: tilt=-0.1 oy=2 sy=-0.03 sx=0.02 lb=0.1
  1500: tilt=0.03 oy=0 sy=0 sx=0 lb=0.02
  1700: tilt=0 lb=0
end
clip sigue_mosca mirror expr=5
  0:    gx=0
  140:  gx=-0.3 gy=-0.3 sy=0.05 oy=-2
  330:  gx=0.8 gy=-0.85 ox=4 tilt=0.08 oy=-3 ps=0.12
  520:  gx=1 gy=0.1 ox=7 tilt=0.12 oy=0
  700:  gx=0.1 gy=0.7 ox=0 tilt=0 oy=3 sy=-0.03 sx=0.02
  900:  gx=-0.9 gy=0.2 ox=-7 tilt=-0.12 oy=0
  1100: gx=-0.3 gy=-0.8 ox=-3 tilt=-0.06 oy=-3 sy=0.05
  1300: gx=0.6 gy=-0.35 ox=4 tilt=0.07 lt=0.1
  1480: gx=0.5 gy=-0.3 ox=3 tilt=0.05 ev=blink
  1750: gx=0 gy=0 ox=0 tilt=0 oy=0 sy=0 sx=0 lt=0 ps=0
end
clip mira_que_pasa mirror expr=5
  0:    gx=0
  110:  gx=0.2 ox=2 sy=-0.04 sx=0.03
  280:  gx=-1 ox=-8 tilt=-0.06 sy=0.07 sx=-0.04 ps=0.12
  500:  gx=-1 ox=-8 tilt=-0.06 sy=0.04 sx=-0.02
  650:  gx=-0.2 ox=-1 sy=-0.02 lt=0.05
  800:  gx=1 ox=8 tilt=0.09 sy=0.06 sx=-0.03
  1050: gx=1 ox=9 tilt=0.1 sy=0.03
  1300: gx=0.2 ox=1 tilt=0.02 ps=0 sy=-0.02 sx=0.01
  1600: gx=0 ox=0 tilt=0 sy=0 sx=0 lt=0
end
clip escondidas mirror expr=5
  0:    lt=0
  130:  lt=0.2 oy=2 sy=0.03
  300:  lt=0.8 lb=0.04 sy=-0.1 sx=0.08 oy=3
  650:  lt=0.8 lb=0.04
  820:  ltl=0.8 ltr=0.3 gx=0.5 sy=-0.05
  1050: ltl=0.8 ltr=0.2 gx=0.6 ps=0.2
  1250: ltl=-0.05 ltr=-0.05 lb=0 sy=0.12 sx=-0.05 oy=-5 ps=0.3 gx=0 ev=spark
  1480: ltl=0 ltr=0 sy=0.02 oy=-1 ps=0.1
  1750: sy=0 sx=0 oy=0 ps=0
end
clip mirada_nerviosa mirror expr=3
  0:
  100:  ox=2 gx=0.2 lt=0.05 ps=-0.15 sy=0.04 sx=-0.02 oy=-1
  240:  gx=-1 ox=-8 tilt=-0.07
  400:  gx=-1 ox=-8 tilt=-0.07 sy=0.02
  560:  gx=1 ox=8 tilt=0.07 sx=-0.03
  720:  gx=1 ox=8 tilt=0.07
  900:  gx=-0.6 ox=-4 tilt=-0.03 oy=1 sy=-0.03
  1150: gx=-0.5 ox=-3 tilt=-0.02
  1500: ox=0 gx=0 lt=0 ps=0 sy=0 tilt=0 sx=0 oy=0
end
clip vibracion mirror expr=7
  0:    ox=0
  50:   ox=5 sy=-0.04 sx=0.03 ps=0.1 ev=shake
  100:  ox=-5
  150:  ox=5 sy=0.03 sx=-0.02
  200:  ox=-5
  250:  ox=4 sy=-0.04 sx=0.03
  300:  ox=-4
  350:  ox=3
  420:  ox=-3 sy=0.02
  520:  ox=1.5
  650:  ox=0 sy=0 sx=0 ps=0
end
```

## 4. (c) Variedad por TAG (tras cortes)
- **TAG 0 (15)**: peso 1.0 por clip, domina el director. Falta vida ambiente *visible pero discreta* (balanceo calmo, "ojos grandes con parpadeo suave") y quitar gestos con significado (negar/encoger) del idle.
- **TAG 1 (21)**: sobrado; ya hay 3 guiños y 3 corazones. No añadir.
- **TAG 2 (14)**: el más flojo: 8 clips son leves y, sobre una base neutra, párpados bajos se leen como sueño. Faltan: bufido, gruñido vibrante (temblor 20 Hz con squash), mirada asesina lenta, "me cruzo de brazos y doy la espalda" (tilt+ox). Pide a A2 un clip con ltl≠ltr marcado (ceja fingida).
- **TAG 3 (15)**: bien tras limpiar; faltaba congelarse de miedo (nuevo), escalofrío corto, mirada esperanzada hacia arriba.
- **TAG 4 (15)**: medias de 2.7 s, largas para un director que dispara cada 2.2-6.4 s: añadir un microsueño de 800 ms.
- **TAG 5 (18)**: faltan lupa (nuevo), mirada por el rabillo con ojos aplastados, "me asomo desde un borde" (ox grande con squash).
- **TAG 6/7**: no los elige el director; ver hallazgo 4.

## 5. (d) Cinco clips nuevos "wow"
```
clip acecho_y_salto mirror expr=5
  0:    gx=0
  160:  gx=0.5 gy=0.3 sy=0.05 oy=-2 ps=0.2
  380:  gx=0.6 gy=0.4 sy=-0.18 sx=0.14 oy=7 ps=0.5 lt=0.15
  560:  ox=2 tilt=0.04
  640:  ox=-2 tilt=-0.04
  720:  ox=2 tilt=0.04
  800:  ox=0 tilt=0 sy=-0.22 sx=0.17 oy=8 lt=0.2
  930:  sy=0.3 sx=-0.14 oy=-15 ox=8 tilt=0.14 gx=0.8 gy=-0.2 lt=-0.1 ps=0.3 ev=spark
  1100: sy=-0.2 sx=0.16 oy=7 ox=5 tilt=0.05 gx=0 gy=0 ps=0 lt=0.1
  1250: sy=0.07 sx=-0.05 oy=-3 ox=2 tilt=0 lt=0
  1400: sy=-0.03 sx=0.02 oy=1 ox=0
  1600: sy=0 sx=0 oy=0
end
clip lupa_inspector mirror expr=5
  0:    gx=0
  180:  tilt=-0.05 ox=-2 sy=0.04
  420:  tilt=0.14 ox=5 oy=-2 sy=0.08 sx=-0.04 ltl=0.5 lbl=0.38 ltr=-0.12 gx=0.5 gy=0.1 ps=0.35
  700:  tilt=0.17 ox=7 ltl=0.55 lbl=0.4 ltr=-0.15 gx=-0.2 gy=0.4
  980:  gx=0.6 gy=-0.2 ox=5 tilt=0.12
  1200: ltl=0.2 lbl=0.1 ltr=-0.2 sy=0.14 sx=-0.07 oy=-4 ps=0.5 tilt=0.05 gx=0 gy=0 ox=0 ev=spark
  1400: ltl=0 lbl=0 ltr=0 sy=0.03 oy=-1 tilt=0 ps=0.1
  1650: sy=0 sx=0 oy=0 ps=0
end
clip regalo_sorpresa expr=1
  0:    gy=0
  200:  gy=0.7 gx=0.2 sy=-0.06 sx=0.04 oy=3 lt=0.12
  520:  sy=-0.08 sx=0.06 oy=4 lt=0.15
  700:  gy=0 gx=0 sy=0.34 sx=-0.14 oy=-8 ps=0.9 lt=-0.1 ev=sparks
  860:  sy=0.2 sx=-0.08 oy=-16 tilt=0.1 ps=0.8
  1020: sy=-0.18 sx=0.14 oy=5 tilt=-0.1 ps=0.6 lb=0.2
  1180: sy=0.2 sx=-0.09 oy=-12 tilt=0.1 ps=0.7 ev=star
  1340: sy=-0.12 sx=0.1 oy=4 tilt=-0.06 lb=0.28 lt=0.2 ps=0.4 ev=hearts
  1560: sy=0.06 sx=-0.04 oy=-2 tilt=0.02 ps=0.15
  1900: sy=-0.03 sx=0.02 oy=1 tilt=0 ps=0 lb=0.15 lt=0.1
  2200: sy=0 sx=0 oy=0 lb=0 lt=0
end
clip bufido_indignado mirror expr=2
  0:    sy=0
  180:  sy=0.1 sx=-0.05 oy=-4 lt=-0.05 ps=0.1
  420:  sy=0.16 sx=-0.08 oy=-6 ps=0.15
  560:  sy=-0.2 sx=0.16 oy=7 lt=0.34 ps=-0.25 ev=lines
  700:  sy=-0.12 sx=0.1 oy=4 lt=0.3 ox=2
  800:  sy=0.1 sx=-0.05 oy=-3 lt=0.1
  980:  sy=-0.24 sx=0.18 oy=8 lt=0.42 ps=-0.3 ev=shake
  1120: sy=-0.14 sx=0.11 oy=5 tilt=0.1 ox=4 lt=0.38 gx=0.8
  1500: sy=-0.1 sx=0.08 oy=4 tilt=0.12 ox=5 lt=0.3 gx=0.9
  1800: sy=0.02 sx=-0.01 oy=-1 tilt=0.03 ox=1 lt=0.08 gx=0.2 ps=0
  2100: sy=0 sx=0 oy=0 tilt=0 ox=0 lt=0 gx=0
end
clip congelado_de_miedo expr=3
  0:    sy=0
  90:   sy=-0.12 sx=0.07 oy=3 ps=-0.2
  240:  sy=0.28 sx=-0.13 oy=-9 ps=-0.75 lt=-0.1 ev=lines
  420:  sy=0.2 sx=-0.09 oy=-5 ps=-0.7
  1000: ox=0
  1080: ox=2 gx=-0.4
  1160: ox=-2 gx=0.4
  1240: ox=1.5 gx=0
  1600: sy=-0.1 sx=0.07 oy=5 ps=0.2 lt=0.2 ox=0 ev=sweat
  1950: sy=0.02 sx=0 oy=-1 ps=0 lt=0
  2200: sy=0 oy=0
end
```
