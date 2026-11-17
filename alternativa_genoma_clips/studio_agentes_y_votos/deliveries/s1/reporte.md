# S1 - Especies tiernas (22)
**Entrega**: s1_tiernas.sp.txt / .h (14 common, 6 uncommon, 2 rare). Hojas: s1_multiexpr_1/2.png (22 especies x 10 expresiones: despierto, curioso, enojado, amor, cargando, triste, cara-llena, hype, bateria-baja, orgulloso), especies.png, criaturas.png. multi.py = script de la hoja multi-expresion.
**Criterios**: rango biblia via sx/sy (pequeños sx 0.68-0.8; grandes 1.2-1.38), gap 0.7-1.25, huevo/gota con c4, asimetrias sutiles (asx, atilt, ac4, aps, ay) en uncommon/rare. Nombres sin tildes/ñ (evita problemas de fuente).
**Ajustes tras revisar**: Cometa fusionaba los ojos (bajé c5/c4, gap 1.1); diferencié Gotita/Gominola/Ojitos de pan/Copito/Burbuja/Panecito que se parecían; "Guiño tierno" -> "Ojo coqueto" (ñ no se mostraba).
**Descartado**: variantes casi iguales a Huevito/Bolita.
**Riesgos**: Panecito (c1/c3 negativos) y Mochi tienen tope plano en domo/enojo; Cometa y Luna llena anchas, cerca del borde en HYPE. check.py: sin errores graves. Hallazgo de herramienta: el viewer reutiliza la DLL cacheada (eyes_*.dll) si solo cambian los .h de datos; borrar sim/build/*.dll para recompilar.
