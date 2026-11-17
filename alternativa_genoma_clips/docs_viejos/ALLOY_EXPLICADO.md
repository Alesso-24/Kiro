# Alloy — el proyecto y todas sus animaciones

## 1. Qué es
Alloy es un tamagotchi minimalista estilo **Lil Guy / Starboy**: una criatura que solo tiene **dos ojos de gelatina de color** sobre fondo negro. Los ojos se deforman, parpadean, miran alrededor, lloran, tiran corazones, etc., según su emoción, su personalidad y lo que le haces. Vive en una pantalla redonda y se controla con dos botones.

- **Hardware:** Seeed XIAO RP2350 (250 MHz) + pantalla redonda GC9A01 de 240×240 (7 pines, sin pin de luz ni TE) + botón azul y botón verde. Todavía no hay acelerómetro, micrófono ni sensor de batería (se simulan).
- **Software:** `EyesCore.h` contiene todo el código de los ojos y es el mismo para la placa y para el simulador de PC. `Alloy_Eyes.ino` maneja la pantalla (doble buffer, DMA por SPI a 41.7 MHz, envía solo las filas que cambian), los botones, el serial de pruebas y la memoria flash.
- **Cada criatura es única:** una semilla de 32 bits decide paleta de color (148), especie de forma (37), pupila (14) y una personalidad de 6 rasgos sesgada por color y forma (curiosa, tímida, enérgica, dormilona, gruñona…). Hay unas 76 mil combinaciones base y además variaciones continuas.
- **Persistencia (provisional):** la última criatura nacida se guarda en flash. Al desconectar y reconectar es la misma y no vuelve a nacer, solo abre los ojos contenta.

## 2. Controles
| Acción | Resultado |
|---|---|
| Azul, pulsación corta | Pasa a la **siguiente emoción en orden** durante unos 3.4 s: curioso, feliz, amor, sorprendido, orgulloso, hype, cara llena, enojado, triste, ansioso, frío, mareado, bostezo, cansado |
| Azul, pulsación larga (0.8 s) | **Ver batería**: cada ojo es un recipiente que muestra el nivel |
| Verde, mantener | **Acariciar**: amor, la criatura feliz tirando corazones rojos |
| Verde, 5 s | **Renacer**: criatura nueva (forma, colores, personalidad), animación de nacimiento y se guarda en lugar de la anterior |

Durante el nacimiento se ignoran botones y eventos.

## 3. Principios de diseño (gustos de Alessandro)
- Colores planos sobre negro, sin degradados. Ojos de un solo color; pupilas simples.
- La pose base son óvalos redondeados; lo irregular (gota, huevo, triángulo…) aparece solo como transición y se acomoda.
- Sin estrellitas ni destellos de 4 puntas. Nada "recargado": no se ponen efectos encima de los ojos.
- Corazones solo rojos y con presencia. Nada de colores lavados (blanco con tinte): o blanco limpio o color definido.
- Animaciones largas, con emoción y algo exageradas, pero sin movimientos laterales bruscos (por el rasgado de pantalla).
- Variación al azar según personalidad en carga, cara llena, enojo y amor.

## 4. Animaciones

### 4.1 Nacimiento (aprobado, ~33 s)
Ocurre la primera vez que se enciende y al mantener el verde 5 s.
1. **Preludio:** fondo negro; 12 líneas claras curvas giran siguiendo la silueta circular y se meten en espiral hacia el centro, acelerando sin frenar (~7 s).
2. Una **partícula-nube blanca** en el centro crece alimentándose de las líneas; reacciona a cada trazo que entra (se abulta hacia ese lado) y tiene espasmos.
3. **Explosión:** aro irregular y 20 rayos desiguales.
4. Nace el **"feto"**: una nube blanca/crema que late y se parte en dos ojos de nube.
5. Los ojos de nube aceleran, dan pasos de forma y color, y **cortan a ojos reales**.
6. **Ruleta:** pasan muchas combinaciones de ojos (de pasteles a muy coloridos), cada vez más rápido, y frena en la elegida.
7. **Cierra los ojos ~2 s**, los abre con unas rayitas y **mira alrededor** según su personalidad.

### 4.2 Expresiones (20)
| Expresión | Qué se ve |
|---|---|
| Despierto | Reposo: óvalos redondeados, respiración sutil, parpadeo de 0.23 s |
| Curioso | Ojos algo más grandes, mirada que explora |
| Dormido | Ojos cerrados en arcos que respiran, sin "Zzz" |
| Dormido tenue | Arcos con paleta apagada y brillo suave |
| Feliz | Ojos en arco contentos, rebote leve |
| Enojado | Ver 4.3 (9 variantes) |
| Amor | Ver 4.4 |
| Cargando | Ver 4.5 |
| Eléctrico | Ojos grandes (×1.22/×1.28), sacudida y destello blanco de los propios ojos al conectar el cable |
| Triste | Ojos más grandes y caídos, lágrimas gordas |
| Cara llena | Ojos como domos con pupila a medias; 6 paletas al azar (cara, ojo y pupila) |
| Hype | Salta y late al compás, emocionado |
| Batería baja | Cansado: párpados pesados y paleta apagada |
| Mareado | Los ojos chapotean contra los bordes y rebotan (±30/±24 px); sin espiral ni estrellas ni dos colores |
| Sorprendido | Ojos que se inflan de golpe, líneas de susto |
| Orgulloso | Ojos entrecerrados, aire de suficiencia |
| Bostezo | Se estira, cierra en arco ^ y se relaja; pupilas no diminutas |
| Ansioso | Mirada nerviosa que se mueve a los lados |
| Frío | Tirita |
| Ver batería | Ver 4.6 |

Cada expresión, al entrar, da una **"patada" de gelatina** distinta (hype salta, sorprendido se infla, triste se hunde…). Todos los parámetros viajan con resortes, así que el paso de una a otra es un morph fluido.

### 4.3 Enojado (9 formas al azar, rojos y naranjas)
Clásico · furia · ceño frío · berrinche · cejas · vapor · fuego (ojos-llama) · chevrones `> <` · hervir (se va enrojeciendo). **Pendiente:** Alessandro debe elegir cuáles le gustan.

### 4.4 Amor / acariciar
La misma criatura, feliz, con sus propios colores, tirando **corazones solo rojos**, grandes (9–25 px), sólidos, con rebote y latido. Cuatro variantes al azar:
1. Feliz con sus colores.
2. Ojos cerrados felices con brillo.
3. Clásico rosa-rojo.
4. Corazones en lugar de pupilas.

### 4.5 Carga
Al conectar USB (hoy solo por serial `u`):
1. **Eléctrico** 1.9 s: ojos grandes, sacudida y destello de los propios ojos, sin nada encima.
2. **Cargando** hasta llenar (+1.6 %/s, mínimo 14 s), con **3 tipos al azar según la personalidad:**
   - Ojos-recipiente que se llenan.
   - Ojos que respiran y brillan.
   - Ronroneo con ojos cerrados y brillo.
3. **Feliz** 2.2 s y vuelve a la normalidad (no se duerme).

### 4.6 Ver batería y batería simulada
- **Ver batería:** cada ojo es un recipiente (contorno oscuro vacío) con un nivel que cambia de rojo a amarillo y a verde, con la superficie ondulando, durante 4.2 s. Sin rayas ni marcas.
- **Batería simulada** (provisional, no hay sensor): baja de 100 % a 3 % en 3 min, 1 min vacía y vuelve a llena. Por debajo de 18 % se ve cansada y pide carga con clips (hambre, tambaleo, apagado lento).

### 4.7 Vida autónoma
- **Parpadeo normal** de 0.23 s.
- **Parpadeo lento con emoción** cada 24–50 s: cierra ~2 s y luego mira a varios lados según los rasgos.
- **Acciones espontáneas** cada pocos segundos, ponderadas por personalidad: curioso, feliz, orgulloso, sorprendido, bostezo, guiño, cambio de look.
- **Aburrimiento** tras 35 s sin interacción (mirada al vacío, suspiro…).
- **Sueño** tras 100 s: bosteza y se duerme en arcos que respiran. La pantalla no se apaga. Cualquier botón la despierta.
- Hay unos **141 clips** de movimiento que el "director" elige según la personalidad.

### 4.8 Reacciones a sensores (preparadas, hoy simuladas por serial)
Sacudir suave → curioso; sacudir fuerte → mareo + enojo; ruido fuerte → ansioso; frío → tirita; desprecio (flick) → triste; tocar 3 veces → amor.

## 5. Estado y pendientes
- **Probado:** estrés de 120 s a ~59 fps de promedio (mínimo ~55), persistencia con reinicios, renacer por serial.
- **Sin probar físicamente:** el botón verde a 5 s.
- **Pendiente principal:** entrecortes/rasgado (tearing) en movimientos laterales rápidos. El panel no tiene pin TE, y el SPI a 41.7 MHz no está confirmado como libre de artefactos.
- **Otros pendientes:** elegir las formas de enojo, seguir puliendo animaciones, votar personalidades, guardar estadísticas en flash, acelerómetro/batería/USB/micrófono reales, regenerar GIFs viejos.

## 6. Archivos clave
`docs/CONTEXTO_COMPLETO.md` (léelo primero) · `docs/COMANDOS.md` · `docs/STATUS.md` · `firmware/Alloy_Eyes/EyesCore.h` · `Alloy_Eyes.ino` · `sim/` (simulador) · `catalogo/anims.html` (todas las animaciones en el navegador) · `firmware/old/v33_guardado/` (último respaldo).
