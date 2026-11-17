// EyesCore.h - ojos de gelatina Lil Guy: animacion + render (RGB565). SIN dependencias de hardware.
// Lo usan el firmware (Alloy_Eyes.ino) y el simulador del PC (sim/eyes_sim.cpp): mismo codigo en los dos.
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

enum : uint8_t { M_NORMAL = 1, M_DIZZY, M_SURPRISED, M_SLEEPY, M_GOOGLY, M_PARTY, M_ANGRY, M_LOVE, M_SAD };
const uint8_t NMOODS = 9;

// parametros de cada animo: tamano, pupila, parpado sup., parpado inf., inclinacion, hundido
struct Mood { float size, pupil, lidT, lidB, slope, sink; };
const Mood MOODS[10] = {
  {1, 1, 0, 0, 0, 0},              // 0 (sin usar)
  {1, 1, 0, 0, 0, 0},              // NORMAL
  {1, 1, 0.08f, 0.04f, 0, 0},      // DIZZY
  {1.14f, 0.55f, 0, 0, 0, 0},      // SURPRISED
  {1, 0.9f, 0.5f, 0.05f, 0, 12},   // SLEEPY
  {1, 1, 0, 0, 0, 0},              // GOOGLY
  {1, 1, 0, 0, 0, 0},              // PARTY
  {1, 0.85f, 0.28f, 0, 0.85f, 0},  // ANGRY
  {1, 1, 0, 0, 0, 0},              // LOVE
  {1, 0.9f, 0.14f, 0.05f, -0.6f, 8},  // SAD
};
// tema que fuerza cada animo (-1 = el tema elegido)
const int8_t MOOD_THEME[10] = {-1, -1, -1, -1, 1, -1, -1, 8, 7, 9};

// temas: color del ojo y color de la pupila
const uint8_t THEMES[10][2][3] = {
  {{250, 205, 200}, {30, 150, 100}},   // 0 rosa / verde
  {{250, 250, 250}, {60, 130, 240}},   // 1 blanco / azul
  {{250, 200, 60}, {210, 110, 50}},    // 2 amarillo / naranja
  {{240, 110, 30}, {150, 40, 20}},     // 3 naranja / rojo oscuro
  {{250, 195, 50}, {40, 200, 220}},    // 4 amarillo / cian
  {{240, 50, 170}, {255, 170, 210}},   // 5 magenta / rosa claro
  {{240, 220, 40}, {60, 170, 60}},     // 6 limon / verde
  {{245, 80, 110}, {120, 20, 50}},     // 7 rojo rosado / vino (amor)
  {{235, 40, 35}, {70, 0, 0}},         // 8 rojo / negro rojizo (enojo)
  {{80, 130, 235}, {20, 40, 120}},     // 9 azul / azul marino (triste)
};
const int NTHEMES = 7;  // los del ciclo normal

struct Eye {
  float ox, oy, ovx, ovy;     // desplazamiento del cuerpo (resorte)
  float sx, svx, sy, svy;     // estiramiento horizontal / vertical (resorte)
  float px, py, pvx, pvy;     // posicion de la pupila dentro del ojo
  float blinkT, blinkTot;     // parpadeo (segundos restantes)
  float rx, ry, ix, iy, prx, pry;
  float ang, egg, scl, pscl, fx, fy, ax, ay;
  float tang, tegg, tscl, tpscl, tfx, tfy, tax, tay;
  float cx, cy, ca, sa;
};
struct Shp { float cx, cy, ca, sa, irx2, iry, egg, hx, hy; };
Eye eye[2];
const float EXC[2] = {80, 160};
const float EYC = 122;
const float BASE_RX = 41, BASE_RY = 53;
float faceX = 0, faceY = 0;
uint32_t formNext = 1500;
uint8_t formMode = 0;

uint8_t mood = M_NORMAL;
uint32_t moodUntil = 0;
float cur[6];
int baseTheme = 0;
float colr[2][3];
float gx = 0, gy = 0, tgx = 0, tgy = 0, pgx = 0, pgy = 0;
uint32_t gazeUntil = 0;           // mirada fijada desde fuera (mouse en el simulador)
float puOv[2] = {-1, -1};         // tamano de pupila forzado por ojo (<0 = automatico)
float act = 0;
uint32_t nextWander = 0, nextBlink = 2000, nextTheme = 25000, lastActivity = 0;
uint32_t rng = 12345;
float DT = 0.0167f, DTR = 0.667f;   // dt real y su razon contra 25 ms (para las constantes de suavizado)
int dirtyY0 = H, dirtyY1 = -1;

float frand() { rng = rng * 1664525u + 1013904223u; return ((rng >> 8) & 0xFFFF) / 65535.0f * 2.0f - 1.0f; }
float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
// factor de suavizado k (pensado para 25 ms) convertido al dt real
float lk(float k) { return 1.0f - powf(1.0f - k, DTR); }

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
// mezcla fg sobre bg con alfa 0..32
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

// ---------- Primitivas con anti-aliasing ----------
// forma irregular del ojo: elipse girada y deformada tipo huevo
Shp makeShp(const Eye& E, float grow) {
  float rx = E.rx + grow, ry = E.ry + grow;
  if (rx < 1) rx = 1;
  if (ry < 1) ry = 1;
  float wx = rx * (1.0f + fabsf(E.egg));
  Shp s = {E.cx, E.cy, E.ca, E.sa, 1.0f / (rx * rx), 1.0f / ry, E.egg,
           sqrtf(wx * wx * E.ca * E.ca + ry * ry * E.sa * E.sa) + 1,
           sqrtf(wx * wx * E.sa * E.sa + ry * ry * E.ca * E.ca) + 1};
  return s;
}
inline bool inShpF(const Shp& s, float x, float y) {
  float dx = x - s.cx, dy = y - s.cy;
  float xr = dx * s.ca + dy * s.sa, yr = -dx * s.sa + dy * s.ca;
  float v = yr * s.iry, w = 1.0f + s.egg * v;
  return xr * xr * s.irx2 <= (1.0f - v * v) * w * w;
}

void discAA(float cx, float cy, float r, uint16_t col, const Shp* clip = nullptr) {
  if (r < 0.3f) return;
  int y0 = max(0, (int)floorf(cy - r - 1)), y1 = min(H - 1, (int)ceilf(cy + r + 1));
  int x0 = max(0, (int)floorf(cx - r - 1)), x1 = min(W - 1, (int)ceilf(cx + r + 1));
  markRows(y0, y1);
  float r2 = r * r, k = 1.0f / (2.0f * r);
  for (int y = y0; y <= y1; y++) {
    float dy = y - cy;
    for (int x = x0; x <= x1; x++) {
      float dx = x - cx, f = dx * dx + dy * dy - r2;
      float a = 0.5f - f * k;
      if (a <= 0) continue;
      if (clip && !inShpF(*clip, x, y)) continue;
      plot(x, y, col, a > 1 ? 1 : a);
    }
  }
}

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

void triAA(float x0, float y0, float x1, float y1, float x2, float y2, uint16_t col) {
  float area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
  if (fabsf(area) < 1e-4f) return;
  if (area < 0) { float tx = x1, ty = y1; x1 = x2; y1 = y2; x2 = tx; y2 = ty; }
  float ex[3] = {x1 - x0, x2 - x1, x0 - x2}, ey[3] = {y1 - y0, y2 - y1, y0 - y2};
  float px[3] = {x0, x1, x2}, py[3] = {y0, y1, y2}, il[3];
  for (int i = 0; i < 3; i++) il[i] = 1.0f / sqrtf(ex[i] * ex[i] + ey[i] * ey[i] + 1e-6f);
  int xa = max(0, (int)floorf(fminf(x0, fminf(x1, x2)) - 1)), xb = min(W - 1, (int)ceilf(fmaxf(x0, fmaxf(x1, x2)) + 1));
  int ya = max(0, (int)floorf(fminf(y0, fminf(y1, y2)) - 1)), yb = min(H - 1, (int)ceilf(fmaxf(y0, fmaxf(y1, y2)) + 1));
  markRows(ya, yb);
  for (int y = ya; y <= yb; y++)
    for (int x = xa; x <= xb; x++) {
      float m = 1e9f;
      for (int i = 0; i < 3; i++) {
        float d = (ex[i] * (y - py[i]) - ey[i] * (x - px[i])) * il[i];
        if (d < m) m = d;
      }
      float a = 0.5f + m;
      if (a <= 0) continue;
      plot(x, y, col, a > 1 ? 1 : a);
    }
}

void heartAA(float cx, float cy, float w, float h, uint16_t col) {
  float r = w * 0.25f, top = cy - h * 0.5f + r;
  discAA(cx - r, top, r, col);
  discAA(cx + r, top, r, col);
  triAA(cx - 2 * r + 0.6f, top + 0.4f, cx + 2 * r - 0.6f, top + 0.4f, cx, cy + h * 0.5f, col);
}

// una "Z" de sueno
void drawZ(float x, float y, float s, uint16_t col) {
  float th = fmaxf(2.0f, s * 0.32f);
  lineAA(x - s, y - s, x + s, y - s, th, col);
  lineAA(x + s, y - s, x - s, y + s, th, col);
  lineAA(x - s, y + s, x + s, y + s, th, col);
}

// ---------- Ojo: forma + pupila + parpados en una sola pasada ----------
void renderEye(const Eye& E, float ft, float fbt, float slope, float inner, uint16_t cEye, uint16_t cPup,
               bool drawPupil) {
  Shp s = makeShp(E, 0.5f);
  float cx = E.cx, cy = E.cy, rx = E.rx, ry = E.ry;
  float botY = cy + ry - fbt * 2 * ry, topBase = cy - ry + ft * 2 * ry, k = slope * ry * inner / rx;
  int y0 = max(0, (int)floorf(cy - s.hy)), y1 = min(H - 1, (int)ceilf(cy + s.hy));
  int x0 = max(0, (int)floorf(cx - s.hx)), x1 = min(W - 1, (int)ceilf(cx + s.hx));
  markRows(y0, y1);
  float sc = 0.5f * sqrtf(rx * ry), fe = 0.5f / sc;
  float prx = drawPupil ? E.prx : 0, pry = drawPupil ? E.pry : 0;
  bool hasP = prx > 0.5f && pry > 0.5f;
  float ipx = 1.0f / (prx * prx + 1e-3f), ipy = 1.0f / (pry * pry + 1e-3f);
  float psc = 0.5f * sqrtf(prx * pry), pfe = 0.5f / (psc + 1e-3f);
  for (int y = y0; y <= y1; y++) {
    float dy = y - cy, dx = x0 - cx;
    float xr = dx * s.ca + dy * s.sa, yr = -dx * s.sa + dy * s.ca;
    float topY = topBase + k * dx;
    uint16_t* row = fb + y * W;
    float pdy = y - E.iy, pdy2 = pdy * pdy * ipy;
    for (int x = x0; x <= x1; x++, xr += s.ca, yr -= s.sa, topY += k) {
      float v = yr * s.iry, w = 1.0f + s.egg * v;
      float f = xr * xr * s.irx2 - (1.0f - v * v) * w * w;
      if (f >= fe) continue;
      float a = f <= -fe ? 1.0f : 0.5f - f * sc;
      float lt = y - topY + 0.5f;  // parpado superior
      if (lt <= 0) continue;
      if (lt < 1) a *= lt;
      float lb = botY - y + 0.5f;  // parpado inferior
      if (lb <= 0) continue;
      if (lb < 1) a *= lb;
      uint16_t col = cEye;
      if (hasP) {
        float pdx = x - E.ix, fp = pdx * pdx * ipx + pdy2 - 1.0f;
        if (fp < pfe) col = fp <= -pfe ? cPup : blend565(cEye, cPup, (int)((0.5f - fp * psc) * 32.0f + 0.5f));
      }
      int ai = (int)(a * 32.0f + 0.5f);
      if (ai <= 0) continue;
      row[x] = ai >= 32 ? col : blend565(row[x], col, ai);
    }
  }
}

// ---------- Animacion ----------
float blinkValue(const Eye& e) {
  if (e.blinkT <= 0) return 0;
  float u = 1.0f - e.blinkT / e.blinkTot;
  return u < 0.5f ? u * 2 : (1 - u) * 2;
}

void setMood(uint8_t m, uint32_t now, uint32_t dur) {
  mood = m;
  moodUntil = dur ? now + dur : 0;
  if (m == M_SURPRISED) for (int e = 0; e < 2; e++) { eye[e].sx = eye[e].sy = 0.25f; eye[e].svx = eye[e].svy = 0; }
  if (m == M_GOOGLY) for (int e = 0; e < 2; e++) { eye[e].pvx = frand() * 200; eye[e].pvy = frand() * 200; }
}

void poke(uint32_t now) {
  for (int e = 0; e < 2; e++) { eye[e].sx = 0.34f; eye[e].sy = -0.30f; eye[e].svx = eye[e].svy = 0; }
  baseTheme = (baseTheme + 1) % NTHEMES;
  nextTheme = now + 25000;
}

void chooseForm(uint32_t now) {
  formNext = now + 2500 + (uint32_t)(fabsf(frand()) * 3500);
  float r = (frand() + 1.0f) * 0.5f;
  uint8_t m = r < 0.30f ? 0 : r < 0.55f ? 1 : r < 0.67f ? 2 : r < 0.79f ? 3 : r < 0.90f ? 4 : 5;
  if (m == formMode && m != 1) m = 1;
  formMode = m;
  faceX = faceY = 0;
  int big = frand() > 0 ? 1 : 0;
  if (m == 1 || m == 4) { faceX = frand() * 14; faceY = frand() * 9; }
  for (int e = 0; e < 2; e++) {
    Eye& E = eye[e];
    E.tang = E.tegg = 0; E.tscl = E.tpscl = 1; E.tfx = E.tfy = 0; E.tax = E.tay = 1;
    switch (m) {
      case 1:
        E.tang = frand() * 0.34f; E.tegg = frand() * 0.26f; E.tscl = 1.0f + frand() * 0.16f;
        E.tpscl = 1.0f + frand() * 0.3f; E.tfx = faceX + frand() * 6; E.tfy = faceY + frand() * 10;
        E.tax = 1.0f + frand() * 0.12f; E.tay = 1.0f + frand() * 0.1f;
        break;
      case 2: E.tax = 0.82f; E.tay = 1.12f; E.tang = e ? -0.12f : 0.12f; break;
      case 3: E.tax = 1.12f; E.tay = 0.80f; E.tpscl = 1.2f; break;
      case 4:
        E.tscl = (e == big) ? 1.2f : 0.80f; E.tfx = faceX; E.tfy = faceY + frand() * 6;
        E.tpscl = (e == big) ? 1.1f : 0.9f;
        break;
      case 5: E.tang = e ? -0.28f : 0.28f; E.tegg = e ? -0.2f : 0.2f; break;
      default: break;
    }
  }
  // tamanos de pupila: iguales grandes, iguales chicas, o uno grande y otro chico
  float pr = (frand() + 1.0f) * 0.5f;
  if (pr >= 0.30f) {
    int bigp = frand() > 0 ? 1 : 0;
    for (int e = 0; e < 2; e++) {
      float v = pr < 0.55f ? 1.45f + 0.15f * frand()
              : pr < 0.70f ? 0.62f + 0.08f * frand()
                           : (e == bigp ? 1.7f : 0.55f);
      eye[e].tpscl = v;
    }
  }
}

float dizzyMeter = 0;

void updateAnimation(uint32_t now) {
  float t = now * 0.001f;
  if (moodUntil && now >= moodUntil) {
    setMood(M_NORMAL, now, 0);
    for (int e = 0; e < 2; e++) eye[e].blinkTot = eye[e].blinkT = 0.35f;
  }
  if (mood == M_NORMAL && now - lastActivity > 60000UL) setMood(M_SLEEPY, now, 0);  // se duerme solo
  const float* tp = &MOODS[mood].size;
  float kc = lk(0.12f);
  for (int i = 0; i < 6; i++) cur[i] += (tp[i] - cur[i]) * kc;

  if (now >= formNext && mood != M_SLEEPY) chooseForm(now);
  if (now >= nextTheme) { baseTheme = (baseTheme + 1) % NTHEMES; nextTheme = now + 25000; }

  // la mirada vaga sola
  if (mood == M_NORMAL && now >= nextWander && now >= gazeUntil) {
    nextWander = now + 1200 + (uint32_t)(fabsf(frand()) * 2500);
    tgx = frand() * 0.7f; tgy = frand() * 0.5f;
  }
  float kg = lk(0.45f);
  gx += (tgx - gx) * kg; gy += (tgy - gy) * kg;
  float dgx = (gx - pgx) / DT, dgy = (gy - pgy) / DT;
  pgx = gx; pgy = gy;
  // agitar la mirada muy rapido -> mareo
  act += (sqrtf(dgx * dgx + dgy * dgy) - act) * lk(0.15f);
  dizzyMeter += fmaxf(0, act - 1.2f) * DT * 0.9f;
  dizzyMeter = fmaxf(0, dizzyMeter - 0.25f * DT);
  if (dizzyMeter >= 1.0f && mood == M_NORMAL) { setMood(M_DIZZY, now, 3500); dizzyMeter = 0; }

  if (now >= nextBlink && mood != M_DIZZY) {
    nextBlink = now + 2200 + (uint32_t)(fabsf(frand()) * 3200);
    for (int e = 0; e < 2; e++) eye[e].blinkTot = eye[e].blinkT = 0.18f;
    if (frand() > 0.7f) nextBlink = now + 380;
  }

  float size = cur[0];
  if (mood == M_PARTY) size *= 1.0f + 0.07f * sinf(TAU * 3.0f * t);
  float sinkY = cur[5];
  float kf = lk(0.06f), kg5 = lk(0.05f);

  for (int e = 0; e < 2; e++) {
    Eye& E = eye[e];
    if (E.blinkT > 0) E.blinkT -= DT;
    float ph = e * 3.14159f;

    float tang = E.tang;
    if (mood == M_ANGRY) tang = e ? -0.30f : 0.30f;
    if (mood == M_SLEEPY) tang = e ? 0.18f : -0.18f;
    if (mood == M_SAD) tang = e ? 0.14f : -0.14f;
    if (mood == M_DIZZY) tang += 0.35f * sinf(TAU * 1.1f * t + ph);
    E.ang += (tang - E.ang) * kf;
    E.egg += (E.tegg - E.egg) * kg5;
    E.scl += (E.tscl - E.scl) * kg5;
    E.pscl += (E.tpscl - E.pscl) * kg5;
    E.fx += (E.tfx - E.fx) * kg5;
    E.fy += (E.tfy - E.fy) * kg5;
    E.ax += (E.tax - E.ax) * kg5;
    E.ay += (E.tay - E.ay) * kg5;

    float k = e ? 115.0f : 140.0f;
    float tx = gx * 10;
    float ty = gy * 8 + sinkY;
    if (mood == M_DIZZY) { float a = TAU * 1.3f * t + ph; tx += 14 * cosf(a); ty += 10 * sinf(a); }
    if (mood == M_PARTY) ty += 6 * sinf(TAU * 3.0f * t + ph);
    if (mood == M_ANGRY) { tx += 1.6f * frand(); ty += 1.6f * frand(); }
    float pvx0 = E.ovx, pvy0 = E.ovy;
    E.ovx += (k * (tx - E.ox) - 8 * E.ovx) * DT;
    E.ovy += (k * (ty - E.oy) - 8 * E.ovy) * DT;
    E.ox += E.ovx * DT; E.oy += E.ovy * DT;
    if (E.ox > 22) { E.ox = 22; E.ovx *= -0.4f; } else if (E.ox < -22) { E.ox = -22; E.ovx *= -0.4f; }
    if (E.oy > 24) { E.oy = 24; E.ovy *= -0.4f; } else if (E.oy < -24) { E.oy = -24; E.ovy *= -0.4f; }
    float ax = (E.ovx - pvx0) / DT, ay = (E.ovy - pvy0) / DT;

    float evx = E.ovx + dgx * 90, evy = E.ovy + dgy * 90;
    float sxt = 0.0009f * (fabsf(evx) - 0.5f * fabsf(evy));
    float syt = 0.0009f * (fabsf(evy) - 0.5f * fabsf(evx));
    if (mood == M_DIZZY) { sxt += 0.12f * sinf(TAU * 1.6f * t + ph); syt -= 0.12f * sinf(TAU * 1.6f * t + ph); }
    if (mood == M_SLEEPY) { sxt += 0.10f; syt -= 0.16f; }
    sxt = clampf(sxt, -0.2f, 0.28f); syt = clampf(syt, -0.2f, 0.28f);
    E.svx += (200 * (sxt - E.sx) - 8 * E.svx) * DT; E.sx += E.svx * DT;
    E.svy += (200 * (syt - E.sy) - 8 * E.svy) * DT; E.sy += E.svy * DT;
    E.sx = clampf(E.sx, -0.4f, 0.45f); E.sy = clampf(E.sy, -0.4f, 0.45f);

    float br = 1.0f + 0.02f * sinf(TAU * 0.3f * t + ph);
    E.rx = clampf(BASE_RX * size * br * E.scl * E.ax * (1 + E.sx), 26, 58);
    E.ry = clampf(BASE_RY * size * br * E.scl * E.ay * (1 + E.sy), 30, 70);
    float angv = E.ang + clampf(E.ovx * 0.0012f, -0.25f, 0.25f);
    E.ca = cosf(angv); E.sa = sinf(angv);
    E.cx = EXC[e] + E.fx + E.ox;
    E.cy = EYC + E.fy + E.oy;

    if (mood == M_GOOGLY) {
      float pr = 0.46f * fminf(E.rx, E.ry), lim = fminf(E.rx, E.ry) - pr - 1;
      E.pvy += 700 * DT;
      E.pvx += clampf(-ax, -2500, 2500) * 0.9f * DT; E.pvy += clampf(-ay, -2500, 2500) * 0.9f * DT;
      E.pvx -= E.pvx * 1.2f * DT; E.pvy -= E.pvy * 1.2f * DT;
      E.px += E.pvx * DT; E.py += E.pvy * DT;
      float d2 = E.px * E.px + E.py * E.py;
      if (d2 > lim * lim) {
        float d = sqrtf(d2), nx = E.px / d, ny = E.py / d;
        E.px = nx * lim; E.py = ny * lim;
        float vn = E.pvx * nx + E.pvy * ny;
        if (vn > 0) { E.pvx -= 1.55f * vn * nx; E.pvy -= 1.55f * vn * ny; }
      }
      E.prx = E.pry = pr;
    } else {
      float ps = cur[1] * (puOv[e] >= 0 ? puOv[e] : E.pscl * (1.0f + 0.04f * sinf(TAU * 0.5f * t + ph)));
      float prx = 0.36f * E.rx * ps, pry = 0.30f * E.ry * ps;
      float lx = fmaxf(1.0f, E.rx - prx - 1), ly = fmaxf(1.0f, E.ry - pry - 1);
      float ptx = gx * 0.9f * lx, pty = gy * 0.9f * ly;
      if (mood == M_SLEEPY) pty = ly * 0.7f;
      if (mood == M_ANGRY) { ptx *= 0.5f; pty = ly * 0.25f; }
      if (mood == M_SAD) pty = ly * 0.55f;
      E.pvx += (220 * (ptx - E.px) - 15 * E.pvx - 0.35f * ax) * DT;
      E.pvy += (220 * (pty - E.py) - 15 * E.pvy - 0.35f * ay) * DT;
      E.px += E.pvx * DT; E.py += E.pvy * DT;
      float nd = (E.px * E.px) / (lx * lx) + (E.py * E.py) / (ly * ly);
      if (nd > 1.0f) { float s = 1.0f / sqrtf(nd); E.px *= s; E.py *= s; }
      E.prx = prx; E.pry = pry;
    }
    E.ix = E.cx + E.px;
    E.iy = E.cy + E.py;
  }
}

uint16_t cEye, cPup;

void updateColors(uint32_t now) {
  int th = baseTheme;
  if (MOOD_THEME[mood] >= 0) th = MOOD_THEME[mood];
  else if (mood == M_DIZZY) th = (now / 350) % NTHEMES;
  else if (mood == M_PARTY) th = (now / 500) % NTHEMES;
  float k = (mood == M_DIZZY || mood == M_PARTY) ? lk(0.35f) : lk(0.12f);
  for (int c = 0; c < 2; c++)
    for (int j = 0; j < 3; j++) colr[c][j] += (THEMES[th][c][j] - colr[c][j]) * k;
  cEye = rgb565((uint8_t)colr[0][0], (uint8_t)colr[0][1], (uint8_t)colr[0][2]);
  cPup = rgb565((uint8_t)colr[1][0], (uint8_t)colr[1][1], (uint8_t)colr[1][2]);
}

void renderFrame(uint32_t now) {
  float t = now * 0.001f;
  for (int e = 0; e < 2; e++) {
    const Eye& E = eye[e];
    bool special = (mood == M_DIZZY || mood == M_LOVE);
    float bl = blinkValue(E);
    float ft = cur[2] + (0.5f - cur[2]) * bl, fbt = cur[3] + (0.5f - cur[3]) * bl;
    renderEye(E, ft, fbt, cur[4], e == 0 ? 1.0f : -1.0f, cEye, cPup, !special);

    if (mood == M_DIZZY) {  // pupila en espiral que gira
      Shp sh = makeShp(E, 0);
      float spin = -TAU * 2.0f * t, rr = fminf(E.rx, E.ry) * 0.85f;
      for (int i = 0; i < 44; i++) {
        float a = spin + i * 0.52f, r = rr * i / 44.0f;
        discAA(E.cx + r * cosf(a), E.cy + r * sinf(a), 2.6f, cPup, &sh);
      }
    } else if (mood == M_LOVE) {
      float s = 1.0f + 0.12f * sinf(TAU * 2.5f * t);
      heartAA(E.ix, E.iy, E.rx * 0.95f * s, E.ry * 0.62f * s, cPup);
    } else if (mood == M_SAD) {  // lagrima que cae
      float u = fmodf(t * 0.7f + e * 0.45f, 1.0f);
      float tx = E.cx + (e ? 22 : -22), ty = E.cy + E.ry * 0.5f + u * 46;
      gAlpha = (uint32_t)(32.0f * (u < 0.75f ? 1.0f : (1.0f - u) * 4.0f));
      uint16_t tc = rgb565(130, 210, 255);
      discAA(tx, ty, 5.5f, tc);
      triAA(tx - 4.6f, ty - 2.2f, tx + 4.6f, ty - 2.2f, tx, ty - 14, tc);
      gAlpha = 32;
    }
  }

  if (mood == M_DIZZY) {  // estrellitas orbitando
    for (int i = 0; i < 4; i++) {
      float a = TAU * 0.9f * t + i * 1.5708f;
      discAA(120 + 62 * cosf(a), 38 + 10 * sinf(a), 3.4f, cEye);
    }
  }
  if (mood == M_LOVE) {  // corazoncitos subiendo
    for (int i = 0; i < 3; i++) {
      float u = fmodf(t * 0.5f + i * 0.33f, 1.0f);
      gAlpha = (uint32_t)(32.0f * (u < 0.7f ? 1.0f : (1.0f - u) * 3.3f));
      heartAA(120 + (i - 1) * 34 + 8 * sinf(TAU * u * 2 + i), 60 - u * 38, 13, 12, cEye);
    }
    gAlpha = 32;
  }
  if (mood == M_SLEEPY) {  // Zzz
    for (int i = 0; i < 3; i++) {
      float u = fmodf(t * 0.3f + i * 0.33f, 1.0f);
      gAlpha = (uint32_t)(32.0f * sinf(3.14159f * u));
      drawZ(150 + u * 26 + i * 3, 74 - u * 46, 4.0f + u * 7.0f, cEye);
    }
    gAlpha = 32;
  }
}

// ---------- Entrada ----------
const uint8_t CYCLE[8] = {M_SURPRISED, M_SLEEPY, M_ANGRY, M_SAD, M_LOVE, M_GOOGLY, M_DIZZY, M_PARTY};
uint8_t cycleIdx = 0;

void wake(uint32_t now) {
  lastActivity = now;
  if (mood == M_SLEEPY && moodUntil == 0) { setMood(M_SURPRISED, now, 1200); }
}
void nextMood(uint32_t now) {
  uint8_t m = CYCLE[cycleIdx];
  cycleIdx = (cycleIdx + 1) % 8;
  setMood(m, now, m == M_SLEEPY ? 8000 : 6000);
}
void nextTheme_(uint32_t now) {
  baseTheme = (baseTheme + 1) % NTHEMES;
  nextTheme = now + 25000;
}
void wink() { eye[1].blinkTot = eye[1].blinkT = 0.55f; }


// ---------- API del core ----------
void setGaze(float x, float y, uint32_t now, uint32_t holdMs) {
  tgx = clampf(x, -1, 1); tgy = clampf(y, -1, 1);
  gazeUntil = now + holdMs;
  lastActivity = now;
}

void coreInit(uint32_t now, uint32_t seed) {
  rng ^= seed;
  const float* tp = &MOODS[M_NORMAL].size;
  for (int i = 0; i < 6; i++) cur[i] = tp[i];
  for (int c = 0; c < 2; c++)
    for (int j = 0; j < 3; j++) colr[c][j] = THEMES[0][c][j];
  for (int e = 0; e < 2; e++) {
    eye[e] = Eye();
    eye[e].rx = BASE_RX;
    eye[e].ry = BASE_RY;
    eye[e].scl = eye[e].pscl = eye[e].tscl = eye[e].tpscl = 1;
    eye[e].ax = eye[e].ay = eye[e].tax = eye[e].tay = 1;
    eye[e].ca = 1;
  }
  setMood(M_SURPRISED, now, 1200);  // al encender "abren los ojos"
  lastActivity = now;
}

// avanza la animacion y dibuja un cuadro completo en fb (que ya debe apuntar a un buffer valido)
void coreFrame(uint32_t now, float dt) {
  DT = clampf(dt, 0.005f, 0.04f);
  DTR = DT / 0.025f;
  updateAnimation(now);
  updateColors(now);
  memset(fb, 0, (size_t)W * H * 2);
  dirtyY0 = H; dirtyY1 = -1;
  renderFrame(now);
}
