# ESTUDIO ALLOY — plan de trabajo (2026-11-14)

Objetivo del cliente (Alessandro): pasar de "20 expresiones" a un **sistema generativo de criaturas** donde cada Lil Guy sea ÚNICO
(cientos/miles de combinaciones de color, forma de ojo, pupila, personalidad, rareza), con **cientos de animaciones** reutilizables
(animaciones por forma + colores intercambiables para ahorrar memoria), **reacciones a movimiento** (acelerómetro futuro), calidad
"Disney", y que **nunca se trabe en la XIAO**. Duración: ~4 h de trabajo autónomo. Todo se documenta aquí para poder retomar si se acaban los tokens.

## Reglas de oro (todos los equipos)
1. **Nada se rompe**: la XIAO tiene que seguir corriendo a >=30 fps sin cuelgues. Presupuesto: RAM de datos < 60 KB extra, flash < 300 KB extra, render < 14 ms/cuadro a 250 MHz (medir en la placa).
2. **Estilo**: colores planos sobre negro, ojos grandes casi tocándose, adorable. Referencias: capturas de lilguynet + `studio/biblia_de_diseno.md`. Nada de degradados.
3. **Un core, dos destinos**: todo es C++ sin hardware dentro de `firmware/Alloy_Eyes/*.h`; el simulador PC compila lo MISMO.
4. **Aislamiento**: cada agente trabaja en su sandbox `studio/work/<agente>/` (se crea con `python tools/mkwork.py <agente>`), valida con `python tools/check.py studio/work/<agente>`, y entrega SOLO archivos de datos en `studio/deliveries/<agente>/` + `reporte.md`. Nadie toca `firmware/` ni `sim/` principales (los integra el equipo de integración).
5. **Datos, no código**: los departamentos de arte entregan tablas (`pal_*.h`, `species_*.h`, `pupils_*.h`, `clips_*.h`, `reactions_*.h`) con el formato definido en `firmware/Alloy_Eyes/CONTRATO_DATOS.md`.
6. Cada entrega incluye imágenes de prueba (hojas de contacto) generadas con el simulador y un reporte de 1 página (qué hizo, qué descartó, riesgos).
7. Bitácora viva: `docs/STATUS.md` (la actualiza el director tras cada fase).

## Organigrama

**Dirección** (Claude principal): director creativo+técnico. Arquitectura, integración final, flasheo y medición en la placa, STATUS.

**Departamento 0 — Investigación** (2 agentes)
- R1 *Investigador de producto*: lilguy.net, creature.company, Instagram público de lilguynet (lo que sea accesible) -> `studio/reportes/R1_producto.md`
- R2 *Analista de referencias*: lee TODAS las capturas de referencia y escribe `studio/biblia_de_diseno.md` (proporciones, paletas, formas, pupilas, timing de cada emoción, qué hace "adorable").

**Departamento 1 — Motor** (Dirección + 1 agente)
- Genoma de criatura, rasgos de personalidad, dos tonos de ojo, reproductor de clips, capa de movimiento. (Dirección)
- E1 *Ingeniero de rendimiento*: perfila/optimiza el render sin cambiar el aspecto; medición en la XIAO.

**Departamento 2 — Color** (3 agentes + 1 auditor)
- P1 paletas cálidas/neutras (comunes) · P2 paletas frías/neón (poco comunes) · P3 paletas raras (dos tonos, contrastes locos, "mythic") · PA auditor de legibilidad/contraste y de que no haya duplicados.
- Meta: >= 160 paletas con nombre y rareza (común/poco común/raro/mítico).

**Departamento 3 — Anatomía** (3 agentes + 1 auditor)
- S1 especies "redondas y tiernas" · S2 especies "altas/elegantes/rendija" · S3 especies "raras/asimétricas/cómicas" · SA auditor de siluetas (todas distintas, ninguna fea, ninguna se monta).
- Meta: >= 48 especies (genes de forma por ojo, huevo/gota/triángulo, inclinaciones, proporciones, parpados por defecto).
- **Pupilas** (2 agentes): U1 pupilas simples (rendija, óvalo, punto, grande, inclinada...) · U2 pupilas especiales (media luna, dos tonos, con brillo, forma de gota, corazón pequeño, estrella). Meta: >= 28 estilos.

**Departamento 4 — Animación** (6 animadores + 1 supervisor) — estilo Disney: anticipación, squash&stretch, follow-through, arcos, timing, exageración, "appeal".
- A1 alegría/amor/cariño · A2 enojo/molestia/orgullo/celos · A3 tristeza/miedo/ansiedad/frío · A4 sueño/cansancio/bostezo/sueños · A5 curiosidad/sorpresa/juego/micro-acciones idle (mirar, guiñar, estornudar...) · A6 energía: carga/batería/electricidad/arranque/apagado/easter eggs.
- AS *Supervisor de animación*: revisa los GIFs de los 6, exige correcciones, aprueba.
- Meta: >= 120 clips (cada uno 0.3–3 s, canales de squash/offset/tilt/párpados/pupila/mirada + eventos de partículas), espejables y recoloreables.

**Departamento 5 — Movimiento / acelerómetro** (3 agentes)
- M1 *Diseñador de reacciones*: tabla de reacciones (inclinación, temblor, caída libre, golpe, giro, boca abajo, quieto mucho tiempo, mecer, volteo...) -> `reactions_*.h` + guion.
- M2 *Ingeniero del simulador*: entrada de movimiento virtual en `viewer.py` (sliders/teclas/arrastre = acelerómetro simulado).
- M3 *Tester de movimiento*: casos de prueba automáticos (señales sintéticas) que verifican que cada gesto dispara lo correcto y no hay falsos positivos.

**Departamento 6 — Calidad y mercado** (5 agentes)
- Q1 *Auditor de rendimiento*: peor caso de render/RAM; plan de pruebas en la XIAO; análisis de riesgos de cuelgue (DMA, SPI, desbordes).
- Q2 *Auditor visual*: revisa cada hoja/GIF contra la biblia; lista de defectos.
- Q3 *Auditor de código/seguridad*: revisión estática de los .h (índices fuera de rango, NaN, divisiones por 0, estados imposibles).
- Q4 *Panel de mercado A* (simulado): 8 personas (niños, adolescentes, adultos, coleccionistas) puntúan lo adorable/único/entendible de 40 criaturas.
- Q5 *Panel de mercado B* (simulado): prueba de "¿se entiende qué emoción es?" ciega sobre las expresiones + detección de combinaciones feas.

**Jefes** (3 agentes de dirección intermedia, revisan y firman)
- J-Arte (color+anatomía+animación), J-Tech (motor+movimiento+rendimiento), J-Calidad (auditorías+mercado). Cada jefe lee los reportes de su área y emite `studio/reportes/J_*.md` con APROBADO / CAMBIOS.

## Fases
| Fase | Qué | Quién | Entregable |
|---|---|---|---|
| 0 | Plan, herramientas (`tools/mkwork.py`, `check.py`), contrato de datos | Dirección | este archivo, `CONTRATO_DATOS.md` |
| 1 | Investigación + motor (genoma, clips, movimiento) | R1, R2, Dirección | biblia, `Genome.h`, `Clips.h`, `Motion.h` |
| 2 | Producción en paralelo: color, anatomía, pupilas, animación, movimiento | P1-3, S1-3, U1-2, A1-6, M1-3 | `studio/deliveries/*` |
| 3 | Supervisión y auditoría: AS, PA, SA, Q1-Q5, E1, jefes | | reportes, listas de cambios |
| 4 | Integración, correcciones, medición en la XIAO, flasheo | Dirección | `firmware/` actualizado, GIFs, `docs/STATUS.md` |
| 5 | Álbum final: hoja de 100 criaturas únicas + catálogo de clips | Dirección | `animaciones/`, `catalogo/` |

## Criterios de aceptación
- 1000+ criaturas únicas verificadas (genoma distinto -> aspecto distinto) y hoja visual de 100.
- >= 120 clips + >= 20 expresiones base funcionando, sin NaN ni salidas de pantalla.
- Prueba de 10 min en la XIAO (serial `d`) sin cuelgues, fps medio >= 40, mínimo >= 25.
- Reacciones de movimiento probadas con señales sintéticas en el simulador.
- Todos los jefes APROBADO o con cambios ya aplicados.

## Protocolo anti-pérdida de contexto
- Después de cada fase: actualizar `docs/STATUS.md` (qué está hecho, qué sigue, comandos exactos) y CLAUDE.md.
- Si se agotan los tokens: retomar leyendo `docs/STATUS.md` -> `studio/reportes/` -> `studio/deliveries/`.
