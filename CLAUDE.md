# Kiro — documento general del proyecto (único md vigente)

> **Nombre**: el proyecto se llama **Kiro** desde 2026-11-17 (antes "Alloy"). Los archivos de `alternativa_genoma_clips/` conservan el nombre Alloy (historia). El magic de la EEPROM sigue siendo `0xA110C0DF` a propósito: cambiarlo borraría la criatura guardada en la placa.

Tamagotchi estilo "Lil Guy / Starboy" de Alessandro: **dos ojos planos de color sobre fondo negro** en pantalla redonda, con 2 botones. Versión **madura (V38, 2026-11-17: V37 + intro KIRO)**: el diseño definitivo es el **Estudio v4** y es lo que se sigue puliendo. Lo anterior (sistema genoma + clips) está intacto en `alternativa_genoma_clips/` (no se usa; ver su `LEEME.md`).

## Estructura
```
Kiro/
  README.md + docs/            <- DOCUMENTACION DEL REPO por capitulos (concepto, hardware, esquematico, PCB, fabricacion, ensamble, firmware, animaciones, inconsistencias, pendientes) con imagenes en docs/img/
  hardware/                    <- COPIA del proyecto Altium PCB_TAMAGOCHI_FINAL (original en C:\Users\Public\Documents\Altium\, NO moverlo: es proyecto Altium 365) + fabricacion/
  CLAUDE.md                    <- contexto tecnico vivo para Claude Code (este documento)
  Abrir estudio.bat            <- abre el estudio en Chrome
  estudio/kiro_v4.html        <- FUENTE DE VERDAD del diseño (todas las animaciones, en el navegador)
  estudio/intro_kiro_10.html   <- propuestas de la intro del logo KIRO (la 12 es la elegida y esta en la placa)
  estudio/intro_completa.html  <- render EXACTO del firmware (GIF) de intro + nacimiento completo
  respaldos/v38_intro_kiro/    <- firmware final cargado (EyesCore.h, .ino)
  estudio/calificar_50.html    <- hoja para calificar 50 criaturas base (guarda JSON en Descargas)
  firmware/Kiro_Eyes/         <- EyesCore.h (motor, sin hardware) + Kiro_Eyes.ino (pantalla, botones, serial, EEPROM)
  firmware/XIAO_GC9A01_Test/   <- prueba original pantalla+botones
  herramientas/arnes/          <- harness.cpp / ident.cpp / soak.cpp: compilan el MISMO EyesCore.h en PC
  herramientas/serial/         <- stress_serial.py, id_serial.py, birth_perf_serial.py (pruebas en la placa)
  respaldos/v37_madura/        <- punto de restauración (EyesCore.h, .ino, kiro_v4.html)
  alternativa_genoma_clips/    <- sistema anterior documentado, NO usado
```
Flujo: se afina la animación en `estudio/kiro_v4.html` (en SU navegador, Chrome) -> se porta 1:1 a `EyesCore.h` -> se verifica con el arnés PC -> se compila y sube -> se mide -> se guarda respaldo en `respaldos/`.

## Hardware y cableado (verificado 2026-11-14)
Seeed XIAO RP2350 + GC9A01 redonda 240x240 (7 pines, sin BL ni TE) + 2 botones a GND (INPUT_PULLUP). `setRotation(2)`. Sin MPU/micrófono/sensor de batería aún.
| Pantalla | XIAO | GPIO | | Botón | XIAO | GPIO |
|---|---|---|---|---|---|---|
| SCL | D8 | 2 | | azul | D6 | 0 |
| SDA | D10 | 3 | | verde | D7 | 1 |
| DC | D0 | 26 | | | | |
| CS | D1 | 27 | | | | |
| RST | D2 | 28 | | | | |
VCC = 3V3.

## Compilar, subir, probar
```
cd firmware
arduino-cli compile --fqbn rp2040:rp2040:seeed_xiao_rp2350:freq=250,opt=Optimize2 Kiro_Eyes
arduino-cli upload -p COM6 --fqbn rp2040:rp2040:seeed_xiao_rp2350:freq=250,opt=Optimize2 Kiro_Eyes
```
Puerto COM6 (a veces COM5); cerrar monitores serie antes de subir. Compila: ~116 KB flash (5 %), ~249 KB RAM (47 %).
- Arnés PC: `g++ -O2 herramientas/arnes/harness.cpp -o harness.exe`; `harness.exe seed e:ID:seg ... | b` vuelca .raw RGB565. `ident.cpp` verifica que arranque y renacer den la misma criatura; `soak.cpp` simula 30 min buscando ojos aplastados.
- Placa: `python herramientas/serial/stress_serial.py` (120 s, usa `n` que NO guarda), `id_serial.py` (imprime `[ID]`), `birth_perf_serial.py`. Resultado de referencia: ~57-60 fps, render 6-8 ms.
- Estudio web: abrir con `chrome.exe --new-window file:///.../estudio/kiro_v4.html#expr=ID&auto=0`. Chrome sin ventana al frente corre la animación MUY lenta (capturas con transiciones a medias).

## Diseño (Estudio v4)
- **Estilo**: siluetas de boceto (huevo de base plana + picos/cresta inclinada, borde que respira), **2 colores planos** (ojo + pupila oscura del mismo tono), sin brillos/estrellas/degradados, pupilas con resorte propio, ojos asimétricos, ojo cerrado = **trazo de pincel** con extremos redondos (curvatura/inclinación por animación). **Sin desvanecidos**: todo cambia de golpe o deformando la forma, nunca mezclando transparencias.
- **Base** de la criatura = óvalo suave (forma de reposo "huevo") + pupila alta. Las formas muy irregulares/exóticas son SOLO momentáneas (50 % de las florituras; se acomodan solas).
- **Expresiones (23)**: despierto, curioso, feliz (mejillas), triste, enojado (V hacia el centro, picos hacia afuera), amor (ojos de corazón + corazones por los lados), sorprendido, orgulloso, ansioso (rebote), hype, mareado, frío, bostezo (curva caída ‿ con arrugas; NO ^ porque parece sonrisa), guiño, nacimiento, pantalla de color (11 combos), cara llena, eléctrico, cargando, ver batería (rampa rojo>naranja>amarillo>lima>esmeralda, nunca verde pantano), batería baja, dormido, dormido tenue.
- **Intro KIRO** (V38, 6.9 s, ANTES del nacimiento; solo cuando nace/renace): hilos blancos entran girando y escriben KIRO en puntos (se sostiene ~0.7 s) -> las letras mutan a saltos de color/grosor/inclinacion (2.6-5.2 s) -> quedan blancas con golpe -> se deshacen en particulas que se atraen al punto blanco; luego los hilos del nacimiento salen de los bordes (3.4 s, gruesos y calmados al principio) y empieza el nacimiento. Propuesta 12 de `estudio/intro_kiro_10.html`; codigo en `EyesCore.h` (`introInit/introDraw`, `introT`, `INTRO_DUR`). Con criatura guardada al encender NO se ve (no nace).
- **Nacimiento** (~33 s): 16 hilos blancos definidos -> partícula-nube alimentada (9.5 s, sin aro ni destellos) -> explosión -> dos manchas viscosas que nacen de un punto -> ojos de nube -> ruleta con manchas giratorias (~6 s) -> siluetas -> cierre por deformación (el contorno se vuelve el trazo de pincel, color cambia con el ojo cerrado, abre a los ~4 s). Botones/eventos ignorados mientras nace.
- **Identidad**: `creatureFromSeed()` = única fuente (colores, forma, pupila, espejo, escala, separación, personalidad y genes de ojos salen SOLO de la semilla). 38,400 bases discretas (100 colores x 64 reposos x 2 espejos x 3 pupilas) + valores continuos.

## Controles
| Acción | Resultado |
|---|---|
| Azul corto | Siguiente emoción (~3.4 s): curioso, feliz, amor, sorprendido, orgulloso, hype, cara llena, pantalla de color, enojado, triste, ansioso, frío, mareado, bostezo, batería baja, guiño, eléctrico (17) |
| Azul largo (0.8 s) | Ver batería (ojos-medidor) |
| Verde (mantener) | Amor |
| Verde 5 s | Renacer: criatura nueva, nacimiento, se guarda (EEPROM, magic `0xA110C0DF` + seed). PROVISIONAL |
Al encender: sin criatura guardada -> nace y guarda; con guardada -> NO nace, abre los ojos contenta.
**Autonomía**: emoción espontánea cada ~25-45 s; 35 s sin tocar = bostezo; 100 s = bostezo y duerme (arcos; la pantalla no se apaga); cualquier botón despierta. Eléctrico y batería baja NO salen solos. Batería SIMULADA (100->3 % en 3 min, 1 min vacía, ciclo).
**Serial 115200**: `1..9` expresión, espacio siguiente, `t` otro color (sin guardar), `n` criatura nueva sin guardar, `R` renacer y guardar, `?` imprime `[ID]`, `w` guiño, `v` batería, `j` ruido, `k`/`K` sacudida, `x` desprecio, `b` batería simulada on/off, `f` velocidad SPI (25/31/42/62 MHz), `d` línea de rendimiento. **Evitar `u` (USB) e `i` (frío)**: dejan banderas activas hasta reiniciar.

## Pantalla y rendimiento (lecciones que no se deben perder)
- Doble buffer RGB565 + DMA por SPI0 a 16 bits; solo se envían las filas que cambiaron (fb vs shown; `forceFull` si el DMA falla).
- **BUG clásico**: tras la DMA el SPI queda en 16 bits y el core Philhower no restaura el formato -> pantalla congelada/negra. Arreglo: `spi_set_format(spi0, 8, ...)` en `pushWait()` antes de `endWrite()`.
- El core deja `clk_peri` a 48 MHz (SPI máx ~24 MHz): se pasa a `clk_sys` en `setup()`. CPU 250 MHz; SPI 41.7 MHz por defecto. Espera de DMA con tiempo límite.
- **Tearing/rasgado** (PENDIENTE principal): el GC9A01 de 7 pines no tiene TE, en movimientos laterales rápidos la parte alta y baja del ojo se desfasan. Mitigado con diff de filas, SPI rápido y límites de velocidad lateral. Idea: `f` -> 62.5 MHz (sin confirmar por Alessandro), pantalla con TE, reducir desplazamiento por cuadro.

## Cómo trabajar con Alessandro (reglas acumuladas)
- Se llama Alessandro (usuario del sistema "jordi"); español, pide ajustes muy concretos por imagen. **Quiere VER todo en SU navegador** (Chrome predeterminado; abrir con chrome.exe directo) antes de dar por buena una animación. No quiere "adaptación de lo viejo": exactamente lo del navegador.
- Gustos: colores planos sobre negro; ojos de un solo color con pupila simple; sin estrellitas/destellos de 4 puntas; **nada recargado** (sin efectos encima de los ojos); corazones solo rojos; sin colores lavados; le molesta lo abrupto y lo desvanecido; animaciones con emoción y algo exageradas; no borrar propuestas (se archivan).
- El asistente NO puede ver la pantalla real: decir siempre qué no se pudo comprobar. Dejar la placa sin banderas de prueba. Registrar todo aquí.
- Trampas de herramienta: comandos Bash largos/heredocs grandes fallan -> escribir script con Write y ejecutarlo; Python en Windows no entiende `/tmp`; `ffmpeg` del PC es de 2013 (usar OpenCV); bajo Windows ojo con CRLF.

## Estado y pendientes
Última carga en la placa documentada: V36d (17 emociones en azul) = la versión del Estudio v4 final; V37 = esta reestructuración (sin cambios de motor; solo se actualizó el comentario de cabecera del .ino). No se re-subió a la placa tras reestructurar.
1. **Calificar las 50 criaturas base** (`estudio/calificar_50.html`) para afinar el generador.
2. Confirmar en la pantalla real: nacimiento V36, SPI 41.7 MHz sin basura, botón verde 5 s físico.
3. Tearing (ver arriba).
3b. Confirmar en pantalla real la intro KIRO (puntos legibles, transicion, caida puntual a 43 fps).
4. Seguir puliendo animaciones en el navegador y portarlas a la placa una a una.
5. Decidir si "eléctrico" queda fijo en la rotación azul (no quitarlo del código).
6. Futuro: guardar stats en flash, MPU6050/micrófono/temperatura/USB/batería reales (la lógica de `Motion.h` está en la alternativa), "swap eyes" entre dispositivos.

## Electronica (resumen; detalle en docs/)
- PCB enviada = **Kiro v1** = `PCB1.PcbDoc` (31-oct, Gerbers_Alux.zip, serigrafia dice "Alux V1" por su nombre anterior; se conserva como v1 aunque es la 2a iteracion). **Falta el diseno de la carcasa** (docs/11-carcasa.md); `PCB_ALTIUM_PRUEBA.PcbDoc` es la placa de practica. Esquematico: `Sheet1.SchDoc` (XIAO + LIS3DHTR + BSS84 + divisor VBAT + 2 botones + header display 1x7).
- **Pinout PCB (D -> GPIO del core RP2350)**: D0=26 VBAT_DIV (ADC), D1=27 CS, D2=28 DC, **D3=5** DISP_PWR (LOW=ON; NO es GPIO29), D4=6 SDA, D5=7 SCL, D6=0 BTN1, **D7=1 LIS_INT2**, D8=2 SCK, **D9=4 BTN2**, D10=3 MOSI. Display RST a 3V3 (RST=-1 en firmware).
- **Antes de subir firmware a la PCB**: DC->D2, sin RST, DISP_PWR=D3, boton verde->D9 (hoy D7 choca con INT2), VBAT_ADC=D0. Ver docs/07 y docs/09.
- docs/09-inconsistencias.md lista 14 hallazgos (criticos: D3 != GPIO29, boton verde vs INT2, pines del LIS3DH en las notas, contorno con pestana, BOM/JST).

## Bitácora breve
- 2026-11-17: repo + documentacion por capitulos (README, docs/01-10, hardware/) y auditoria de inconsistencias entre notas, Altium y firmware.
- 2026-11-17: **V38 (cargada en la XIAO, aprobada por Alessandro "asi se queda")**: intro KIRO 6.9 s (colores cambiantes ~3 s, final en partículas al punto) + transición suave: los hilos del nacimiento salen de los bordes (3.4 s, más gruesos y calmados al inicio; `gi`/`cf` en `drawBirth`). 52-60 fps (caída puntual 43). Respaldo `respaldos/v38_intro_kiro/`. La placa quedó con una criatura de prueba (`n`, no guardada): al reiniciar vuelve la guardada.
- 2026-11-14: cableado, pantalla, bug SPI/DMA, simulador, 1.ª versión aprobada.
- 2026-11-14..16: estudio de agentes, sistema genoma/clips, votaciones, nacimiento aprobado (todo en `alternativa_genoma_clips/`).
- 2026-11-17: V33 criatura guardada; V34-V36 port del Estudio v4 (motor nuevo), V36b bug de identidad (semilla guardada != criatura), V36c sin ojos aplastados, V36d eléctrico en azul.
- 2026-11-17: proyecto renombrado a **Kiro** (carpeta, sketch `Kiro_Eyes`, `kiro_v4.html`, textos).
- 2026-11-17: **V37 versión madura**: se elige `estudio/kiro_v4.html`; proyecto reestructurado, un solo md, lo anterior archivado y documentado.
