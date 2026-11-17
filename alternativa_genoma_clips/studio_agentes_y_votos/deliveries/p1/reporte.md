# P1 - paletas calidas/pastel/tierra/neutros
70 paletas (40 common, 22 uncommon, 8 rare) en p1_calidas.pal.txt -> p1_calidas.h (prefijo p1). check.py: OK sin errores graves. Imagen: paletas.png (indices 16+ son las mias; 0-15 base).
Criterios: contraste pupila/ojo ~>=3:1 (WCAG, script propio; minimo ~2.6), ojos nunca casi negros (luminancia >=0.1), acento claramente distinto del ojo (dE>=12), nombres <=23 car. sin acentos.
Descartado/ajustado: ~25 casi-duplicados entre mis paletas y la base (rosa/crema/arena con pupila similar) cambiando pupila o ojo; pupilas naranja sobre lila subidas de oscuridad.
Riesgos: aun quedan ~13 pares con dE medio 13-18 (familias pastel parecidas: Marfil frambuesa/Hortensia rosa, Coral laguna/Tostado petroleo, Jengibre/Atardecer); aceptables pero candidatos a recorte. Contraste bajo en Frambuesa helada (~2.6) y Cereza menta pero legible.
Nota: por error ejecute mkwork una vez y borre mi sandbox; reconstrui el .txt completo, compilado y validado de nuevo.
