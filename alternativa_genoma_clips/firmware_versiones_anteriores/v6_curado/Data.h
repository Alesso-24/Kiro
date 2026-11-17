// Data.h - tipos y registros de DATOS del estudio (paletas, especies, pupilas, clips).
// Los departamentos entregan archivos de datos; aqui se registran. NO poner logica aqui.
// Para agregar datos nuevos: ver CONTRATO_DATOS.md  (editar solo los .inc de la carpeta datos/).
#pragma once
#include <stdint.h>

// ---- paleta: color de ojo, de pupila y de acento (segundo tono / brillos) ----
struct Pal { uint8_t e[3], p[3], a[3]; uint8_t rarity; const char* name; };
// ---- especie = anatomia: escalas, inclinacion, armonicos de forma y asimetrias entre ojo izq. y der. ----
struct Species {
  float gap, sx, sy, tilt, c1, c2, c3, c4, c5, asx, atilt, ac4, aps, ay;
  uint8_t rarity; const char* name;
};
// ---- estilo de pupila ----
//  kind: 0 normal | 1 brillo | 2 dos tonos | 3 brillo+dos tonos | 4 media luna | 5 anillo
struct PupilStyle { float prx, pry, pdx, pdy, ptilt, pc1, pc2; uint8_t kind, rarity; const char* name; };
// ---- animacion: 12 canales cuantizados (int8 = valor/rango*127), interpolacion Catmull-Rom ----
//  canales: 0 sx 1 sy 2 ox 3 oy 4 tilt 5 ltl 6 ltr 7 lbl 8 lbr 9 ps 10 gx 11 gy
struct Key { uint16_t t; int8_t v[12]; uint8_t ev; };
//  flags: 1 espejable | 2 loop | 4 ease | 8 linear      tag (contexto): 0 idle 1 alegria 2 enojo 3 triste/miedo
//  4 sueno 5 curioso 6 energia 7 reaccion-movimiento
struct Clip { const char* name; const Key* keys; uint8_t n; uint8_t flags; uint16_t dur; uint8_t tag; };

static const Pal PALS[] = {
#include "src/datos/pal_all.h"
};
static const Species SPECIES[] = {
#include "src/datos/sp_all.h"
};
static const PupilStyle PUPILS[] = {
#include "src/datos/pu_all.h"
};
#include "src/datos/clips_all.h"          // define los arreglos Key y las macros CLIPLIST_xxx
static const Clip CLIPS[] = {
#include "src/datos/clips_list.h"
};
static const int NPALS = sizeof(PALS) / sizeof(PALS[0]);
static const int NSPECIES = sizeof(SPECIES) / sizeof(SPECIES[0]);
static const int NPUPILS = sizeof(PUPILS) / sizeof(PUPILS[0]);
static const int NCLIPS = sizeof(CLIPS) / sizeof(CLIPS[0]);
