# AUD_tecnico — rendimiento, código y cuelgues (2026-11-14)

Sandbox: `studio/work/aud_tech/` (proyecto sin editar). Sin placa: hardware = modelo.

**Veredicto:** 0 estados inválidos, 0 cuelgues, 0 escrituras fuera de pantalla en ~5 M de cuadros. 2 ALTO (pila; DMA sin timeout) y varios MEDIO. El cuello de fps es el SPI, no el render; 25 % de bytes SPI se evitan sin cambiar un píxel.

## 1. Hallazgos (EyesCore.h = E, Motion.h = M, .ino = I)

| # | Sev | Dónde | Hallazgo y evidencia | Parche |
|---|---|---|---|---|
| 1 | ALTO | E:565 | `float w[NCLIPS]` en la pila. `-fstack-usage`: `directorClip` = 704 B. Pila core0 = **2 KB** (mapa del ELF). Cadena loop→updateAnimation(312)→directorClip ≈ 1,1 KB (55 %); con 300 clips ≈ 1,65 KB (80 %); ~450 desborda. | función `clipWeight(i)` (el mismo `switch`, `default:0`) y dos pasadas (suma, elegir) sin arreglo. Compilado: 704→**24 B**. |
| 2 | ALTO | I:48-49 | `dma_channel_wait_for_finish_blocking` y `while(spi_is_busy)` sin límite ni watchdog: un atasco = cuelgue permanente (no verificable sin placa). | `uint32_t t0=micros(); while(dma_channel_is_busy(dmaCh)) if(micros()-t0>100000){dma_channel_abort(dmaCh);break;}` y `rp2040.wdt_begin(2000)` en setup + `rp2040.wdt_reset()` en loop. |
| 3 | MEDIO | E:815,828,840,890,896,909 | Temporizadores con `now >= t` fallan al desbordar `millis()` (49,7 d). `wrap2.exe`: en el segundo previo al desborde `chooseForm` corre **56-59 veces/s** (normal ≤1), cambios de expresión 21/s, clips del director 5/s. Ráfaga de 1-3 s cada 49,7 días. | `static inline bool due(uint32_t n,uint32_t t){return (int32_t)(n-t)>=0;}` y sustituir `now >= X` por `due(now,X)`. |
| 4 | MEDIO | E:812,1041 | `t = now*0.001f` pierde precisión (24 bits): ulp 7,8 ms a 24 h; a 72 h el 2 % de cuadros repite `t`, a 240 h el **74 %**, a 1000 h el 93 % → gelatina/temblor/ruido a saltos tras días encendido. | `float t = (now % 600000u) * 0.001f;`. |
| 5 | MEDIO | E:29,1179 | `clampf` deja pasar NaN. Un solo `dt=NaN` envenena `cur/vel` para siempre (`nan.exe`: tras 600 cuadros normales, 0 píxeles encendidos). En firmware `dt` viene de `micros()`: latente (sim/API). UB: `(uint8_t)cur[]` con NaN (E:877). | `return !(v>=lo)?lo:(v>hi?hi:v);` |
| 6 | MEDIO (arte) | E:226,241-248 | La cota analítica `ext=1.03+0.5Σabs(c)` **recorta** la forma (el lóbulo cúbico c5/c3 no está acotado): "diente" visible, ver `extimg_sorprendido.png` (izq. actual, der. cota 1,7). 26 % de cuadros muestreados difieren; pico 1,5 k px. El aspecto aprobado incluye el recorte: decisión de arte. | Si se quiere suave: `ext=1.03+1.0*Σabs(c)` (+29 % píxeles evaluados). |
| 7 | MEDIO | E:582-583 | Si `frand01()==1.0` (1/65536 por llamada) el director cae a `pick=NCLIPS-1` (tag 7, `cosquillas_de_mov`) por redondeo: con r forzado, **9994/20000** genomas lo eligen. | Bucle del parche 1: `if(w<=0)continue; pick=i; if(r<w)break; r-=w;` (0/40000). |
| 8 | MEDIO | I:193 | `Serial.printf` por segundo puede bloquear si el host está conectado y no drena el CDC. | `if(Serial.availableForWrite()>120)` |
| 9 | BAJO | E:503 | `speed` NaN → clip sin fin. | `speed=!(speed>=0.2f)?0.2f:(speed>6?6:speed);` |
| 10 | BAJO | E:610,630 | `evSet` sin rango: `x>=NEXPR` lee `POSE[]` fuera (solo sim/API). | `if(x>=NEXPR)x=0;` en `startStep`. |
| 11 | BAJO | E:559,1182 | Eventos de clips `loop` solo en la 1.ª vuelta; `memset` de 115 KB por cuadro en `deepSleep`. | Reiniciar `evi`; saltar `memset` dormido. |
| 12 | BAJO | datos | Duplicados `verguenza`, `acurrucarse`, `arrullo` (#95,#155: M:138 reproduce el idle, no el de M1). `timido_contento`, `celos`, `cruzar_brazos` no acaban en reposo (salto final). | Corregir datos. |
| 13 | BAJO | E:26,973 | `rng` y `static int sd` no se reinician: `sim_init` no reproducible. 3 clips suman oy=32,5 px sin tope. | Reiniciar. |

**Sin hallazgo:** sin recursión; bucles terminan (`chanAt` con t1>t0, `K[i+2]` protegido); índices `slots[3]`, `parts[56]`, `pcols[7]`, `acc[5]`, `nph`, `mCool[24]` correctos; Motion.h saneado y modular; sin carrera DMA/render (se dibuja en `fb`, el DMA lee `shown`, swap tras `pushWait`); `directorClip` corre ~1 vez/3 s. Datos: n 2..12, t creciente, dur=última clave, ev≤17, rareza≤3, clips de Motion.h presentes.

## 2. Rendimiento

**Modelo SPI** (31,25 MHz, 16 bit): ms = filas·240·16/31,25e6 + 0,25. Filas enviadas promedio (20 expr × 3 criaturas) = 147,7 → 18,4 ms (techo 54 fps); cara-llena 240 filas = 29,7 ms (33,6 fps). Coincide con la placa (prom 45-48, mín 31).

**Costo de render** (host x86 ×30, calibrado con STATUS «render 5-12 ms»; ~90 ciclos/píxel evaluado, estimación sin placa).

| Escenario | µs host | ms placa est. | Notas |
|---|---|---|---|
| A nominal despierto | 295 | ~9 | 26 k px |
| D dos ojos grandes, 2 tonos, pupila brillo | 647 | ~19 | 47 k px |
| E D + 3 clips | 609 | ~18 | `clipUpdate` ≈ 15 µs |
| F E + 56 partículas mixtas | 771 | ~23 | +16 k px de primitivas |
| G E + 56 chispas | 1087 | ~33 | 36 k px `triAA` (8 triángulos/chispa) |
| I 25 partículas (uso alto real) | 330 | ~10 | |

F/G exceden 16,7 ms (~30-40 fps); no es uso real (pool típico <25).

**Optimizaciones (prototipos en el sandbox, salida idéntica bit a bit):**

1. **Filas sucias exactas** (`opt2/`): `plot()` y `renderEye` actualizan `dirtyY0/Y1` con la fila realmente escrita; se elimina `markRows` de las primitivas (~10 líneas). Filas 147,7→110,7 (**−25 %**), SPI 18,4→13,9 ms: techo 54→60 fps; hash de 12 000 cuadros idéntico, 0 píxeles fuera de lo marcado. Por expresión −5 % (feliz) a −46 % (triste).
2. **`renderEye`: párpados antes que `f` + compuerta de pupila** (radio seguro derivado de `pc1/pc2`, `lb` por renglón): **−10 a −27 %** de tiempo, mediana −20 % (A ≈ −15 %, D ≈ −17 %, E ≈ −26 %, L ≈ −22 %; mediciones de host con ruido de ±5 %), 27,6 M de píxeles idénticos.
3. **SPI 41,7 MHz** (ya existe la tecla `f`): −25 % del tiempo SPI en todos los cuadros; cara-llena 29,7→22,3 ms (45 fps). Requiere prueba en placa (pantalla sin basura). Después: ventana por columnas (DMA encadenado), −20 % de bytes más (`logs/spi.log`).

Descartado: recorte por renglón de `triAA`/`ellipseAA` (±3 %).

## 3. Pruebas adversas (todas con `aud_o2.exe`, ver `logs/`)

| Prueba | Resultado |
|---|---|
| 2000 semillas × 20 expresiones (+7 secuencias de 700 ms), auto ON | 1 800 000 cuadros, **0 fallos** |
| Igual, auto OFF | 1 800 000 cuadros, 0 fallos |
| Build `-fsanitize=undefined,float-cast-overflow` (trap): 540 k cuadros + clips + motion + tormenta | 0 trampas |
| 169 clips × espejo × vel 0,2/1/3 × 3 expresiones | 3042 corridas, 0 inválidas, 0 sin terminar, eventos 115/115 |
| 3-4 clips simultáneos (2028 pruebas, 4.º desaloja el más viejo) | 0 inválidas |
| `motion`: 15³ valores (NaN, ±inf, ±1e30, FLT_MAX…) × 10 `dt` (0, <0, NaN, inf, 1e9…) | 33 750, 0 inválidas |
| Ruido σ=1,2/6 g, 120 k cuadros, reloj hacia atrás | 0 inválidas |
| Tormenta de eventos (100 × 1800 cuadros) | 0 fallos; 151 `spawn` descartados (inofensivo) |
| `coreFrame` con `dt` 0, −1, 1e-9, 100, 1e9, inf | válido; **NaN irrecuperable** (5) |
| Desborde de `millis()` | ráfaga (hallazgo 3) |
| Cuelgues (watchdog 8 s) | ninguno |

## 4. Memoria (compilado: `arduino-cli`, 250 MHz, Optimize2)

Hoy: flash **149 964 B** (7 % de 2 088 960); RAM estática **251 328 B** (47 %; libre 272 960). `bufA+bufB` = 230 400 B (92 % de la RAM usada). Datos en flash: PALS 3136, SPECIES 4096, PUPILS 1296, CLIPS 2704, claves 21 712 (1357 × 16 B, prom. 8/clip), nombres ≈ 5,6 KB → ≈ 38,5 KB; RAM por datos 0.

Proyección 250 paletas / 80 especies / 300 clips (36 pupilas): ≈ 62 KB de datos (prom. 8 claves) → flash ≈ **174 KB** (8,3 %); con todos los clips a 12 claves ≈ 193 KB (9,2 %). RAM estática +0. Marginal: paleta ≈ 29 B, especie ≈ 77 B, clip ≈ 157 B. El flash no limita; **el límite real es la pila**: aplicar el parche 1 antes de ~350 clips.

## Reproducir
```
cd studio/work/aud_tech && python mkaud.py && g++ -std=c++17 -O2 -I aud aud.cpp -o aud_o2.exe
./aud_o2.exe info | seeds 0 2000 1 24 | clips | triple | motion | dt | extdiff 60 | spi 200 | perf 500
```
Extras en la misma carpeta (`mkopt*.py`, `bench2.cpp`, `spi2.cpp`...).
