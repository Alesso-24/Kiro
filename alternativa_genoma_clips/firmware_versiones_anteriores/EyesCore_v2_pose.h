// EyesCore.h - "Lil Guy" tamagotchi: expresiones + animacion + render RGB565. SIN dependencias de hardware.
// Lo usan el firmware (Alloy_Eyes.ino) y el simulador del PC (sim/eyes_sim.cpp): mismo codigo en los dos.
//
// Diseno (basado en las capturas de referencia de Lil Guy):
//   * Cada expresion es una POSE (tabla de parametros). Todos los parametros viajan hacia la pose nueva con
//     resortes, asi que cualquier cambio se ve como un "morph" fluido (ojo -> arco de dormir, etc.).
//   * Ojos grandes, ovalados, casi pegados; pupilas pequenas/estrechas abajo-adentro; parpados rectos.
//   * Colores planos sobre negro. Extras: corazones, lagrimas, rayos, barra de carga, Zzz, espiral.
//   * Capa "tamagotchi": energia (bateria), diversion y cariño; acciones tocar / jugar / cargar / dormir.
#pragma once
#include <stdint.h>
#include <string.h>
#include <math.h>
#ifndef ARDUINO
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

const int W = 240, H = 240;
const float TAU = 6.2831853f;
uint16_t* fb = nullptr;   // framebuffer donde se dibuja (lo asigna quien use el core)

// ============================================================================================
//  Utilidades
// ============================================================================================
uint32_t rng = 12345;
float frand() { rng = rng * 1664525u + 1013904223u; return ((rng >> 8) & 0xFFFF) / 65535.0f * 2.0f - 1.0f; }  // -1..1
float frand01() { return (frand() + 1.0f) * 0.5f; }
float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
float DT = 0.0167f;
int dirtyY0 = H, dirtyY1 = -1;

void markRows(float ya, float yb) {
  int a = (int)floorf(ya) - 1, b = (int)ceilf(yb) + 1;
  if (a < 0) a = 0;
  if (b > H - 1) b = H - 1;
  if (a < dirtyY0) dirtyY0 = a;
  if (b > dirtyY1) dirtyY1 = b;
}

// ---------- Color ----------
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
uint32_t gAlpha = 32;  // opacidad global de lo que se dibuje (0..32)
static inline uint16_t blend565(uint16_t bg, uint16_t fg, uint32_t a) {
  uint32_t b = (bg | ((uint32_t)bg << 16)) & 0x07E0F81F;
  uint32_t f = (fg | ((uint32_t)fg << 16)) & 0x07E0F81F;
  uint32_t r = ((((f - b) * a) >> 5) + b) & 0x07E0F81F;
  return (uint16_t)(r | (r >> 16));
}
static inline void plot(int x, int y, uint16_t col, float a) {
  int ai = (int)(a * (float)gAlpha + 0.5f);
  if (ai <= 0) return;
  uint16_t* p = &fb[y * W + x];
  *p = ai >= 32 ? col : blend565(*p, col, ai);
}
void setAlpha(float a) { gAlpha = (uint32_t)(clampf(a, 0, 1) * 32.0f + 0.5f); }

// ============================================================================================
//  Primitivas con anti-aliasing
// ============================================================================================
void ellipseAA(float cx, float cy, float rx, float ry, uint16_t col) {
  if (rx < 0.4f || ry < 0.4f) return;
  int y0 = max(0, (int)floorf(cy - ry - 1)), y1 = min(H - 1, (int)ceilf(cy + ry + 1));
  int x0 = max(0, (int)floorf(cx - rx - 1)), x1 = min(W - 1, (int)ceilf(cx + rx + 1));
  markRows(y0, y1);
  float ix = 1.0f / (rx * rx), iy = 1.0f / (ry * ry), sc = 0.5f * sqrtf(rx * ry);
  for (int y = y0; y <= y1; y++) {
    float dy = y - cy, dy2 = dy * dy * iy;
    for (int x = x0; x <= x1; x++) {
      float dx = x - cx, f = dx * dx * ix + dy2 - 1.0f;
      float a = 0.5f - f * sc;
      if (a <= 0) continue;
      plot(x, y, col, a > 1 ? 1 : a);
    }
  }
}
void discAA(float cx, float cy, float r, uint16_t col) { ellipseAA(cx, cy, r, r, col); }

// segmento grueso con extremos redondeados
void lineAA(float x0, float y0, float x1, float y1, float th, uint16_t col) {
  float h = th * 0.5f, h2 = h * h, k = 1.0f / (2.0f * h);
  float vx = x1 - x0, vy = y1 - y0, l2 = vx * vx + vy * vy;
  float il2 = l2 > 0 ? 1.0f / l2 : 0.0f;
  int xa = max(0, (int)floorf(fminf(x0, x1) - h - 1)), xb = min(W - 1, (int)ceilf(fmaxf(x0, x1) + h + 1));
  int ya = max(0, (int)floorf(fminf(y0, y1) - h - 1)), yb = min(H - 1, (int)ceilf(fmaxf(y0, y1) + h + 1));
  markRows(ya, yb);
  for (int y = ya; y <= yb; y++) {
    float wy = y - y0;
    for (int x = xa; x <= xb; x++) {
      float wx = x - x0;
      float t = (wx * vx + wy * vy) * il2;
      t = t < 0 ? 0 : (t > 1 ? 1 : t);
      float dx = wx - t * vx, dy = wy - t * vy;
      float a = 0.5f - (dx * dx + dy * dy - h2) * k;
      if (a <= 0) continue;
      plot(x, y, col, a > 1 ? 1 : a);
    }
  }
}

// triangulo con borde suave; grow > 0 lo agranda (para que no se vean costuras entre triangulos)
void triAA(float x0, float y0, float x1, float y1, float x2, float y2, uint16_t col, float grow = 0) {
  float area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
  if (fabsf(area) < 1e-4f) return;
  if (area < 0) { float tx = x1, ty = y1; x1 = x2; y1 = y2; x2 = tx; y2 = ty; }
  float ex[3] = {x1 - x0, x2 - x1, x0 - x2}, ey[3] = {y1 - y0, y2 - y1, y0 - y2};
  float px[3] = {x0, x1, x2}, py[3] = {y0, y1, y2}, il[3];
  for (int i = 0; i < 3; i++) il[i] = 1.0f / sqrtf(ex[i] * ex[i] + ey[i] * ey[i] + 1e-6f);
  int xa = max(0, (int)floorf(fminf(x0, fminf(x1, x2)) - 2)), xb = min(W - 1, (int)ceilf(fmaxf(x0, fmaxf(x1, x2)) + 2));
  int ya = max(0, (int)floorf(fminf(y0, fminf(y1, y2)) - 2)), yb = min(H - 1, (int)ceilf(fmaxf(y0, fmaxf(y1, y2)) + 2));
  markRows(ya, yb);
  for (int y = ya; y <= yb; y++)
    for (int x = xa; x <= xb; x++) {
      float m = 1e9f;
      for (int i = 0; i < 3; i++) {
        float d = (ex[i] * (y - py[i]) - ey[i] * (x - px[i])) * il[i];
        if (d < m) m = d;
      }
      float a = 0.5f + grow + m;
      if (a <= 0) continue;
      plot(x, y, col, a > 1 ? 1 : a);
    }
}

void heartAA(float cx, float cy, float w, float h, uint16_t col) {
  float r = w * 0.25f, top = cy - h * 0.5f + r;
  discAA(cx - r, top, r, col);
  discAA(cx + r, top, r, col);
  triAA(cx - 2 * r + 0.6f, top + 0.4f, cx + 2 * r - 0.6f, top + 0.4f, cx, cy + h * 0.5f, col, 0.3f);
}

// relampago centrado en (cx,cy); s = escala (1 = 52 px de alto)
void boltAA(float cx, float cy, float s, uint16_t col) {
  static const float P[7][2] = {{6, -26}, {-14, 4}, {-3, 4}, {-8, 26}, {14, -6}, {3, -6}, {12, -26}};
  float x[7], y[7];
  for (int i = 0; i < 7; i++) { x[i] = cx + P[i][0] * s; y[i] = cy + P[i][1] * s; }
  triAA(x[0], y[0], x[1], y[1], x[2], y[2], col, 0.5f);
  triAA(x[0], y[0], x[2], y[2], x[5], y[5], col, 0.5f);
  triAA(x[0], y[0], x[5], y[5], x[6], y[6], col, 0.5f);
  triAA(x[2], y[2], x[3], y[3], x[4], y[4], col, 0.5f);
  triAA(x[2], y[2], x[4], y[4], x[5], y[5], col, 0.5f);
}

// una "Z" de sueno
void drawZ(float x, float y, float s, uint16_t col) {
  float th = fmaxf(2.0f, s * 0.32f);
  lineAA(x - s, y - s, x + s, y - s, th, col);
  lineAA(x + s, y - s, x - s, y + s, th, col);
  lineAA(x - s, y + s, x + s, y + s, th, col);
}

// arco (ojo cerrado). dir = +1 sonrisa (extremos arriba), -1 feliz (extremos abajo)
void arcAA(float cx, float cy, float halfW, float sag, float dir, float th, uint16_t col) {
  const int N = 16;
  float px = cx - halfW, py = cy - dir * sag * 0.5f;
  for (int i = 1; i <= N; i++) {
    float u = -1.0f + 2.0f * i / N;
    float nx = cx + u * halfW, ny = cy + dir * sag * (0.5f - u * u);   // extremos u=+-1 -> -dir*sag/2 ... centro +dir*sag/2
    lineAA(px, py, nx, ny, th, col);
    px = nx; py = ny;
  }
}

// ============================================================================================
//  Forma del ojo: elipse girada + "huevo" + parpados rectos + pupila, en UNA pasada con AA
// ============================================================================================
struct Shp { float cx, cy, ca, sa, irx2, iry, egg, hx, hy; };
Shp makeShp(float cx, float cy, float rx, float ry, float ang, float egg, float grow) {
  rx += grow; ry += grow;
  if (rx < 1) rx = 1;
  if (ry < 1) ry = 1;
  float ca = cosf(ang), sa = sinf(ang);
  float wx = rx * (1.0f + fabsf(egg));
  Shp s = {cx, cy, ca, sa, 1.0f / (rx * rx), 1.0f / ry, egg,
           sqrtf(wx * wx * ca * ca + ry * ry * sa * sa) + 1, sqrtf(wx * wx * sa * sa + ry * ry * ca * ca) + 1};
  return s;
}
inline bool inShpF(const Shp& s, float x, float y) {
  float dx = x - s.cx, dy = y - s.cy;
  float xr = dx * s.ca + dy * s.sa, yr = -dx * s.sa + dy * s.ca;
  float v = yr * s.iry, w = 1.0f + s.egg * v;
  return xr * xr * s.irx2 <= (1.0f - v * v) * w * w;
}

struct EyeDraw {
  float cx, cy, rx, ry, ang, egg;
  float lidT, lidB, slope, inner;     // parpados (fracciones de 2*ry), pendiente, +1 ojo izq / -1 der
  float pcx, pcy, prx, pry;           // pupila
  uint16_t cEye, cPup;
};

void renderEye(const EyeDraw& D) {
  float rx = D.rx, ry = D.ry;
  if (rx < 1 || ry < 1) return;
  Shp s = makeShp(D.cx, D.cy, rx, ry, D.ang, D.egg, 0.5f);
  float cx = D.cx, cy = D.cy;
  float botY = cy + ry - D.lidB * 2 * ry, topBase = cy - ry + D.lidT * 2 * ry, k = D.slope * ry * D.inner / rx;
  int y0 = max(0, (int)floorf(cy - s.hy)), y1 = min(H - 1, (int)ceilf(cy + s.hy));
  int x0 = max(0, (int)floorf(cx - s.hx)), x1 = min(W - 1, (int)ceilf(cx + s.hx));
  markRows(y0, y1);
  float sc = 0.5f * sqrtf(rx * ry), fe = 0.5f / sc;
  bool hasP = D.prx > 0.5f && D.pry > 0.5f;
  float ipx = 1.0f / (D.prx * D.prx + 1e-3f), ipy = 1.0f / (D.pry * D.pry + 1e-3f);
  float psc = 0.5f * sqrtf(D.prx * D.pry), pfe = 0.5f / (psc + 1e-3f);
  for (int y = y0; y <= y1; y++) {
    float dy = y - cy, dx = x0 - cx;
    float xr = dx * s.ca + dy * s.sa, yr = -dx * s.sa + dy * s.ca;
    float topY = topBase + k * dx;
    uint16_t* row = fb + y * W;
    float pdy = y - D.pcy, pdy2 = pdy * pdy * ipy;
    for (int x = x0; x <= x1; x++, xr += s.ca, yr -= s.sa, topY += k) {
      float v = yr * s.iry, w = 1.0f + s.egg * v;
      float f = xr * xr * s.irx2 - (1.0f - v * v) * w * w;
      if (f >= fe) continue;
      float a = f <= -fe ? 1.0f : 0.5f - f * sc;
      float lt = y - topY + 0.5f;
      if (lt <= 0) continue;
      if (lt < 1) a *= lt;
      float lb = botY - y + 0.5f;
      if (lb <= 0) continue;
      if (lb < 1) a *= lb;
      uint16_t col = D.cEye;
      if (hasP) {
        float pdx = x - D.pcx, fp = pdx * pdx * ipx + pdy2 - 1.0f;
        if (fp < pfe) col = fp <= -pfe ? D.cPup : blend565(D.cEye, D.cPup, (int)((0.5f - fp * psc) * 32.0f + 0.5f));
      }
      int ai = (int)(a * (float)gAlpha + 0.5f);
      if (ai <= 0) continue;
      row[x] = ai >= 32 ? col : blend565(row[x], col, ai);
    }
  }
}

// ============================================================================================
//  EXPRESIONES = poses. Cada fila es el objetivo de todos los parametros.
// ============================================================================================
enum : uint8_t {
  X_AWAKE = 1, X_CURIOUS, X_SLEEP, X_HAPPY, X_ANGRY, X_LOVE, X_CHARGING, X_ELECTRIC, X_DIMSLEEP, X_SAD,
  X_FULL, X_HYPE, X_LOWBAT, X_DIZZY, X_SURPRISE, X_SMUG, NEXPR
};
const char* const XNAME[NEXPR] = {"", "despierto", "curioso", "dormido", "feliz", "enojado", "amor", "cargando",
                                  "electrico", "dormido-tenue", "triste", "cara-llena", "hype", "bateria-baja",
                                  "mareado", "sorprendido", "orgulloso"};

// indices de parametros
enum { P_GAP, P_RX, P_RY, P_TILT, P_EGG, P_LIDT, P_LIDB, P_SLOPE, P_PRX, P_PRY, P_PDX, P_PDY, P_OPEN, P_ARCDIR,
       P_ARCW, P_TH, P_Y, P_ER, P_EG, P_EB, P_PR, P_PG, P_PB, NP };

// GAP  RX  RY  TILT  EGG  LIDT LIDB SLOPE PRX PRY PDX PDY OPEN ARCDIR ARCW TH   Y   ER  EG  EB   PR  PG  PB
const float POSES[NEXPR][NP] = {
  {0},
  // 1 DESPIERTO: ojos pelo-durazno altos y casi pegados, pupila verde estrecha abajo-adentro
  {40, 38, 60, 0.07f, 0.04f, 0, 0, 0,     5.5f, 17, 15, 11, 1, 1, 34, 3.5f, 0,   245, 208, 192,  46, 190, 70},
  // 2 CURIOSO: blancos, pupilas azules grandes
  {44, 42, 62, 0.05f, 0, 0, 0, 0,        15, 26, 10, 13, 1, 1, 34, 3.5f, -3,   250, 250, 250,  50, 140, 235},
  // 3 DORMIDO: dos arcos blancos tipo sonrisa
  {40, 38, 60, 0, 0, 0.5f, 0.5f, 0,       0, 0, 0, 0,   0, 1, 34, 3.6f, 22,   255, 255, 255,  255, 255, 255},
  // 4 FELIZ: arcos blancos hacia arriba
  {40, 38, 60, 0, 0, 0.5f, 0.5f, 0,       0, 0, 0, 0,   0, -1, 33, 4.2f, 6,   255, 255, 255,  255, 255, 255},
  // 5 ENOJADO: rojo, parpado inclinado, brillo blanco chico
  {42, 38, 56, -0.20f, 0, 0.26f, 0, 0.95f, 7, 9, 12, 12, 1, 1, 34, 3.5f, 4,   235, 55, 48,   255, 255, 255},
  // 6 AMOR: domos rosa (parpado inferior plano), sin pupila; los corazones son extras
  {43, 40, 58, 0.05f, 0, 0, 0.40f, 0,     0, 0, 0, 0,   1, 1, 34, 3.5f, 4,    245, 105, 215,  255, 60, 90},
  // 7 CARGANDO: amarillo con la barra cian cruzando (extra)
  {40, 36, 54, 0.0f, 0, 0, 0, 0,          0, 0, 0, 0,   1, 1, 34, 3.5f, 0,    245, 195, 20,   0, 215, 235},
  // 8 ELECTRICO: amarillo con rayos cian (extra)
  {42, 34, 48, 0.0f, 0, 0, 0, 0,          0, 0, 0, 0,   1, 1, 34, 3.5f, 0,    248, 190, 20,   0, 215, 235},
  // 9 DORMIDO TENUE: arcos naranjas con resplandor (extra)
  {40, 38, 60, 0, 0, 0.5f, 0.5f, 0,       0, 0, 0, 0,   0, 1, 34, 4.0f, 22,   235, 120, 30,  235, 120, 30},
  // 10 TRISTE: cuencos rojos (parpado superior plano), lagrimas turquesa (extra)
  {46, 34, 52, 0.0f, 0, 0.62f, 0, -0.30f, 0, 0, 0, 0,   1, 1, 34, 3.5f, 4,    220, 30, 45,    90, 190, 170},
  // 11 CARA LLENA: disco amarillo (extra) con pupilas verdes
  {42, 40, 60, 0.0f, 0, 0, 0, 0,          13, 17, 0, 14, 1, 1, 34, 3.5f, 6,   250, 205, 20,   50, 135, 55},
  // 12 HYPE: naranja con grandes pupilas blancas, domos
  {42, 38, 52, 0.22f, 0, 0, 0.18f, 0,     14, 14, 8, 10, 1, 1, 34, 3.5f, 0,   255, 120, 0,    255, 255, 255},
  // 13 BATERIA BAJA: amarillo chato con pupilitas cian
  {42, 36, 42, 0.0f, 0, 0.18f, 0, 0,      6.5f, 3.8f, 6, 6, 1, 1, 34, 3.5f, 8,   248, 195, 20,  0, 215, 235},
  // 14 MAREADO: espiral (extra)
  {42, 38, 56, 0.0f, 0, 0.06f, 0.04f, 0,  0, 0, 0, 0,   1, 1, 34, 3.5f, 0,    250, 200, 60,   40, 200, 220},
  // 15 SORPRENDIDO: ojos enormes, pupila minima
  {44, 42, 68, 0.0f, 0, 0, 0, 0,          3.5f, 6, 6, 8, 1, 1, 34, 3.5f, -2,  245, 208, 192,  46, 190, 70},
  // 16 ORGULLOSO: amarillo con parpado inclinado suave, pupila naranja grande
  {42, 38, 58, 0.0f, 0, 0.22f, 0, 0.50f,  13, 21, 12, 14, 1, 1, 34, 3.5f, 4,  252, 200, 50,   225, 120, 50},
};

// temas de color del estado DESPIERTO (ojo / pupila), como las distintas capturas
const uint8_t AWAKE_THEMES[5][2][3] = {
  {{245, 208, 192}, {46, 190, 70}},    // durazno / verde
  {{252, 200, 50}, {225, 120, 50}},    // amarillo / naranja
  {{215, 100, 60}, {140, 35, 40}},     // naranja quemado / vino
  {{250, 250, 250}, {50, 140, 235}},   // blanco / azul
  {{245, 90, 190}, {255, 190, 225}},   // rosa / rosa claro
};
const int NTHEMES = 5;

// ============================================================================================
//  Estado de la animacion
// ============================================================================================
float cur[NP], vel[NP], tgt[NP];
uint8_t expr = X_AWAKE;         // expresion pedida
uint32_t exprStart = 0;
int awakeTheme = 0;

// resortes de cuerpo por ojo (gelatina)
struct Eye {
  float ox, oy, ovx, ovy, sx, svx, sy, svy, px, py, pvx, pvy, blinkT, blinkTot;
};
Eye eye[2];
float gx = 0, gy = 0, tgx = 0, tgy = 0, pgx = 0, pgy = 0;
uint32_t gazeUntil = 0, nextWander = 0, nextBlink = 2500, nextIdleAct = 9000;
float shakeAmp = 0, hopT = 0;
float presence[8];              // fundido de extras
enum { XT_HEARTS, XT_TEARS, XT_BOLTS, XT_CHARGE, XT_ZZZ, XT_FULL, XT_SPIRAL, XT_GLOW };
float puOv[2] = {-1, -1};       // tamano de pupila forzado por ojo (<0 = automatico)

// --- secuencias de expresiones ---
struct Step { uint8_t x; uint16_t dur; };
Step seq[3];
uint8_t seqLen = 0, seqIdx = 0;
uint32_t stepEnd = 0;

// --- tamagotchi ---
bool usb = false, lowBat = false, deepSleep = false, fullBat = false;
float energy = 85, fun = 75, affection = 40;   // 0..100
float timeScale = 1;
uint32_t lastActivity = 0, lastTap = 0;
uint8_t tapCount = 0;
bool petting = false;
uint32_t petLast = 0;

uint8_t baseExpr() {
  if (usb) return X_CHARGING;
  if (energy < 18 || lowBat) return X_LOWBAT;
  if (fun < 18) return X_SAD;
  return X_AWAKE;
}

void startStep(uint32_t now) {
  uint8_t x = seq[seqIdx].x;
  expr = x ? x : baseExpr();
  exprStart = now;
  stepEnd = seq[seqIdx].dur ? now + seq[seqIdx].dur : 0;
}
void play(uint32_t now, uint8_t n, Step a, Step b = {0, 0}, Step c = {0, 0}) {
  seq[0] = a; seq[1] = b; seq[2] = c;
  seqLen = n; seqIdx = 0;
  startStep(now);
}

// ============================================================================================
//  Acciones / eventos (los dispara el firmware con los botones y el simulador con el teclado)
// ============================================================================================
void wake(uint32_t now) {
  lastActivity = now;
  if (deepSleep) { deepSleep = false; play(now, 2, {X_SURPRISE, 700}, {0, 0}); return; }
  if (expr == X_SLEEP || expr == X_DIMSLEEP) play(now, 2, {X_CURIOUS, 1100}, {0, 0});
}
bool isAsleep() { return deepSleep || expr == X_SLEEP || expr == X_DIMSLEEP; }

void evSet(uint8_t x, uint32_t now, uint16_t dur) {            // forzar una expresion un rato
  lastActivity = now; deepSleep = false;
  if (dur) play(now, 2, {x, dur}, {0, 0}); else play(now, 1, {x, 0});
}
void evTap(uint32_t now) {                                     // toque: 3 seguidos = amor
  bool slept = isAsleep();
  wake(now);
  if (slept) return;
  if (now - lastTap > 1800) tapCount = 0;
  lastTap = now;
  fun = clampf(fun + 2, 0, 100);
  for (int e = 0; e < 2; e++) { eye[e].sx = 0.10f; eye[e].sy = -0.12f; }   // un respingo
  if (++tapCount >= 3) {
    tapCount = 0; affection = clampf(affection + 10, 0, 100);
    play(now, 2, {X_LOVE, 3200}, {0, 0});
  }
}
void evPet(uint32_t now, bool on) {                            // mantener acariciando
  if (on) {
    if (isAsleep()) { wake(now); return; }
    lastActivity = now; petLast = now;
    if (!petting) { petting = true; play(now, 2, {X_LOVE, 1600}, {0, 0}); }
    else if (expr != X_LOVE) play(now, 2, {X_LOVE, 1600}, {0, 0});
    affection = clampf(affection + 8 * DT, 0, 100);
    fun = clampf(fun + 6 * DT, 0, 100);
  } else petting = false;
}
void evPlay(uint32_t now) {                                    // jugar
  if (isAsleep()) { wake(now); return; }
  lastActivity = now;
  fun = clampf(fun + 22, 0, 100);
  energy = clampf(energy - 3, 0, 100);
  play(now, 2, {X_HYPE, 2600}, {0, 0});
}
void evShake(uint32_t now, bool strong) {                      // sacudida suave = curioso, fuerte = enojado
  if (isAsleep()) { wake(now); return; }
  lastActivity = now;
  shakeAmp = strong ? 7.0f : 3.0f;
  if (strong) { fun = clampf(fun - 4, 0, 100); play(now, 3, {X_DIZZY, 1800}, {X_ANGRY, 1800}, {0, 0}); }
  else play(now, 2, {X_CURIOUS, 1800}, {0, 0});
}
void evUsb(uint32_t now, bool on) {
  usb = on; lastActivity = now; deepSleep = false; fullBat = false;
  if (on) play(now, 2, {X_ELECTRIC, 1800}, {0, 0});            // al enchufar: chispas, luego "cargando"
  else play(now, 2, {X_HAPPY, 900}, {0, 0});
}
void evBatFull(uint32_t now) {
  if (usb) { fullBat = true; energy = 100; play(now, 2, {X_DIMSLEEP, 1200}, {X_SLEEP, 0}); }
}
void evBatLow(uint32_t now, bool on) { lowBat = on; if (!usb) play(now, 1, {0, 0}); }
void evPoke(uint32_t now) {                                    // piquete: se aplastan
  wake(now);
  for (int e = 0; e < 2; e++) { eye[e].sx = 0.34f; eye[e].sy = -0.30f; eye[e].svx = eye[e].svy = 0; }
  if (expr == X_AWAKE) play(now, 2, {X_SURPRISE, 700}, {0, 0});
}
void evWink() { eye[1].blinkTot = eye[1].blinkT = 0.5f; }
void evTheme(uint32_t now) { wake(now); awakeTheme = (awakeTheme + 1) % NTHEMES; }
uint8_t cycleIdx = 0;
const uint8_t CYCLE[15] = {X_CURIOUS, X_HAPPY, X_ANGRY, X_LOVE, X_SAD, X_HYPE, X_LOWBAT, X_SURPRISE, X_SMUG,
                           X_DIZZY, X_ELECTRIC, X_CHARGING, X_DIMSLEEP, X_FULL, X_SLEEP};
void evNextExpr(uint32_t now) {                                // para probar: pasa por todas
  uint8_t x = CYCLE[cycleIdx];
  cycleIdx = (cycleIdx + 1) % 15;
  evSet(x, now, (x == X_SLEEP || x == X_DIMSLEEP || x == X_CHARGING) ? 4500 : 3200);
}
void setGaze(float x, float y, uint32_t now, uint32_t holdMs) {
  tgx = clampf(x, -1, 1); tgy = clampf(y, -1, 1);
  gazeUntil = now + holdMs;
}

// ============================================================================================
//  Animacion
// ============================================================================================
uint16_t cEye, cPup;

void setTargets() {
  const float* P = POSES[expr];
  for (int i = 0; i < NP; i++) tgt[i] = P[i];
  if (expr == X_AWAKE) {  // color segun el tema elegido
    for (int j = 0; j < 3; j++) { tgt[P_ER + j] = AWAKE_THEMES[awakeTheme][0][j]; tgt[P_PR + j] = AWAKE_THEMES[awakeTheme][1][j]; }
  }
}

void updateStats(uint32_t now) {
  float dt = DT * timeScale;
  bool sleeping = isAsleep();
  if (usb) energy = clampf(energy + 2.5f * dt, 0, 100);
  else energy = clampf(energy - (sleeping ? -0.35f : 0.07f) * dt, 0, 100);
  fun = clampf(fun - (sleeping ? 0.01f : 0.10f) * dt, 0, 100);
  affection = clampf(affection - 0.03f * dt, 0, 100);
  if (usb && energy >= 99.9f && !fullBat) evBatFull(now);
}

void updateAnimation(uint32_t now) {
  float t = now * 0.001f;
  // --- secuencias y estado base ---
  if (stepEnd && now >= stepEnd) {
    if (seqIdx + 1 < seqLen) { seqIdx++; startStep(now); }
    else stepEnd = 0;
  }
  bool seqDone = (stepEnd == 0);
  if (petting && now - petLast > 250) petting = false;
  uint32_t idle = now - lastActivity;
  if (seqDone && seq[seqIdx].x == 0) {
    uint8_t b = baseExpr();
    if (expr != b && expr != X_SLEEP && expr != X_DIMSLEEP) { expr = b; exprStart = now; }
    // dormirse solo tras 30 s quieto (o cuando se queda sin energia)
    if (!usb && (expr == X_AWAKE || expr == X_LOWBAT) && (idle > 30000 || energy < 6)) {
      play(now, 1, {X_SLEEP, 0});
    }
    // dormido profundo tras 5 min
    if (expr == X_SLEEP && idle > 300000) deepSleep = true;
    // a veces hace algo por su cuenta
    if (expr == X_AWAKE && now >= nextIdleAct) {
      nextIdleAct = now + 6000 + (uint32_t)(frand01() * 11000);
      float r = frand01();
      if (r < 0.28f) play(now, 2, {X_CURIOUS, 1500}, {0, 0});
      else if (r < 0.50f) play(now, 2, {X_HAPPY, 1300}, {0, 0});
      else if (r < 0.65f) play(now, 2, {X_SMUG, 2000}, {0, 0});
      else if (r < 0.77f) play(now, 2, {X_SURPRISE, 900}, {0, 0});
      else if (r < 0.87f) { evWink(); }
      else awakeTheme = (awakeTheme + 1) % NTHEMES;
    }
  }
  if (seq[seqIdx].x == X_SLEEP && seqLen == 1 && expr == X_SLEEP) {}  // se queda dormido hasta que lo toquen

  // --- objetivos ---
  setTargets();
  if (shakeAmp > 0.05f) shakeAmp *= powf(0.02f, DT);   // se calma en ~1 s
  // el pulso de HYPE / respiracion
  float pulse = 0.5f - 0.5f * cosf(TAU * fmodf(t, 0.6f) / 0.6f);
  if (expr == X_HYPE) { tgt[P_RX] *= 1.0f + 0.09f * pulse; tgt[P_RY] *= 1.0f + 0.09f * pulse; }
  if (expr == X_CURIOUS && now - exprStart < 300) tgt[P_Y] += -6.0f * sinf(3.14159f * (now - exprStart) / 300.0f);
  if (expr == X_SLEEP || expr == X_DIMSLEEP) tgt[P_Y] += 2.5f * sinf(TAU * t / 3.0f);    // respira
  if (expr == X_LOWBAT) tgt[P_Y] += 1.5f * sinf(TAU * t / 2.4f);

  // --- resortes de los parametros (morph entre expresiones) ---
  for (int i = 0; i < NP; i++) {
    bool color = (i >= P_ER);
    float k = color ? 150.0f : (i == P_OPEN ? 220.0f : 190.0f);
    float c = color ? 24.0f : (i == P_OPEN ? 26.0f : 19.0f);    // algo subamortiguado: "rebota" un poco
    float a = k * (tgt[i] - cur[i]) - c * vel[i];
    vel[i] += a * DT;
    cur[i] += vel[i] * DT;
  }
  for (int j = 0; j < 3; j++) {
    cur[P_ER + j] = clampf(cur[P_ER + j], 0, 255);
    cur[P_PR + j] = clampf(cur[P_PR + j], 0, 255);
  }
  cEye = rgb565((uint8_t)cur[P_ER], (uint8_t)cur[P_EG], (uint8_t)cur[P_EB]);
  cPup = rgb565((uint8_t)cur[P_PR], (uint8_t)cur[P_PG], (uint8_t)cur[P_PB]);

  // --- fundido de extras ---
  float tp[8] = {0};
  tp[XT_HEARTS] = expr == X_LOVE;
  tp[XT_TEARS] = expr == X_SAD;
  tp[XT_BOLTS] = expr == X_ELECTRIC;
  tp[XT_CHARGE] = expr == X_CHARGING;
  tp[XT_ZZZ] = 0;   // en las capturas de referencia el dormido NO lleva Zzz
  tp[XT_FULL] = expr == X_FULL;
  tp[XT_SPIRAL] = expr == X_DIZZY;
  tp[XT_GLOW] = expr == X_DIMSLEEP;
  for (int i = 0; i < 8; i++) {
    float rate = (i == XT_FULL) ? 5.0f : 9.0f;
    presence[i] += (tp[i] - presence[i]) * (1.0f - expf(-rate * DT));
  }

  // --- mirada ---
  if (now >= nextWander && now >= gazeUntil) {
    nextWander = now + 900 + (uint32_t)(frand01() * 2600);
    if (frand01() < 0.3f) { tgx = 0; tgy = 0; } else { tgx = frand() * 0.8f; tgy = frand() * 0.6f; }
  }
  float kg = 1.0f - expf(-18.0f * DT);
  gx += (tgx - gx) * kg; gy += (tgy - gy) * kg;
  float dgx = (gx - pgx) / DT, dgy = (gy - pgy) / DT;
  pgx = gx; pgy = gy;

  // --- parpadeo ---
  bool canBlink = cur[P_OPEN] > 0.9f && expr != X_DIZZY;
  if (canBlink && now >= nextBlink) {
    nextBlink = now + 2400 + (uint32_t)(frand01() * 3200);
    if (expr == X_LOWBAT) nextBlink = now + 1300 + (uint32_t)(frand01() * 900);
    for (int e = 0; e < 2; e++) eye[e].blinkTot = eye[e].blinkT = 0.17f;
    if (frand() > 0.65f) nextBlink = now + 360;      // doble parpadeo
  }

  // --- gelatina: cuerpo, estiramiento y pupilas por ojo ---
  float shake = shakeAmp * sinf(TAU * 14.0f * t);
  for (int e = 0; e < 2; e++) {
    Eye& E = eye[e];
    if (E.blinkT > 0) E.blinkT -= DT;
    float ph = e * 3.14159f, inner = e ? -1.0f : 1.0f;
    float k = e ? 120.0f : 140.0f;
    float tx = gx * 9 + shake, ty = gy * 7;
    if (expr == X_DIZZY) { float a = TAU * 1.2f * t + ph; tx += 12 * cosf(a); ty += 9 * sinf(a); }
    if (expr == X_HYPE) ty += 4 * sinf(TAU * 3.0f * t + ph);
    if (expr == X_ANGRY) { tx += 1.3f * frand(); ty += 1.0f * frand(); }
    float pvx0 = E.ovx, pvy0 = E.ovy;
    E.ovx += (k * (tx - E.ox) - 8 * E.ovx) * DT;
    E.ovy += (k * (ty - E.oy) - 8 * E.ovy) * DT;
    E.ox += E.ovx * DT; E.oy += E.ovy * DT;
    E.ox = clampf(E.ox, -20, 20); E.oy = clampf(E.oy, -20, 20);
    float ax = (E.ovx - pvx0) / DT, ay = (E.ovy - pvy0) / DT;
    float sxt = 0.0009f * (fabsf(E.ovx + dgx * 90) - 0.5f * fabsf(E.ovy + dgy * 90));
    float syt = 0.0009f * (fabsf(E.ovy + dgy * 90) - 0.5f * fabsf(E.ovx + dgx * 90));
    if (expr == X_DIZZY) { sxt += 0.10f * sinf(TAU * 1.5f * t + ph); syt -= 0.10f * sinf(TAU * 1.5f * t + ph); }
    sxt = clampf(sxt, -0.2f, 0.25f); syt = clampf(syt, -0.2f, 0.25f);
    E.svx += (210 * (sxt - E.sx) - 9 * E.svx) * DT; E.sx += E.svx * DT;
    E.svy += (210 * (syt - E.sy) - 9 * E.svy) * DT; E.sy += E.svy * DT;
    E.sx = clampf(E.sx, -0.4f, 0.45f); E.sy = clampf(E.sy, -0.4f, 0.45f);

    // pupila: posicion base abajo-adentro + mirada
    float prx = cur[P_PRX], pry = cur[P_PRY];
    if (puOv[e] >= 0) { prx *= puOv[e]; pry *= puOv[e]; }
    float lx = fmaxf(1.0f, cur[P_RX] - prx - 2), ly = fmaxf(1.0f, cur[P_RY] - pry - 2);
    float ptx = inner * cur[P_PDX] + gx * 0.55f * lx, pty = cur[P_PDY] + gy * 0.45f * ly;
    E.pvx += (230 * (ptx - E.px) - 16 * E.pvx - 0.30f * ax) * DT;
    E.pvy += (230 * (pty - E.py) - 16 * E.pvy - 0.30f * ay) * DT;
    E.px += E.pvx * DT; E.py += E.pvy * DT;
    float nd = (E.px * E.px) / (lx * lx) + (E.py * E.py) / (ly * ly);
    if (nd > 1.0f) { float s = 1.0f / sqrtf(nd); E.px *= s; E.py *= s; }
  }
}

// ============================================================================================
//  Dibujo
// ============================================================================================
void renderFrame(uint32_t now) {
  float t = now * 0.001f;
  uint16_t cB = rgb565(0, 215, 235), cRed = rgb565(255, 55, 75);

  // fondo: disco amarillo de CARA LLENA (crece desde el centro)
  if (presence[XT_FULL] > 0.02f) {
    float r = 170.0f * clampf(presence[XT_FULL], 0, 1);
    uint16_t cy = rgb565((uint8_t)cur[P_ER], (uint8_t)cur[P_EG], (uint8_t)cur[P_EB]);
    if (r > 150) { for (int i = 0; i < W * H; i++) fb[i] = cy; markRows(0, H - 1); }
    else discAA(120, 120, r, cy);
  }

  float open = clampf(cur[P_OPEN], 0, 1);
  for (int e = 0; e < 2; e++) {
    const Eye& E = eye[e];
    float side = e ? 1.0f : -1.0f, inner = e ? -1.0f : 1.0f;
    float blink = 0;
    if (E.blinkT > 0) { float u = 1.0f - E.blinkT / E.blinkTot; blink = u < 0.5f ? u * 2 : (1 - u) * 2; }
    float cx = 120 + side * cur[P_GAP] + E.ox;
    float cy = 124 + cur[P_Y] + E.oy;

    // ojo abierto (se cierra con el parpadeo y al pasar a arcos)
    if (open > 0.04f) {
      EyeDraw D;
      D.cx = cx; D.cy = cy;
      D.rx = clampf(cur[P_RX] * (1 + E.sx), 8, 70);
      D.ry = clampf(cur[P_RY] * (1 + E.sy), 8, 90);
      D.ang = side * cur[P_TILT] + clampf(E.ovx * 0.0012f, -0.2f, 0.2f);
      D.egg = cur[P_EGG] * side;
      float closing = 1.0f - open;
      float lt = clampf(cur[P_LIDT], 0, 0.85f), lb = clampf(cur[P_LIDB], 0, 0.85f);
      D.lidT = lt + (0.5f - lt) * fmaxf(blink, closing);
      D.lidB = lb + (0.5f - lb) * fmaxf(blink, closing);
      D.slope = cur[P_SLOPE]; D.inner = inner;
      D.pcx = cx + E.px; D.pcy = cy + E.py;
      float prx = cur[P_PRX], pry = cur[P_PRY];
      if (puOv[e] >= 0) { prx *= puOv[e]; pry *= puOv[e]; }
      D.prx = prx * (1 - blink * 0.3f); D.pry = pry * (1 - blink * 0.3f);
      D.cEye = cEye; D.cPup = cPup;
      setAlpha(clampf((open - 0.04f) * 8.0f, 0, 1));
      renderEye(D);
      setAlpha(1);

      // corazon / espiral / rayo dentro del ojo
      if (presence[XT_SPIRAL] > 0.05f) {
        Shp sh = makeShp(cx, cy, D.rx, D.ry, D.ang, D.egg, 0);
        float spin = -TAU * 2.0f * t, rr = fminf(D.rx, D.ry) * 0.85f;
        setAlpha(presence[XT_SPIRAL]);
        for (int i = 0; i < 44; i++) {
          float a = spin + i * 0.52f, r = rr * i / 44.0f;
          float px = cx + r * cosf(a), py = cy + r * sinf(a);
          if (inShpF(sh, px, py)) discAA(px, py, 2.6f, cPup);
        }
        setAlpha(1);
      }
      if (presence[XT_BOLTS] > 0.05f) {
        float fl = 0.90f + 0.10f * sinf(TAU * 5.0f * t + e);
        setAlpha(presence[XT_BOLTS] * fl);
        boltAA(cx, cy, fminf(D.rx, D.ry) / 25.0f * 1.12f, cB);
        setAlpha(1);
      }
    }

    // ojo cerrado = arco
    if (open < 0.4f) {
      float a = clampf((0.4f - open) / 0.3f, 0, 1);
      float dir = cur[P_ARCDIR];
      float ay = cy + (dir > 0 ? 12 : -2) * 0.0f;
      setAlpha(a);
      if (presence[XT_GLOW] > 0.05f) {   // resplandor suave
        float g = presence[XT_GLOW] * a * (0.8f + 0.2f * sinf(TAU * t / 2.0f));
        float th0 = cur[P_TH];
        for (int k = 3; k >= 1; k--) { setAlpha(g * 0.13f); arcAA(cx, ay, cur[P_ARCW], 13.0f * dir, dir, th0 + 5.0f * k, cEye); }
        setAlpha(a * (0.55f + 0.2f * g));
      }
      arcAA(cx, ay, cur[P_ARCW], 13.0f * dir, dir, cur[P_TH], cEye);
      setAlpha(1);
    }
  }

  // barra cian de CARGANDO: elipse horizontal cruzando los dos ojos
  if (presence[XT_CHARGE] > 0.03f) {
    float pl = 0.5f - 0.5f * cosf(TAU * fmodf(t, 0.9f) / 0.9f);
    setAlpha(presence[XT_CHARGE] * (0.78f + 0.22f * pl));
    ellipseAA(120, 124 + cur[P_Y], 38 + 3 * pl, 10 + 2 * pl, cB);
    setAlpha(1);
  }
  // corazoncitos rojos subiendo (AMOR)
  if (presence[XT_HEARTS] > 0.03f) {
    static const float HX[6] = {-62, -38, -10, 22, 50, 70}, HS[6] = {10, 13, 9, 12, 10, 8}, HP[6] = {0.0f, 0.33f, 0.66f, 0.15f, 0.5f, 0.82f};
    for (int i = 0; i < 6; i++) {
      float u = fmodf(t * 0.45f + HP[i], 1.0f);
      setAlpha(presence[XT_HEARTS] * sinf(3.14159f * clampf(u * 1.0f, 0, 1)));
      heartAA(120 + HX[i] + 5 * sinf(TAU * u * 1.5f + i), 112 - u * 62, HS[i] + 2, HS[i], cRed);
    }
    setAlpha(1);
  }
  // lagrimas (TRISTE)
  if (presence[XT_TEARS] > 0.03f) {
    uint16_t tc = rgb565(55, 185, 165);
    for (int e = 0; e < 2; e++) {
      float u = fmodf(t * 0.55f + e * 0.5f, 1.0f);
      float tx = 120 + (e ? 1 : -1) * (cur[P_GAP] + 6) + (e ? 1 : -1) * 10, ty = 66 + u * 78;
      setAlpha(presence[XT_TEARS] * (u < 0.7f ? 1.0f : (1.0f - u) / 0.3f));
      float s = 0.8f + 0.5f * u;
      discAA(tx, ty, 6.0f * s, tc);
      triAA(tx - 5.0f * s, ty - 2.0f, tx + 5.0f * s, ty - 2.0f, tx, ty - 17 * s, tc, 0.2f);
      setAlpha(1);
    }
  }
  // Zzz (DORMIDO)
  if (presence[XT_ZZZ] > 0.03f) {
    uint16_t cw = rgb565(255, 255, 255);
    for (int i = 0; i < 3; i++) {
      float u = fmodf(t * 0.3f + i * 0.33f, 1.0f);
      setAlpha(presence[XT_ZZZ] * sinf(3.14159f * u) * 0.9f);
      drawZ(152 + u * 26 + i * 3, 86 - u * 46, 4.0f + u * 7.0f, cw);
    }
    setAlpha(1);
  }
  // estrellitas orbitando (MAREADO)
  if (presence[XT_SPIRAL] > 0.05f) {
    setAlpha(presence[XT_SPIRAL]);
    for (int i = 0; i < 4; i++) {
      float a = TAU * 0.9f * t + i * 1.5708f;
      discAA(120 + 62 * cosf(a), 38 + 10 * sinf(a), 3.4f, cEye);
    }
    setAlpha(1);
  }
}

// ============================================================================================
//  API del core
// ============================================================================================
void coreInit(uint32_t now, uint32_t seed) {
  rng ^= seed;
  expr = X_AWAKE;
  setTargets();
  for (int i = 0; i < NP; i++) { cur[i] = tgt[i]; vel[i] = 0; }
  cur[P_OPEN] = 0; tgt[P_OPEN] = 1;                 // al encender "abre los ojos"
  for (int e = 0; e < 2; e++) eye[e] = Eye();
  lastActivity = now;
  play(now, 3, {X_HYPE, 1700}, {X_CURIOUS, 1000}, {0, 0});   // secuencia de encendido (como el spec)
}

// avanza la animacion y dibuja un cuadro completo en fb (que ya debe apuntar a un buffer valido)
void coreFrame(uint32_t now, float dt) {
  DT = clampf(dt, 0.005f, 0.04f);
  updateStats(now);
  updateAnimation(now);
  memset(fb, 0, (size_t)W * H * 2);
  dirtyY0 = H; dirtyY1 = -1;
  if (!deepSleep) renderFrame(now); else markRows(0, 0);
}
