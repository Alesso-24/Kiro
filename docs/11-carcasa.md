# 11. Carcasa

> **Estado: pendiente.** La PCB (**Kiro v1**) y el firmware ya existen; falta diseñar la carcasa. Este capítulo reúne lo que ya se sabe para empezar el diseño.

## Qué debe resolver
- Alojar la **PCB circular** (~55 mm de diámetro, con una pestaña rectangular hacia el header del display; ver [capítulo 4](04-pcb.md)).
- Dejar ver el **display GC9A01** redondo (1.28").
- Dar acceso al **USB-C** del XIAO (carga y programación).
- Alojar la **batería LiPo** (Unit 60-20-35, 400 mAh; por el código probablemente 6.0 × 20 × 35 mm — medir la real).
- Exponer los **dos botones** (azul y verde, táctiles de 6×6 mm, conectados por cables a J3/J4).
- Sujetar la placa con los **3 agujeros M2** (taladros NPTH de 2.2 mm, numerados 7, 8 y 9 en la captura de Altium).
- Material previsto: **PLA** impreso en 3D (las notas lo mencionan al hablar de la calibración de temperatura).

## Datos mecánicos conocidos

| Dato | Valor | Fuente |
|---|---|---|
| Cuerpo de la PCB | círculo de radio 27.5 mm (Ø 55 mm) + pestaña rectangular | `PCB1.PcbDoc`, Gerbers |
| Espesor de la PCB | 1.0 mm | notas / [fabricación](05-fabricacion.md) |
| Agujeros de montaje | 3 × Ø 2.2 mm (M2), NPTH | `PCB1.DRR` |
| Huella del XIAO | 17.78 × 20.96 mm (X 51.11–68.89, Y 42.52–63.48 mm), centro (60, 53.06) | DRC / Pick & Place |
| Header del display J1 | 1×7, 2.54 mm, centro (60, 86.57) mm | Pick & Place |
| Conectores J2, J3, J4 | JST PH 2.0 mm, 2 pines, en Top | esquemático |
| Componentes en Top | J1–J4, U2, XIAO, Q1, R4, C3, C5 | Pick & Place |
| Componentes en Bottom | R1, R2, R3, R5, C1, C2, C4 | Pick & Place |

Coordenadas en mm, origen del proyecto de Altium (centro aproximado de la placa en (60, 60)).

## Por definir
- [ ] Medidas reales: batería, display (diámetro del vidrio y alto del módulo) y botones.
- [ ] Orientación del USB-C del XIAO respecto al borde y dónde queda la abertura.
- [ ] Cómo se monta el display (a presión, con marco, con cinta) y altura sobre la PCB (el header J1 es through-hole).
- [ ] Ubicación de los botones (en el costado, arriba o en la tapa) y cómo se sujetan los cables a J3/J4.
- [ ] Cerrar con tornillos M2 o con broches; ventilación si hace falta.
- [ ] Efecto en el sensor de temperatura del LIS3DHTR: el PLA cerrado genera un offset constante → calibrar con la carcasa puesta.
- [ ] Zona de movimiento: el acelerómetro debe quedar firme (sin holgura) para que sacudidas y vibraciones se lean bien.
- [ ] Acabado: color, texto/logotipo ("Kiro") y si lleva llavero o cordón.

## Entregables esperados
1. Modelo 3D (STEP + STL) de carcasa de dos piezas.
2. Renders y fotos del prototipo impreso.
3. Archivos fuente en `hardware/carcasa/` (a crear).
4. Notas de impresión: material, altura de capa, relleno, soportes.

## Cómo obtener el modelo 3D de la PCB
En Altium: *File → Export → STEP 3D* (en el editor de PCB). Ese STEP sirve de referencia para modelar la carcasa alrededor de los componentes reales.
