# AUD_mercado - Panel de mercado simulado (10 personas)

Método: vi las 64 criaturas (1-64, izq-der, arriba-abajo), paletas, especies, pupilas, la hoja de 20 expresiones y refs 4/12/18/24/27/28/31. Daltonismo: simulé protanopía y deuteranopía (Machado) en `work/aud_mercado/crea_prot.png` y `crea_deut.png`. Panel simulado: detecta problemas, no mide demanda.

## A. Favoritas y rechazadas por persona

- **Niña 8** - Fav: 4, 7, 36, 59, 62. "Son rosas y brillantes como dulces." No: 26, 41, 52. "Esos ojos de monstruo no me miran."
- **Niño 10** - Fav: 10, 20, 38, 45, 64. "Parecen de robot o dragón, ¡dan miedo bonito!" No: 13, 43, 59. "Esos son de bebé."
- **Adolescente 15 (Pokémon/Tamagotchi)** - Fav: 2, 21, 30, 38, 47. "Las pupilas de anillo y los dos tonos parecen un Pokémon raro que quiero conseguir." No: 8, 23, 37. "Café y gris triste, no se ve que tengan personalidad."
- **Universitaria 20 (llaveros)** - Fav: 7, 29, 39, 43, 59. "Pastel, tipo Sanrio, combinan con mi mochila." No: 10, 26, 53. "Muy agresivos o borrosos, no son kawaii."
- **Diseñador 30** - Fav: 12, 20, 35, 51, 64. "Pupila clara sobre un color limpio, buen contraste, paleta con intención." No: 15, 25, 53. "Dos tonos sucios y ojos fusionados, mala jerarquía."
- **Mamá 38** - Fav: 3, 14, 34, 60, 62. "Cálidos, simpáticos, nada que asuste a mi hijo." No: 10, 26, 45. "Rojo con cara de enojo o ojos muertos, no."
- **Coleccionista 45 (art toys)** - Fav: 2, 15, 21, 30, 38. "Formas y combinaciones raras, eso se presume en la vitrina." No: 3, 11, 32. "Genéricos, parecen el 'común' de cualquier set."
- **Abuelo 65** - Fav: 11, 12, 34, 51, 60. "Grandes, con la pupila bien visible; sin lentes los entiendo." No: 16, 22, 41. "Esa pupila es un puntito, no sé adónde mira."
- **Daltonismo rojo-verde** - Fav: 4, 12, 24, 51, 64. "Azules, amarillos y claros: el contraste es de luz, no de color." No: 5, 10, 45. "El rojo se me vuelve oliva oscuro y el ojo se funde con el fondo."
- **Crítico tecnológico** - Fav: 2, 12, 21, 30, 51. "Con ojos así se puede leer emoción; la mirada manda." No: 22, 41, 54. "Si el ojo no tiene dirección ni expresión, es un fondo de pantalla."

## B. Prueba ciega de emociones

Aviso: mi primera lectura traía etiquetas; la ciega (`hoja_ciega.png`) puede estar sesgada, y marqué fallo ante la duda.

Formato: n. veo / real / veredicto.
1 neutral / despierto / sí. 2 curioso / curioso / sí. 3 dormido / dormido / sí. 4 contento / feliz / sí. 5 enojado / enojado / sí. 6 enamorado / amor / sí. 7 energía / cargando / sí, pero igual a 8. 8 eléctrico / eléctrico / sí. 9 dormido / dormido-tenue / parcial (es 3 en naranja). 10 triste-enojado / triste / parcial (rojo y ceja contradicen la lágrima). 11 asomándose o adormilado / cara-llena / **NO**. 12 eufórico / hype / sí. 13 cansado / batería-baja / **NO**. 14 glitch, ojos rotos / mareado / **NO**. 15 asustado / sorprendido / sí, casi igual a 1. 16 gruñón / orgulloso / parcial. 17 aburrido / bostezo / parcial. 18 nervioso (gota de 2 px) / ansioso / parcial. 19 frío o tranquilo / frío / parcial (sin temblor, los destellos parecen alegría). 20 enfermo o en blanco / ver-batería / **NO**.

**Hallazgo clave:** 4 de 20 no se entienden (11, 13, 14, 20) y 6 más son ambiguas (9, 10, 16, 17, 18, 19). Se entiende lo que tiene silueta propia (arcos, corazones, rayo, ceja) y falla lo que depende de movimiento (mareo, frío, bostezo, ansiedad) o de código semántico (batería). Además, 9 vs 3, 7 vs 8 y 1/15/17/18 son prácticamente el mismo dibujo (ojo durazno + punto verde). Solo ~10 de 20 son claramente distintas en foto.

## C. Daltonismo rojo-verde (simulación)

- **Se pierden (protan/deutan):** rojos y rojo-naranja sobre negro. 45 casi desaparece y solo flotan las pupilas blancas; 10 pasa a oliva oscuro con pupila navy borrosa; 26, 37, 52 y 8 quedan casi negros. Paletas rojas con el mismo riesgo: 14, 47, 48, 111, 112, 149, 185.
- **Pupila verde/oliva sobre rosa, coral o marrón:** 5 (rendija verde sobre rosa) casi desaparece; 36 y 62 quedan apagadas; 6 y 56 pierden la pupila.
- **Se salvan:** azules, amarillos, blancos, cian, y todo lo que contrasta por luminosidad (12, 51, 64, 4, 24, 61, 2). Anillos y media luna aguantan; **rendija fina, puntito y semilla** (pupilas 3, 8, 16) fallan para todos y más para él.
- **Producto:** 20 ver-batería (verde) y 13 batería-baja no deben depender del color.

## D. Ranking agregado (votos de favoritas/rechazadas + mi criterio)

**Top 10:** 12, 51, 2, 21, 30, 38, 64, 62, 59, 60.
**Bottom 10:** 26, 41, 53, 22, 10, 45, 37, 54, 8, 25. (10 y 45 son polarizantes: el niño los ama, otros 3 los rechazan.)

- **Predictores de "adorable":** ojos anchos u ovalados (no altos); pupila grande (≥ ~25% del ojo), redonda u ovalada, oscura sobre ojo claro (51, 12, 59, 62); azules, periwinkle, rosa, blanco; tamaños desiguales con gracia (24, 59); anillos y dos tonos solo con buen contraste (2, 38).
- **Predictores de "raro/feo":** pupila de punto (22, 26, 37, 41, 54): ojos muertos; ojos altos y delgados con rendija (5, 16, 40, 55): reptil/alien; ojos fusionados (25, 53); marrones/terracota/oliva (8, 26, 37, 52, 56): "sucio", y hay unas 25-30 paletas de tierra de 196; rojo saturado con pupila oscura (10, 45): enojo permanente.

## E. ¿Lo comprarías? (supuesto: precio de Starboy $250-440 vs. ~$40)

- **Niña 8:** sí a $40 si lo pagan papás; a $300 no. **Niño 10:** sí si hay rojo/dragón y reacciona al sacudirlo; se aburre pronto.
- **Adolescente:** sí a $40-60 (coleccionar, "swap eyes"); a $300 no. **Universitaria:** sí si es llavero con carcasa bonita; un circuito pelón no.
- **Diseñador:** sí a $40 como objeto de escritorio; a $300 no ("la diversión se agota rápido", como dice la prensa). **Mamá:** a $40 solo con modo calmado y que no se rompa.
- **Coleccionista:** solo si la rareza es verificable y hay series; hoy no sabe qué es raro. **Abuelo:** quizá para un nieto; no entiende la batería ni ojos pequeños.
- **Daltonismo:** solo si pasa un filtro de contraste; hoy a ~1 de 6 criaturas le falla. **Crítico:** no, "fondo de pantalla animado caro".

**5 cambios de mayor impacto**
1. **Legibilidad emocional:** rediseñar 11, 13, 14, 20, 19, 9 con silueta única (espirales/ojos desalineados para mareo, barra de nivel para batería, sin depender del color) y separar 1/15/17/18.
2. **Piso de calidad del generador:** pupila mínima (~25% del ojo en comunes), prohibir puntitos y rendijas finas salvo en raros, y filtrar o rehacer tierras sucias y rojo-sobre-negro.
3. **Filtro daltónico automático** en `check.py` (Machado + contraste de luminancia pupila/ojo y ojo/fondo) y rojos con pupila blanca.
4. **Retención** (la crítica real de la prensa): persistir estado en flash, que la personalidad se revele en días, y "swap eyes" temporal.
5. **Objeto físico y precio:** carcasa tipo llavero; sin eso, es un módulo sin deseo de compra.

## F. Rareza para que coleccionar funcione

- **Común (~60%):** ya adorables: ojo oval ancho, pupila grande, un solo tono, paletas pastel/azul/rosa/crema. La peor criatura común debe ser alguien que quieras traer puesto. Hoy 8, 23 y 37 serían comunes que nadie quiere.
- **Poco común (~25%):** dos tonos, anillo, media luna, ligera asimetría (2, 38, 39, 43).
- **Raro (~12%):** formas con carácter (gota, aletas, ojos de tamaño desigual, pupilas desiguales como 44), pupila cometa/eclipse.
- **Mítico (≤3%):** paletas neón/brillantes con rasgo animado único, pupilas especiales o reacciones exclusivas.
- **Regla:** raro significa mejor y más peculiar, nunca más feo. Con ~450 mil combinaciones la rareza no se percibe: muestra nivel y nombre de familia al encender.
