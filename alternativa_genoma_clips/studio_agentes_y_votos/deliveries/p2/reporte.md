# P2 - 70 paletas frias/neon/oscuras/alto contraste
Archivo: p2_frias.pal.txt (+ p2_frias.h). Rareza 35 common / 25 uncommon / 10 rare. Familias: azules, turquesas/cianes, verdes, lilas/violetas, magentas/rosas, oscuras (vino, ladrillo, marino, bosque con pupila clara), neon, alto contraste.
Criterios (verificados con script): contraste pupila/ojo >=3:1, ojo/negro >=2.2:1, nombres <=23, distancia RGB >=110 entre pares y vs base. Revisado en out/paletas.png (indices 16-85).
Descartado: ojos muy oscuros (navy/vino/eclipse puros, se perdian en negro) y varias pupilas pastel con contraste <3.
Riesgos: nombres sin acentos (ASCII) por seguridad del compilador; el acento solo se ve en rare/dos tonos, juzgar en vivo.
