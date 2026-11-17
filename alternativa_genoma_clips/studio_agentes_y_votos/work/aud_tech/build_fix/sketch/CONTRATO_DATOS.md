#line 1 "C:\\Users\\jordi\\OneDrive\\Escritorio\\Alloy\\studio\\work\\aud_tech\\sketch_fix\\Alloy_Eyes\\CONTRATO_DATOS.md"
# CONTRATO DE DATOS — estudio Alloy

Todo el contenido artístico son **archivos de texto** que `tools/studio_compile.py` convierte en cabeceras C++ (carpeta `src/datos/`).
Cada agente trabaja en su sandbox `studio/work/<agente>/` (creado con `python tools/mkwork.py <agente>`), con la misma estructura que el proyecto:
`firmware/Alloy_Eyes/src/datos/` (datos), `sim/` (simulador), `out/` (imágenes de prueba).

Flujo de un agente:
1. `python tools/mkwork.py <agente>`  → sandbox.
2. Escribir tus archivos de texto en `studio/work/<agente>/firmware/Alloy_Eyes/src/datos/` (nombres con tu prefijo, p.ej. `p1_calidas.pal.txt`).
3. Compilar a cabecera: `python tools/studio_compile.py <pal|sp|pu|clip> <entrada> <salida.h> [--prefix xx]`.
4. Registrar tu cabecera en el manifiesto del sandbox (añadir UNA línea `#include "tuarchivo.h"`):
   paletas → `pal_all.h` · especies → `sp_all.h` · pupilas → `pu_all.h` · clips → `clips_all.h` **y además** una línea `CLIPLIST_<prefix>` en `clips_list.h`.
5. Validar: `python tools/check.py studio/work/<agente>` (compila el simulador con tus datos, prueba NaN/desbordes y genera hojas en `out/`). **Lee tus imágenes con la herramienta Read** y corrige.
6. Entregar: copiar a `studio/deliveries/<agente>/` tus `.txt` fuente + `.h` generados + las imágenes de `out/` que muestren tu trabajo + `reporte.md` (1 página: qué hiciste, qué descartaste, riesgos). **No toques `firmware/` ni `sim/` del proyecto principal.**

## Estilo (obligatorio)
Colores planos sobre negro, ojos grandes casi tocándose, lindos ("adorables"), sin degradados. Lee `studio/biblia_de_diseno.md` y mira `studio/referencias/ref_*.png`.
Pantalla redonda 240×240; el centro de la cara es (120, 124). Ojo típico: 76 px de ancho × 112 de alto, separación casi 0.

## Paletas — `*.pal.txt`
`nombre | rareza | #ojo | #pupila | #acento` (acento opcional: se usa como 2.º tono del ojo/pupila y brillos; si falta se deduce). Comentarios: líneas que empiezan con `//`.
Rareza: `common` (60 % de las criaturas), `uncommon` (25 %), `rare` (12 %), `mythic` (3 %). Los `rare/mythic` además tienen más probabilidad de ojo de dos tonos.
Reglas: pupila legible sobre el ojo (contraste ≥ 3:1 aprox.), ojo visible sobre negro, nombre ≤ 23 caracteres, sin duplicados ni casi-duplicados (ΔE visible). Nombre en español, evocador (p. ej. `Helado de menta`).

## Especies (anatomía de ojos) — `*.sp.txt`
`nombre | rareza | clave=valor clave=valor ...` — todas las claves son opcionales (por defecto neutro) y se SUMAN/MULTIPLICAN sobre cualquier expresión:
| clave | significado | rango | neutro |
|---|---|---|---|
| gap | multiplicador de la separación entre ojos (el motor impide que se monten) | 0.7–1.4 | 1 |
| sx, sy | multiplicador de ancho / alto del ojo | 0.6–1.5 | 1 |
| tilt | inclinación simétrica (rad, + = parte alta hacia afuera) | −0.7–0.7 | 0 |
| c1 | aplastar en diagonal | −0.4–0.4 | 0 |
| c2 | torcer/inclinar la forma (espejado) | −0.4–0.4 | 0 |
| c3 | abultar hacia adentro (+) o afuera (−) (espejado) | −0.4–0.4 | 0 |
| c4 | gota/huevo: + más ancho abajo, − más ancho arriba | −0.4–0.4 | 0 |
| c5 | punta triangular hacia afuera (espejado) | −0.4–0.4 | 0 |
| asx | asimetría de ancho: ojo derecho ×(1+asx), izquierdo ×(1−asx) | −0.4–0.4 | 0 |
| atilt | inclinación extra solo del ojo derecho (rad) | −0.5–0.5 | 0 |
| ac4 | asimetría de gota (+ derecho, − izquierdo) | −0.4–0.4 | 0 |
| aps | asimetría de tamaño de pupila (derecha ×(1+aps), izquierda ×(1−aps)) | −0.6–0.6 | 0 |
| ay | desfase vertical (px): derecho baja, izquierdo sube | −14–14 | 0 |
Reglas: la silueta debe leerse como *ojo* en cualquier expresión (no usar valores extremos juntos); ninguna combinación fea; siluetas distintas entre sí.

## Estilos de pupila — `*.pu.txt`
`nombre | rareza | prx pry pdx pdy ptilt pc1 pc2 | tipo`
prx/pry = radios (px), pdx = desplazamiento hacia el interior (+) , pdy = hacia abajo (+), ptilt = giro (rad, espejado), pc1 = aplastar diagonal, pc2 = gota (+ punta arriba).
tipo: `none`, `glint` (brillo blanco), `split` (mitad inferior de otro tono = acento), `glintsplit`, `halfmoon` (pupila cortada abajo), `ring` (anillo).
Rangos: prx 0–24, pry 0–30, pdx −20–20, pdy −20–24, ptilt −1.2–1.2, pc1/pc2 −0.45–0.45. Una pupila debe caber en un ojo de 76×112 (el motor la recorta a 62 % del ojo).

## Animaciones (clips) — `*.clip.txt`
Un clip es una capa ADITIVA que se suma a la pose actual (cualquier expresión y cualquier criatura), así sirve con todos los colores y formas.
```
clip nombre [mirror] [loop] [expr=TAG]
  0:    (estado inicial: lo no escrito vale 0)
  120:  sy=-0.14 sx=0.10 oy=2
  300:  sy=0.22 sx=-0.10 oy=-9 ev=sparks
  620:  sy=0 sx=0 oy=0
end
```
Tiempos en **ms** (estrictamente crecientes, ≤ 6000; acaba SIEMPRE en reposo = todo 0, salvo `loop`). Entre keyframes interpola con curva Catmull-Rom (suave, con rebote natural si pasas de largo). Los canales no escritos en un keyframe **mantienen** el valor anterior.
Canales (valor natural; rango de cuantización):
| canal | efecto | rango |
|---|---|---|
| sx, sy | estirar (+) / aplastar (−) ancho y alto del ojo (fracción) | ±0.5 |
| ox, oy | desplazar la cara (px; + derecha / + abajo) | ±24 |
| tilt | girar toda la cara (rad; + horario) | ±0.6 |
| ltl, ltr | párpado superior izq./der. (fracción de la altura que baja) | ±1 |
| lbl, lbr | párpado inferior izq./der. (sube) | ±1 |
| lt, lb | atajos: ambos ojos | ±1 |
| ps | tamaño de pupila (+ agranda, − encoge; fracción) | ±1 |
| gx, gy | mirada forzada (−1 izquierda/arriba … +1 derecha/abajo) | ±1 |
`ev=` dispara un efecto en ese instante: `heart hearts spark sparks drop bolt bolts line lines dots star wink blink shake sweat snow note`.
Opciones: `mirror` = se puede reproducir en espejo (derecha↔izquierda: el motor la voltea sola; úsala para todo lo que no sea simétrico); `loop` = repite; `linear`/`ease` = tipo de interpolación.
**TAG** (`expr=`): 0 idle/neutral · 1 alegría/cariño · 2 enojo/molestia · 3 tristeza/miedo/ansiedad · 4 sueño/cansancio · 5 curiosidad/sorpresa/juego · 6 energía/sistema · 7 reacción a movimiento (solo se dispara por eventos; nunca en idle).
Los clips con TAG 0–6 los elige solo el "director" mientras la criatura está despierta, ponderados por su personalidad (energía, timidez, gruñón, sueño, curiosidad, cariño).
Reglas de calidad (principios de animación): **anticipación** (un pequeño contra-movimiento antes de la acción), **squash & stretch** (conserva volumen: si sy sube, sx baja), **overshoot y asentamiento**, **arcos** (ox/oy no en línea recta), **timing** (acciones rápidas 150–400 ms, reposo largo), **follow-through** (pupila/párpados se retrasan respecto al cuerpo), **exageración** medida, **appeal** (que dé ternura), claridad en silueta. Duración típica 350–2500 ms.
Nombres únicos y descriptivos en español, snake_case, ≤ 23 caracteres (los de reacción a movimiento deben llamarse exactamente como pide la tabla de `Motion.h` si el brief lo indica: `sacudida_suave`, `sacudida_fuerte`, `caida`, `impacto`, `boca_abajo`, `asiente`).

## Reacciones de movimiento (M1/M2/M3) — `Motion.h`
Convención de ejes (g): x derecha de la pantalla, y ARRIBA de la pantalla, z hacia el espectador; el sensor lee la aceleración que se opone a la gravedad: de pie = (0,+1,0); plano boca arriba = (0,0,+1); de cabeza = (0,−1,0).
API: `motionFeed(ax, ay, az, now_ms, dt_s)` cada cuadro (60 Hz). Efectos continuos: `mLeanX/mLeanY/mTiltFace` (gravedad resbala ojos y ladea cara). Eventos: `motionReact(what, strength, now)`.

## Límites de memoria / rendimiento
Por clip ≤ 12 keyframes (16 B c/u). Totales objetivo: 200 paletas, 64 especies, 32 pupilas, 150 clips (≈ 20 KB de flash, 0 RAM extra). El render no depende de los datos (cuesta lo mismo) — no agregues código; solo datos.
