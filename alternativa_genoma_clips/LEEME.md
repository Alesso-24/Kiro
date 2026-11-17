> Nota: el proyecto se llamaba "Alloy" cuando se hizo todo esto; hoy se llama **Kiro**. Aquí se conserva el nombre antiguo y rutas como `firmware/Alloy_Eyes` (ahora `firmware/Kiro_Eyes`); al recuperar, usa la carpeta/sketch actual.

# Alternativa: sistema "genoma + clips" (NO se usa en el proyecto)

Esta carpeta guarda **todo lo anterior al Estudio v4**, intacto. Es una alternativa de animación completa y funcional que Alessandro decidió **no usar** (2026-11-17). Se conserva por si algún día se quiere recuperar una idea. El proyecto vigente está descrito en `../CLAUDE.md`.

## Qué era este sistema
Cada criatura salía de una **semilla de 32 bits** que combinaba paleta (148) x especie de forma (37) x pupila (14) x personalidad (6 rasgos), ~76 mil combinaciones base. Las animaciones eran **141 clips aditivos** (12 canales: squash/stretch, desplazamiento, giro, párpados, pupila, mirada) elegidos por un "director" según la personalidad. Ojos = elipses con armónicos (gelatina), partículas, tamagotchi con stats, movimiento por acelerómetro (`Motion.h`), nacimiento por hilos -> partícula -> explosión -> nube -> ruleta.
Por qué se dejó: Alessandro prefirió el estilo del Estudio v4 (siluetas de boceto, 2 colores planos, pincel para ojo cerrado, sin desvanecidos) y que la placa mostrara exactamente lo del navegador.

## Contenido
| Carpeta | Qué hay |
|---|---|
| `firmware_versiones_anteriores/` | Todas las versiones del core (v1 Goo ... v36d). **Último firmware del sistema genoma = `v33_guardado/`** (EyesCore.h + .ino). `nacimiento_aprobado_2026-11-16/` = firmware completo con datos + LEEME de la secuencia. v34-v36 ya son el motor nuevo. |
| `firmware_sistema_viejo/` | `Data.h`, `Motion.h` y `src/datos/` (paletas, especies, pupilas, clips compilados a .h). |
| `simulador_viejo/` | Simulador PC (`viewer.py`, `eyes_sim.cpp`, pygame) + `Abrir simulador.bat`. Solo funciona con el core viejo. |
| `herramientas_viejas/` | `integrate.py`, `curate.py`, `aplicar_votos.py`, `check.py`, `encuesta*.py`, `studio_compile.py`... (pipeline de datos y votaciones). |
| `studio_agentes_y_votos/` | Biblia de diseño, reportes de auditoría, entregas de los ~17 agentes, votos de las 4 rondas (`votos/`). |
| `catalogo/`, `animaciones/`, `out/` | Catálogo web de criaturas/animaciones (`anims.html`, encuestas), GIFs de las 20 expresiones, nacimiento en webp, hojas de contacto. |
| `estudio_propuestas/` | Propuestas HTML previas del estudio v4 (irregulares, mezcla, Wall-E/EVA, calificadores, versiones `alloy_v4_antes_*`) e `index.html`. |
| `docs_viejos/` | `CONTEXTO_COMPLETO.md` (hasta V36d), `STATUS.md` (bitácora V1..V36d, muy detallada), `CLAUDE_historico.md`, `COMANDOS.md`, `ALLOY_EXPLICADO.md`, `PLAN_ESTUDIO.md`, `CONTRATO_DATOS.md` (formato de paletas/especies/pupilas/clips). |

## Cómo recuperarlo (si hiciera falta)
1. Respaldar el firmware actual (`../respaldos/`).
2. Copiar `firmware_versiones_anteriores/v33_guardado/*` a `firmware/Alloy_Eyes/` y los datos (`Data.h`, `Motion.h`, `src/`) de `firmware_sistema_viejo/` al mismo lugar.
3. Para el simulador: copiar `simulador_viejo/` a `Alloy/sim/` y `herramientas_viejas/` a `Alloy/tools/` (usan rutas relativas `../firmware/Alloy_Eyes`).
4. Leer `docs_viejos/STATUS.md` para comandos y trampas.

## Ideas rescatables
Clips aditivos por personalidad, genoma por semilla, `Motion.h` (gravedad + eventos para el MPU6050, ya probado con señales sintéticas), reacciones a sensores, lección del SPI/DMA (también está en `../CLAUDE.md`).
