# Reporte S2 - especies elegantes (18)
Archivo: s2_elegantes.sp.txt (+ s2_elegantes.h, registrado en sp_all.h del sandbox). Reparto: 8 common, 6 uncommon, 3 rare, 1 mythic (Oraculo).
Criterios: ojos altos/delgados (sx 0.6-0.86, sy 1.04-1.35), almendra con punta suave (c5<=0.16), inclinaciones moderadas (|tilt|<=0.2), gap>=0.75; asimetrias sutiles (asx, atilt, ay, aps) en Guino de hada, Ceja alzada, Cometa, Vigia, Rendija de luna, Oraculo.
Descartado: tilt 0.2-0.3 con c5 alto (Hoja, Pestana, Zorrito, Cometa, Pluma): se leian en V y casi se tocaban abajo / enojados; se suavizaron.
Validacion: check.py OK (60 semillas x 20 expresiones, sin errores). Hoja s2_expresiones.png (script s2_expr.py): 18 especies x 10 expresiones, legibles.
Riesgos: siluetas del bloque almendra (Almendra, Gato, Pestana, Cometa) son parientes cercanas; con gap bajo + HYPE, Zorrito/Cometa/Pluma se acercan por abajo (no se montan). Trampa: viewer.py cachea la DLL por mtime del core, no de los datos; borrar sim/build/*.dll tras cambiar .txt.
