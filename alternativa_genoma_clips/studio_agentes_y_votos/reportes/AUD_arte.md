# AUD_arte - auditoría de paletas, especies, pupilas y criaturas (2026-11-14)

Método: 196 paletas medidas por contraste WCAG ojo/pupila + distancia RGB entre pares, y revisión visual de las 4 hojas. Criaturas citadas como (fila,columna) de `criaturas.png` (8x8). Especies/pupilas se ven con ojo naranja/pupila verde fijos: la silueta es fiable, el color solo se juzga en `paletas.png`.

## 1) PALETAS: cortar 31 (quedan 165), ajustar 9
**CORTAR** (entre paréntesis, la que se conserva)
| Motivo | Cortar |
|---|---|
| Casi duplicadas | 112 Vino tinto (5), 134 Ladrillo y hielo (34), 113 Ladrillo antiguo (54), 170 Esmeralda real (95), 186 Plasma electrico (123), 192 Luna carmesi (188), 118 Perla negra (171), 117 Cielo y noche (189), 173 Neon retro (157), 178 Cereza con menta (169), 184 Nova dorada (174), 191 Oro y turquesa (174), 172 Ambar antiguo (160), 163 Galaxia tierna (104), 181 Uva y durazno (104), 180 Te de lavanda (175), 166 Coral vivo (83), 167 Lago de montana (91), 156 Algodon de azucar (110), 177 Gato de jade (121), 182 Piruleta acida (121), 187 Veneno de hada (148), 161 Atardecer en playa (56, mismo ojo #FF7A45), 176 Mar y fuego (185), 179 Limonada rosa (17, mismo nombre y ojo), 162 Aurora boreal (143, mismo nombre) |
| Sucias/apagadas | 99 Pantano, 29 Gris calido siena, 77 Piedra rosada (gris-beige sin carácter) |
| Exceso de azules oscuros | 136 Noche y fucsia, 120 Tinta y hielo |

Nombres engañosos: 178 y 191 no llevan menta/turquesa.

**AJUSTAR**
| Idx | Problema | Nuevo |
|---|---|---|
| 0 Durazno verde | contraste 1.7, verde claro se funde | pupila #0F8F46 |
| 15 Amarillo naranja | 1.9 | pupila #B8501E |
| 12 Lila ocre | 2.0 | pupila #8A4A00 |
| 10 Marfil cian | 2.2 | pupila #0A8FB0 |
| 5 Vino crema | ojo lum 0.10, sombrío | ojo #B24558 |
| 111 Cereza noche | ojo oscuro | ojo #A83B6B |
| 132 Vino y cian | ojo oscuro | ojo #A8285A |
| 131 Bosque y menta | ojo oscuro, parecido a 115 | ojo #14704A |
| 90 Azul pizarra | gris sucio | ojo #86A6F5 |

**Regla transversal**: ~35 paletas del bloque 121-195 tienen pupila casi negra (lum <0.015) que se confunde con el fondo y se lee como agujero (1,4) (2,2). Subir a un oscuro saturado con lum ~0.03: 185 #0E0018 -> #2A0840, 188 #16002E -> #2E0A5A, 151 #04041A -> #12126A, 174 #2A0A50 -> #3A1470, 123 #00203A -> #0A3A66. Tras el corte siguen saturadas las familias de blancos/cremas (~11), azul-índigo (~10) y lima/neón (4). Ninguna es ilegible por contraste salvo las 4 ajustadas.

## 2) ESPECIES: cortar 16 (quedan 48), ajustar 11
Siluetas realmente distintas: ~36; el resto son familias.
| Familia | Miembros | Conservar | CORTAR |
|---|---|---|---|
| Óvalo base/huevo | 0,1,2,3,6,8,16,18,22,33,39 | 0 Clasico, 2 Altito, 8 Bolita, 16 Ojitos de pan, 18 Burbuja | 1 Huevito, 6 Huevito tibio, 3 Regordete, 22 Cejita, 33 Sereno |
| Gota/almendra inclinada hacia dentro | 5,7,9,19,23,28,29,30,36,38,42 | 7,9,19,28,29,36,42 | 5 Gota, 23 Cometa (s1; nombre x3), 30 Hoja de otono, 38 Guino de hada |
| Alto y fino | 15,31,32,35,41,43,44,45 | 15,32,41,43,45 | 31 Luna menguante, 35 Brisa, 44 Rendija de luna (=32) |
| Ancho/aplastado | 10,13,14,25,26,55,60 | 10,14,26,55,60 | 13 Panecito, 25 Panzoncito (=10) |
| Pequeño | 11,17,24 | 11,24 | 17 Chispita |
| Asimétrico | 4,20,21,27,46,47,48,49,53 | 4,46,47,53 | 27 Duo disparejo (=53) |

Segunda ronda opcional (casi-duplicados que quedan, para bajar a ~36 reales): 20, 21, 48, 49, 50, 51, 40, 12, 39, 58.

**Bug de render (prioritario)**: fondo plano recortado en 1,3,5,6,10,13,25 y arriba en 63 (la caja del ojo no incluye el abultamiento c4/c1; la biblia lo lista como defecto). Revisar `renderEye`.

**Ajustes (clave=valor)**
| Idx | Problema | Valores |
|---|---|---|
| 14 Gominola | ojos solapados | añadir gap=1.1 |
| 19 Copito | bases se tocan | tilt=0.18 gap=1.05 |
| 54 Gotas Gemelas | se funden abajo | tilt=0.24 gap=1.15 |
| 24 Lentejita | insecto (diminuto) | sx=0.72 sy=0.78 gap=1.15 |
| 26 Luna llena | roza el borde | sx=1.25 sy=1.12 |
| 37 Pluma | se cruzan arriba | tilt=0.0 c2=0.06 |
| 59 Ciclope Tierno | ojo chico = astilla | asx=0.28 |
| 61 Cangrejito | espigado, roza borde | sx=0.7 sy=1.3 |
| 63 Torbellino | caótico y recortado | ac4=0.14 ay=5 c2=0.1 |
| 60 Alienigena Dulce | cuña enojada | tilt=0.4 |
| 62 Ojo y Rendija | casi sale del círculo | ay=-6 |

## 3) PUPILAS: cortar 18 (quedan 18), ajustar 2
| Cortar | Por qué |
|---|---|
| 16 Semilla, 8 Rendija fina | = 3 Puntito, = 0 Rendija |
| 14 Pelota media, 15 Pelota grande, 18 Ancha chata | = 2 Redonda; 15 y 6 rozan >55% del ojo (gigante); 18 mirada de cabra |
| 11 Rendija ladeada, 5 Ovalo medio, 20 Mirada centrada | = 4, = 1/17; 20 "mirada de pez" (biblia) |
| 7 Con brillo, 26 Gato con brillo | = 22 Lucero, = 0 + brillo invisible |
| 27 Ovalo bicolor, 29 Gota bicolor, 31 Rendija bicolor | = 23 Mitad y mitad; 31 ilegible |
| 28 Luna menguante, 33 Domo luna | = 24 Media luna |
| 30 Aro grande, 35 Eclipse | = 25 Anillo |
| 34 Cometa | sucia (glint + split + inclinada) |

Quedan: 0,1,2,3,4,6,9,10,12,13,17,19,21,22,23,24,25,32.
**Se pierden**: 3 y 16 (puntos), 10 (raya diminuta), 13 Canica en ojos grandes, 31, 34. Ajustes: 3 Puntito prx=5 pry=5 (hoy invisible en (6,1), (5,5), (8,7)); 10 Rendija bajita pry=18.

## 4) CRIATURAS: 18/64 se ven mal (28%), 8 dudosas
Malas (fila,col):
- Ojo de dos tonos diagonal + anillo/split/V: (3,4) (2,7) (5,6) (5,7) (8,7) (6,3).
- Ojos fusionados/solapados: (4,1) (7,5).
- Pupila punto en ojo grande, aspecto muerto: (3,6) (6,1) (4,2) (5,5).
- Media luna sobre especie de fondo recortado: (7,2) (2,5).
- Ojo chico + pupila grande (insecto): (3,3) (6,4).
- V con pupila diminuta / pupila perdida en ojo fino: (2,1) (2,8).

Dudosas: (1,2) (1,4) (2,2) (5,8) (6,7) (8,3) (7,8) (1,5).

**Reglas de exclusión**
1. Ojo de dos tonos NO con pupilas split/glintsplit/ring ni con especies de tilt >0.2 hacia dentro; solo si el acento difiere >0.2 de lum del ojo.
2. Especies con sx>=1.2 (Mochi, Luna llena, Gran Angular, Burbuja) exigen gap>=1.0; separación mínima final 3 px tras variación de personalidad.
3. Especies con fondo recortado (hasta arreglar el bug) NO con Media luna/Domo.
4. Ojo pequeño (sx*sy<0.65: Pelusa, Lentejita) solo con pupila de radio <=11; ojos grandes nunca con pupila de radio <=5.
5. Pupila con lum <0.015 NO con radio >14 ni con ring/glint (agujero o demonio: (1,4) (2,2)).
6. Especies finas (sx<=0.7: Junco, Pluma, Cangrejito, Oraculo) con pupila prx>=6 (rendija sobre rendija se pierde).
7. Contraste ojo/pupila >=3.0 obligatorio al sortear.

## 5) Adorable (fila,col = paleta + pupila)
**Mejores**
| # | Criatura |
|---|---|
| 1 | (7,3) 189 Cometa nevado, redonda morada |
| 2 | (2,6) 106 Piruleta azul, redonda granate |
| 3 | (1,8) 30 Chocolate caramelo, óvalo crema |
| 4 | (3,2) 50 Lavanda miel, redonda café |
| 5 | (4,7) 2 Menta lila, inclinada |
| 6 | (5,4) 18 Sandia fresca, redonda con brillo |
| 7 | (8,6) 18 Sandia fresca, pelota grande |
| 8 | (8,4) 31 Cafe con leche, redonda café |
| 9 | (6,5) 149 Sangre y plata, redonda blanca |
| 10 | (4,5) 21 Vainilla ciruela, media luna bicolor |

Patrón: ojo liso claro, pupila redonda baja de color pleno, ojos casi tocándose.

**Peores**
| # | Criatura |
|---|---|
| 1 | (4,1) 76 Petalo pistacho, anillo, fusionados |
| 2 | (7,5) 68 Arena granada, split, fusionados |
| 3 | (3,4) cian/azul con franja y rendija blanca |
| 4 | (2,7) rosa/amarillo en V |
| 5 | (5,6) azul/amarillo con anillos oscuros |
| 6 | (8,7) 147 Cristal de neon dos tonos, puntos |
| 7 | (3,6) 62 Sorbete de lima, punto invisible, fondo recortado |
| 8 | (7,2) 8 Lavanda limon, media luna, recortado |
| 9 | (6,1) 28 Marfil frambuesa, semilla |
| 10 | (5,5) 54 Arcilla marfil, puntito |

Las paletas de (3,4), (2,7), (5,6) se infirieron por color; no hay índice en la imagen (recomiendo rotular criaturas.png con semilla/ids).
