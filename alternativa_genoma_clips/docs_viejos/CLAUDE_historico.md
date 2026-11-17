# Alloy

> **LEER PRIMERO: `docs/CONTEXTO_COMPLETO.md`** (estado actual, reglas de gusto de Alessandro, cómo compilar/probar, pendientes). Lo de abajo es contexto histórico; si hay contradicción, manda CONTEXTO_COMPLETO.md y `docs/STATUS.md`.
> Resumen 2026-11-17: la placa guarda la última criatura en flash y NO vuelve a nacer al encender; botón verde 5 s = renacer y guardar (provisional). Azul corto = emociones en orden; azul largo = ver batería; verde = acariciar. Pendiente principal: entrecortes/rasgado de la pantalla.


Proyecto de Alessandro: **tamagotchi estilo "Lil Guy / Starboy"** con XIAO RP2350 + pantalla redonda GC9A01 + 2 botones.
Ojos de gelatina a color que se deforman con la emoción. Este archivo es el contexto vivo del proyecto: se actualiza con cada avance.

## Hardware
- MCU: Seeed XIAO RP2350 (sin WiFi/BT). Core Earle Philhower `rp2040:rp2040`, FQBN base `rp2040:rp2040:seeed_xiao_rp2350`.
- Pantalla: GC9A01 redonda 240x240, SPI, 7 pines (VCC GND SCL SDA DC CS RST), sin pin BL (luz siempre encendida).
- 2 botones (azul y verde), a GND con INPUT_PULLUP (LOW = pulsado). Sin MPU/acelerómetro por ahora.

## Cableado (verificado funcionando 2026-11-14)
| Pantalla | XIAO | GPIO |
|---|---|---|
| VCC | 3V3 | - |
| GND | GND | - |
| SCL (SCK) | D8 | 2 |
| SDA (MOSI) | D10 | 3 |
| DC | D0 | 26 |
| CS | D1 | 27 |
| RST | D2 | 28 |
| Botón azul | D6 | 0 |
| Botón verde | D7 | 1 |

## Estructura
- `firmware/Alloy_Eyes/EyesCore.h` — **TODO el código de los ojos** (poses, formas, física, partículas, tamagotchi). Sin hardware: lo comparten firmware y simulador.
- `firmware/Alloy_Eyes/Alloy_Eyes.ino` — pantalla (doble buffer RGB565 + DMA), botones, serial de pruebas.
- `sim/` — **simulador en PC** (doble clic en `Abrir simulador.bat`). `eyes_sim.cpp` compila el MISMO `EyesCore.h` a DLL (g++ de Strawberry; recompila solo con nombre único en `sim/build/`), `viewer.py` = ventana pygame. Modos sin ventana para revisar: `python viewer.py --snap shots` (hoja de todas las expresiones), `--gallery` (16 looks), `--idle 48` (48 s en modo autónomo), `--film "1:1.2,3:2,6:2"` (transiciones).
- `animaciones/` — **un GIF por cada una de las 20 expresiones** + `hoja.png` + `looks_galeria.png` (ver `animaciones/LEEME.md`; regenerar con `python viewer.py --export ../animaciones`).
- `firmware/old/` — versiones anteriores del core (v1 Goo, v2 poses, y `*_v3_aprobado_2026-11-14*` = punto de restauración aprobado por Alessandro) por si se quiere recuperar algo.
- `firmware/XIAO_GC9A01_Test/` — prueba original de pantalla+botones.

Flujo: iterar en el simulador -> cuando gusta, compilar y subir (no hay nada que portar).
Compilar/subir: `arduino-cli compile --fqbn rp2040:rp2040:seeed_xiao_rp2350:freq=250,opt=Optimize2 Alloy_Eyes` y `arduino-cli upload -p COM6 --fqbn ...` (cerrar Serial Monitor; el puerto puede ser COM5/COM6).

## Diseño de los ojos (v3, 2026-11-14)
- **Poses**: cada expresión es una fila de `POSES` (+ `SHAPES`); TODOS los parámetros viajan con resortes (algo subamortiguados) => morph fluido entre expresiones (ojo -> arco de dormir, etc.).
- **Formas irregulares**: frontera `u²+v²-1 - c1(u²-v²) - 2c2uv - c3u - c4v - c5u(u²-3v²)` (armónicos: aplastar, inclinar, abultar hacia adentro, gota/huevo, triángulo). Cada ojo suma: pose + forma del momento (`chooseForm`, cambia cada 2-6 s: normal/irregular/alto/ancho/uno grande/inclinados/gota) + ruido orgánico siempre vivo. Pupilas: elipse girada con armónicos propios y tamaños distintos por ojo (iguales grandes / chicas / una grande y una chica).
- **Looks** (como la galería de lilguy.net, "cada uno trae sus propios ojos"): 16 paletas ojo/pupila x 7 estilos de pupila (rendija, óvalo, redonda, puntito, inclinada...). Cada encendido sortea uno; ENTER / `evTheme` sortea otro.
- **Partículas** (pool de 56): corazones, destellos de 4 puntas, gotas con salpicadura, rayitos, líneas de susto. Marca de enojo tipo manga.
- **Gelatina**: cada expresión al entrar da una "patada" de estiramiento/aplastamiento distinta (HYPE salta, SURPRISE se infla, SAD se hunde...). Latido en AMOR, compás en HYPE, tiritar en FRIO, mirada nerviosa en ANSIOSO.
- Expresiones (20): despierto, curioso, dormido (arcos, sin Zzz, como las capturas), feliz, enojado, amor, cargando, eléctrico, dormido-tenue, triste, cara-llena, hype, batería-baja, mareado, sorprendido, orgulloso, bostezo, ansioso, frío, ver-batería.
- Referencias: capturas de lilguynet (Instagram) y https://lilguy.net. Comportamientos que cita el sitio: ruido fuerte -> ansioso; frío -> tirita; agitarlo -> se marea y se enoja; pulgar arriba -> ver nivel de batería; desprecio ("flick") -> triste; "swap eyes" entre dispositivos; ~400 looks con rareza; "adorable", "just a little guy".

## Tamagotchi
- Stats 0-100: energía (=batería), diversión, cariño; decaen con el tiempo (`timeScale` x1/x5/x20 en el simulador).
- Acciones: tocar (3 seguidos = AMOR), acariciar (mantener = amor), jugar (HYPE, sube diversión, gasta energía), USB (SORPRENDIDO -> ELECTRICO -> CARGANDO -> llena -> FELIZ -> normal), sacudir (curioso / fuerte: mareo+enojo), ruido, frío, pulgar, desprecio.
- Autónomo: parpadea, mira alrededor, a veces hace cosas solo (curioso, feliz, orgulloso, sorprendido, bostezo, guiño, cambia de look). 35 s sin tocar -> aburrimiento; 100 s -> bostezo y duerme; dormido se queda mostrando los arcos (sin sueño profundo). Energía < 18% -> BATERÍA BAJA; diversión < 18% -> TRISTE.
- **Botones (firmware)**: ver `docs/COMANDOS.md` (azul corto = emoción por un momento, azul largo = ver batería, verde = acariciar). Cada encendido nace una criatura distinta (ruleta de ojos que acelera -> cierra -> abre con el elegido); no se guarda. Personalidad sesgada por color y forma (ver STATUS V10).
- **Serial 115200 (pruebas)**: `1..9` expresión, ` ` siguiente, `t` look, `p` piquete, `w` guiño, `g` jugar, `u` USB, `j` ruido, `k`/`K` sacudir suave/fuerte, `v` pulgar (batería), `x` desprecio, `i` frío on/off, `d` línea de rendimiento, `f` cambia velocidad SPI (25 / 31 / 42 / 62 MHz).
- Teclas del simulador: ver docstring de `sim/viewer.py` (1-0 C H B M N O Y expresiones, clic = tocar, clic der = acariciar, G, U, F, L, K, J, I, T, X, ENTER, +/- tiempo, Q/A E/D pupilas).

## Rendimiento / pantalla (lecciones)
- Doble buffer RGB565 en RAM + DMA por SPI0 a 16 bits (big-endian directo) mientras se dibuja el siguiente; solo se envían las filas que cambian.
- **BUG que dejaba la pantalla congelada/negra tras ~1 s**: tras la DMA el SPI queda en 16 bits y el core Philhower NO restaura el formato si los ajustes no cambian (`beginTransaction` "reusing"), así que los comandos del cuadro siguiente salían de 16 bits y corrompían la ventana. Arreglo: `spi_set_format(spi0, 8, ...)` en `pushWait()` antes de `endWrite()`. (El SPI a 50 MHz NO era la causa probable.)
- El core deja `clk_peri` a 48 MHz (SPI máx ~24 MHz): se reconfigura a `clk_sys` en `setup()`. Con CPU 250 MHz: SPI 25 / 31.25 / 41.7 / 62.5 MHz; `spi_set_baudrate` elige el mayor ≤ pedido (pedir 31.3 M para 31.25).
- Medido en la placa (75 s ciclando expresiones, sin cuelgues): promedio ~48 fps, mínimo 31 (CARA-LLENA = 240 filas), 60 con pocas filas. Render 5-12 ms; el cuello es el SPI.
- `renderEye` recorre por renglón solo el tramo que cubre la elipse (cota analítica) en vez de toda la caja.

## v5: sistema generativo + estudio (2026-11-14 noche)
- **Genoma**: una semilla de 32 bits = criatura única (paleta x especie x pupila x ojo de dos tonos x personalidad x variaciones continuas). Datos: **196 paletas, 64 especies, 36 pupilas, 169 clips** => ~450 mil combinaciones base. Catálogo visual en `catalogo/` (100 criaturas únicas, paletas, especies, pupilas, tiras de clips por tag).
- **Clips**: capas aditivas de 12 canales (squash/stretch, desplazamiento, giro, párpados por ojo, pupila, mirada) con Catmull-Rom, espejables, con eventos de partículas; el "director" elige clips de idle según la personalidad (los de energía/sistema tag 6 y reacción tag 7 nunca salen solos).
- **Movimiento** (`Motion.h`, de M1): gravedad continua + eventos (sacudida suave/fuerte, caída, impacto, de cabeza, boca abajo, recogido...). Simulador: mantener TAB e inclinar con el mouse; gestos S, Shift+S, F, V, B, N, U, W, R, O (ver `studio/deliveries/m2/LEEME_movimiento.md`).
- **Estudio**: `docs/PLAN_ESTUDIO.md` (organigrama), `docs/STATUS.md` (estado y cómo retomar), `studio/` (biblia de diseño, reportes, entregas por agente), `tools/` (`mkwork.py`, `check.py`, `studio_compile.py`, `integrate.py`), contrato de datos en `firmware/Alloy_Eyes/CONTRATO_DATOS.md`. Datos en `firmware/Alloy_Eyes/src/datos/` (Arduino solo copia .h/.cpp, por eso `.h` y no `.inc`).
- Serial extra: `n` criatura nueva, `c` siguiente clip.
- Datos curados tras auditoría: 165 paletas, 48 especies, 18 pupilas, 141 clips (~142 mil combinaciones). `python tools/integrate.py --agents ... && python tools/curate.py` regenera todo.
- Placa: versión final cargada (144 KB flash, RAM 47%), 120 s de estrés sin cuelgues, 46 fps prom (mín 31). Auditoría técnica aplicada (pila, NaN, timeout del DMA).

## Bitácora
- 2026-11-14: cableado, test de pantalla + botones (confirmado por Alessandro, rotación 180° = `setRotation(2)`).
- 2026-11-14: simulador en PC con el mismo core; core v1 (Goo) -> v2 (poses según capturas) -> v3 (formas irregulares con armónicos, looks, partículas, 20 expresiones, tamagotchi, comportamientos de lilguy.net).
- 2026-11-14: encontrado y corregido el bug de congelamiento (formato SPI). Firmware v3 cargado en la placa a CPU 250 MHz, SPI 31.25 MHz; prueba de 75 s con cambios de expresión sin cuelgues.

- 2026-11-14: revisión por agente externo (compara simulador vs capturas) -> aplicado: separación garantizada entre ojos, parpadeo que aplasta el óvalo (ya no rectángulos), pupilas más visibles (estilos preferidos, mínimo 0.85x), CARA-LLENA con domos claros y pupilas medio domo, TRISTE más grande con lágrimas grandes, ORGULLOSO/ENOJADO/SORPRENDIDO más distintos, bostezo que cierra en arco ^, más acciones espontáneas (cada 3-8 s). NO aplicado a propósito: Zzz en dormido (las capturas no lo llevan), quitar la barra cian de CARGANDO (sí sale en las capturas), brillos en pupilas (estilo plano).

- 2026-11-14: Alessandro vio la placa atascada en "ojos amarillos + barra azul": (1) era estado CARGANDO persistente porque mis pruebas por serial dejaron `usb=true` en RAM (tecla `u`) — NUNCA dejar la placa con flags de prueba, reiniciar/re-subir al terminar; (2) la barra cian se leía como una pupila unida entre los dos ojos -> CARGANDO ahora = un rayo cian en cada ojo que pulsa + el ojo se llena como medidor según la energía. Además **verde largo = recorrer todas las expresiones** (antes guiño) para que se vean las 20 con solo los botones.

- 2026-11-14: Alessandro aprobó esta versión ("me agrada"). Guardados: GIF de cada animación en `animaciones/` y copia del core/ino aprobados en `firmware/old/`.

## Pendientes
- **Alessandro debe confirmar en la pantalla real**: que ya no se congela, que no hay parpadeo/tearing, colores correctos. Si hay basura a 31 MHz: serial `f` para bajar a 25 MHz (y poner `spiHz` más bajo en el .ino).
- Que Alessandro revise en el simulador qué expresiones/animaciones le gustan y cuáles ajustar.
- Imágenes/bitmaps RGB565 (logos/iconos) además de lo procedural.
- Sensores reales: MPU6050 (sacudir/golpe/inclinación), micrófono (ruido), temperatura (frío), cámara (pulgar) — hoy se simulan con teclas/serial.
- Guardar stats en flash (que el bicho "recuerde" entre apagados), "swap eyes" entre dos dispositivos.


## BETA — estado al cierre del 2026-11-14 (sesión de votaciones)
- **Cómo interactuar**: `docs/COMANDOS.md` (2 botones). Placa cargada limpia (sin banderas de prueba) con la versión `firmware/old/v8_beta/`. Estrés 120 s: 49 fps prom, mín 31.
- **Proceso de diseño por votos** (Alessandro vota en el navegador): `tools/encuesta.py <ronda> [N]` -> `catalogo/encuesta_rN.html` (caritas fijas + animación solo de la seleccionada, teclas 1/2/3, notas); `tools/encuesta_anim.py` -> `catalogo/anims.html` (20 expresiones + ~24 escenarios/transiciones en 2 criaturas). Servidor: `python -m http.server 8765 --bind 127.0.0.1` dentro de `catalogo/` (hay que relanzarlo cada sesión). Votos guardados en `studio/votos/ronda1-4.json` (**lo no marcado = me encanta**); `tools/aplicar_votos.py` aplica recortes por nombre (ya cortó pupilas con brillo/dos colores/anillo/puntito/canica/rendijas, 15 especies, 3 paletas). Quedan 162 paletas, 37 especies, 14 pupilas, 141 clips.
- **Reglas de gusto de Alessandro** (acumuladas): flat sobre negro; ojos de un solo color; pupilas simples (nada de brillo blanco, dos colores, anillo, puntito, canica, rendijas finas); pose BASE = óvalos redondeados (lo irregular = animación ocasional que se acomoda: `chooseForm` transitorio + especies suavizadas x0.4); **sin estrellitas** (`spawn` ignora PT_SPARK); mareo = ojos chapoteando contra los bordes (no espiral); le encantan: desprecio, tocar, cambiar de look, ojos rojos de amor.
- **Comportamiento aplicado**: carga con ojos grandes -> chispa -> cargando -> feliz -> normal; sueño solo tras 100 s con aburrimiento desde 35 s; batería baja = cansado + pide carga; ver batería = ojos-medidor rojo->verde; chapoteo (`slosh`) con rebote en bordes; corazones/rayitas irregulares; transiciones más exageradas.
- **Pendientes para mañana**: seguir evaluando `anims.html` (Alessandro pegó notas de ronda de animaciones; falta su juicio de 'Dormido-tenue a dormido' que solo dijo 'no'); mareo/sacudida sin pupilas (¿agregar?); más variedad de transiciones; ronda 5 de combinaciones con el nuevo reposo suave; votar personalidades; guardar stats en flash; rarity al encender; filtro daltónico; MPU real (umbrales de `Motion.h`); regenerar `animaciones/` GIFs.


## ESTADO ACTUAL (2026-11-16, cierre de esta sesión)
- **Nacimiento APROBADO** ("nacimiento"): restore en `firmware/old/nacimiento_aprobado_2026-11-16/` (LEEME.md con secuencia + parámetros), animaciones en `animaciones/nacimiento/`. Secuencia: fondo negro, 12 líneas curvas que giran siguiendo la silueta y se meten en espiral acelerando; partícula-nube blanca que crece alimentándose y reacciona a cada trazo, con espasmos; explosión (aro irregular + rayos); nace el "feto" (nube) que se parte en dos ojos de nube; esos ojos aceleran, dan pasitos de forma/color y cortan a los ojos reales; ruleta de pastel a muy colorido; frena, cierra ~2 s, abre y mira alrededor según personalidad. ~33 s; nada se guarda; botones/eventos ignorados mientras nace.
- **Botones** (`docs/COMANDOS.md`): azul corto = emoción por un momento, azul largo = ver batería (ojos medidor), verde mantener = acariciar. Cada encendido nace una criatura distinta (semilla `rp2040.hwrand32()`, nada se guarda).
- **Datos** (tras votos y regla de color): 148 paletas, 37 especies, 14 pupilas, 141 clips. Reglas de gusto: ojos de un solo color; pupilas simples; pose base = óvalos redondeados; sin estrellitas; nada de colores lavados (blanco con toque de tono): o blanco o color definido (regla en `tools/aplicar_votos.py`).
- **Placa**: SPI 41.7 MHz, envío por diferencias de filas, estrés 120 s ~59 fps prom (mín ~51). Último firmware = nacimiento V29 + paletas recortadas.
- **PENDIENTE principal**: entrecortes/rasgado de la pantalla (tearing, cosa del panel); luego: evaluar más animaciones en `catalogo/anims.html` (servidor `cd catalogo && python -m http.server 8765 --bind 127.0.0.1`), personalidades, guardar stats en flash, MPU real, regenerar GIFs en `animaciones/`.
- Herramientas: `tools/encuesta.py` (combinaciones), `tools/encuesta_anim.py` (anims.html: expresiones, escenarios, nacimiento), `tools/aplicar_votos.py`, `tools/check.py`; votos en `studio/votos/`; restore points en `firmware/old/` (v8..v29, nacimiento_aprobado).
- Para analizar videos: `watch` skill sin configurar; método usado: OpenCV extrae fotogramas y se leen (video de la revelación del Starboy, WhatsApp 2026-11-16).

- Regla de color 2 (2026-11-16): las pastel pálidas tiran a blanco (ojo 80 % blanco, pupila conservada); regla 1 corta los blancos con toque de tono (sat 0.06-0.215). Ambas en `tools/aplicar_votos.py` (idempotente; correrlo tras integrate+curate).


## V33 (2026-11-17): criatura guardada + verde 5 s = renacer
- EEPROM (flash): `SEED_MAGIC 0xA110C0DF` + seed. Al encender: si hay seed válida -> `birthSkip` (no nace, solo abre los ojos contenta); si no -> nace y se guarda. Botón verde 5 s o serial `R` -> `doRebirth` (seed nueva, nacimiento, guarda). Serial `n` = nueva sin guardar. PROVISIONAL a pedido de Alessandro.
- Todo el contexto consolidado en `docs/CONTEXTO_COMPLETO.md`. Scripts de prueba en `tools/` (`stress_serial.py`, `id_serial.py`, `birth_perf_serial.py`).
