# Alloy BETA — cómo interactuar (2 botones)

| Acción | Qué hace |
|---|---|
| **Al conectar / encender** | **Nacimiento (~33 s)**. *Preludio* (~11.5 s): fondo NEGRO con 12 líneas claras limpias y parejas (poca variación de largo, curvatura y velocidad, sin ondular), como ríos en espiral, que fluyen y giran acelerando sin parar mientras todo se va metiendo hacia el centro (los bordes quedan vacíos) y convergen en una partícula-nube blanca de un solo color que se va alimentando: crece más (hasta ~26 px), reacciona a cada trazo que le llega (se abulta hacia donde entra y crece un poco con cada bocado) y tiene espasmos cada vez más seguidos; sin frenar, de golpe EXPLOTA (aro delgado algo irregular y rayos desiguales mientras la nube se encoge) y nace una nube blanca (el "feto") que se parte en dos ojos de nube. *Ruleta* (~20 s): esos ojos aceleran; con cada cambio brusco dan un pasito (forma, menos bultos, más color) hasta cortar a los ojos de verdad; siguen cambiando de forma y color (de pastel a muy colorido) con rayas de velocidad, frenan, cierran ~2 s, abren y miran alrededor según su personalidad. Nada se guarda: cada vez nace distinta. Mientras nace, botones y eventos se ignoran. |
| **Azul, toque corto** | Pasa a la **SIGUIENTE emoción en orden** (no al azar) por ~3.4 s y vuelve a la normalidad; cada pulsación avanza: curioso, feliz, amor, sorprendido, orgulloso, hype, cara llena, enojado, triste, ansioso, frío, mareado, bostezo, cansado, y vuelve a empezar. |
| **Azul, mantener (~0.8 s)** | Ver batería: los ojos son el medidor (rojo → amarillo → verde), ~4 s. |
| **Verde, mantener** | Acariciar = AMOR: la misma criatura feliz con sus propios colores tirando corazones de varios tamaños y colores. 4 variantes al azar: feliz con sus colores, ojos cerrados felices con brillo, clásico rosa-rojo, corazones en los ojos. |
| Sin tocarlo 35 s+ | Se aburre (mira al vacío, suspira, cabecea…). |
| Sin tocarlo 100 s+ | Bosteza y se duerme (arcos). Cualquier botón lo despierta. |
| Dormido | Sigue mostrando los arcos respirando (la pantalla ya no se apaga); un botón lo despierta. |
| Energía < 18 % | Cansado: pide carga con varias animaciones. |
| Diversión < 18 % | Triste. |

Cada encendido sortea una criatura única (paleta × forma × pupila × personalidad).

## Pruebas por serial (115200, solo desarrollo)
`1..9` expresión · espacio = siguiente expresión · `n` criatura nueva con nacimiento · `?` dice qué ojos tiene · `c` siguiente clip · `t` otro look · `p` piquete · `g` jugar (ya sin botón) · `w` guiño · `k` / `K` sacudida suave / fuerte (chapoteo contra los bordes) · `j` ruido · `v` ver batería · `x` desprecio · `f` cambia velocidad SPI · `d` línea de rendimiento.
**Evitar `u` (USB) e `i` (frío)**: dejan banderas de prueba activas hasta reiniciar.

## Sin implementar aún (necesita hardware)
Acelerómetro MPU6050 (sacudir/inclinar/caída → `Motion.h` ya listo), micrófono (ruido), temperatura (frío), detección real de USB/carga.

**Parpadeo lento con emoción**: de vez en cuando (cada 24–50 s, estando tranquilo) cierra los ojos despacio ~2 s, los abre y mira a varios lados según su personalidad. Los parpadeos normales son un poco más lentos que antes.
**SPI a 41.7 MHz** (antes 31.25): 60 fps en la ruleta. Si ves rayas/basura en la pantalla, serial `f` cambia la velocidad (25/31/42/62 MHz) o poner `spiHz = 31300000UL` en el .ino.

**Rasgado (tearing)**: la pantalla se refresca sola y esta placa no tiene pin TE para sincronizar, así que en movimientos laterales rápidos la parte de arriba y la de abajo del ojo pueden verse desfasadas. Mitigación: velocidad lateral máxima ~19 px/cuadro en la ruleta (sin estirar los ojos) y SPI rápido. Prueba: serial `f` una vez = 62.5 MHz (escribe más rápido que el barrido de la pantalla; si aparecen rayas/basura, `f` otra vez vuelve atrás).

## Carga, batería, cara llena y enojo (2026-11-17)
- **Le conectan el cargador**: sorpresa -> **fogonazo blanco doble + sacudida fuerte + rayitas** (como si le metieran el cargador) -> CARGANDO, de 3 tipos al azar según personalidad: *llenado* (los ojos son recipientes que se llenan con burbujitas y superficie ondulante, con la energía real), *ondas de energía* (ojos más grandes que laten y sueltan ondas) o *ronroneo* (ojos cerrados felices con brillo). Al llenarse: feliz y vuelve a la normalidad.
- **Ver batería** (azul largo): cada ojo es un **recipiente**: se ve el contorno vacío oscuro, el nivel con marcas de 25/50/75 % y color rojo -> amarillo -> verde.
- **Batería simulada** (mientras no hay sensor; serial `b` la activa/desactiva): baja de 100 % a 3 % en 3 min, se queda vacía 1 min y vuelve a 100 %. Por eso a ratos se verá cansado (<18 %).
- **Cara llena**: 6 paletas (amarillo/verde, menta/azul, rosa/morado, naranja/rojo, azul/amarillo, lima/verde azulado).
- **Enojado**: 4 formas (normal, furia, ceño frío, berrinche) y 6 colores de rojos/naranjas al azar.

- Ver batería: sin rayas/marcas dentro del recipiente (se quitaron).

## Actualización 2026-11-17 (limpieza de cargando, corazones rojos, 9 formas de enojo)
- **Cargando**: sin nada encima de los ojos (se quitaron rayitas, fogonazo, anillos y burbujas: se sentía "recargado"). Al conectar: ojos enormes + sacudida + destello DE LOS PROPIOS OJOS, luego uno de 3 tipos: ojos-recipiente que se llenan (limpio), ojos que respiran y brillan, o ronroneo de ojos cerrados.
- **Corazones**: siempre rojos, más grandes (9-25 px), aparecen con rebote, laten, sólidos (casi sin desvanecerse) y salen con más fuerza.
- **Enojado, 9 formas** (al azar, colores rojos/naranjas): clásico, furia, ceño frío, berrinche, **cejas**, **vapor**, **fuego** (ojos-llama), **chevrones > <**, **hervir** (empieza con sus colores y se va enrojeciendo y apretando).

## Actualización 2026-11-17 (criatura guardada, verde 5 s)
- **La criatura se guarda** en la memoria flash: al desconectar y volver a conectar es la misma y **NO vuelve a nacer** (solo abre los ojos contenta).
- **Verde mantenido 5 s = renacer**: criatura nueva (forma, colores, personalidad), con su animación de nacimiento, y se guarda reemplazando a la anterior (hasta que lo vuelvas a hacer). Mientras lo mantienes sigue acariciando; a los 5 s arranca el nacimiento. PROVISIONAL.
- Serial: `R` = renacer y guardar; `n` = criatura nueva SIN guardar (pruebas); `?` = muestra qué criatura es (`guardada=1` si cargó la guardada, `naciendo=1` si está en el nacimiento).
- Primera vez tras cargar este firmware no había criatura guardada, así que nació y se guardó esa.
