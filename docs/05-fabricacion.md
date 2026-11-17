# 5. Fabricación

## Paquete de Gerbers (`hardware/fabricacion/Gerbers_Alux.zip`, 31-oct-2026 20:35)

Generado desde `PCB1.PcbDoc` con *File → Fabrication Outputs → Gerber Files / NC Drill Files*.

| Archivo | Capa |
|---|---|
| `PCB1.GTL` / `PCB1.GBL` | Cobre Top / Bottom |
| `PCB1.GTS` / `PCB1.GBS` | Máscara de soldadura Top / Bottom |
| `PCB1.GTO` / `PCB1.GBO` | Serigrafía Top / Bottom |
| `PCB1.GTP` / `PCB1.GBP` | Pasta Top / Bottom |
| `PCB1.GM` | Mechanical 1 (contorno) |
| `PCB1-Plated.TXT` / `PCB1-NonPlated.TXT` | NC Drill PTH / NPTH |
| `PCB1.DRR`, `PCB1.REP`, `PCB1.EXTREP`, `PCB1.apr`, `PCB1.LDP` | Reportes y apertura |
| `Pick Place for PCB1.csv` | Posiciones de componentes |
| `BOM_ALUX.csv` | BOM (versión antigua; ver nota) |
| `Design Rule Check - PCB1.*` | Informe DRC |

Reporte de taladros (`PCB1.DRR`):

| Herramienta | Diámetro | Cantidad | Tipo |
|---|---|---|---|
| T1 | 0.711 mm | 9 | PTH |
| T2 | 0.8 mm | 6 | PTH |
| T3 | 1.0 mm | 24 | PTH |
| T4 | 1.02 mm | 7 | PTH |
| T5 | 2.2 mm | 3 | NPTH |
| | | **49** | |

## Parámetros de pedido en JLCPCB (PCB only, sin PCBA)

| Opción | Valor |
|---|---|
| Cantidad | 5 |
| Capas | 2 |
| Espesor | **1.0 mm** (cambiar el 1.6 por defecto) |
| Acabado superficial | **ENIG**, oro 1 U" |
| Color de máscara | Azul |
| Serigrafía | Blanca |
| Cobre | 1 oz |

El ensamble de SMD (0402, SOT-23, LGA-16) se hace **a mano** (ver [ensamble](06-ensamble.md)); por eso no se contrata PCBA.

## Componentes en LCSC

| LCSC # | Componente | Cant. |
|---|---|---|
| C1525 | 100 nF 0402 | 100 |
| C19702 (o equiv. 10 V X5R) | 10 µF 0402 | 100 |
| C25900 | 4.7 kΩ 0402 | 100 |
| C25741 | 100 kΩ 0402 | 100 |
| C82079 | BSS84LT1G SOT-23 | 10 |
| C15134 | LIS3DHTR LGA-16 | 7 |
| C157932 | S2B-PH-K-S (JST PH 2.0) | 5 |

Hay también una exportación del carrito de partes de JLCPCB del 23-oct en `hardware/fabricacion/2026-10-23 08_46 From Parts Cart JLCPCB  Parts.xlsx`.

## Estado de los pedidos (según las notas del equipo; no verificable desde los archivos)
- JLCPCB: 5 PCBs circulares **ordenadas, en tránsito** (~1–2 días).
- LCSC: componentes SMD **ordenados, en tránsito** (~5–6 días).
- En mano: XIAO RP2350, display GC9A01, batería LiPo, botones táctiles.

## Notas y riesgos
- El `BOM_ALUX.csv` incluido en el zip es de la primera exportación: lista C4 como **C15525 (6.3 V)** y usa el nombre "Alux". El BOM correcto es el de la tabla de arriba. Las notas mencionan `BOM_Kiro.csv` y `Gerbers_Kiro.zip`; **no se encontraron en este equipo** (solo `Gerbers_Alux.zip`).
- Cantidad de J2/J3/J4: ver [inconsistencia 8](09-inconsistencias.md) (5 conectores PH para 5 placas × 3 conectores no alcanzan).
- `PCB1.GM` no es una extensión estándar de contorno (`.GKO`/`.GM1`); según las notas JLCPCB lo reconoció en su visor, pero conviene confirmarlo.
