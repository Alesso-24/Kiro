# STATUS (bitácora para retomar) — actualizada por la Dirección

## 2026-11-14 (noche) — Estudio Alloy en marcha
**Pedido del cliente**: estudio de 20-30 agentes (animadores Disney, supervisores, auditores, mercado), cientos de criaturas únicas, muchas animaciones (por forma + colores intercambiables), reacciones a acelerómetro (futuro), que no se trabe en la XIAO. Plan: `docs/PLAN_ESTUDIO.md`.

### Hecho
- Fase 0: plan, `tools/studio_compile.py` (txt->h), `tools/mkwork.py` (sandbox por agente), `tools/check.py` (validador + hojas), contrato `firmware/Alloy_Eyes/CONTRATO_DATOS.md`.
- Fase 1: R1 (`studio/reportes/R1_producto.md`: Starboy real = 500+ animaciones de ex-Disney, OLED 400x400 60 fps, acelerómetro/cámara/mic/temperatura, intercambio de ojos por BT, 400-5000 looks con rareza), R2 (`studio/biblia_de_diseno.md`).
- Motor v4 (Dirección): GENOMA (semilla -> paleta x especie x pupila x dos tonos x personalidad x variaciones), `Data.h` (registros), `src/datos/*` (datos), reproductor de CLIPS aditivos (12 canales, Catmull-Rom, espejo, eventos de partículas, director por personalidad), `Motion.h` (acelerómetro: gravedad continua + eventos), tipos de pupila (brillo, dos tonos, media luna, anillo), ojo de dos tonos.
- BUG corregido: un comentario `//` se tragaba la línea de `presence` -> cara-llena/mareado/dormido-tenue no dibujaban sus extras (afectó el firmware que probó Alessandro tras el cambio de CARGANDO).
- Firmware v4 base cargado en la placa (46 fps prom., sin cuelgues en prueba de 75 s con n/c/etc). Arduino solo copia `.h/.cpp` => los datos viven en `src/datos/*.h` (no `.inc`).

### En curso — Fase 2 lanzada (17 agentes en paralelo, cada uno en `studio/work/<id>` -> entrega en `studio/deliveries/<id>`)
P1 paletas cálidas(70) · P2 frías/neón(70) · P3 raras/míticas(40) · S1 especies tiernas(22) · S2 elegantes(18) · S3 raras(18) · U1 pupilas simples(14) · U2 especiales(14) ·
A1 alegría(24) · A2 enojo(22) · A3 tristeza/miedo(22) · A4 sueño(20) · A5 curiosidad/juego(28) · A6 energía/sistema(22) · M1 reacciones(20 clips + Motion.h + guion) · M2 acelerómetro virtual en viewer.py · M3 banco de pruebas de movimiento.
Cuando llegue cada reporte: revisar `studio/deliveries/<id>/reporte.md` + imágenes.
**Integración**: `python tools/integrate.py --agents p1,p2,...` (regenera manifiestos desde las entregas; luego `tools/check.py`).
Pendiente de lanzar (Fase 3, tras Fase 2): AS supervisor animación, PA auditor color, SA auditor siluetas, E1 rendimiento, Q1 perf, Q2 visual, Q3 código, Q4/Q5 paneles de mercado, J-Arte/J-Tech/J-Calidad.

### Siguiente
Fase 3 (auditoría/supervisión/mercado/jefes) -> Dirección integra `studio/deliveries/*` en `firmware/Alloy_Eyes/src/datos/` (concatenar .h + manifiestos), regenera hojas/GIFs, flashea, mide 10 min, actualiza catálogo y CLAUDE.md.

### Comandos clave
- Validar principal: `python tools/mkwork.py main_test && python tools/check.py studio/work/main_test`
- Compilar placa: `cd firmware && arduino-cli compile --fqbn rp2040:rp2040:seeed_xiao_rp2350:freq=250,opt=Optimize2 Alloy_Eyes` ; subir `-p COM6` (NO dejar flags de prueba por serial: reiniciar/re-subir al terminar).
- Prueba de estabilidad: `/tmp/stress.py` (envía teclas serial n c j k v x g t p w 1..9; mide [PERF]).

## CORTE POR LÍMITE DE USO (2026-11-14, noche) — LEER PARA RETOMAR
- ENTREGADOS en `studio/deliveries/`: u1 (14 pupilas), p2 (70 paletas), p3 (40 paletas raras), s3 (18 especies). Reportes en cada carpeta (`reporte.md`).
- Los demás agentes (p1, s1, s2, u2, a1-a6, m1, m2, m3) SIGUEN trabajando en segundo plano y dejarán su entrega en `studio/deliveries/<id>/`; revisa qué carpetas ya tienen `reporte.md`.
- Hallazgos a atender al integrar: (1) el motor encoge/recorta pupilas en sorprendido/curioso/ansioso y en triste casi no se ven (U1); (2) viewer.py cacheaba la DLL si solo cambiaban datos -> YA corregido en `sim/viewer.py` (invalida con cualquier .h/.txt); (3) p3 no verificó los dos tonos visualmente: probar con `tools/check.py`; (4) Triangulito (S3) parece moño en MAREADO.
- FALTA: esperar el resto de entregas; Fase 3 (supervisor animación, auditores, mercado, jefes, perf); `python tools/integrate.py --agents p1,p2,p3,s1,s2,s3,u1,u2,a1,a2,a3,a4,a5,a6,m1` ; `python tools/check.py studio/work/<copia>`; mezclar `Motion.h` de m1 y `viewer.py` de m2 (revisar a mano); compilar y flashear (`firmware`, FQBN ...:freq=250,opt=Optimize2), probar 10 min con /tmp/stress.py (recrear si falta; NO dejar flags usb/frío), regenerar GIFs/catálogo, actualizar CLAUDE.md.
- La placa tiene cargado el motor v4 BASE (genoma + clips base), sin los datos de los agentes.


## FASE 2 COMPLETA + INTEGRADA (2026-11-14 noche, tras reanudar agentes)
- Entregaron los 17 agentes (carpetas `studio/deliveries/*`; a4 y m3 no pudieron escribir reporte.md: su contenido vino en el mensaje; m3 dejó `reporte_actual.md`/`reporte_propuesto.md`/`Motion_propuesto.h`).
- INTEGRADO en `firmware/Alloy_Eyes/src/datos/` con `tools/integrate.py`: **196 paletas, 64 especies, 36 pupilas, 169 clips** (451,584 combinaciones base x semilla continua x personalidad). Duplicados de nombre detectados (cometa, verguenza, acurrucarse, arrullo, 3 paletas): inofensivos, limpiar en auditoría.
- Cambios de Dirección: `Motion.h` = versión de M1 (copia anterior en `firmware/old/v4_base/`), `sim/viewer.py` y `sim/eyes_sim.cpp` = versión de M2 (acelerómetro virtual: mantener TAB e inclinar con mouse; gestos S/Shift+S/F/V/B/N/U/W/R/O) + `sim_auto`; director NUNCA elige clips tag 6/7; suma de párpados pose+clip acotada a 0.84 (el ojo no desaparece); `autoOn` para pruebas deterministas; `tools/check.py` ahora usa sim_auto(0).
- PROBADO: `check.py` OK (40 semillas x 20 expr, 169 clips normal+espejo, 600 cuadros de movimiento aleatorio). **Placa**: flasheado (150 KB flash, RAM 47%); 120 s ciclando criaturas/clips/expresiones SIN cuelgues, 45 fps prom, mín 31.
- Punto de restauración: `firmware/old/v5_integrado/`.
- Hallazgos pendientes de los agentes: M3 probó el Motion.h ANTIGUO (no el de M1): re-correr `studio/deliveries/m3/test_motion.py --motion firmware/Alloy_Eyes/Motion.h` en un sandbox (mkwork + copiar test + eyes_sim_m3.cpp) para confirmar umbrales; ruido blanco 0.3 g dispara sacudida suave con umbral 0.5 (decisión de diseño); `recogido` casi inalcanzable en el Motion antiguo (M1 dice corregido); clips de movimiento `boca_abajo`/`arrullo` se ven casi negros por párpados (intencional, revisar); A2: no hay párpado inclinado en clips (la expresión ENOJADO sí); A3: vibraciones de miedo/frío se ven poco.
- SIGUIENTE: Fase 3 auditorías (arte, animación, mercado, técnico) -> lista de podas/arreglos -> aplicar -> regenerar catálogo (`animaciones/`, `catalogo/`) -> flashear limpio (`stress` NO deja flags) -> actualizar CLAUDE.md.

## FASE 3 (auditorías) APLICADA (2026-11-14 noche)
- Informes: `studio/reportes/AUD_arte.md`, `AUD_animacion.md`, `AUD_mercado.md` (+ `AUD_tecnico.md` si ya llegó).
- `python tools/integrate.py --agents p1,p2,p3,s1,s2,s3,u1,u2,a1,a2,a3,a4,a5,a6,m1 && python tools/curate.py` aplica TODO (curate.py = cortes y ajustes de los auditores; ejecutarlo SIEMPRE justo después de integrate).
- Resultado curado: **165 paletas, 48 especies, 18 pupilas, 141 clips (142,560 combinaciones base)**. Se cortaron duplicados/feos, se subieron pupilas casi negras, se cerraron clips en reposo, se quitaron los clips en loop (arrullo/respira_dormido) y se añadieron los 5 clips nuevos + 8 ajustados del supervisor.
- Motor: corregido el recorte plano del ojo con gota/huevo (caja de párpados incluye el abultamiento c4); reglas de exclusión del genoma (pupila diminuta en ojo grande; dos tonos + pupila partida/anillo).
- Placa: cargada la versión curada (143 KB flash, RAM 47%), 120 s de estrés sin cuelgues, 45 fps prom (mín 33). Punto de restauración: `firmware/old/v6_curado/`.
- Pendiente (de mercado): rediseñar expresiones confusas (cara-llena, batería-baja, mareado, ver-batería; 9≈3, 7≈8, 1/15/17/18 muy parecidas), filtro daltónico en check.py, guardar estado en flash, mostrar rareza al encender, conectar clips tag 6 a eventos (USB/carga/batería), re-correr test_motion.py de M3 contra el Motion.h de M1.

## AUDITORÍA TÉCNICA APLICADA (fin de la sesión de estudio, 2026-11-14)
- `studio/reportes/AUD_tecnico.md`: ~5 M de cuadros de prueba, 0 estados inválidos/cuelgues; 13 hallazgos.
- APLICADO: `directorClip` sin arreglo en la pila (el core 0 tiene 2 KB) y sin elegir el último clip por redondeo; `clampf` convierte NaN en el mínimo; espera del DMA/SPI con tiempo límite (aborta en vez de congelar).
- NO aplicado (decisión): filas sucias exactas (-25% SPI, techo 54->60 fps), `renderEye` con párpados primero (-20% render), SPI a 41.7 MHz (probar con serial `f` en la placa), desborde de millis() a los 49 días, `t` en float tras 240 h, `Serial.printf` bloqueante, diente del lóbulo cúbico (decisión de arte).
- Placa: versión final cargada (144 KB flash), 120 s de estrés sin cuelgues, 46 fps prom (mín 31). Respaldo: `firmware/old/v7_final/`.
- Lo que falta (ver también sección de mercado arriba): rediseñar expresiones confusas, filtro daltónico, guardar estado en flash, mostrar rareza al encender, conectar clips tag 6 a eventos, re-correr M3 contra Motion.h de M1, probar umbrales con MPU real.

## RONDA 1 DE VOTOS (2026-11-14)
- Alessandro votó 61 combinaciones (28 me encanta / 19 más o menos / 14 no): `studio/votos/ronda1.json`. Rechazo claro: pupilas con punto blanco, de dos colores (partida), anillo y puntito.
- `tools/aplicar_votos.py` (por nombre, idempotente) cortó 5 pupilas, 6 especies, 3 paletas; se agregó `src/datos/v1_votos.pu.txt` (6 pupilas nuevas de las familias bien votadas). Ahora: 162 paletas, 42 especies, 19 pupilas. Si se vuelve a correr integrate+curate hay que correr aplicar_votos después (y re-agregar v1_votos al manifiesto).
- Mareo: sin espiral en pupilas (ojos normales), órbita 19x15 px; ojos de un solo color (`toneMode = 0`).
- Encuesta: `python tools/encuesta.py <ronda> [N] [semilla]` -> `catalogo/encuesta_rN.html` (webp animado 30 fps, solo anima las visibles; muestreo ponderado por `studio/votos/ronda*.json`). Servir con `python -m http.server 8765 --bind 127.0.0.1` dentro de `catalogo/`. Votos exportados -> copiar a `studio/votos/rondaN.json`.
- Firmware con estos datos cargado en la XIAO.

## RONDA 2 DE VOTOS
- `studio/votos/ronda2.json` (29 me encanta / 23 más o menos / 8 no). Recortes extra en `aplicar_votos.py`: pupila Canica; especies Cometa, Ojo y Rendija, Oraculo, Altito. Quedan 162 paletas, 38 especies, 18 pupilas. Encuesta ronda 3 en `catalogo/encuesta_r3.html` (vista de caritas fijas + animación solo de la seleccionada, teclas 1/2/3). Firmware actualizado en la placa.
- Pendiente: las "personalidades" aún no se votan (solo paleta/especie/pupila); idea ronda 4: mostrar clips/rasgos de personalidad.

## RONDA 3 DE VOTOS + CAMBIO DE DISEÑO
- `studio/votos/ronda3.json` (13 me encanta / 40 más o menos / 7 no). Recortes: pupilas Redonda media y Rendija girada, especie Pluma. Quedan 162 paletas, 37 especies, 16 pupilas.
- Dirección de Alessandro: la **pose base** debe parecerse a los ojos que le encantan (óvalos redondeados, distintos pero cercanos) y lo irregular/variado debe ser **animación** (cuando se mueve y se acomoda). Aplicado en EyesCore.h: las especies se suavizan en reposo (armónicos/inclinaciones x0.4, escalas/gap x0.6, ay x0.5) y `chooseForm` ahora es transitorio (`settleAt`: a los 0.9-1.8 s vuelve a la forma de reposo; cambios cada 3.2-8 s). Encuesta ronda 4: `catalogo/encuesta_r4.html`. Firmware cargado.

## RONDA 4 + PRUEBA DE ANIMACIONES
- `studio/votos/ronda4.json`: 16 "más o menos" explícitos (9 eran rendijas), el resto implícito "me encanta". **Regla nueva**: lo no marcado = me encanta (el sitio exporta todo + campo `nota`). Cortadas pupilas Rendija y Rendija gruesa (quedan 14 pupilas).
- Dirección: pose base = las que le encantan; lo demás animaciones ocasionales / al acomodarse tras movimiento brusco (acelerómetro).
- `python tools/encuesta_anim.py` -> `catalogo/anims.html` (20 expresiones + 141 clips en 2 criaturas favoritas, voto + nota, exporta `alloy_animaciones.json`). Plantilla `tools/anim_template.html`.

- catalogo/anims.html ahora evalua EXPRESIONES (20) + ESCENARIOS/transiciones (20: carga, bateria baja, dormir/despertar, agitar, ruido, frio, desprecio, tocar/amor/acariciar/jugar, cambio de look, emocion a emocion) en 2 criaturas; no los 141 clips. Generador: tools/encuesta_anim.py.

## FEEDBACK DE ANIMACIONES (Alessandro, notas en su JSON) -> APLICADO
- Sin estrellitas (PT_SPARK desactivado en `spawn`, también las que orbitaban en mareo y copos del frío).
- CARGA: ojos grandes (sorprendido) -> chispa (eléctrico) -> cargando (>=14 s, 1.6 %/s) -> feliz -> vuelve a DESPIERTO (ya no se duerme al llenarse).
- SUEÑO: solo tras >100 s sin interacción; antes (>35 s) animaciones de aburrimiento (mirada_vacio, suspiro_triste, mirada_de_lado, parpados_pesados, pensar, cabeceo).
- BATERÍA BAJA = cansado (paleta de la criatura atenuada, párpados pesados, pupila normal) y pide carga con clips hambre_bateria / tambaleo_sin_energia / apagado_lento cada 8-14 s.
- VER BATERÍA (botón verde largo = evThumbs; ' ' sigue recorriendo expresiones por serial): los ojos son el medidor, se llenan de abajo arriba en 1.1 s y pasan rojo -> amarillo -> verde continuo; dura 4.2 s.
- Bostezo/cansancio con pupila normal (no diminuta).
- Movimiento: `slosh` (chapoteo con rebote en bordes: ±30/±24 px, squash al chocar); sacudida suave 0.55 / fuerte 1.0, decae 1.7 s; HYPE rebota de lado a lado; mareo = chapoteo + pequeña órbita. Motion.h llama evShake, así que el acelerómetro hereda esto.
- Corazones de tamaños/vidas variados; rayitas (piquete/susto) 3-6, irregulares. Transiciones: kicks x1.45 y resortes menos amortiguados (c 19->16).
- Respaldo del core previo: firmware/old/EyesCore_pre_ronda4.h. Placa cargada; estrés 120 s: 49 fps prom, mín 31.

## CIERRE DEL DÍA 2026-11-14
Placa cargada limpia con la beta (`firmware/old/v8_beta/`), comandos en `docs/COMANDOS.md`, contexto completo en `CLAUDE.md` (sección BETA). Mañana: continuar evaluación de animaciones en `catalogo/anims.html` (relanzar servidor: `cd catalogo && python -m http.server 8765 --bind 127.0.0.1`).
- Quitado el sueño profundo (a los 5 min de inactividad dejaba de dibujar y la pantalla parecía apagada). Placa recargada.

## NACIMIENTO + BOTONES NUEVOS (2026-11-16)
- Cada encendido: semilla aleatoria (`rp2040.hwrand32()`), **no se guarda** (decisión de Alessandro: cada vez que se desconecta y vuelve a nacer tiene ojos distintos). La criatura no cambia de aspecto sola durante la sesión (idle ya no llama `newLook`).
- Animación de nacimiento: `evBirth` + `birthAdd/birthTgt` en EyesCore.h, 8 variantes (NBIRTH), dirección y duración aleatorias (3.3-4.1 s) y luego feliz. Simulador: `sim_birth`, página `catalogo/anims.html` grupo "Nacimiento".
- Botones: azul corto = `evEmotion` (emoción al azar 2.4-3.3 s), azul largo = ver batería, verde (mantener) = acariciar (`evPet`). Quitados: piquete, jugar, amor-combo por botón.
- Serial: `?` imprime paleta/especie/pupila/nacimiento. Respaldo: firmware/old/v9_nacimiento/.

## V10 (2026-11-16): personalidad por color/forma, nacimiento estrella fugaz, sin superposición arco/ojo
- Colores: CURIOSO/ORGULLOSO/HYPE/MAREADO/TRISTE/FRÍO ya usan la paleta de la criatura (antes CURIOSO era blanco con pupila azul fija = "ojos azules con blanco de la nada"). Siguen con color propio solo las emocionales: enojado (rojo), amor (rosa), eléctrico/cargando (amarillo), medidor de batería. Pupila del estilo de la criatura en todas ellas.
- Personalidad: `genomeFromSeed` sesga los 6 rasgos según color (cálido=energético/cariñoso, frío=tímido/sonoliento, oscuro=gruñón/sueño, verde=curioso, pastel/rosa=cariño) y forma (tamaño, inclinación, asimetría). Las acciones espontáneas (curioso, feliz, orgulloso, sorpresa, bostezo, guiño, ansioso) usan esos rasgos como pesos, y hay ratos de tranquilidad.
- Arcos de ojo cerrado: el ojo se desvanece por completo (open<0.10) ANTES de que aparezca el arco; sin superposición. Arcos suprimidos durante el nacimiento.
- Nacimiento: estrella fugaz (estela 15 puntos, cabeza blanca) -> BOOM en 30 % (destello, 2 ondas, 18-28 partículas) -> los ojos nacen con 8 estilos. Código: evBirth/birthAdd/birthTgt/birthFx en EyesCore.h. Simulador: sim_birth, sim_birth_off. Respaldo: firmware/old/v10_estrella/ (y EyesCore_pre_v10.h).
- Placa: estrés 120 s, 50 fps prom, mín 31.

## V11 (2026-11-16): nacimiento = RULETA (reemplaza el de estrella fugaz, que no gustó)
- Al encender: 70 % del tiempo pasan combinaciones aleatorias de paleta x especie x pupila de izquierda a derecha con velocidad creciente (1.2 -> ~12 combos/s, estela de velocidad = estiramiento horizontal), luego cierra los ojos (línea), cambia por la criatura elegida (`finalG`), los abre con rebote y suelta 14 puntitos. Código: evBirth/birthAdd/birthTgt en EyesCore.h (`NBIRTH = 1`). Página de pruebas: grupo "Nacimiento". Respaldo: firmware/old/v11_ruleta/ (y EyesCore_pre_ruleta.h con la versión de estrella fugaz).
- Placa: estrés 120 s, 50 fps prom, mín 31.

## V12 (2026-11-16): ruleta rápida y fluida + parpadeo lento con miradas por personalidad
- Medición en placa (31.25 MHz): ruleta a 55-60 fps pero entrecortada: al final cada combinación cruzaba la pantalla en ~83 ms (>70 px/cuadro). Rediseño: movimiento continuo (500 -> 2800 px/s), combinación nueva cada 110 px (a veces con el ojo visible), ojo estirado según lo que avanza por cuadro (sin huecos), ojos más bajitos (menos filas = menos SPI), 2.5 s de ruleta. **SPI por defecto 41.7 MHz** (60 fps en la ruleta; estrés 120 s: 58 fps prom, mín 42). PENDIENTE de Alessandro: confirmar que no hay basura/rayas a 41.7 MHz (si hay: serial `f` o `spiHz = 31300000UL`).
- Tras la ruleta: cierra 0.55 s, cerrado 2 s (arcos), abre con rebote y `planLooks` (patrón de miradas según rasgos: curiosa/tímida/enérgica/dormilona/gruñona). Mismo mecanismo `evSlowBlink` como evento idle raro (24-50 s). Parpadeo normal 0.17 -> 0.23 s.
- Página de pruebas: la ruleta se graba a 50 fps reales (webp 20 ms). Respaldo: firmware/old/v12_ruleta_rapida/ (y EyesCore_pre_v12.h, _pre_rapida.h).

## V13 (2026-11-16): ruleta más larga y con drama
- Pedido: "dure más la animación de buscar y seleccionar, casi no tiene emoción". RU_T 2.5 -> 5.6 s; velocidad 320 + 500x + 2000x³ px/s; tensión (inclinación, vaivén y temblor crecientes), golpe seco (shakeAmp) al parar, 26 puntitos al abrir. Nacimiento total ~11 s. Placa: 60 fps durante la ruleta; estrés 120 s 58 fps prom, mín 42. Respaldo: firmware/old/v13_ruleta_drama/.

## V14 (2026-11-16): ruleta larga y suave (feedback con fotos: rasgado/tearing)
- Fotos de Alessandro muestran: (1) tearing: mitad superior del ojo desfasada de la inferior en movimiento lateral rápido (panel sin pin TE, barrido libre); (2) mi "estela" (ojo estirado x1.75) se veía como óvalos enormes mezclados.
- Cambios: sin estiramiento; velocidad lateral 220 -> 1150 px/s (máx ~19 px/cuadro), combinación nueva cada 150 -> 60 px (1.5 -> 19/s) durante 7.6 s; luego FRENA suave (ease-out cúbico hasta el centro) en vez de parar en seco; sin golpe seco; cierre lento ~2 s; final: 7 corazoncitos y 9 burbujas que suben (reemplaza la mini explosión). Nacimiento ~13 s. Placa: 60 fps en toda la ruleta, estrés 58 fps prom mín 42. Respaldo: firmware/old/v14_ruleta_suave/.
- Pendiente: Alessandro prueba serial `f` (62.5 MHz: escribir más rápido que el barrido reduce el tearing). Idea futura: otros movimientos laterales rápidos (sacudida) también limitar px/cuadro.

## V15 (2026-11-16): ruleta EN EL SITIO (contra el rasgado), sin corazones
- Feedback: seguía el desfase arriba/abajo del ojo, corazones feos, quería mucha más velocidad y más tiempo. Causa del desfase: cualquier desplazamiento lateral grande + panel sin TE. Solución de diseño: los ojos NO viajan (entran como mucho 25-55 px desde la izquierda y se asientan) y lo que corre es el cambio de combinación (intervalo geométrico 650 -> 33 ms, a tope 33 ms = 2 cuadros, frenada 33 -> 420 ms), con rayas finas (PT_LINE, 2000-4000 px/s, y 56-184 px) de izquierda a derecha para dar velocidad. ~120 combinaciones en 11 s; tiempos irregulares (jitter 0.8-1.2, dudas del 4 %) y pequeño estiron al entrar para que no sea mecánico. Final sobrio: rayitas radiales en 5 tandas desiguales (sin corazones ni burbujas).
- Placa: 60 fps en toda la ruleta (≈140 filas), estrés 58 fps prom, mín 42. Respaldo: firmware/old/v15_ruleta_en_sitio/.
- Pendiente: confirmar con Alessandro si desapareció el rasgado; si no, probar serial `f` (62.5 MHz).

## V16 (2026-11-16): el nacimiento está aislado
- Mientras `birthVar >= 0` (nacimiento, ~16 s) se ignoran: botones/eventos (`play` bloqueado salvo el propio nacimiento), `clipPlay`, `evShake` y todas las partículas ajenas (`spawn` solo deja pasar las del nacimiento vía `birthSpawn`). Al empezar se limpian partículas, clips, slosh y shake. Antes los botones/eventos podían disparar lágrimas, sudor o rayos de carga encima.
- Probado en simulador: 11 eventos disparados durante el nacimiento -> la expresión se queda en "despierto", 0 clips. Los estados reales (usb, batería baja, frío) se guardan y se aplican al terminar. Respaldo: firmware/old/v16_nacer_aislado/.

## V17 (2026-11-16): preludio de nacimiento (de un video de la revelación del Starboy de Lil Guy)
- Alessandro pasó un video (WhatsApp, 52 s, 384x832, creadora ellie.voxel) y pidió: lo que va ANTES de que pasen varios diseños de ojos como el nacimiento, y luego nuestra ruleta. Análisis por fotogramas (OpenCV): pantalla blanca-rosada con rayitas oscuras elongadas que corretean (9-13 s) -> un solo punto que crece (13-14 s) -> onda que oscurece la pantalla (14 s) -> mancha blanca/crema con núcleo blanco y contorno orgánico que crece durante ~8 s -> se pinza arriba y abajo y se parte en dos lóbulos (22.6-26 s) -> desaparecen y quedan dos arcos de ojos cerrados (26.4-27.3 s) -> ahí empieza la ronda de diseños (que ya teníamos). El video usa estrellitas blancas; Alessandro no las quiere, no se copiaron.
- Implementado (comprimido a 9.1 s): `birthFx` dibuja el preludio (PR_W 2.2 s blanco+rayitas, PR_D 0.7 punto, PR_R 0.4 onda, PR_B 3.2 mancha crece, PR_S 1.3 se parte, PR_H 0.5 respira, PR_A 0.8 lóbulos->arcos) con `blobAA` (disco central + 9 discos perimetrales con ondulación). Ojos normales ocultos en el preludio; la ruleta arranca en `PRE`. Placa: 42 fps con pantalla blanca (240 filas), 53-60 el resto; estrés 120 s ok. Respaldo: firmware/old/v17_preludio/.
- Si el arranque (~26 s) resulta largo: se puede acortar PR_* o RU_T1.

## V18 (2026-11-16): preludio más vivo + envío por diferencias de filas
- Feedback: el blanco iba muy lento, quiere más animación ahí, foco en el puntito donde nace, la mancha más fluida y rápida, y el corte del blanco a la ruleta era brusco.
- Preludio: PR_W 1.8 s (22 rayitas/s, 60-210 px/s, al final convergen al centro), el punto nace 450 ms antes con rebote (`grow`), late (squash/stretch a 4.5 Hz, tembleque) y suelta ondas suaves; onda oscura 0.45 s; mancha crece con rebote en 2.3 s, contorno ondula x1.8, late; se parte 0.9 s; arcos 0.6 s. Total 7.15 s (antes 9.1). Transición: los arcos se desvanecen 160 ms mientras los ojos se abren desde una línea en 420 ms (antes corte).
- Cambio de fondo en el .ino: se envían SOLO las filas que cambiaron entre `fb` y `shown` (comparación 32 bits), `forceFull` tras un fallo de DMA. Estrés 120 s: 59 fps prom, mín 50 (antes 56/41); el blanco ya no fuerza 240 filas. Menos filas por SPI también reduce el rasgado. Respaldo: firmware/old/v18_preludio_vivo/ (y *_pre_v18.*).

## V19-V23 (2026-11-16): iteración del nacimiento con feedback de Alessandro
- V19-V20: guion nuevo (sin fondo blanco entonces): rayas orbitando y acelerando, punto negro, nube blanca, ojos de nube blancos que aceleran tomando forma/color; crossfade nube->ojos reales.
- V21: punto negro pequeño (dotR 2.2-9.7), nace de él la nube (CY 118->122, latido lub-dub); SIN desvanecido: cada cambio brusco hace un pasito escalonado (forma, bump 1->0, tinte, `cloudSat`) y en un cambio (x>=0.30) corte a los ojos reales.
- V22: cloudPup=0 (el blanco hace de pupila), pupila de ojos reales claras al inicio (WP 228,228,238), sin borde gris del punto, rayas más rápidas (wmax 4300/r) y más tiempo (PR_W 7 s).
- V23 (actual): vuelve el FONDO BLANCO con RAYAS NEGRAS pero curvas: arcos de 4 segmentos que siguen anillos concéntricos y se cierran en espiral al punto negro; la oscuridad nace del punto sin borde gris (discAA negro creciente); luego fondo negro. Rendimiento: `fillAll` rápido; 60 fps casi todo el nacimiento (45-58 en el blanco); estrés 120 s 58 fps prom, mín 46. Respaldo: firmware/old/v23_nacimiento_blanco/ (también v22_nacimiento, *_pre_v2x.h).

## V24 (2026-11-16): hilos tipo río que convergen y explotan
- Pedido: en vez de rayitas, hilos largos como ríos que fluyen y convergen al centro; aceleran, nunca bajan la velocidad, y de golpe explotan en el puntito (el "feto").
- Implementado: 10 brazos espirales (24 segmentos, curvatura `ph = 3.2·u^0.8`, radio `dotR + (112-dotR)(1-u)^1.3`) con trazos que fluyen hacia el centro (patrón de guiones desplazado por `armFl`); giro w = 1+33x² rad/s y flujo 0.5+13x² sin frenar; PR_D=0 (no hay pausa), PR_W=7 s; explosión PR_R=380 ms: disco negro `dotR + 175·u^0.55` + 18 rayos blancos que se desvanecen; sigue la nube/feto. Placa: fase de hilos 45 fps (cambian las 240 filas; limitado por SPI a 41.7 MHz; `f` -> 62.5 MHz lo subiría), resto 60; estrés 120 s 56 fps prom, mín 45. Respaldo: firmware/old/v24_rios/.

## V25 (2026-11-16): hilos orgánicos que se van metiendo
- Pedido: menos mecánico/simétrico y que vaya fluyendo hacia dentro, sin dejar nada en los bordes al acelerar.
- Implementado: 14 `Thread` con parámetros aleatorios por nacimiento (ángulo, curvatura k 2.0-4.4, largo 0.55-1, ritmo de contracción 0.92-1.14, velocidad 0.7-1.3, frecuencia de trazos, ondulación angular y radial, grosor, opacidad); el radio exterior de cada hilo se contrae `ro = dotR + (108-dotR)(1-x·rate)^0.9·len` => los bordes se vacían y al final todo cae al punto; luego explota. Estrés 120 s ok. Respaldo: firmware/old/v25_hilos_organicos/.

## V26 (2026-11-16): preludio invertido (fondo negro, bolita blanca)
- Alessandro: le gustan ~2/3 pero "da miedo"; pidió invertir colores: fondo negro, bolita blanca. Hecho: bg=0, hilos `lite` (238,240,250), bolita blanca; explosión = anillo blanco que se expande (`discAA` blanco + hueco negro) + 18 rayos blancos + destello; el blob (feto) nace de la bolita blanca. Respaldo: firmware/old/v26_invertido/ (la versión blanca/negra: v25_hilos_organicos).

## V27 (2026-11-16): líneas más limpias (punto medio entre mecánico y orgánico)
- Alessandro: "demasiado orgánico, quizá mejor líneas". Ajuste: NTH 12, ángulos casi equiespaciados (+-0.25 rad), curvatura k 2.7-3.6, largo 0.72-1, grosor parejo (2.2-2.8), sin ondulación radial y casi sin angular (wa 0.015-0.035), guiones regulares (duty 0.52). Siguen drenando hacia el centro. Fondo negro / bolita blanca. Respaldo: firmware/old/v27_lineas/ (v25 = orgánico, v26 = invertido orgánico).

## V28 (2026-11-16): el centro es una partícula-nube que se alimenta
- Alessandro: le gustan mucho las líneas actuales (v27); el punto central no debe ser estático: una partícula como las nubes del "feto" pero de un solo color blanco, que crece al alimentarse y tiene espasmos.
- Implementado: `blobAA` blanco (bump 1) en el centro, `dotR = 3 + 15·g^1.5` (crece), `spasm` (impulsos aleatorios cada vez más frecuentes, probabilidad 0.8+10x² por segundo, decae e^-8t) que deforma ancho/alto y sacude; velocidad de ondulación sube con x. Explosión: aro blanco delgado `(1-u)²` + 18 rayos y la nube se encoge en 0.6 u. FIX: había borrado sin querer (v27) el reinicio de las variables de la nube en `evBirth` => un segundo nacimiento en la misma sesión se saltaba la fase de nube; restaurado. Respaldo: firmware/old/v28_particula/.

## V29 (2026-11-16): la partícula reacciona a lo que le entra
- Pedido: que crezca un poquito más; explosión un poco más orgánica (no tanto); que la partícula reaccione a la interacción de todo lo que le entra, que se vea que de verdad la alimentan.
- Implementado: `dotR = 3 + 23·g^1.5` (máx ~26 px); detección de llegada por hilo (`Thread.pin`: cuando un guion nuevo entra por el extremo interior) -> empuje `bulX/bulY` hacia el ángulo de entrada (decae e^-7t, ±7 px) y `feed` (+0.055, decae e^-6t, tope 0.45) que agranda el radio; el blob se estira según el empuje. La explosión: aro irregular (40 segmentos con perturbación sinusoidal 3 y 5 lóbulos) y 20 rayos con ángulo/largo/retardo pseudoaleatorios por índice. Respaldo: firmware/old/v29_alimenta/.

## NACIMIENTO APROBADO + REGLA DE COLOR (2026-11-16)
- Alessandro aprobó el nacimiento V29 ("me encantó"). Guardado como **nacimiento**: `firmware/old/nacimiento_aprobado_2026-11-16/` (código + datos + LEEME.md con la secuencia y parámetros) y `animaciones/nacimiento/*.webp`.
- PENDIENTE (para después): resolver los entrecortes/rasgado (tearing): "es cosa de la pantalla" (GC9A01 sin TE). Ya mitigado: SPI 41.7 MHz, solo filas cambiadas, velocidades limitadas. Ideas: probar serial `f` (62.5 MHz), pantalla con pin TE, sincronía por temporización.
- Observación de color (foto: ojos rosa-blanco con pupila azul "como un filtro"): quitar de las combinaciones los blancos/colores lavados con un toque de tono; que sean claramente blancos o claramente de un color. `tools/aplicar_votos.py` tiene ahora la regla `0.06 <= sat < 0.215 y valor >= 0.89` sobre el color de ojo: cortó 14 paletas (Crema aguamarina, Marfil frambuesa, Hortensia rosa, Algodon jade, Hueso azul, Crema orquidea, Petalo pistacho, Hielo y zafiro, Pino y niebla, Hielo flash, Opalo lechoso, Eclipse solar, Cometa nevado, Marfil cian). Quedan 148 paletas, 37 especies, 14 pupilas, 141 clips (~76 mil combinaciones base). Se conserva el blanco casi puro (Perla oliva, sat 0.05). Por ampliar si hace falta: más lavadas hasta sat 0.28 (13 más, p. ej. Malva, Rosa bebe uva, Lila bruma, Lila atardecer, Rosa chicle...). Placa recargada; estrés 120 s: 59 fps prom, mín 51.

## REGLA DE COLOR 2 (2026-11-16): lo pálido tira a blanco
- Fotos de Alessandro (ojos lila-rosado lavado, y blancos con pupila morada): todavía le parecían raros esos colores; pidió que tiren un poco más a blanco.
- `tools/aplicar_votos.py` regla 2: pastel pálidas (0.215 <= sat < 0.30, valor >= 0.85) -> color de ojo mezclado 80 % con blanco (queda blanco limpio, sat <= 0.06), se conserva el color de pupila. 14 convertidas (Vainilla ciruela, Rosa bebe uva, Lila atardecer, Cielo coral, Rosa te oliva, Nube celeste, Avena canela, Rosa pizarra, Rubor zafiro, Luna de miel, Lila bruma, Durazno verde, Rosa chicle, Crema salmon). Con la regla 1 (cortar 0.06-0.215 muy claros) quedan 148 paletas; Malva (sat 0.21, valor 0.88) se queda como lila definido. Idempotente. Placa recargada; estrés 59 fps prom.

## V30 (2026-11-17): carga exagerada (3 tipos), recipientes de batería, cara llena x6, enojado variable
- Pedido (y revisión contra notas de ayer, sin contradicciones): CARGANDO no gustaba, en especial las líneas verdes del centro (rayos cian: `boltAA` + partículas PT_BOLT, eliminados); que se note exagerado, "como si le metieron el cargador", varios tipos al azar según personalidad; CARA LLENA con varias paletas; VER BATERÍA con ojos-recipiente (se note cuánto falta) y batería simulada mientras no haya sensor (3 min bajando, 1 min vacía, luego llena); ENOJADO con variantes y colores rojos/naranjas.
- Implementado en EyesCore.h: `onEnter` elige `chargeType` (pesos: llenado 1, ondas 0.5+energía, ronroneo 0.4+cariño+0.6·sueño), `fullPal` (FULLP[6]), `angryVar` (4) y `angryCol` (ANGC[6]); ELECTRIC = jolt (ojos x1.22/1.28, sacudida 8, destello blanco doble, 14 rayitas blancas); CHARGING tipo 0 = recipientes (ghost EyeDraw con lids 0 + relleno por lids, burbujas PT_DOT, energía real), tipo 1 = ondas (2 anillos elípticos de 28 segmentos por ojo), tipo 2 = arcos felices con glow; BATCHECK = recipiente con marcas 25/50/75 % y superficie ondulante; FULL usa FULLP para cara+ojo+pupila; ANGRY colores + 4 formas (jitter, pendiente, pupila, salto) y vena en lado variable. Batería simulada `simBatt` (SIM_DRAIN 180 s, SIM_HOLD 60 s, 100 -> 3 %), la placa la activa en setup, serial `b`; no duerme por energía mientras simula. Respaldo: firmware/old/v30_carga_bateria/. Página: grupos Carga (3 tipos), Cara llena (6), Enojado (6).

## V31 (2026-11-17): amor variado, botón azul en orden, recipiente sin rayas
- Quitadas las marcas 25/50/75 % del recipiente (no le gustaron). El ghost vacío oscuro se queda.
- Botón azul = emoción SIGUIENTE EN ORDEN (`emoIdx`, `EMO[14]`: curioso, feliz, amor, sorprendido, orgulloso, hype, cara llena, enojado, triste, ansioso, frío, mareado, bostezo, cansado), 3.4 s cada una.
- AMOR/ACARICIAR: misma criatura (paleta propia) feliz tirando corazones; 4 variantes (`loveVar`): 0 feliz con sus colores y rebote, 1 ojos cerrados felices con glow, 2 clásico rosa-rojo (el que le había encantado), 3 corazones en los ojos. Corazones: `heartCol()` mezcla rojo-rosa, rosa suave, color del ojo y acento; tamaños 4-20; cadencias distintas. pcols ampliado a 10. Respaldo: firmware/old/v31_amor_orden/.

## V32 (2026-11-17): cargando limpio, corazones rojos con presencia, enojado x9
- Feedback: "el cargando no me gustó nada ¿qué pasó con los rayos encima de los ojos? se siente súper recargado" (recargado = sobrecargado visualmente) -> eliminados: rayitas radiales del jolt, fogonazo blanco de pantalla, anillos de energía, burbujas y las rayitas de la SORPRESA previa (ahora al conectar: ELECTRIC directo, ojos x1.22/1.28 + shake 8 + destello blanco de los propios ojos 0.45 s y 0.55-0.85 s). Tipo 1 = respira y brilla; tipo 0 = recipiente sin extras; tipo 2 = ronroneo.
- Corazones solo rojos (heartCol()=2), tamaño 9-25, aparecen con rebote (0.3->1.15->1), laten, alpha sólido (fade-in 5 %, fade-out 12 %), más velocidad y cadencia (RT 3.4-6/s).
- Enojado: 9 estilos (`angryVar`): 0 clásico, 1 furia, 2 ceño frío, 3 berrinche, 4 cejas (barras angulares), 5 vapor (PT_DOT blancos), 6 fuego (c4 -0.24, wob 0.14, parpadeo de color, sin pupila), 7 chevrones > <, 8 hervir (rampa 2.6 s de color de la criatura a rojo + shake creciente + 2 venas). Colores ANGC (rojos/naranjas). Respaldo: firmware/old/v32_limpio_enojo/.

## V33 (2026-11-17): criatura guardada + verde 5 s = renacer (PROVISIONAL) y cierre de sesión
- Pedido: guardar el último nacimiento hasta que Alessandro pique el botón verde 5 s; al reconectar NO vuelve a nacer. Hecho con EEPROM emulada (magic 0xA110C0DF). Verificado: reinicios seguidos = misma criatura, `guardada=1 naciendo=0`; serial `R` guardó la nueva y tras reiniciar se mantuvo. Prueba de estrés ok (59 fps prom, mín 55). Respaldo: firmware/old/v33_guardado/.
- Contexto consolidado: `docs/CONTEXTO_COMPLETO.md` (nuevo, leer primero), memoria `project_alloy.md` y `feedback_alloy_diseno.md`.

## V34 (2026-11-17): primer port del estilo nuevo del estudio web a la placa
- Origen: `studio/estilo_tinta/alloy_v4.html` (Estudio v4). Restore previo: `firmware/old/v34_pre_estilo_nuevo/`; esta versión: `firmware/old/v34_estilo_nuevo/`.
- Portado a EyesCore.h: (1) PICOS de boceto en la silueta (F_CR/F_CX/F_CY + `D.cr/cdx/cdy` en renderEye; modo 7 de `chooseForm`, 34 % de las deformaciones, se acomodan solos); (2) AMOR: siempre corazones en los ojos (`loveVar = 3`) y corazones flotantes por los lados; (3) BOSTEZO cierra en curva caída (ARCDIR +1) y se estira, no en ^ (parecía sonrisa); (4) rampa de color de VER BATERÍA rojo>naranja>amarillo>lima>esmeralda (sin verde pantano).
- NO portado todavía (solo en el estudio web): ojo cerrado pincel con inclinación/curvatura por expresión, bostezo con arrugas, feliz con mejillas, enojado con V y picos hacia afuera, orgulloso, ansioso con rebote, modo pantalla de color, pupilas con resorte propio, nacimiento con formas de boceto.
- Placa: compila (7 % flash, 48 % RAM), `check.py` OK, estrés ~59 fps prom (mín 53), criatura guardada intacta (`guardada=1 naciendo=0`).

## V34b (2026-11-17): render nuevo COMPLETO en la placa (siluetas de boceto) + renacer
- `renderEye` ahora dibuja un POLÍGONO de boceto (huevo de base plana + cresta/punta inclinada, borde que respira) en vez de la elipse con armónicos; cobertura por 2 sub-renglones; pupila y párpados como antes. Poses (POSEV: huevo, bandera, pico, torre, agachado, cola, gancho, cabecita, furiaL/R) se mezclan con resortes (F_FL..F_NT); `chooseForm` = 30 % reposo, 50 % pose de boceto, resto movimientos; `styleMir` espeja el ojo derecho (por criatura).
- Enojado/triste/curioso usan poses fijas (enojado: picos hacia afuera). Ojo cerrado = `brushArc` (pincel, extremos redondos, inclinación por expresión: dormido .35, tenue .5, bostezo .6, nacimiento .25). Ruleta del nacimiento: cada paso = otra silueta.
- Sigue sin portar: feliz con mejillas, orgulloso/ansioso nuevos, pantalla de color, bostezo con arrugas, V del enojado (usa la pendiente propia), pupilas con resorte propio extra.
- Restore: `firmware/old/v34b_render_nuevo/`. Placa: nacimiento 54-60 fps. Se envió `R` (criatura nueva guardada).

## V35 (2026-11-17): MOTOR NUEVO = port 1:1 del Estudio v4 (reemplaza todo lo anterior)
- Pedido de Alessandro: en la placa quiere EXACTAMENTE las animaciones del navegador (`studio/estilo_tinta/alloy_v4.html`), no una adaptación de las viejas; solo conservar los controles de botones.
- `firmware/Alloy_Eyes/EyesCore.h` reescrito: primitivas AA + renderizador de polígono (siluetas de boceto, 2 colores planos, pupilas con resorte propio, párpado/mejillas/corazón/medidor por píxel) + motor del estudio (21 expresiones + pantalla de color + nacimiento ruleta con siluetas + ojo cerrado de pincel con curvatura/inclinación + bostezo con arrugas, feliz con mejillas, enojado en V con picos hacia afuera, orgulloso, ansioso con rebote, ver batería con rampa nueva). Se acabaron genoma/paletas/clips viejos (siguen en `firmware/old/`).
- Botones/serial → mismas funciones ev*: azul corto = siguiente emoción (curioso, feliz, amor, sorprendido, orgulloso, hype, cara llena, pantalla de color, enojado, triste, ansioso, frío, mareado, bostezo, batería baja, guiño); azul largo = ver batería; verde = amor mientras se mantiene; verde 5 s = renacer y guardar. Espontáneo: cada ~25-45 s una emoción; 35 s sin tocar = bostezo; 100 s = bostezo y se duerme (arcos).
- Verificación: arnés PC `sim/new/harness.cpp` (compila el mismo EyesCore.h; `harness.exe seed e:ID:seg ... | b`), hojas de todas las expresiones y del nacimiento revisadas; placa 57-60 fps, 5.9 ms de render. El simulador viejo (`sim/viewer.py`) YA NO funciona con este core.
- Restore: `firmware/old/v35_estudio_v4/` (antes de esto: `v34c_pre_port_total/`). Se envió `R` (criatura nueva guardada).

- V35b: nacimiento, cierre de ojos suave antes de revelar el color: parpados bajan 1.7 s (sin saltos) -> trazo -> cambio de color con el ojo cerrado -> abre pesado a los 3 s. Restore: firmware/old/v35b_cierre_suave/.

## V36 (2026-11-17): estudio v4 final cargado en la placa
- Port de: hilos definidos (16, blancos, afinados), partícula 9.5 s sin aro, manchas viscosas tras la explosión, ruleta con manchas giratorias, cierre por deformación (EDraw.morph), base suave + pupila alta, florituras exóticas momentáneas (genExoPair). Placa: 60 fps, render 8 ms (nacimiento). Restore `firmware/old/v36_estudio_final/`. Se envió `R` (criatura nueva).

## V36b (2026-11-17): BUG corregido — la criatura que nace NO era la que se guardaba
- Síntoma (reportado por Alessandro): no se conservaba cuál fue el último nacimiento. Causa: al renacer (`R`/verde 5 s) la criatura se generaba con `evNewCreature` (generador A) y al reiniciar con `coreInit` (generador B), así que la misma semilla guardada daba OTRA criatura al encender; además los genes de los ojos (resortes, tamaño, fase) se sorteaban al azar en cada arranque.
- Arreglo: `creatureFromSeed()` en EyesCore.h = única fuente de identidad (colores, forma, pupila, espejo, escala, separación, personalidad y genes de los ojos salen SOLO de la semilla); `coreInit` y `evNewCreature` la usan igual. Verificado en PC (`sim/new/ident.cpp`: arranque = renacer) y en la placa: tras nacer `criatura=549` y tras reiniciar `criatura=549`.
- La semilla se guarda en EEPROM (`0xA110C0DF` + seed; `R`/verde 5 s guardan; `n` = prueba sin guardar; `t`/ENTER = otro color sin guardar). Criatura guardada ahora: id 549 (serial `?` imprime `[ID] criatura=..`). Restore: `firmware/old/v36b_identidad_guardada/`.

## V36c (2026-11-17): ojos medio cerrados / aplastados "de a ratos"
- Causas halladas con una prueba de 30 min simulados (`sim/new/soak.cpp`): (1) las formas exóticas momentáneas podían salir muy anchas y bajas (ry 32) y se quedaban 1-2 s aplastadas -> ahora ry >= 42 y rx <= 46 (máx. aplastado seguido 2.1 s -> 0.4 s); (2) la batería es SIMULADA (3 min baja, 1 min vacía, en ciclo) y a veces disparaba sola "batería baja" (párpados caídos + ojos agachados) -> quitado, ahora solo sale si Alessandro la elige con el botón azul; (3) por si acaso: los eventos que reabren los ojos (parpadeo lento, guiño) ya no se cancelan al cambiar de expresión y hay una guardia que reabre cualquier ojo que se quede cerrado >1.4 s.
- Lo que SÍ es por diseño: tras 100 s sin tocar bosteza y se duerme (arcos cerrados) hasta que se pulsa un botón; el parpadeo lento dura ~2.5 s; amor y batería baja agachan los ojos. Restore: `firmware/old/v36c_sin_aplastados/`.

- Nota 2026-11-17: tras V36c la placa reportó criatura=302 (antes 549). El generador es idéntico entre versiones (verificado con sim/new/ident.cpp), así que la semilla guardada cambió por un renacer manual (verde 5 s) o similar. El id actual siempre se lee con serial `?`.

## Decisión 2026-11-17: ELÉCTRICO queda como expresión candidata (sin cambios de código). Solo sale al conectar USB / serial `u`; no entra en la rotación azul. Contexto actualizado en CONTEXTO_COMPLETO.md.

## V36d (2026-11-17): ELÉCTRICO agregado a la rotación del botón azul (17 emociones; última). Sigue sin salir sola.
