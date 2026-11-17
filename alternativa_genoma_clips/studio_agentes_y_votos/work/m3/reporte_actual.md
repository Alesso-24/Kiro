# Resultados automaticos de test_motion.py (20 semillas/caso)
Motion.h probado: `C:\Users\jordi\OneDrive\Escritorio\Alloy\firmware\Alloy_Eyes\Motion.h`

Eventos: 0=shake_suave, 1=shake_fuerte, 2=caida, 3=impacto, 4=de_cabeza, 5=vuelve_normal, 6=boca_abajo, 7=recogido

Clips usados por Motion.h y si existen en los datos: sacudida_suave=NO, sacudida_fuerte=NO, caida=NO, impacto=NO, boca_abajo=NO, asiente=si

| caso | esperado | observado (semillas con el evento) | tasa OK | veredicto |
|---|---|---|---|---|
| quieto_de_pie | nada | (nada) | 100% | PASA |
| quieto_plano | nada (sin 'recogido') | (nada) | 100% | PASA |
| inclinacion_0_60 | nada brusco (solo efecto continuo) | (nada) | 100% | PASA |
| mecer_0.5Hz_20deg | arrullo: sin sacudida/caida | (nada) | 100% | PASA |
| mecer_1Hz_20deg | arrullo: sin sacudida/caida | (nada) | 100% | PASA |
| sacudida_suave | shake_suave, sin caida/impacto | (nada) | 0% | FALLA |
| sacudida_fuerte | shake_fuerte, sin caida ni impacto espurios | shake_suave 20/20, shake_fuerte 1/20, impacto 20/20 | 0% | FALLA |
| caida_libre+impacto | caida (0.3-0.6 s |a|~0) e impacto | shake_suave 17/20, caida 20/20, impacto 20/20 | 15% | FALLA |
| golpe_seco_2g | impacto (tap +2 g, 2 muestras) | (nada) | 0% | FALLA |
| golpe_debil_1g (info) | informativo: no debe ser sacudida/caida | (nada) | 100% | PASA |
| de_cabeza_sostenido | de_cabeza tras ~1.2 s | shake_suave 20/20, de_cabeza 20/20 | 0% | FALLA |
| de_cabeza_y_vuelta | de_cabeza y luego vuelve_normal | shake_suave 20/20, de_cabeza 20/20, vuelve_normal 20/20 | 0% | FALLA |
| boca_abajo_sostenido | boca_abajo tras ~1 s | boca_abajo 20/20 | 100% | PASA |
| recoger_desde_plano | recogido | (nada) | 0% | FALLA |
| caminar_1.8Hz_0.4g | nada | (nada) | 100% | PASA |
| correr_2.8Hz_1.2g | como mucho shake_suave; no fuerte/caida/impacto | shake_suave 20/20 | 100% | PASA |
| vibracion_motor_60Hz | nada (alias de 80-200 Hz) | (nada) | 100% | PASA |
| ruido_puro_0.05g | nada | (nada) | 100% | PASA |
| ruido_fuerte_0.3g | nada (sensor ruidoso/EMI) | (nada) | 100% | PASA |
| ruido_fuerte_plano | nada (plano, sensor ruidoso) | (nada) | 100% | PASA |

## Estadisticas de senal (min / mediana / max sobre semillas)

| caso | mShake pico | min abs(a) | max abs(a) | dev pico |
|---|---|---|---|---|
| quieto_de_pie | 0.04 / 0.06 / 0.10 | 0.86 / 0.91 / 0.94 | 1.06 / 1.08 / 1.16 | 0.08 / 0.11 / 0.18 |
| quieto_plano | 0.04 / 0.06 / 0.10 | 0.86 / 0.91 / 0.95 | 1.06 / 1.09 / 1.15 | 0.08 / 0.11 / 0.18 |
| inclinacion_0_60 | 0.06 / 0.08 / 0.10 | 0.87 / 0.92 / 0.94 | 1.05 / 1.08 / 1.14 | 0.10 / 0.14 / 0.19 |
| mecer_0.5Hz_20deg | 0.17 / 0.19 / 0.23 | 0.86 / 0.91 / 0.95 | 1.07 / 1.11 / 1.18 | 0.22 / 0.27 / 0.33 |
| mecer_1Hz_20deg | 0.25 / 0.27 / 0.33 | 0.88 / 0.91 / 0.95 | 1.08 / 1.12 / 1.22 | 0.34 / 0.39 / 0.48 |
| sacudida_suave | 0.70 / 0.73 / 0.79 | 0.37 / 0.76 / 0.97 | 1.49 / 1.83 / 2.02 | 0.99 / 1.06 / 1.14 |
| sacudida_fuerte | 1.98 / 2.00 / 2.04 | 0.39 / 0.82 / 0.97 | 3.16 / 3.71 / 3.95 | 2.87 / 2.97 / 3.17 |
| caida_libre+impacto | 0.79 / 0.96 / 1.26 | 0.01 / 0.05 / 0.14 | 3.33 / 4.72 / 5.86 | 2.99 / 4.16 / 5.40 |
| golpe_seco_2g | 0.32 / 0.35 / 0.37 | 0.85 / 0.91 / 0.94 | 2.73 / 2.91 / 3.00 | 1.80 / 1.87 / 1.94 |
| golpe_debil_1g (info) | 0.17 / 0.20 / 0.22 | 0.85 / 0.91 / 0.94 | 1.77 / 1.93 / 2.01 | 0.87 / 0.94 / 1.00 |
| de_cabeza_sostenido | 0.88 / 0.90 / 1.00 | 0.55 / 0.60 / 0.93 | 1.06 / 1.13 / 1.47 | 1.12 / 1.17 / 1.23 |
| de_cabeza_y_vuelta | 0.93 / 0.94 / 0.95 | 0.86 / 0.91 / 0.95 | 1.06 / 1.08 / 1.16 | 1.13 / 1.16 / 1.21 |
| boca_abajo_sostenido | 0.57 / 0.58 / 0.59 | 0.86 / 0.92 / 0.94 | 1.05 / 1.09 / 1.14 | 0.70 / 0.72 / 0.78 |
| recoger_desde_plano | 0.48 / 0.51 / 0.61 | 0.39 / 0.82 / 0.93 | 1.08 / 1.15 / 1.51 | 0.56 / 0.60 / 0.78 |
| caminar_1.8Hz_0.4g | 0.31 / 0.34 / 0.37 | 0.52 / 0.54 / 0.61 | 1.45 / 1.49 / 1.55 | 0.45 / 0.49 / 0.51 |
| correr_2.8Hz_1.2g | 1.16 / 1.20 / 1.24 | 0.08 / 0.31 / 0.57 | 2.91 / 2.96 / 3.02 | 1.75 / 1.80 / 1.87 |
| vibracion_motor_60Hz | 0.12 / 0.16 / 0.20 | 0.74 / 0.86 / 0.95 | 1.09 / 1.18 / 1.27 | 0.24 / 0.27 / 0.33 |
| ruido_puro_0.05g | 0.09 / 0.10 / 0.11 | 0.81 / 0.86 / 0.89 | 1.12 / 1.15 / 1.21 | 0.16 / 0.19 / 0.21 |
| ruido_fuerte_0.3g | 0.55 / 0.58 / 0.65 | 0.11 / 0.29 / 0.38 | 1.75 / 1.97 / 2.27 | 0.94 / 1.13 / 1.27 |
| ruido_fuerte_plano | 0.55 / 0.58 / 0.65 | 0.16 / 0.31 / 0.41 | 1.81 / 1.97 / 2.11 | 0.94 / 1.13 / 1.27 |

## Latencias (mediana desde inicio de la senal) y clips/expresiones observados

- quieto_de_pie: lat [] ; clips {'salto': 8, 'asiente': 4, 'meneo': 15, 'guino': 10} ; expr(id:cambios) {2: 20, 1: 20}
- quieto_plano: lat [] ; clips {'guino': 12, 'salto': 11, 'asiente': 6, 'meneo': 10} ; expr(id:cambios) {2: 20, 1: 20}
- inclinacion_0_60: lat [] ; clips {'meneo': 5, 'asiente': 7, 'guino': 16, 'salto': 9} ; expr(id:cambios) {2: 20, 1: 20}
- mecer_0.5Hz_20deg: lat [] ; clips - ; expr(id:cambios) {2: 20, 10: 20}
- mecer_1Hz_20deg: lat [] ; clips - ; expr(id:cambios) {2: 20, 10: 20}
- sacudida_suave: lat [] ; clips - ; expr(id:cambios) {2: 20, 10: 1, 13: 19}
- sacudida_fuerte: lat [shake_suave=1.67s, shake_fuerte=3.15s, impacto=1.75s] ; clips - ; expr(id:cambios) {2: 59, 14: 51}
- caida_libre+impacto: lat [shake_suave=0.58s, caida=0.28s, impacto=0.58s] ; clips - ; expr(id:cambios) {2: 40, 18: 20, 14: 14, 17: 19}
- golpe_seco_2g: lat [] ; clips - ; expr(id:cambios) {2: 20, 17: 20, 3: 20}
- golpe_debil_1g (info): lat [] ; clips - ; expr(id:cambios) {2: 20, 17: 20, 3: 20}
- de_cabeza_sostenido: lat [shake_suave=0.70s, de_cabeza=2.08s] ; clips - ; expr(id:cambios) {2: 20, 10: 20}
- de_cabeza_y_vuelta: lat [shake_suave=2.27s, de_cabeza=2.08s, vuelve_normal=4.05s] ; clips - ; expr(id:cambios) {2: 40, 10: 20, 4: 20, 17: 20, 3: 20}
- boca_abajo_sostenido: lat [boca_abajo=1.82s] ; clips - ; expr(id:cambios) {2: 20, 17: 7, 3: 20, 13: 13}
- recoger_desde_plano: lat [] ; clips - ; expr(id:cambios) {2: 20, 13: 20}
- caminar_1.8Hz_0.4g: lat [] ; clips - ; expr(id:cambios) {2: 20, 13: 16, 17: 7, 3: 6}
- correr_2.8Hz_1.2g: lat [shake_suave=4.03s] ; clips - ; expr(id:cambios) -
- vibracion_motor_60Hz: lat [] ; clips - ; expr(id:cambios) {2: 20, 17: 11, 3: 10, 13: 14}
- ruido_puro_0.05g: lat [] ; clips - ; expr(id:cambios) {2: 20, 17: 9, 3: 9, 13: 17}
- ruido_fuerte_0.3g: lat [] ; clips - ; expr(id:cambios) {2: 20, 13: 19, 17: 9, 3: 9}
- ruido_fuerte_plano: lat [] ; clips - ; expr(id:cambios) {2: 20, 13: 17, 17: 10, 3: 8}
