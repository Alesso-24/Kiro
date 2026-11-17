# ALLOY — CONTEXTO COMPLETO (leer esto primero al retomar)

> ## ⭐ ACTUALIZACIÓN 2026-11-17 (V35): EL DISEÑO DEFINITIVO ES EL ESTUDIO v4 — manda sobre todo lo que sigue
> Alessandro eligió quedarse con el diseño de `studio/estilo_tinta/alloy_v4.html` (índice: `studio/estilo_tinta/index.html`). La placa corre un port 1:1 en `firmware/Alloy_Eyes/EyesCore.h` (motor nuevo; el genoma/paletas/clips/EEPROM-de-identidad viejos ya NO se usan; restore viejo en `firmware/old/v34c_pre_port_total/`). Lo viejo de abajo (secciones 3-6) describe el sistema anterior: úsalo solo como historia.
> - **Estilo**: siluetas de boceto (huevo de base plana + picos/cresta inclinada, borde que respira), 2 colores planos (ojo + pupila oscura del mismo tono), SIN brillos/estrellas/degradados, pupilas con resorte propio, ojos asimétricos, ojo cerrado = trazo de PINCEL con extremos redondos (curvatura `cd` e inclinación `ci` distintas por animación). SIN desvanecidos entre estados (todo cambia de golpe o con deformación de forma, nunca con mezcla de transparencias).
> - **Expresiones (23)**: despierto, curioso, feliz (mejillas+euforia), triste, enojado (recorte en V hacia el centro, picos hacia afuera), amor (siempre ojos de corazón, corazones por los lados), sorprendido, orgulloso (mirada superior + inflado), ansioso (rebote, casi sin temblor), hype, mareado, frío, bostezo (curva caída ‿ con arrugas, estirada, suspiro; NO ^ porque parece sonrisa), guiño, nacimiento, pantalla de color (fondo de un color + ojos de otro, 11 combos), cara llena, eléctrico, cargando, ver batería (rampa rojo>naranja>amarillo>lima>esmeralda, NUNCA verde pantano), batería baja, dormido, dormido tenue.
> - **Nacimiento** = secuencia aprobada (hilos→partícula→explosión→nube→2 ojos de nube→ruleta) PERO con siluetas de boceto y color que empieza antes; cierre SUAVE (párpados bajan 1.7 s, trazo, cambio de color con el ojo cerrado, abre a los 3 s).
> - **Combinaciones**: 38,400 base discretas (100 colores × 64 reposos × 2 espejo × 3 pupilas) + valores continuos (decenas de millones).
> - **Botones** (iguales): azul corto = siguiente emoción, azul largo = ver batería, verde = amor mientras se mantiene, verde 5 s = renacer y guardar semilla (EEPROM `0xA110C0DF`). Serial de pruebas igual.
> - **Herramientas nuevas**: estudio web (abrir con Chrome `--new-window file:///...alloy_v4.html#expr=ID&auto=0`; Chrome sin ventana corre la animación MUY lento, las capturas muestran transiciones a medias); arnés PC `sim/new/harness.cpp` (g++ -O2; `harness.exe seed e:ID:seg … | b`, vuelca .raw RGB565). El simulador viejo `sim/viewer.py` ya no sirve.
> - **Reglas de trabajo de Alessandro (esta sesión)**: quiere VER todo en SU navegador (Chrome predeterminado; abrir con chrome.exe directo); no borrar propuestas viejas (index.html las lista todas); pide ajustes muy concretos por imagen; no quiere "adaptación de lo viejo" sino exactamente lo del navegador; le molesta lo abrupto/desvanecido; sigue valorando que se diga qué no se pudo ver (pantalla real).
> - **V36 (2026-11-17, cargada en la placa)**: base de la criatura = óvalo suave (forma de reposo 'huevo') + pupila alta; las formas MUY irregulares/exóticas (silueta inventada al azar, pico, muesca, inclinación) son SOLO momentáneas (50 % de las florituras, se acomodan solas) y expresiones; en el estudio quedan como opciones 'Exótica única' (forma de reposo) y pupilas 'Irregular suave'/'Exótica como el ojo'. Nacimiento FINAL: 16 hilos blancos sólidos y definidos + partícula alimentada (sin aro ni destellos), partícula 9.5 s, tras la explosión DOS MANCHAS VISCOSAS que nacen de un punto y crecen en su lugar (variantes A/B/C de tiro parabólico solo en el estudio), ruleta con manchas viscosas giratorias ~6 s antes de pasar a siluetas, CIERRE POR DEFORMACIÓN (el contorno del ojo se vuelve el trazo de pincel; párpado ligado; cambio de color con el ojo cerrado; abre a los 4 s). Calificador: studio/estilo_tinta/calificar_50.html (ojos suaves/exóticos × pupilas × parejos; guarda JSON en Descargas). Restore: firmware/old/v36_estudio_final/.
> - **Identidad guardada (V36b)**: la criatura sale SOLO de la semilla guardada en EEPROM (`creatureFromSeed`); al reiniciar es la MISMA que nació (verificado, id 549 el 2026-11-17). Antes había un bug (renacer y reiniciar usaban generadores distintos). `R`/verde 5 s = nueva y guardada; `n` = prueba sin guardar.
> - **Eléctrico (2026-11-17)**: Alessandro quiere poder PONERLA en la XIAO: ya está en la rotación del botón azul (última de 17: ... guiño, eléctrico) además de al conectar USB (`evUsb`) y serial `u`; NO sale sola entre las espontáneas. Aún decide si la deja fija; no quitarla del código. Restore: firmware/old/v36d_electrico_en_azul/.
> - **Pendiente**: Alessandro debe calificar las 50 criaturas base (calificar_50.html) para afinar el generador; confirmar en la pantalla real el nacimiento V36; seguir pulsando animaciones en el navegador y portar a la placa cada vez.
Escrito el 2026-11-17 al cerrar la sesión larga (2026-11-14 a 2026-11-17). Si algo contradice a `CLAUDE.md` o `docs/STATUS.md`, **manda este archivo** (es el más reciente). `docs/STATUS.md` = bitácora por versión (V1..V33); `docs/COMANDOS.md` = manual de uso; `CLAUDE.md` = contexto histórico (hardware, cableado, lecciones de pantalla).

## 0. Quién y cómo trabajar con Alessandro
- Alessandro (usuario del sistema "jordi"), habla español, pide cosas con mucho detalle visual/emocional. El asistente **no puede ver la pantalla real**: la confirmación visual la da él (fotos/videos). Yo reviso con el simulador (hojas de fotogramas con `Read`) y con medidas en la placa.
- Estilo de trabajo que funciona: proponer → implementar → **mirar hojas de fotogramas del simulador** → cargar a la placa → medir fps + prueba de estrés → regenerar `catalogo/anims.html` → documentar en STATUS/COMANDOS → guardar punto de restauración en `firmware/old/vNN_*`. Responder en español, claro, sin tecnicismos innecesarios, diciendo qué NO pude comprobar (la pantalla real).
- Siempre "guardar contexto" cuando lo pida: STATUS.md + CLAUDE.md + memoria (`project_alloy.md`, `feedback_alloy_diseno.md`).
- **Trampas de herramienta**: (1) comandos Bash muy largos / heredocs grandes fallan ("unexpected EOF") → escribir el parche como script con la herramienta Write en la carpeta scratchpad y ejecutarlo con `python`; (2) Python en Windows no entiende `/tmp` → usar rutas `C:/Users/...`; (3) al recortar bloques de código con `index()`, revisar que no se borre el resto de la línea (bug de v27: se borró el reinicio de la nube); (4) `ffmpeg` del PC es de 2013: para video usar OpenCV (`cv2`) y leer los fotogramas; (5) no dejar banderas de prueba en la placa: serial `u` (USB) e `i` (frío) dejan estados activos hasta reiniciar; (6) cerrar monitores serie antes de subir; el puerto es COM6 (puede ser COM5).

## 1. Hardware y cableado
Seeed XIAO RP2350 + pantalla redonda GC9A01 240x240 (7 pines, sin backlight pin) + 2 botones (azul, verde) a GND con INPUT_PULLUP. Sin MPU/micrófono/sensor de batería todavía.
| Pantalla | XIAO | GPIO | | Botón | XIAO | GPIO |
|---|---|---|---|---|---|---|
| SCL | D8 | 2 | | azul | D6 | 0 |
| SDA | D10 | 3 | | verde | D7 | 1 |
| DC | D0 | 26 | | | | |
| CS | D1 | 27 | | | | |
| RST | D2 | 28 | | | | |
`setRotation(2)` (pantalla girada 180°). VCC=3V3.

## 2. Cómo compilar, subir y probar
```
cd firmware
arduino-cli compile --fqbn rp2040:rp2040:seeed_xiao_rp2350:freq=250,opt=Optimize2 Alloy_Eyes
arduino-cli upload  -p COM6 --fqbn rp2040:rp2040:seeed_xiao_rp2350:freq=250,opt=Optimize2 Alloy_Eyes
```
- Validación en PC (simulador, sin placa): `python tools/check.py . --creatures 6 --seeds 40` (debe terminar con "OK: sin errores graves").
- Prueba de estrés en placa 120 s: `python tools/stress_serial.py` (reporta fps prom/mín; usa la tecla `n`, que NO guarda). Resultado actual: ~59 fps prom, mín ~53, sin cuelgues.
- Identidad por serial: `python tools/id_serial.py` (imprime `[ID] paleta=… especie=… pupila=… guardada=… naciendo=…`). `python tools/birth_perf_serial.py` mide fps durante el nacimiento.
- Página de animaciones: `cd catalogo && python -m http.server 8765 --bind 127.0.0.1` y abrir `http://127.0.0.1:8765/anims.html` (regenerar con `python tools/encuesta_anim.py`, ~8 min, 71 elementos, 69 MB). Encuesta de combinaciones: `python tools/encuesta.py <ronda> 60`.
- Simulador PC: `sim/Abrir simulador.bat` (mismo `EyesCore.h` compilado a DLL por g++ de Strawberry).

## 3. Arquitectura
- `firmware/Alloy_Eyes/EyesCore.h` (~1700 líneas, SIN hardware, compartido por placa y simulador): render por píxel con antialias (frontera con armónicos), poses con resortes, formas, pupilas, partículas, genoma, motor de clips, tamagochi, nacimiento, variantes de carga/enojo/amor/cara llena, batería simulada.
- `Alloy_Eyes.ino`: pantalla (doble buffer RGB565 + DMA SPI0 16 bits; **tras el DMA hay que restaurar `spi_set_format(8 bits)`** o la pantalla se congela), SPI 41.7 MHz, `clk_peri` reconfigurado a `clk_sys`, **envío solo de las filas que cambiaron** (comparación fb vs shown), botones, serial de pruebas, EEPROM (criatura guardada).
- `Data.h` + `src/datos/*.h`: paletas, especies, pupilas, clips (datos de texto compilados por `tools/studio_compile.py`; Arduino solo copia `.h/.cpp/.ino` y `src/`).
- `Motion.h`: acelerómetro (listo, sin sensor): `motionFeed`, `motionReact`; usa `evShake` (chapoteo contra bordes).
- `sim/` (`eyes_sim.cpp`, `viewer.py`): exports usados por las pruebas: `sim_birth`, `sim_birth_off`, `sim_force`, `sim_force_love`, `sim_simbatt`, `sim_set_energy`, `sim_reset_state`, `sim_emotion`, `sim_slowblink`, etc.
- Pipeline de datos: `integrate.py` → `curate.py` → **`aplicar_votos.py`** (recortes por nombre + 2 reglas de color; idempotente; correrlo siempre al final). Votos en `studio/votos/ronda1..4.json` (lo no marcado = me encanta).
- Restore points: `firmware/old/` (v3 aprobado, v7_final, v8_beta, v9..v33, `nacimiento_aprobado_2026-11-16/` con LEEME.md, y `EyesCore_pre_vNN.h`). **El más reciente = `v33_guardado/`**.
- Animaciones exportadas: `animaciones/` (GIF antiguos de la v3) y `animaciones/nacimiento/*.webp`.

## 4. Estado actual del comportamiento (qué hace la placa)
**Encendido**: si no hay criatura guardada → NACE (y se guarda). Si ya hay guardada → NO vuelve a nacer: abre los ojos contenta (HAPPY 1.4 s) y sigue. **Botón verde 5 s (o serial `R`) = renacer: criatura nueva (forma/colores/personalidad), animación de nacimiento y se guarda** (reemplaza a la anterior; sobrevive a desconectar). Es **PROVISIONAL** (decisión de Alessandro "solo para ahorita"). EEPROM: magic `0xA110C0DF` + seed (offset 0/4). Serial `n` = criatura nueva SIN guardar (pruebas). OJO: si se cambian paletas/especies/pupilas (o reglas de color), la misma semilla puede dar una criatura de otro aspecto.
**Botones**: azul corto = SIGUIENTE emoción en orden (3.4 s): curioso, feliz, amor, sorprendido, orgulloso, hype, cara llena, enojado, triste, ansioso, frío, mareado, bostezo, cansado; azul largo (0.8 s) = ver batería (ojos-recipiente); verde mantener = acariciar (amor); verde 5 s = renacer. Durante el nacimiento se ignoran botones/eventos (play/clipPlay/spawn bloqueados salvo los del nacimiento).
**Nacimiento APROBADO (~33 s)** — detalle en `firmware/old/nacimiento_aprobado_2026-11-16/LEEME.md`. Fondo negro; 12 líneas claras curvas (arcos de 26 segmentos, guiones que fluyen hacia dentro) giran siguiendo la silueta y se meten en espiral acelerando sin frenar (7 s); en el centro una partícula-nube blanca crece alimentándose, reacciona a cada trazo que entra (se abulta hacia ese lado + crece un poco) y tiene espasmos; explota (aro irregular + 20 rayos desiguales); nace el "feto" (nube blanca/crema con latido) que se parte en dos ojos de nube; esos ojos aceleran con pasitos de forma/color y cortan a ojos reales; ruleta de pastel a muy colorido (elige paletas saturadas); frena, cierra ~2 s (arcos), abre con rayitas y mira alrededor según personalidad.
**Autonomía**: parpadeo normal 0.23 s; parpadeo lento con emoción (24–50 s, cierra ~2 s y mira a varios lados según rasgos: curiosa/tímida/enérgica/dormilona/gruñona); acciones espontáneas ponderadas por personalidad; 35 s sin interacción = aburrimiento (clips mirada_vacio, suspiro, etc.); 100 s = bosteza y se duerme (arcos respirando; **no hay sueño profundo**: la pantalla no se apaga); cualquier botón despierta.
**Carga**: al conectar USB (hoy solo por serial `u`): ELECTRIC 1.9 s (ojos x1.22/1.28, sacudida 8, destello blanco DE LOS PROPIOS OJOS, **sin nada encima**) → CARGANDO hasta llenarse (+1.6 %/s, mínimo 14 s) con 3 tipos al azar según personalidad (0 ojos-recipiente que se llenan, 1 ojos que respiran y brillan, 2 ronroneo de ojos cerrados con brillo) → FELIZ 2.2 s → normal (no se duerme).
**Ver batería**: cada ojo es un recipiente (contorno oscuro vacío + nivel con color rojo→amarillo→verde, superficie ondulante, 4.2 s). Sin marcas/rayas (las quitó).
**Batería simulada** (`simBatt`, activa en la placa; serial `b` alterna): 100 %→3 % en 3 min, 1 min vacía, vuelve a 100 %. A <18 % se ve CANSADO (párpados pesados, paleta apagada) y pide carga con clips (hambre_bateria, tambaleo_sin_energia, apagado_lento). No duerme por energía mientras simula.
**Amor / acariciar**: la misma criatura feliz con sus colores tirando **corazones SOLO ROJOS**, grandes (9–25 px), sólidos, aparecen con rebote y laten; 4 variantes: 0 feliz con sus colores, 1 ojos cerrados felices con brillo, 2 clásico rosa-rojo (el que le había encantado), 3 corazones en los ojos.
**Enojado**: 9 formas al azar con rojos/naranjas: clásico, furia, ceño frío, berrinche, cejas, vapor, fuego (ojos-llama), chevrones > <, hervir (se va enrojeciendo). Alessandro debe elegir cuáles le gustan.
**Cara llena**: 6 paletas (cara+ojo+pupila) al azar. **Mareado**: ojos normales que chapotean contra los bordes (slosh, rebote a ±30/±24 px), sin espiral ni estrellas. **Sacudida** (motion/serial `k`/`K`): chapoteo + enojo si fuerte. **Sin estrellitas en ninguna parte** (`spawn` ignora PT_SPARK).

## 5. Reglas de gusto de Alessandro (ACUMULADAS — respetar)
Colores planos sobre negro, sin degradados. Ojos de UN solo color (nada de dos tonos). Pupilas simples (nada de brillo blanco, dos colores, anillo, puntito, canica, rendijas finas). Pose base = óvalos redondeados parecidos a los que le encantan; lo irregular/variado es animación ocasional que se acomoda tras un movimiento (formas transitorias; acelerómetro futuro). Cada color y forma con su personalidad (rasgos sesgados por color/forma). **Sin estrellitas/copos/destellos de 4 puntas.** **Nada "recargado"**: no poner efectos encima de los ojos (rayos, anillos, fogonazos, burbujas) — lo molesta. Corazones solo rojos, con presencia (no tenues). Colores lavados (blanco con toque de tono) no: o blanco limpio o color definido (regla en `aplicar_votos.py`; las pastel pálidas tiran a blanco conservando pupila). Pupilas no diminutas al bostezar/cansancio. Animaciones largas y con emoción; transiciones algo exageradas pero no abruptas. Evitar movimientos laterales rápidos (rasgado de pantalla). Quiere "variación" (varias versiones al azar por personalidad) en carga, cara llena, enojo, amor. Le gustan: desprecio (flick), tocar, cambiar de look, el nacimiento actual. Notas sin resolver de la ronda de animaciones: "Dormido-tenue → dormido → despertar: no" (sin motivo dado).
Pegó su JSON de animaciones dos veces (ayer/hoy): todo lo pedido ya está aplicado salvo la nota anterior.

## 6. Datos actuales
148 paletas, 37 especies, 14 pupilas, 141 clips (~76 mil combinaciones base; cada criatura = semilla + personalidad (6 rasgos sesgados por color/forma) + variaciones continuas). Flash ~163 KB (7 %), RAM ~254 KB (48 %). Reglas de color en `aplicar_votos.py`: (1) cortar blancos con tinte 0.06 ≤ sat < 0.215 y v ≥ 0.89 (14 paletas); (2) pastel pálidas 0.215 ≤ sat < 0.30 y v ≥ 0.85 → ojo 80 % hacia blanco (14 paletas). Recortes por votos: pupilas (Puntito, Lucero, Mitad y mitad, Anillo, Inclinada brillo, Canica, Redonda media, Rendija girada, Rendija, Rendija gruesa), especies (15), paletas (3).

## 7. Pantalla y rendimiento (lecciones)
SPI0 a 41.7 MHz (serial `f` cicla 25/31/42/62 MHz; **no confirmado por Alessandro que no haya basura a 41.7**; a 62.5 escribe más rápido que el barrido y reduce el rasgado, sin probar por él). El panel GC9A01 de 7 pines no tiene TE → **RASGADO/ENTRECORTES (tearing)** en movimientos laterales rápidos: parte superior/inferior del ojo desfasada. Es el PENDIENTE principal ("es cosa de la pantalla"). Mitigaciones hechas: envío por diferencias de filas, velocidades laterales limitadas, relleno rápido (`fillAll`), SPI rápido. Ideas: probar `f`→62.5 MHz, otra pantalla con TE/otra sincronía, reducir desplazamientos por cuadro.
El diff de filas asume que `shown` == lo que está en la pantalla; si un DMA aborta se pone `forceFull`.

## 8. PENDIENTES (prioridad)
1. **Entrecortes/tearing** de la pantalla (ver §7).
2. Alessandro "aún falta bastante por pulir": seguir iterando por voto/foto en `anims.html` (carga, enojo 9 formas — elegir, cara llena, amor, emociones, transiciones).
3. Confirmar en pantalla real: SPI 41.7 sin basura, colores (reglas), batería simulada/recipientes, botón verde 5 s.
4. Probar el botón verde 5 s físicamente (solo probé `R` por serial y persistencia con reinicios).
5. Votar "personalidades" (hasta hoy solo se votan combinaciones/animaciones).
6. Más variaciones de carga/cara llena; revisar sueño ("Dormido-tenue").
7. Guardar stats (energía/diversión/cariño) en flash (hoy solo se guarda la semilla); mostrar rareza al encender; filtro daltónico en check.py.
8. Acelerómetro MPU6050 real: afinar umbrales de `Motion.h`, re-correr `test_motion.py` de M3 contra M1.
9. Batería real (hoy simulada), USB real (hoy serial `u`), micrófono (ruido), temperatura (frío).
10. Regenerar GIFs de `animaciones/` para las expresiones/escenarios nuevos; "swap eyes" entre dos dispositivos (idea de lilguy).
11. Skill `watch` sin configurar (necesita `yt-dlp` solo para enlaces; para archivos locales se usa OpenCV).

## 9. Historial resumido de versiones
v3 (aprobado 2026-11-14) → v5/v6/v7 estudio de agentes (paletas/especies/pupilas/clips, auditorías, curado) → v8 beta (botones, votos 1–4) → v9 nacimiento con EEPROM (luego quitado) → v10 estrella fugaz → v11–v18 ruleta (en sitio, drama, preludio de video) → v19–v29 preludio final (fondo negro, líneas curvas, partícula que se alimenta) = **nacimiento aprobado** → reglas de color → v30 carga/batería/cara llena/enojo ×4 → v31 amor/orden/sin rayas → v32 cargando limpio + enojo ×9 + corazones rojos → **v33 criatura guardada + verde 5 s**.
Referencia del nacimiento: video WhatsApp 2026-11-16 de la revelación del Starboy de Lil Guy (blanco con rayitas → punto → oscuridad → nube blanca que se parte en dos → ojos cerrados → ruleta de diseños); Alessandro después pidió invertir a fondo negro y líneas curvas que convergen.
