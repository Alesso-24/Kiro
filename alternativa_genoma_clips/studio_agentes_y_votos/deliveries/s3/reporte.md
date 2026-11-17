# S3 - 18 especies raras/asimetricas (s3_raras.sp.txt)
4 uncommon, 9 rare, 5 mythic. Cada una probada en 8 expresiones (s3_hoja.png, generada con mysheet.py) + check.py sin errores.
Uncommon: Ojito y Ojote, Ay Tropezado, Huevo y Gota, Bizco Tierno. Rare: Ojo de Pez, Pegaditos, Subibaja, Cometa y Luna, Gotas Gemelas, Gran Angular, Veleta, Triangulito, Buho Distraido. Mythic: Ciclope Tierno, Alienigena Dulce, Cangrejito, Ojo y Rendija, Torbellino.
Descartado/ajustado: Triangulito (c5 alto = mono de lazo) y Ojo de Pez/Gran Angular (se parecian) se suavizaron/separaron.
Riesgos: Triangulito sigue algo "mariposa" en MAREADO; Ciclope y Rendija tienen ojo pequeno (~0.35 del grande): revisar con paletas oscuras.
Hallazgo herramienta: viewer.build() cachea la DLL por mtime de EyesCore.h/eyes_sim.cpp; al cambiar solo los .h de datos hay que "touch EyesCore.h" o se ven datos viejos (check.py recompila bien).
