# U2 - 14 pupilas especiales (u2_especiales.pu.txt)
Indices 8-21 tras las 8 base. Rareza: 3 common (Lucero, Mitad y mitad, Media luna), 5 uncommon (Anillo, Gato con brillo, Ovalo bicolor, Luna menguante, Gota bicolor), 4 rare (Aro grande, Rendija bicolor, Inclinada brillo, Domo luna), 2 mythic (Cometa, Eclipse).
Cubre glint, split, glintsplit, halfmoon, ring; formas raras: gota (pc2 .4), rendija con brillo, grande inclinada con brillo (ptilt .6), anillo oval (Eclipse pc1 .3).
Validado: check.py OK (0 errores). Hojas: u2_looks.png (14 estilos x 8 paletas), u2_expr.png (8 expresiones), pupilas.png. Script: u2_hojas.py.
## Hallazgos de motor
1. sim/viewer.py build(): el nombre de la DLL usa solo mtime de EyesCore.h y el .cpp; cambiar src/datos/*.h NO recompila (stale DLL). Hay que borrar sim/build/*.dll.
2. Glint: punto blanco chico (~3-4 px en 104px de hoja), correcto dentro de la pupila, pero muy discreto en Lucero/Gato; no se sale. En rojo oscuro/azul el brillo se ve bien.
3. split/halfmoon/ring: sin fallos; el 2.o tono se deduce del acento (version clara de la pupila). Cometa (ptilt -0.5, pc1 .15, pc2 .25) es el mas denso: bicolor diagonal + brillo, legible pero al limite.
4. Con expresiones de parpado cerrado/arco las pupilas desaparecen (esperado).
## Descartado
Pupilas muy grandes (>22 radio): el motor recorta a 62%/60% del ojo, sin efecto.
