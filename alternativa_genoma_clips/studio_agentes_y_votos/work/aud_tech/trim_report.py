import re
p = r'C:\Users\jordi\OneDrive\Escritorio\Alloy\studio\reportes\AUD_tecnico.md'
s = open(p, encoding='utf-8').read()


def rep(a, b):
    global s
    assert a in s, a[:60]
    s = s.replace(a, b)


rep("Sandbox: `studio/work/aud_tech/` (nada del proyecto fue editado). Núcleo instrumentado con contadores + validador estricto (`aud.cpp`, `mkaud.py`). Sin placa: todo lo \"de hardware\" es análisis o modelo, marcado como tal.", "Sandbox: `studio/work/aud_tech/` (proyecto sin editar). Sin placa: lo de hardware es análisis/modelo.")
rep("Hay 1 hallazgo ALTO de escala (pila), 1 ALTO de resiliencia (DMA sin timeout) y varios MEDIO. El cuello real de fps es el SPI, no el render; hay 25 % de bytes SPI evitables sin cambiar un píxel.", "2 ALTO (pila; DMA sin timeout) y varios MEDIO. El cuello de fps es el SPI, no el render; 25 % de bytes SPI se evitan sin cambiar un píxel.")
rep(" (mapa: `__StackBottom 0x20081800`..`__StackTop 0x20082000`)", " (mapa del ELF)")
rep(" (55 %); con 300 clips ≈ 1,65 KB (80 %, sin contar IRQ USB); ~450 clips desborda.", " (55 %); con 300 clips ≈ 1,65 KB (80 %); ~450 desborda.")
rep("`static float clipWeight(int i){switch(CLIPS[i].tag){case 0:return 1;case 1:return .5f+G.trait[5]+G.trait[0];case 2:return .1f+.8f*G.trait[2];case 3:return .1f+.8f*G.trait[1];case 4:return .1f+.8f*G.trait[3];case 5:return .5f+G.trait[4];default:return 0;}}` y recorrer dos veces (suma, luego elegir). Compilado: frame 704→**24 B**.", "función `clipWeight(i)` (el mismo `switch`, `default:0`) y dos pasadas (suma, elegir) sin arreglo. Compilado: 704→**24 B**.")
rep(" Ráfaga de ~1-3 s, una vez cada 49,7 días.", " Ráfaga de 1-3 s cada 49,7 días.")
rep(" (salto de fase del ruido ambiental cada 10 min, invisible)", "")
rep(" En firmware `dt` sale de `micros()` (no NaN): latente vía sim/API.", " En firmware `dt` viene de `micros()`: latente (sim/API).")
rep(" El aspecto aprobado ya incluye el recorte: decisión de Alessandro.", " El aspecto aprobado incluye el recorte: decisión de arte.")
rep(" Rompe \"el director nunca elige tag 6/7\".", "")
rep(" (probado: 0/40000)", " (0/40000)")
rep("| 9 | BAJO | E:503 | `speed` NaN → clip que nunca termina; `inf` con `now==t0` → NaN. |", "| 9 | BAJO | E:503 | `speed` NaN → clip sin fin. |")
rep("| 10 | BAJO | E:610,630 | `startStep/evSet` sin rango: `x>=NEXPR` lee `POSE[expr]` fuera de rango (solo vía sim/API; el serial limita a 1-9). |", "| 10 | BAJO | E:610,630 | `evSet` sin rango: `x>=NEXPR` lee `POSE[]` fuera (solo sim/API). |")
rep("| 11 | BAJO | E:559,1182 | Eventos de clips `loop` solo disparan en la 1.ª vuelta (`evi` no se reinicia); en `deepSleep` se hace `memset` de 115 KB por cuadro. | Reiniciar `evi` al dar la vuelta; saltar `memset` si `deepSleep`. |", "| 11 | BAJO | E:559,1182 | Eventos de clips `loop` solo en la 1.ª vuelta; `memset` de 115 KB por cuadro en `deepSleep`. | Reiniciar `evi`; saltar `memset` dormido. |")
rep("| 12 | BAJO | datos | Nombres duplicados `verguenza`(#15,#70), `acurrucarse`(#20,#84), `arrullo`(#95,#155): `clipFind` devuelve el primero, M:138 reproduce el idle tag 4, no el de M1 tag 7. Tres clips no acaban en reposo (`timido_contento` c5=6, `celos` c0=-8, `cruzar_brazos` c11=51): \"salto\" al terminar. | Renombrar/corregir en datos. |", "| 12 | BAJO | datos | Duplicados `verguenza`, `acurrucarse`, `arrullo` (#95,#155: M:138 reproduce el idle, no el de M1). `timido_contento`, `celos`, `cruzar_brazos` no acaban en reposo (salto final). | Corregir datos. |")
rep("| 13 | BAJO | E:26,454,973 | `static int sd` y `rng` no se reinician en `coreInit`: `sim_init(seed)` no es reproducible. Suma de 3 clips: oy llega a 32,5 px (rango 24), sin tope. | Reiniciar en `coreInit`. |", "| 13 | BAJO | E:26,973 | `rng` y `static int sd` no se reinician: `sim_init` no reproducible. 3 clips suman oy=32,5 px sin tope. | Reiniciar. |")
rep("**Revisado sin hallazgo:** sin recursión; todos los bucles terminan (`while(acc>=1)` acotado, `chanAt` con t1>t0 garantizado, n=1 inalcanzable, `K[i+2]` protegido); índices `slots[3]`, `parts[56]`, `pcols[7]` (col≤6), `acc[5]`, `nph[2][12][4]`, `mCool[24]` correctos; Motion.h saneado (`mClean`, dt) y `now - mCool[k]` modular; sin carrera DMA/render (se dibuja en `fb`, el DMA lee `shown`, el intercambio es tras `pushWait`); `directorClip` corre ~1 vez/3 s, no por cuadro. Datos: n 2..12, t estrictamente creciente, dur=última clave, ev≤17, rareza≤3, todos los clips que pide Motion.h existen.", "**Sin hallazgo:** sin recursión; bucles terminan (`chanAt` con t1>t0, `K[i+2]` protegido); índices `slots[3]`, `parts[56]`, `pcols[7]`, `acc[5]`, `nph`, `mCool[24]` correctos; Motion.h saneado y modular; sin carrera DMA/render (se dibuja en `fb`, el DMA lee `shown`, swap tras `pushWait`); `directorClip` corre ~1 vez/3 s. Datos: n 2..12, t creciente, dur=última clave, ev≤17, rareza≤3, clips de Motion.h presentes.")
rep(" `markRows` marca la caja conservadora, no lo dibujado.", "")
rep("| E D + 3 clips | 609 | ~18 | clipUpdate ≈ 36 `chanAt` ≈ 15 µs: despreciable |", "| E D + 3 clips | 609 | ~18 | `clipUpdate` ≈ 15 µs |")
rep("Peor caso teórico F/G excede 16,7 ms (~30-40 fps solo por render); no ocurre en uso real (el pool típico es <25).", "F/G exceden 16,7 ms (~30-40 fps); no es uso real (pool típico <25).")
rep(" ~90 ciclos por píxel evaluado de ojo; píxeles evaluados/cuadro: nominal 21-26 k, ojos al tope 74×92 + 2 tonos + pupila brillo 47 k.", " ~90 ciclos/píxel evaluado.")
rep(" Por expresión: −5 % (feliz) a −46 % (triste).", " Por expresión −5 % (feliz) a −46 % (triste).")
rep("27,6 M de píxeles idénticos en 12 escenarios × 40 cuadros.", "27,6 M de píxeles idénticos.")
rep(" Siguiente paso, mayor y más caro: ventana por columnas (DMA encadenado): −20 % de bytes adicionales (tabla `logs/spi.log`, hasta −59 % filas+columnas en triste/amor).", " Después: ventana por columnas (DMA encadenado), −20 % de bytes más (`logs/spi.log`).")
rep("Resultado negativo: recortar `triAA`/`ellipseAA` por renglón no dio ganancia medible (±3 %); descartado.", "Descartado: recorte por renglón de `triAA`/`ellipseAA` (±3 %).")
rep("| Compilación `-fsanitize=undefined,float-cast-overflow` (trap), 540 k cuadros + clips + motion + tormenta | 0 trampas |", "| Build `-fsanitize=undefined,float-cast-overflow` (trap): 540 k cuadros + clips + motion + tormenta | 0 trampas |")
rep("| 169 clips × espejo × vel 0,2/1,0/3,0 × 3 expresiones | 3042 corridas, 0 inválidas, 0 sin terminar, eventos 115/115 |", "| 169 clips × espejo × vel 0,2/1/3 × 3 expresiones | 3042 corridas, 0 inválidas, 0 sin terminar, eventos 115/115 |")
rep("| `motion`: 15³ valores (NaN, ±inf, ±1e30, FLT_MAX, ±31,9, 100…) × 10 `dt` (0, <0, NaN, inf, 1e9…) | 33 750 combinaciones, 0 inválidas |", "| `motion`: 15³ valores (NaN, ±inf, ±1e30, FLT_MAX…) × 10 `dt` (0, <0, NaN, inf, 1e9…) | 33 750, 0 inválidas |")
rep("| Ruido σ=1,2 y 6 g, 40×3000 cuadros, reloj hacia atrás | 0 inválidas |", "| Ruido σ=1,2/6 g, 120 k cuadros, reloj hacia atrás | 0 inválidas |")
rep("| Tormenta de eventos (100 semillas × 1800 cuadros) | 0 fallos; 151 `spawn` descartados (pool lleno, inofensivo) |", "| Tormenta de eventos (100 × 1800 cuadros) | 0 fallos; 151 `spawn` descartados (inofensivo) |")
rep("| `coreFrame` con `dt` 0, −1, 1e-9, 1, 100, 1e9, inf | válidos y se recupera; **NaN: irrecuperable** (hallazgo 5) |", "| `coreFrame` con `dt` 0, −1, 1e-9, 100, 1e9, inf | válido; **NaN irrecuperable** (5) |")
rep("| Cuelgues (watchdog de 8 s en el arnés) | ninguno |", "| Cuelgues (watchdog 8 s) | ninguno |")
rep(" Datos en flash (`r` en el ELF): PALS 3136, SPECIES 4096, PUPILS 1296, CLIPS 2704, claves 21 712 (1357 × 16 B, prom. 8,0/clip, máx 12), nombres ≈ 5,6 KB → ≈ 38,5 KB. RAM por datos: 0.", " Datos en flash: PALS 3136, SPECIES 4096, PUPILS 1296, CLIPS 2704, claves 21 712 (1357 × 16 B, prom. 8/clip), nombres ≈ 5,6 KB → ≈ 38,5 KB; RAM por datos 0.")
rep(" Costo marginal: paleta ≈ 29 B, especie ≈ 77 B, clip ≈ 157 B. El flash no es límite (cabrían miles de clips). **El límite real es la pila (hallazgo 1)**: aplicar el parche 1 antes de pasar de ~350 clips. `clipFind` (strcmp lineal) a 300 clips ≈ 25 µs por reacción: irrelevante.", " Marginal: paleta ≈ 29 B, especie ≈ 77 B, clip ≈ 157 B. El flash no limita; **el límite real es la pila**: aplicar el parche 1 antes de ~350 clips.")
rep("python mkopt.py; python mkopt2.py   # opt/ y opt2/ ; bench2.cpp, spi2.cpp, dirtest.cpp, wrap2.cpp, nan.cpp\n```", "```\nExtras: `mkopt.py`, `mkopt2.py`, `bench2.cpp`, `spi2.cpp`, `dirtest.cpp`, `wrap2.cpp`, `nan.cpp`.")
open(p, 'w', encoding='utf-8').write(s)
print(len(re.findall(r'\S+', s)), len([x for x in re.findall(r'\S+', s) if re.search(r'\w', x)]))
