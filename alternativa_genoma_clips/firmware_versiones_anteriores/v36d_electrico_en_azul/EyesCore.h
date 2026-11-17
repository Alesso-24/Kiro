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
float clampf(float v, float lo, float hi) { return v != v ? lo : (v < lo ? lo : (v > hi ? hi : v)); }   // NaN -> lo
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

// ---- extras de dibujo ----
// destello de 4 puntas
void sparkAA(float cx, float cy, float r, float rot, uint16_t col) {
  float w = r * 0.34f, x[8], y[8];
  float ca = cosf(rot), sa = sinf(rot);
  for (int i = 0; i < 8; i++) {
    float rr = (i & 1) ? w : r, a = i * 0.7853982f - 1.5707963f;
    float px = cosf(a) * rr, py = sinf(a) * rr;
    x[i] = cx + px * ca - py * sa; y[i] = cy + px * sa + py * ca;
  }
  for (int i = 0; i < 8; i++) {
    int j = (i + 1) & 7;
    triAA(cx, cy, x[i], y[i], x[j], y[j], col, 0.35f);
  }
}
// marca de enojo tipo manga (4 corchetes alrededor de un centro)
void veinAA(float cx, float cy, float s, uint16_t col) {
  for (int k = 0; k < 4; k++) {
    float d = k * 1.5707963f + 0.7853982f, dx = cosf(d), dy = sinf(d);
    float ox = cx + dx * s * 0.95f, oy = cy + dy * s * 0.95f, base = atan2f(-dy, -dx);
    float px = 0, py = 0;
    for (int i = 0; i <= 6; i++) {
      float a = base - 0.75f + 1.5f * i / 6.0f;
      float nx = ox + cosf(a) * s * 0.78f, ny = oy + sinf(a) * s * 0.78f;
      if (i) lineAA(px, py, nx, ny, 2.8f, col);
      px = nx; py = ny;
    }
  }
}

// ============================================================================================
//  ALLOY · MOTOR DEL ESTUDIO v4 (port 1:1 de studio/estilo_tinta/alloy_v4.html)
//  Siluetas de boceto (poligono), 2 colores planos, pupilas con resorte propio, ojos cerrados de pincel,
//  21 expresiones + pantalla de color + nacimiento. Los botones/serial llaman a las funciones ev*().
// ============================================================================================
struct C3 { float r, g, b; };
static C3 mix3(C3 a, C3 b, float k) { return {a.r + (b.r - a.r) * k, a.g + (b.g - a.g) * k, a.b + (b.b - a.b) * k}; }
static uint16_t c565(C3 c) { return rgb565((uint8_t)clampf(c.r, 0, 255), (uint8_t)clampf(c.g, 0, 255), (uint8_t)clampf(c.b, 0, 255)); }
static C3 hexc(uint32_t h) { return {(float)((h >> 16) & 255), (float)((h >> 8) & 255), (float)(h & 255)}; }
static float Rr(float a, float b) { return a + (b - a) * frand01(); }
static float sstepf(float a, float b, float x) { float k = clampf((x - a) / (b - a), 0, 1); return k * k * (3 - 2 * k); }
static float growF(float x) { if (x <= 0) return 0; if (x >= 1) return 1; return 1.0f - expf(-6.0f * x) * cosf(11.0f * x); }

const uint32_t NEON[10] = {0x4de3ff, 0x7dff6a, 0xff9f3d, 0xff5fa2, 0xb48cff, 0xffe14d, 0xff5a4d, 0x2effc0, 0x5b8cff, 0xff8ad8};
const uint32_t ALLCOL[26] = {0x4de3ff, 0x7dff6a, 0xff9f3d, 0xff5fa2, 0xb48cff, 0xffe14d, 0xff5a4d, 0x2effc0, 0x5b8cff, 0xff8ad8, 0xff5a4d, 0xffd23d, 0xc25bff, 0x1fb6ff, 0x38f0e0,
                             0x8dff6a, 0xc6ff3d, 0x5dd16a, 0xff3d6e, 0xf3eee3, 0xe8f6ff, 0xfff3e0, 0xff4d6d, 0xff9a5c, 0x7dff6a, 0xffe14d};
// pantalla de color: fondo, ojo, pupila
const uint32_t FBP[11][3] = {{0x4de3ff, 0x0a1f2b, 0x4de3ff}, {0xff5fa2, 0xffffff, 0x2a0614}, {0xffe14d, 0x1a1200, 0xffe14d}, {0x8dff6a, 0x06200a, 0x8dff6a}, {0xb48cff, 0xffffff, 0x1a0b3d}, {0xff5a4d, 0xffffff, 0x2a0a06},
                             {0x1a0b3d, 0xffe14d, 0x1a0b3d}, {0x0a1f2b, 0x2effc0, 0x0a1f2b}, {0xf3eee3, 0xff5a4d, 0x2a0a06}, {0xff9f3d, 0x2b1400, 0xff9f3d}, {0x2effc0, 0x2a0830, 0x2effc0}};

// ---------------- poses de los bocetos: rx, ry, base plana, angulo cresta, alto cresta, ancho cresta, alto punta, ancho punta, inclinacion, gancho, muesca, giro(grados)
enum { PO_REST = -1, PO_HUEVO = 0, PO_BANDERA, PO_PICO, PO_TORRE, PO_AGACHADO, PO_COLA, PO_GANCHO, PO_CABECITA, PO_FURIAL, PO_FURIAR, NPO };
const float PZT[NPO][12] = {
  {40, 46, .12f, -1.57f, 0, .3f, 0, .05f, 0, 0, 0, 0},       {42, 40, .2f, -1.9f, .55f, .35f, .5f, .04f, .25f, .3f, 0, 0},
  {38, 42, .2f, -1.3f, .45f, .25f, .9f, .03f, .3f, 0, 0, 0}, {34, 50, .25f, -1.2f, .5f, .4f, 1.1f, .035f, .45f, .2f, .25f, 0},
  {48, 32, .35f, -1.57f, .15f, .5f, .25f, .05f, 0, 0, 0, 0}, {38, 44, .2f, -2.0f, .5f, .3f, .95f, .035f, -.4f, -.2f, 0, 0},
  {40, 44, .25f, -1.8f, .45f, .3f, .8f, .04f, -.35f, -.7f, .3f, 0}, {36, 48, .25f, -1.0f, .35f, .2f, .6f, .05f, .1f, .5f, 0, 0},
  {38, 42, .2f, -2.1f, .45f, .25f, .9f, .03f, -.3f, 0, 0, 0},   {38, 42, .2f, -1.04f, .45f, .25f, .9f, .03f, .3f, 0, 0, 0}};
const uint8_t FLOUR[6] = {PO_BANDERA, PO_PICO, PO_TORRE, PO_COLA, PO_GANCHO, PO_CABECITA};

const int NPT = 72;
float ptsX[NPT], ptsY[NPT], covRow[W];

struct EDraw {
  float cx, cy, rot, flip;               // centro, giro (rad), +1/-1 espejo horizontal
  float sang, sstr;                      // estiramiento por velocidad
  float p[12], t, id, sx, sy;            // pose y escala del contorno
  uint16_t cEye, cPup;
  bool hasPup; float ppx, ppy, prx, pry;
  bool heart; float hs;
  bool lid; float lidY, lidK;
  bool cheek; float chCy, chRx, chRy;
  bool meter; float mTop, mBot, mLv, mT; uint16_t cDark;
  float morph, bw, bdep, bdir, bcd, bci, blo, bkk;   // deformacion ojo -> trazo de pincel
};

static void outlinePts(const float* p, float t, float id, float sx, float sy, float* X, float* Y) {
  float rx = p[0], ry = p[1], flat = p[2], ca = p[3], ch = p[4], cw = fmaxf(p[5], 0.02f), th = p[6], tw = fmaxf(p[7], 0.01f), lean = p[8], hook = p[9], notch = p[10];
  for (int i = 0; i < NPT; i++) {
    float a = TAU * i / NPT, sn = sinf(a), cs = cosf(a);
    float r = 1.0f + 0.028f * sinf(3 * a + t * 1.3f + id * 2) + 0.022f * sinf(5 * a - t * 0.9f + id) + 0.015f * sinf(2 * a + t * 0.7f);
    if (ch > 0.01f || th > 0.01f) {
      float d = a - ca; d = atan2f(sinf(d), cosf(d));
      float d2 = a - ca - hook * 0.5f; d2 = atan2f(sinf(d2), cosf(d2));
      r += ch * expf(-d * d / cw) + th * 0.6f * expf(-d2 * d2 / tw) * (1.0f + 0.07f * sinf(t * 2.3f + id));
      if (notch > 0.01f) { float dn = a - ca - 0.55f; dn = atan2f(sinf(dn), cosf(dn)); r -= notch * expf(-dn * dn / 0.02f); }
    }
    float x = cs * rx * r, y = sn * ry * r;
    if (y > 0) y = ry * powf(sn, 1.0f - flat * 0.7f) * r;
    x += -y * lean * 0.55f;
    X[i] = x * sx; Y[i] = y * sy;
  }
}

void renderEye(const EDraw& D) {
  outlinePts(D.p, D.t, D.id, D.sx, D.sy, ptsX, ptsY);
  if (D.morph > .003f) {                                          // el contorno del ojo se vuelve el trazo de pincel (sin cambio brusco)
    float ez = D.morph * D.morph * (3 - 2 * D.morph);
    for (int i = 0; i < NPT; i++) {
      float a = TAU * i / NPT, ux = cosf(a), x = D.bw * ux, th = 15 * D.bkk * sqrtf(fmaxf(0, 1 - powf(fabsf(ux), 6))) + .6f;
      float yc = D.bdir * D.bdep * D.bcd * (1 - ux * ux) + 2 + D.bci * D.blo * x * .5f, y = yc + (sinf(a) >= 0 ? th / 2 : -th / 2);
      ptsX[i] += (x - ptsX[i]) * ez; ptsY[i] += (y - ptsY[i]) * ez;
    }
  }
  float cr = cosf(D.rot), sr = sinf(D.rot), ca = cosf(D.sang), sa = sinf(D.sang);
  float xmin = 1e9f, xmax = -1e9f, ymin = 1e9f, ymax = -1e9f;
  for (int i = 0; i < NPT; i++) {
    float x = ptsX[i], y = ptsY[i];
    if (D.sstr > 1.001f) {                                   // estirar en la direccion del movimiento
      float u = x * ca + y * sa, v = -x * sa + y * ca; u *= D.sstr; v /= sqrtf(D.sstr);
      x = u * ca - v * sa; y = u * sa + v * ca;
    }
    float rx_ = x * cr - y * sr, ry_ = x * sr + y * cr;
    ptsX[i] = D.cx + D.flip * rx_; ptsY[i] = D.cy + ry_;
    xmin = fminf(xmin, ptsX[i]); xmax = fmaxf(xmax, ptsX[i]); ymin = fminf(ymin, ptsY[i]); ymax = fmaxf(ymax, ptsY[i]);
  }
  int y0 = max(0, (int)floorf(ymin) - 1), y1 = min(H - 1, (int)ceilf(ymax) + 1);
  int x0 = max(0, (int)floorf(xmin) - 1), x1 = min(W - 1, (int)ceilf(xmax) + 1);
  markRows(y0, y1);
  for (int x = x0; x <= x1; x++) covRow[x] = 0;
  float ipx = 1.0f / (D.prx + 1e-3f), ipy = 1.0f / (D.pry + 1e-3f), psc = 0.5f * sqrtf(fmaxf(D.prx * D.pry, 0.01f)), pfe = 0.5f / (psc + 1e-3f);
  float ccx = 1.0f / (D.chRx + 1e-3f), ccy = 1.0f / (D.chRy + 1e-3f), cmn = fminf(D.chRx, D.chRy);
  for (int y = y0; y <= y1; y++) {
    bool any = false;
    for (int sub = 0; sub < 2; sub++) {
      float sy = y + 0.25f + 0.5f * sub, xs[16]; int n = 0;
      for (int i = 0, j = NPT - 1; i < NPT; j = i++) {
        float ya = ptsY[j], yb = ptsY[i];
        if ((ya <= sy) != (yb <= sy)) { float xx = ptsX[j] + (sy - ya) / (yb - ya) * (ptsX[i] - ptsX[j]); if (n < 16) { int q = n++; while (q > 0 && xs[q - 1] > xx) { xs[q] = xs[q - 1]; q--; } xs[q] = xx; } }
      }
      for (int q = 0; q + 1 < n; q += 2) {
        float xa = clampf(xs[q], 0, W - 1), xb = clampf(xs[q + 1], 0, W - 1);
        for (int ix = (int)xa; ix <= (int)xb && ix < W; ix++) { float l = fmaxf(xa, (float)ix), r = fminf(xb, (float)ix + 1); if (r > l) { covRow[ix] += 0.5f * (r - l); any = true; } }
      }
    }
    if (!any) continue;
    uint16_t* row = fb + y * W;
    float dy = y - D.cy;
    for (int x = x0; x <= x1; x++) {
      float a = covRow[x]; if (a <= 0) continue; covRow[x] = 0; if (a > 1) a = 1;
      float dx = D.flip * (x - D.cx);
      float lx = dx * cr + dy * sr, ly = -dx * sr + dy * cr;             // coordenadas locales del ojo
      if (D.lid) { float lt = ly - (D.lidY - D.lidK * lx) + 0.5f; if (lt <= 0) continue; if (lt < 1) a *= lt; }
      if (D.cheek) { float qx = lx * ccx, qy = (ly - D.chCy) * ccy, f2 = qx * qx + qy * qy; if (f2 < 1.0f) { float g = (sqrtf(f2) - 1.0f) * cmn + 0.5f; if (g <= 0) continue; if (g < 1) a *= g; } }
      uint16_t col = D.cEye;
      if (D.meter) { float wy = D.mBot - D.mLv * (D.mBot - D.mTop) + sinf(lx * 0.12f + D.mT * 3.0f) * 2.2f * fminf(1.0f, D.mLv * 4.0f); col = ly >= wy ? D.cEye : D.cDark; }
      if (D.heart) {
        float hu = (lx - D.ppx) / (D.hs * 1.05f), hv = -(ly - (D.ppy + 3 + 0.15f * D.hs)) / (D.hs * 0.78f);
        float h1 = hu * hu + hv * hv - 1.0f, hf = h1 * h1 * h1 - hu * hu * hv * hv * hv;
        if (hf < 0) col = rgb565(255, 42, 69);
      } else if (D.hasPup) {
        float pa = (lx - D.ppx) * ipx, pb = (ly - D.ppy) * ipy, fp = pa * pa + pb * pb - 1;
        if (fp < pfe) col = fp <= -pfe ? D.cPup : blend565(col, D.cPup, (int)((0.5f - fp * psc) * 32.0f + 0.5f));
      }
      int ai = (int)(a * (float)gAlpha + 0.5f);
      if (ai <= 0) continue;
      row[x] = ai >= 32 ? col : blend565(row[x], col, ai);
    }
  }
}

// ---- ojo cerrado: pincel (grueso al centro, extremos redondos), con curvatura (cd) e inclinacion (ci) ----
static void toScreen(float cx, float cy, float rot, float flip, float x, float y, float& ox, float& oy) { float c = cosf(rot), s = sinf(rot); ox = cx + flip * (x * c - y * s); oy = cy + (x * s + y * c); }

// ---------------- estado ----------------
enum { K_LIFT, K_SCR, K_CD, K_CI, K_CHEEK, K_EUPH, K_TILT, K_PUP, K_SCALE, K_LID, K_SLANT, K_ARC, K_DIR, K_DIM, K_JIT, K_SLOSH, K_HEART, K_BOUNCE, K_CMIX, K_BREATH, NK };
float Vv[NK], Tt[NK]; C3 Vcm = {255, 255, 255}, Tcol = {255, 255, 255};
static void tgDefault() { for (int i = 0; i < NK; i++) Tt[i] = 0; Tt[K_CD] = 1; Tt[K_PUP] = 1; Tt[K_SCALE] = 1; Tt[K_DIR] = 1; }

enum { X_AWAKE, X_CURIOUS, X_HAPPY, X_SAD, X_ANGRY, X_LOVE, X_SURPRISE, X_SMUG, X_ANXIOUS, X_HYPE, X_DIZZY, X_COLD, X_YAWN, X_WINK, X_BIRTH, X_PANT, X_FULL, X_ELECTRIC, X_CHARGING, X_BATCHECK, X_LOWBAT, X_SLEEP, X_DIMSLEEP, NEXPR };
enum { L_NORMAL, L_STILL, L_DOWN, L_UP, L_FAST, L_CIRCLE, L_SIDE, L_SMUG, L_FREE = 20 };
const char* XNAME[NEXPR] = {"despierto", "curioso", "feliz", "triste", "enojado", "amor", "sorprendido", "orgulloso", "ansioso", "hype", "mareado", "frio", "bostezo", "guino", "nacimiento", "pantalla", "cara-llena", "electrico", "cargando", "ver-bateria", "bateria-baja", "dormido", "dormido-tenue"};
const float XDUR[NEXPR] = {5, 4, 4, 5, 4, 5, 3, 4, 4, 5, 4, 4, 7, 2.5f, 60, 8, 4, 3, 12, 5, 5, 6, 6};

struct SEye {
  float p[12], v[12], tgt[12], k, d, bk, bd;
  float o, ov, ot, gx, gy, gvx, gvy, px, py, pvx, pvy, kp, sz, szv, szT, jx, jy, jn, ph, gene;
};
SEye eyes[2];
struct Creature { float scale, gap, curious, energy, sleepy, grumpy, shy, pd, restK; C3 c1, c2; bool two, mir; uint8_t restA, restB, pstyle; } cr;
struct Genome { int pal, species, pupil; } G;
struct Part { bool on; uint8_t k; float x, y, vx, vy, a, l, s, len, w; C3 c; };
const int NPARTS = 48;
Part parts[NPARTS];
static void addPart(const Part& q) { for (int i = 0; i < NPARTS; i++) if (!parts[i].on) { parts[i] = q; parts[i].on = true; return; } }

float St = 0, gx_t = 0, gy_t = 0, pop = 1, popv = 0, Vcharge = 0.15f, energy = 80, simT = 0;
float lastActivityS = 0;
uint32_t gNow = 0;
int8_t birthVar = -1;
bool birthSkip = false, simBatt = true;
uint32_t birthPick = 0;
int curX = X_AWAKE;
struct Fx { float b, sc, fl, w, sz, sl, u, ex, fbn, pn, sleepPlan; uint8_t look; bool blinkOK, slowB, flash, fb, pieceHeart; int meter; int part; float bx, by; float sideX; uint8_t pso; bool hasPso; bool usb; int usbStage; float usbT; float holdPet; uint32_t petGen; int puffGen; int beatGen; bool wasSleepy; } E;
C3 fbBg, fbEye, fbPup;
float bsat = -1;   // >=0 durante la ruleta del nacimiento: de blanco a color

// ---- agenda de eventos (setTimeout) ----
struct Ev { float at; uint8_t code, a; float f, g; bool keep, on; };
Ev evq[48];
enum { EV_OT, EV_LOOK, EV_TG, EV_POSE, EV_POP, EV_SZ, EV_SETTLE, EV_BLINK, EV_EXPR, EV_PUFF, EV_BEAT, EV_FLO, EV_SZONE, EV_SLOWEND };
static void sched(float after, int code, int a = 0, float f = 0, float g = 0, bool keep = false) {
  for (int i = 0; i < 48; i++) if (!evq[i].on) { evq[i] = {St + after, (uint8_t)code, (uint8_t)a, f, g, keep, true}; return; }
}
static void clearTimers() { for (int i = 0; i < 48; i++) if (evq[i].on && !evq[i].keep) evq[i].on = false; }

static const float* restVec(int i, float* tmp) { int pz = i ? cr.restB : cr.restA; for (int k = 0; k < 12; k++) tmp[k] = PZT[0][k] + (PZT[pz][k] - PZT[0][k]) * cr.restK; return tmp; }
float exoV[2][12];
const int PO_XA = 100;
static void genEyeExoC(float* o) { bool crest = frand01() < .78f; o[0] = Rr(30, 46); o[1] = Rr(42, 58); o[2] = Rr(.05f, .42f); o[3] = Rr(-2.35f, -.75f); o[4] = crest ? Rr(.12f, .6f) : Rr(0, .12f); o[5] = Rr(.16f, .5f); o[6] = crest ? Rr(.2f, 1.05f) : Rr(0, .25f); o[7] = Rr(.028f, .058f); o[8] = Rr(-.5f, .5f); o[9] = Rr(-.8f, .8f); o[10] = frand01() < .35f ? Rr(.1f, .32f) : 0; o[11] = Rr(-8, 8); }
static void genExoPair() {   // ojos casi iguales entre si (forma exotica momentanea)
  genEyeExoC(exoV[0]);
  for (int k = 0; k < 12; k++) exoV[1][k] = k == 3 ? exoV[0][k] + Rr(-.12f, .12f) : k == 11 ? exoV[0][k] + Rr(-2, 2) : exoV[0][k] * (1 + Rr(-.07f, .07f));
}
static void setPose(int i, int id) {
  float tmp[12]; const float* P = id >= PO_XA ? exoV[id - PO_XA] : id == PO_REST ? restVec(i, tmp) : PZT[id];
  for (int k = 0; k < 12; k++) eyes[i].tgt[k] = P[k] * (k < 2 ? Rr(0.95f, 1.05f) : 1.0f);
}
static void look(float x, float y) { gx_t = x; gy_t = y; }
static void blinkEye(int i, float d) { sched(d, EV_OT, i, 1.08f, 0, true); sched(d + 0.05f, EV_OT, i, 0, 0, true); sched(d + (E.slowB ? 0.42f : 0.24f), EV_OT, i, 1, 0, true); }
static void blink() { blinkEye(0, 0); blinkEye(1, Rr(0.015f, 0.06f)); }
static void goPose(int a, int b, float delay = 0, bool ant = true) {
  if (ant) { setPose(0, PO_AGACHADO); setPose(1, PO_AGACHADO); sched(0.11f, EV_POSE, 0, a); sched(0.11f, EV_POP, 0, 2.2f); sched(0.11f + delay, EV_POSE, 1, cr.mir ? a : b); }
  else { setPose(0, a); popv += 2.2f; sched(delay, EV_POSE, 1, cr.mir ? a : b); }
}
static void settle(float ms) { sched(ms / 1000.0f, EV_SETTLE); }

// ---- criatura ----
static void fresh(bool silent) {
  cr.scale = Rr(0.98f, 1.08f); cr.gap = Rr(112, 122);
  cr.c1 = hexc(NEON[(int)(frand01() * 9.99f)]); cr.c2 = hexc(NEON[(int)(frand01() * 9.99f)]);
  cr.two = frand01() < 0.2f; cr.pd = Rr(-1, 1);
  cr.curious = frand01(); cr.energy = Rr(0.5f, 1.5f); cr.sleepy = frand01(); cr.grumpy = frand01(); cr.shy = frand01();
  static const uint8_t BP[8] = {PO_HUEVO, PO_BANDERA, PO_PICO, PO_TORRE, PO_AGACHADO, PO_COLA, PO_GANCHO, PO_CABECITA};
  cr.restA = BP[(int)(frand01() * 7.99f)]; cr.restB = frand01() < 0.45f ? cr.restA : BP[(int)(frand01() * 7.99f)]; cr.restK = 0.0f;
  cr.mir = frand01() < 0.5f; cr.pstyle = 0;
  G.pal = (int)(cr.c1.r + cr.c1.g * 3 + cr.c1.b * 7) % 997; G.species = cr.restA * 8 + cr.restB; G.pupil = cr.pstyle;
  if (!silent) { popv -= 8; sched(0.1f, EV_POP, 0, 10.0f); }
}

// ---- expresion ----
static void setExpr(int id, bool silent = false);
static void startBirth();
void setExprImpl(int id, bool silent);
static void setExpr(int id, bool silent) { setExprImpl(id, silent); }

// ---- nacimiento (estado) ----
const float PR_W = 9500, PR_R = 380, PR_B = 3000, PR_S = 1300, PR_H = 800, PRE_MS = PR_W + PR_R + PR_B + PR_S + PR_H, RU_T1 = 15000, RU_C = 550, RU_H = 2000, RU_O = 750, BDUR = PRE_MS + RU_T1 + RU_C + RU_H + RU_O + 3700;
struct Thr { float a0, k, len, rate, spd, df, wa, wf, wp, th, al, ph, fl, pin; };
struct Birth {
  float t0, spin; Thr thr[16]; float dotR, spasm, bulX, bulY, feed;
  float cRx, cRy, cGap, cOffX, cOffY, cBump, cSat, cSf; bool cDone; int cPA, cPB; C3 cCol;
  float nextSwap, lastSwap, swapDur, streak; int stage; float dt; int cs; float lm; bool restored, looked; float planT[6], planX[6], planY[6]; int planN, pi; C3 fc1, fc2; bool ftwo; float fpd;
} B;

static float satOf(C3 c) { return fmaxf(c.r, fmaxf(c.g, c.b)) - fminf(c.r, fminf(c.g, c.b)); }
static C3 rndAll() { return hexc(ALLCOL[(int)(frand01() * 25.99f)]); }

void setExprImpl(int id, bool silent) {
  curX = id; E.ex = St; E.u = 0; clearTimers();
  if (birthVar >= 0 && id != X_BIRTH) { birthVar = -1; cr.c1 = B.fc1; cr.c2 = B.fc2; cr.two = B.ftwo; cr.pd = B.fpd; }
  bsat = -1; E.bx = E.by = 0; E.hasPso = false;
  tgDefault(); Tcol = {255, 255, 255};
  E.fb = false; E.fbn = St + 2.4f; E.look = L_NORMAL; E.part = 0; E.blinkOK = true; E.slowB = false; E.flash = false; E.meter = 0; E.sideX = (frand() > 0 ? 1 : -1) * 14.0f;
  E.puffGen++; E.beatGen++;
  int pL = PO_REST, pR = PO_REST; bool pPose = true;
  switch (id) {
    case X_CURIOUS: pL = PO_CABECITA; pR = PO_PICO; Tt[K_TILT] = .4f; Tt[K_PUP] = 1.12f; E.look = L_SIDE; break;
    case X_HAPPY: Tt[K_CHEEK] = .85f; Tt[K_EUPH] = 1; Tt[K_BOUNCE] = .8f; Tt[K_PUP] = 1.15f; Tt[K_SCALE] = 1.05f; E.look = L_STILL; break;
    case X_SAD: pL = PO_GANCHO; pR = PO_COLA; Tt[K_TILT] = -.6f; Tt[K_SLANT] = -1; Tt[K_LID] = .22f; Tt[K_PUP] = 1.12f; E.look = L_DOWN; E.part = 2; break;
    case X_ANGRY: pL = PO_FURIAL; pR = PO_FURIAR; Tt[K_SLANT] = 1; Tt[K_LID] = .3f; Tt[K_PUP] = .8f; Tt[K_CMIX] = .55f; Tt[K_JIT] = .35f; Tt[K_TILT] = .5f; Tcol = {255, 70, 40}; E.look = L_STILL; break;
    case X_LOVE: pL = pR = PO_AGACHADO; Tt[K_PUP] = 1.15f; Tt[K_BOUNCE] = .25f; Tt[K_SCALE] = 1.05f; Tt[K_HEART] = 1; Vv[K_HEART] = 1; E.part = 1; break;
    case X_SURPRISE: pL = pR = PO_TORRE; Tt[K_SCALE] = 1.1f; Tt[K_PUP] = .58f; E.look = L_STILL; break;
    case X_SMUG: Tt[K_LID] = .3f; Tt[K_TILT] = -.2f; Tt[K_PUP] = 1.1f; Tt[K_SCALE] = 1.06f; Tt[K_BOUNCE] = .12f; E.look = L_SMUG; sched(1.4f, EV_PUFF, 0, 1, 0); break;
    case X_ANXIOUS: Tt[K_SCALE] = .94f; Tt[K_PUP] = .72f; Tt[K_JIT] = .04f; Tt[K_BOUNCE] = .5f; E.look = L_FAST; break;
    case X_HYPE: pL = PO_TORRE; pR = PO_AGACHADO; Tt[K_BOUNCE] = 1; Tt[K_SCALE] = 1.1f; Tt[K_PUP] = 1.22f; E.look = L_FAST; sched(0.52f, EV_BEAT, 1, 0, 0); break;
    case X_DIZZY: Tt[K_SLOSH] = 1; Tt[K_PUP] = .95f; E.look = L_CIRCLE; break;
    case X_COLD: Tt[K_CMIX] = .8f; Tt[K_JIT] = .5f; Tt[K_LID] = .22f; Tt[K_PUP] = .8f; Tcol = {130, 200, 255}; E.look = L_STILL; break;
    case X_YAWN: E.look = L_DOWN; break;
    case X_WINK: break;
    case X_BIRTH: break;
    case X_PANT: Tt[K_BOUNCE] = .5f; Tt[K_SCALE] = 1.08f; Tt[K_PUP] = 1.15f; Tt[K_EUPH] = .5f; E.look = L_FAST; E.fb = true; { int i = (int)(frand01() * 10.99f); fbBg = hexc(FBP[i][0]); fbEye = hexc(FBP[i][1]); fbPup = hexc(FBP[i][2]); } break;
    case X_FULL: Tt[K_SCALE] = 1.75f; E.look = L_STILL; break;
    case X_ELECTRIC: Tt[K_SCALE] = 1.25f; Tt[K_JIT] = .6f; E.look = L_STILL; E.flash = true; break;
    case X_CHARGING: Tt[K_PUP] = 1.05f; E.look = L_STILL; E.meter = 1; Vcharge = .15f; break;
    case X_BATCHECK: E.look = L_STILL; E.meter = 2; break;
    case X_LOWBAT: pL = pR = PO_AGACHADO; Tt[K_LID] = .45f; Tt[K_SLANT] = -.4f; Tt[K_DIM] = .3f; Tt[K_CMIX] = .35f; Tt[K_PUP] = .9f; Tcol = {255, 90, 60}; E.look = L_STILL; E.slowB = true; break;
    case X_SLEEP: Tt[K_ARC] = 1; Tt[K_DIR] = 1; Tt[K_DIM] = .12f; Tt[K_BREATH] = 1; Tt[K_CD] = .55f; Tt[K_CI] = .35f; E.look = L_STILL; E.blinkOK = false; Vv[K_DIR] = 1; break;
    case X_DIMSLEEP: Tt[K_ARC] = 1; Tt[K_DIR] = 1; Tt[K_DIM] = .62f; Tt[K_BREATH] = 1; Tt[K_CD] = .4f; Tt[K_CI] = .5f; E.look = L_STILL; E.blinkOK = false; Vv[K_DIR] = 1; break;
  }
  if (id == X_BIRTH) startBirth();
  else if (Tt[K_ARC] > 0) { setPose(0, PO_REST); setPose(1, PO_REST); }
  else goPose(pL, pR, Rr(0.04f, 0.16f), !silent);
  for (int i = 0; i < 2; i++) eyes[i].szT = 1;
  if (id == X_CURIOUS) eyes[frand() > 0 ? 0 : 1].szT = 1.25f;
  if (id == X_YAWN) {   // parpados que pesan -> ojos apretados con arrugas y estirada -> suspiro -> abre pesado
    Tt[K_LID] = .15f;
    sched(0.5f, EV_TG, K_LID, .4f); sched(0.5f, EV_TG, K_PUP, .85f);
    sched(1.1f, EV_TG, K_DIR, 1); sched(1.1f, EV_TG, K_CD, .9f); sched(1.1f, EV_TG, K_CI, .55f);
    sched(1.4f, EV_TG, K_LID, 0); sched(1.4f, EV_TG, K_ARC, 1); sched(1.4f, EV_TG, K_SCALE, 1.1f); sched(1.4f, EV_TG, K_LIFT, 1); sched(1.4f, EV_TG, K_SCR, 1); sched(1.4f, EV_TG, K_JIT, .1f); sched(1.4f, EV_POP, 0, 2);
    sched(2.7f, EV_TG, K_SCALE, 1); sched(2.7f, EV_TG, K_LIFT, 0); sched(2.7f, EV_TG, K_SCR, 0); sched(2.7f, EV_TG, K_JIT, 0); sched(2.7f, EV_TG, K_CD, .5f); sched(2.7f, EV_TG, K_CI, .65f); sched(2.7f, EV_POP, 0, -2);
    sched(3.5f, EV_TG, K_ARC, 0); sched(3.5f, EV_TG, K_LID, .6f); sched(3.5f, EV_TG, K_PUP, .9f); sched(3.5f, EV_POP, 0, 1.5f);
    sched(4.0f, EV_TG, K_LID, .35f); sched(4.5f, EV_BLINK); sched(4.5f, EV_TG, K_LID, .2f); sched(5.0f, EV_TG, K_LID, .1f); sched(5.0f, EV_TG, K_PUP, 1); sched(5.5f, EV_TG, K_LID, 0); sched(5.5f, EV_TG, K_CD, 1); sched(5.5f, EV_TG, K_CI, 0);
  }
  if (id == X_WINK) { sched(0.35f, EV_OT, 1, 0, 0, true); sched(0.35f, EV_TG, K_TILT, .25f); sched(1.3f, EV_OT, 1, 1, 0, true); sched(1.3f, EV_TG, K_TILT, 0); }
  (void)pPose;
  if (!silent) popv += 1.5f;
}

// ---- nacimiento ----
static void startBirth() {
  birthVar = 0; E.look = L_STILL; E.blinkOK = false; Tt[K_PUP] = .3f;
  B.fc1 = cr.c1; B.fc2 = cr.c2; B.ftwo = cr.two; B.fpd = cr.pd;
  for (int a = 0; a < 16; a++) B.thr[a] = {TAU * a / 16 + .25f * frand(), 2.7f + .9f * frand01(), .72f + .28f * frand01(), .95f + .12f * frand01(), .85f + .3f * frand01(), 4.0f + 1.6f * frand01(), .015f + .02f * frand01(), 1.0f, TAU * frand01(), 2.2f + .6f * frand01(), .80f + .15f * frand01(), 0.0f, TAU * frand01(), 0.0f};
  B.t0 = St; B.spin = 0; B.dotR = 3; B.spasm = B.bulX = B.bulY = B.feed = 0;
  B.cRx = 34; B.cRy = 55; B.cGap = 38; B.cOffX = B.cOffY = 0; B.cBump = 1; B.cSat = 0; B.cSf = 0; B.cDone = false; B.cPA = PO_BANDERA; B.cPB = PO_PICO; B.cCol = {250, 226, 216};
  B.nextSwap = 900; B.lastSwap = -1e6f; B.swapDur = 600; B.streak = 0; B.stage = 0; B.dt = .016f; B.cs = 0; B.lm = 0; B.restored = false; B.looked = false; B.planN = 0; B.pi = 0;
  bsat = 0; E.hasPso = false; setPose(0, PO_REST); setPose(1, PO_REST);
}
static float speedNorm(float iv) { return clampf(logf(650.0f / iv) / logf(650.0f / 33.0f), 0, 1); }
static void birthStep(float dt) {
  float ms = (St - B.t0) * 1000.0f; B.dt = dt;
  if (ms >= BDUR) { birthVar = -1; cr.c1 = B.fc1; cr.c2 = B.fc2; cr.two = B.ftwo; cr.pd = B.fpd; bsat = -1; E.bx = E.by = 0; E.hasPso = false; Tt[K_PUP] = 1; setExpr(X_AWAKE, true); return; }
  if (ms < PRE_MS) { E.bx = E.by = 0; return; }
  float r = ms - PRE_MS;
  if (r < RU_T1) {
    float x = r / RU_T1;
    Tt[K_PUP] = 1 - .7f * (1 - sstepf(4500, 9000, r)); bsat = powf(x, .5f);
    float itv; if (x < .8f) itv = 650.0f * powf(33.0f / 650.0f, x / .8f); else if (x < .9f) itv = 33; else itv = 33.0f * powf(420.0f / 33.0f, (x - .9f) / .1f);
    float sn = speedNorm(itv);
    if (r >= B.nextSwap) {
      float jit = .8f + .4f * frand01(); if (x > .15f && x < .75f && frand01() < .04f) jit = 2.2f;
      B.swapDur = fmaxf(33.0f, itv * jit); B.lastSwap = r; B.nextSwap = r + B.swapDur;
      if (!B.cDone) { float cf = clampf(x / .5f, 0, 1), va = .06f + .5f * cf; B.cRx = 34 * (1 + va * frand()); B.cRy = 55 * (1 + .7f * va * frand()); B.cGap = 38 * (1 + .35f * va * frand()); B.cBump = 1 - cf * .9f; B.cSat = powf(x, .5f); B.cSf = clampf((cf - .55f) * 2.2f, 0, 1); B.cPA = 1 + (int)(frand01() * 6.99f); B.cPB = 1 + (int)(frand01() * 6.99f); if (x >= .5f) B.cDone = true; }
      C3 bp = rndAll(); if (x > .3f) { int tr = 1 + (int)(3 * x); for (int q = 0; q < tr; q++) { C3 c = rndAll(); if (satOf(c) > satOf(bp)) bp = c; } }
      cr.c1 = bp; C3 b2 = rndAll(); if (x > .3f) { for (int q = 0; q < 1 + (int)(3 * x); q++) { C3 c = rndAll(); if (satOf(c) > satOf(b2)) b2 = c; } } cr.c2 = b2; cr.two = frand01() < .4f; B.cCol = bp;
      E.pso = frand() > 0 ? 0 : 1; E.hasPso = true;
      float sf = clampf(.3f + 1.1f * x, 0, 1);
      for (int i = 0; i < 2; i++) { int pz = 1 + (int)(frand01() * 6.99f); for (int k = 0; k < 12; k++) { float v = PZT[0][k] + (PZT[pz][k] - PZT[0][k]) * sf; eyes[i].p[k] = v; eyes[i].tgt[k] = v; eyes[i].v[k] = 0; } }
    }
    float tc = clampf((r - B.lastSwap) / B.swapDur, 0, 1), e2 = (1 - tc) * (1 - tc), A = x < .9f ? 55 - 30 * clampf(x / .8f, 0, 1) : 25 + 40 * ((x - .9f) / .1f);
    E.bx = -A * e2; B.cOffX = -A * e2; float tens = x < .9f ? x * x : 0; E.by = B.cOffY = (1.5f + 2.5f * tens) * sinf(TAU * 2 * r / 1000.0f);
    float rate = x < .93f ? 2.5f + 55 * sn : 0; B.streak += rate * dt;
    while (B.streak >= 1) { B.streak -= 1; Part q = {}; q.k = 3; q.x = -30 + 40 * frand01(); q.y = 56 + 128 * frand01(); q.vx = 1900 + 2300 * frand01() * (.4f + sn); q.vy = 0; q.l = .13f + .12f * frand01(); q.len = 16 + 36 * frand01() * (.4f + sn); q.w = 1.6f; q.c = frand01() < .65f ? C3{255, 255, 255} : cr.c1; addPart(q); }
  } else {
    float m = r - RU_T1; E.bx = E.by = 0;
    const float OPEN_AT = 4000;
    if (B.cs < 3) Tt[K_LID] = .9f * sstepf(0, 2200, m);                      // los parpados bajan despacio, sin saltos
    if (m > 0 && B.cs < 1) { B.cs = 1; Tt[K_PUP] = .85f; Tt[K_CD] = .7f; Tt[K_CI] = .25f; E.look = L_DOWN; }
    if (m > 2200 && B.cs < 3) { B.cs = 3; B.lm = .9f; Tt[K_ARC] = 1; Tt[K_DIR] = 1; }     // el ojo se deforma hasta ser trazo
    if (m >= OPEN_AT && B.cs < 4) { B.cs = 4; Tt[K_ARC] = 0; B.lm = .75f; Tt[K_PUP] = .9f; popv += 1.5f; }
    if (m > OPEN_AT + 600 && B.cs < 5) { B.cs = 5; B.lm = .5f; }
    if (m > OPEN_AT + 1250 && B.cs < 6) { B.cs = 6; B.lm = .25f; Tt[K_PUP] = 1; Tt[K_CD] = 1; Tt[K_CI] = 0; }
    if (m > OPEN_AT + 1900 && B.cs < 7) { B.cs = 7; B.lm = 0; popv += 2.5f; }
    if (B.cs >= 3) Tt[K_LID] = B.lm * (1 - clampf(Vv[K_ARC], 0, 1));         // el parpado se va junto con el trazo
    if (m > 3400 && !B.restored) { B.restored = true; cr.c1 = B.fc1; cr.c2 = B.fc2; cr.two = B.ftwo; cr.pd = B.fpd; E.hasPso = false; bsat = -1; setPose(0, PO_REST); setPose(1, PO_REST); for (int i = 0; i < 2; i++) for (int k = 0; k < 12; k++) { eyes[i].p[k] = eyes[i].tgt[k]; eyes[i].v[k] = 0; } }
    float mo = m - (OPEN_AT + 150); static const float ST[5] = {0, 70, 150, 250, 370};
    while (B.stage < 5 && mo >= ST[B.stage]) { int n = 2 + (int)(frand01() * 1.99f); for (int q = 0; q < n; q++) { float a = TAU * frand01(), v = 110 + 120 * frand01(); Part pp = {}; pp.k = 3; pp.x = 120 + cosf(a) * (58 + 8 * frand01()); pp.y = 118 + sinf(a) * (54 + 8 * frand01()); pp.vx = cosf(a) * v; pp.vy = sinf(a) * v; pp.l = .3f + .25f * frand01(); pp.len = 7 + 15 * frand01(); pp.w = 2.2f; pp.c = frand01() < .6f ? C3{255, 255, 255} : B.fc1; addPart(pp); } B.stage++; }
    if (m >= OPEN_AT + 600 && !B.looked) {   // al abrir mira alrededor segun su personalidad
      B.looked = true; E.look = L_FREE;
      float w[5] = {.05f + 1.6f * cr.curious * cr.curious, .05f + 1.6f * cr.shy * cr.shy, .05f + 1.6f * (cr.energy - .5f) * (cr.energy - .5f), .05f + 1.6f * cr.sleepy * cr.sleepy, .05f + 1.2f * cr.grumpy * cr.grumpy};
      float tot = w[0] + w[1] + w[2] + w[3] + w[4], rr = frand01() * tot; int k = 0; while (k < 4 && rr > w[k]) { rr -= w[k]; k++; }
      static const float PL[5][6][2] = {{{-1, -.2f}, {1, -.3f}, {.2f, -1}, {-.7f, .4f}, {.8f, .3f}, {0, 0}}, {{-.5f, .7f}, {.4f, .6f}, {-.3f, .8f}, {0, .2f}, {0, 0}, {0, 0}}, {{-1, 0}, {1, .1f}, {-.8f, -.6f}, {.9f, -.5f}, {-.4f, .5f}, {0, 0}}, {{-.5f, .3f}, {.5f, .2f}, {0, .5f}, {0, 0}, {0, 0}, {0, 0}}, {{-1, .2f}, {1, .2f}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}};
      static const int PN[5] = {6, 4, 6, 4, 3}; static const float PS[5] = {650, 800, 450, 1100, 800};
      float sg = frand() > 0 ? 1.0f : -1.0f; B.planN = PN[k]; B.pi = 0;
      for (int q = 0; q < 6; q++) { B.planT[q] = m + q * PS[k]; B.planX[q] = PL[k][q][0] * sg * 14; B.planY[q] = PL[k][q][1] * 9; }
    }
    if (B.planN && B.pi < B.planN && m >= B.planT[B.pi]) { look(B.planX[B.pi], B.planY[B.pi]); B.pi++; }
  }
}

static void blobAA(float cx, float cy, float rx, float ry, float t, float seed, uint16_t col, float bump = 1.0f) {
  const int N = 7;
  float w0 = 1 + 0.06f * bump * sinf(1.1f * t + seed), sc = 0.80f + 0.20f * (1 - bump);
  ellipseAA(cx, cy, rx * sc * w0, ry * sc * w0, col);
  float rho = 0.46f * fminf(rx, ry) * bump;
  if (rho < 0.8f) return;
  for (int k = 0; k < N; k++) {
    float a = TAU * k / N + 0.30f * bump * sinf(0.8f * t + k * 1.7f + seed);
    float w = 1 + bump * (0.13f * sinf(1.3f * t + 2.1f * k + seed) + 0.07f * sinf(2.9f * t + 1.3f * k));
    discAA(cx + cosf(a) * rx * 0.58f * w, cy + sinf(a) * ry * 0.58f * w, rho * (0.85f + 0.2f * sinf(1.7f * t + k)), col);
  }
}
static void fillAll(uint16_t col) { uint32_t v = (uint32_t)col | ((uint32_t)col << 16); uint32_t* q = (uint32_t*)fb; for (int i = 0; i < W * H / 2; i++) q[i] = v; markRows(0, H - 1); }

// ---- relleno de poligono simple (para las manchas viscosas) ----
static void fillPoly(const float* X, const float* Y, int n, uint16_t col) {
  float xmin = 1e9f, xmax = -1e9f, ymin = 1e9f, ymax = -1e9f;
  for (int i = 0; i < n; i++) { xmin = fminf(xmin, X[i]); xmax = fmaxf(xmax, X[i]); ymin = fminf(ymin, Y[i]); ymax = fmaxf(ymax, Y[i]); }
  int y0 = max(0, (int)floorf(ymin) - 1), y1 = min(H - 1, (int)ceilf(ymax) + 1), x0 = max(0, (int)floorf(xmin) - 1), x1 = min(W - 1, (int)ceilf(xmax) + 1);
  markRows(y0, y1);
  for (int x = x0; x <= x1; x++) covRow[x] = 0;
  for (int y = y0; y <= y1; y++) {
    bool any = false;
    for (int sub = 0; sub < 2; sub++) {
      float sy = y + 0.25f + 0.5f * sub, xs[16]; int c = 0;
      for (int i = 0, j = n - 1; i < n; j = i++) { float ya = Y[j], yb = Y[i]; if ((ya <= sy) != (yb <= sy)) { float xx = X[j] + (sy - ya) / (yb - ya) * (X[i] - X[j]); if (c < 16) { int q = c++; while (q > 0 && xs[q - 1] > xx) { xs[q] = xs[q - 1]; q--; } xs[q] = xx; } } }
      for (int q = 0; q + 1 < c; q += 2) { float xa = clampf(xs[q], 0, W - 1), xb = clampf(xs[q + 1], 0, W - 1); for (int ix = (int)xa; ix <= (int)xb && ix < W; ix++) { float l = fmaxf(xa, (float)ix), r = fminf(xb, (float)ix + 1); if (r > l) { covRow[ix] += 0.5f * (r - l); any = true; } } }
    }
    if (!any) continue;
    uint16_t* row = fb + y * W;
    for (int x = x0; x <= x1; x++) { float a = covRow[x]; if (a <= 0) continue; covRow[x] = 0; if (a > 1) a = 1; int ai = (int)(a * (float)gAlpha + 0.5f); if (ai <= 0) continue; row[x] = ai >= 32 ? col : blend565(row[x], col, ai); }
  }
}
static void viscBlob(float cx, float cy, float rx, float ry, float t, float sd, uint16_t col, float spin = 0) {   // mancha viscosa: onda y gira sobre su eje
  rx = fmaxf(.8f, rx); ry = fmaxf(.8f, ry);
  const int N = 56; float X[N], Y[N], rot = .28f * sinf(.8f * t + sd * 1.7f) + spin, c = cosf(rot), s_ = sinf(rot);
  for (int i = 0; i < N; i++) {
    float a = TAU * i / N, r = 1 + .11f * sinf(2 * a + 1.9f * t + sd) + .08f * sinf(3 * a - 2.6f * t + sd * 2) + .05f * sinf(5 * a + 3.4f * t + sd * 3) + .03f * sinf(7 * a - 4.2f * t);
    float x = cosf(a) * rx * r, y = sinf(a) * ry * r; X[i] = cx + x * c - y * s_; Y[i] = cy + x * s_ + y * c;
  }
  fillPoly(X, Y, N, col);
}
// ---- dibujo del nacimiento ----
static void drawBirth(float ms, float dt) {
  float t = ms * 0.001f;
  uint16_t cream = rgb565(250, 226, 216), white = 0xFFFF, lite = rgb565(238, 240, 250);
  setAlpha(1);
  if (ms >= PRE_MS) {                                       // ojos de nube que aceleran, cambian de forma y toman color
    B.spin += dt * (1.0f + 5.5f * clampf((ms - PRE_MS) / RU_T1 / .5f, 0, 1));
    float x = clampf((ms - PRE_MS) / RU_T1, 0, 1), tw = t * (1.8f + 3 * x);
    uint16_t tint = c565(mix3({250, 226, 216}, B.cCol, B.cSat));
    for (int lobe = 0; lobe < 2; lobe++) {
      float cx = 120 + (lobe ? B.cGap : -B.cGap) + B.cOffX, cy = 122 + B.cOffY, sd = lobe * 2.3f;
      if (B.cSf > .04f) {
        EDraw Q; memset(&Q, 0, sizeof(Q)); int pz = lobe ? B.cPB : B.cPA;
        for (int k = 0; k < 12; k++) Q.p[k] = PZT[0][k] + (PZT[pz][k] - PZT[0][k]) * B.cSf;
        Q.cx = cx; Q.cy = cy; Q.flip = (cr.mir && lobe) ? -1.0f : 1.0f; Q.sstr = 1; Q.t = tw; Q.id = (float)lobe; Q.sx = B.cRx / 40 * .95f; Q.sy = B.cRy / 46 * .95f; Q.cEye = tint;
        renderEye(Q);
      } else viscBlob(cx, cy, B.cRx, B.cRy, tw, sd, tint, B.spin * (lobe ? -1.0f : 1.0f));
    }
    return;
  }
  float tb = ms - (PR_W + PR_R), x = clampf(ms / PR_W, 0, 1);
  if (ms < PR_W) {
    float gg = clampf(ms / PR_W, 0, 1); B.dotR = 3 + 23 * powf(gg, 1.5f); float dotR = B.dotR; const int NS = 26;
    float w = 1 + 33 * x * x, fv = .5f + 13 * x * x;
    for (int a = 0; a < 16; a++) {
      Thr& h = B.thr[a]; h.ph += w * h.spd * dt; h.fl += fv * h.spd * dt;
      float ro = dotR + (108 - dotR) * powf(fmaxf(0, 1 - x * h.rate), .9f) * h.len; if (ro < dotR + 1.5f) continue;
      float pin = fmodf(h.df - h.fl, 1.0f); if (pin < 0) pin += 1;
      if (pin > h.pin + .5f) { float ai = h.a0 + h.ph + h.k + h.wa * sinf(TAU * (h.wf - .35f * t) + h.wp), mg = 2.4f * (1 - .65f * x) * h.al; B.bulX += cosf(ai) * mg; B.bulY += sinf(ai) * mg; B.feed += .055f * (1 - .5f * x); }
      h.pin = pin; float px = 0, py = 0;
      for (int q = 0; q <= NS; q++) {
        float u = (float)q / NS, rr = dotR + (ro - dotR) * powf(1 - u, 1.3f), ang = h.a0 + h.ph + h.k * powf(u, .8f) + h.wa * sinf(TAU * (h.wf * u - .35f * t) + h.wp), xx = 120 + rr * cosf(ang), yy = 118 + rr * sinf(ang);
        if (q > 0) { float um = u - .5f / NS, ph2 = fmodf(um * h.df - h.fl, 1.0f); if (ph2 < 0) ph2 += 1; if (ph2 < .6f) { setAlpha(1); lineAA(px, py, xx, yy, h.th * (.55f + .75f * u) * 1.05f, white); } }
        px = xx; py = yy;
      }
    }
    B.spasm *= expf(-dt * 8); B.bulX *= expf(-dt * 7); B.bulY *= expf(-dt * 7); B.feed *= expf(-dt * 6);
    B.bulX = clampf(B.bulX, -7, 7); B.bulY = clampf(B.bulY, -7, 7); B.feed = fminf(B.feed, .45f);
    if (frand01() < dt * (.8f + 10 * x * x)) B.spasm += (.35f + .45f * frand01()) * (frand() > 0 ? 1.0f : -.6f);
    float rr = dotR * (1 + .10f * sinf(TAU * (2.5f + 8 * x) * t)) * (1 + B.feed), sxs = 1 + .45f * B.spasm + .05f * fabsf(B.bulX), sys = 1 - .30f * B.spasm + .05f * fabsf(B.bulY);
    setAlpha(1); blobAA(120 + B.bulX + 2 * B.spasm * sinf(t * 40), 118 + B.bulY + 1.6f * B.spasm * cosf(t * 33), rr * sxs, rr * sys, t * (2.5f + 7 * x), .7f, white);
  } else if (ms < PR_W + PR_R) {
    float u = (ms - PR_W) / PR_R, dotR = B.dotR, Rr_ = dotR + 175 * powf(u, .55f);
    setAlpha((1 - u) * (1 - u) * .95f); float lx = 0, ly = 0;
    for (int q = 0; q <= 40; q++) { float a = TAU * q / 40, rg = Rr_ * (1 + .07f * sinf(3 * a + 1.7f + 6 * u) + .05f * sinf(5 * a + 4.1f - 4 * u)), xx = 120 + cosf(a) * rg, yy = 118 + sinf(a) * rg; if (q > 0) lineAA(lx, ly, xx, yy, 2.8f, white); lx = xx; ly = yy; }
    for (int q = 0; q < 20; q++) {
      float h1 = sinf(q * 12.9898f) * 43758.5453f; h1 -= floorf(h1); float h2 = sinf(q * 78.233f) * 12345.678f; h2 -= floorf(h2);
      float a = TAU * q / 20 + (h1 - .5f) * .35f, dl = h2 * .25f; if (u <= dl) continue;
      float r0 = Rr_ * (.15f + .35f * h1), r1 = Rr_ * (.55f + .38f * h2); setAlpha((1 - u) * (.55f + .40f * h2));
      lineAA(120 + cosf(a) * r0, 118 + sinf(a) * r0, 120 + cosf(a) * r1, 118 + sinf(a) * r1, 1.5f + 1.8f * h1, white);
    }
    setAlpha(1); if (u < .6f) blobAA(120, 118, dotR * (1 - u / .6f) + 1, dotR * (1 - u / .6f) + 1, t * 9, .7f, white);
    blobAA(120, 118, 2 + 6 * u, 2 + 6 * u, t * 1.8f, .5f, white);
  } else {                                                    // manchas viscosas: nacen de un punto, salen y crecen en su sitio
    float tw = t * 1.8f; const float CX = 120, CY = 118, fin = 34, fiy = 55;
    float p = sstepf(500, 1700, tb), k = growF(clampf((tb - 1400) / 2800.0f, 0, 1)), br = tb > 4200 ? 2.5f * sinf(TAU * 1.8f * t) : 0;
    uint16_t cream = rgb565(250, 226, 216);
    if (tb < 500) { float r = 3 + 7 * sstepf(0, 500, tb); viscBlob(CX, CY, r, r, tw, 0, cream); }
    else for (int l = 0; l < 2; l++) { float sd = l ? 1.0f : -1.0f, x = CX + sd * 38 * p, y = CY + 4 * p + sinf(t * 1.3f + l * 2) * 1.5f * p; viscBlob(x, y, 10 + (fin - 10) * k + 2 * (1 - p), 12 + (fiy - 12) * k + br + 2 * (1 - p), tw, l * 2.3f, cream); }
  }
  setAlpha(1);
}

// ---------------- paso del motor ----------------
static void runEv(const Ev& v) {
  switch (v.code) {
    case EV_OT: eyes[v.a].ot = v.f; break;
    case EV_LOOK: look(v.f, v.g); break;
    case EV_TG: Tt[v.a] = v.f; break;
    case EV_POSE: setPose(v.a, (int)v.f); break;
    case EV_POP: popv += v.f; break;
    case EV_SZ: eyes[0].szT = eyes[1].szT = 1; break;
    case EV_SETTLE: setPose(0, PO_REST); sched(Rr(0.04f, 0.15f), EV_POSE, 1, PO_REST); popv += 1.0f; break;
    case EV_BLINK: blink(); break;
    case EV_EXPR: setExpr(v.a, true); break;
    case EV_PUFF: if (curX == X_SMUG && (int)v.f == 1) { static float sg = 1; sg = -sg; Tt[K_TILT] = sg * .28f; popv += 2.8f; eyes[frand() > 0 ? 0 : 1].szT = 1.2f; sched(0.7f, EV_SZ); sched(1.4f, EV_PUFF, 0, 1, 0); } break;
    case EV_BEAT: if (curX == X_HYPE) { static bool on = false; on = !on; goPose(on ? PO_TORRE : PO_AGACHADO, on ? PO_AGACHADO : PO_TORRE, 0, false); sched(0.52f, EV_BEAT, 1, 0, 0); } break;
    case EV_SLOWEND: look(0, 0); eyes[0].ot = 1; sched(0.09f, EV_OT, 1, 1, 0, true); popv += 2; break;
  }
}
static void slowBlink() {
  if (curX != X_AWAKE && curX != X_CURIOUS) return;
  E.u = St + 2.7f; eyes[0].ot = 0; sched(0.07f, EV_OT, 1, 0, 0, true);
  sched(0.6f, EV_LOOK, 0, Rr(-14, 14), Rr(-7, 7)); sched(1.2f, EV_LOOK, 0, Rr(-14, 14), Rr(-7, 7)); sched(1.8f, EV_LOOK, 0, Rr(-14, 14), Rr(-7, 7)); sched(2.3f, EV_SLOWEND, 0, 0, 0, true);
}
static void lookStep(float t) {
  switch (E.look) {
    case L_NORMAL: if (t > E.sc && t > E.u) { frand01() < .3f ? look(0, 0) : look(Rr(-15, 15), Rr(-9, 9)); E.sc = t + (1.5f + frand01() * 2.5f) / (.6f + cr.curious * .6f); } break;
    case L_FAST: if (t > E.sc) { look(Rr(-10, 10), Rr(-6, 6)); E.sc = t + Rr(.25f, .7f); } break;
    case L_STILL: look(0, 0); break;
    case L_DOWN: look(sinf(t * .7f) * 3, 7); break;
    case L_UP: look(sinf(t * .6f) * 6, -8); break;
    case L_CIRCLE: look(cosf(t * 3.2f) * 11, sinf(t * 3.2f) * 8); break;
    case L_SIDE: look(E.sideX, -4); break;
    case L_SMUG: look(sinf(t * .6f) * 11, -3 + sinf(t * 1.3f) * 2); break;
    default: break;
  }
}
static void startExprFromEv(int x) { setExpr(x); }
static void sleepTick(float t);

static void stepEngine(float dt) {
  St += dt; float t = St;
  if (simBatt) { simT += dt; float c = fmodf(simT, 240.0f); energy = c < 180 ? 100 - 95 * c / 180 : 5; }
  for (int i = 0; i < 48; i++) if (evq[i].on && t >= evq[i].at) { Ev v = evq[i]; evq[i].on = false; runEv(v); }
  if (birthVar >= 0) birthStep(dt);
  // —— regreso a "despierto" y comportamiento espontaneo (en el estudio era el modo automatico)
  bool idleX = curX == X_AWAKE;
  if (birthVar < 0) {
    if (E.meter == 1) { Vcharge = fminf(1, Vcharge + dt * .075f); }
    float dur = XDUR[curX];
    if (curX == X_CHARGING) { if (Vcharge >= 1 && t - E.ex > 14) setExpr(X_HAPPY); }
    else if (curX == X_ELECTRIC && E.usb) { if (t - E.ex > 1.9f) setExpr(X_CHARGING); }
    else if (E.usb && curX == X_AWAKE) { setExpr(X_ELECTRIC); }
    else if (curX == X_SLEEP || curX == X_DIMSLEEP) { /* duerme hasta que algo lo despierte */ }
    else if (!idleX && E.holdPet <= 0 && t - E.ex > dur) {
      if (curX == X_YAWN && E.wasSleepy) { E.wasSleepy = false; setExpr(X_SLEEP); } else { if (curX == X_HAPPY && E.usb) E.usb = false; setExpr(X_AWAKE); }
    }
    if (E.holdPet > 0 && curX == X_LOVE) { /* mientras acaricia se queda en amor */ }
    else if (E.holdPet > 0) E.holdPet = 0;
    if (idleX) {
      if (t > E.sleepPlan && t - lastActivityS > 100) { E.wasSleepy = true; setExpr(X_YAWN); }
      else if (t > E.fl + 0) {}
      static float nextSpont = 25;
      if (t > nextSpont) {
        nextSpont = t + 24 + frand01() * 22 - cr.energy * 4;
        float r = frand01(); int pick = r < .13f ? X_CURIOUS : r < .26f ? X_HAPPY : r < .36f ? X_SMUG : r < .46f ? X_WINK : r < .55f ? X_HYPE : r < .63f ? X_SURPRISE : r < .70f ? X_ANXIOUS : r < .78f ? X_DIZZY : r < .86f ? X_LOVE : r < .93f ? X_PANT : X_YAWN;
        if (t - lastActivityS > 35 && frand01() < .4f) pick = X_YAWN;
        if (pick == X_YAWN) E.wasSleepy = false;
        setExpr(pick);
      }
    }
  }
  if (curX == X_BATCHECK) { /* el nivel es la bateria */ }
  if (E.blinkOK && t > E.b && t > E.u) { blink(); if (frand01() < .2f) sched(.33f, EV_BLINK); E.b = t + (E.slowB ? Rr(4, 7) : (2 + frand01() * 3.4f) / (.6f + cr.energy * .4f)); }
  if (curX == X_AWAKE || curX == X_CURIOUS) {
    if (curX == X_AWAKE && t > E.w && t > E.u) { blinkEye(frand() > 0 ? 0 : 1, 0); E.w = t + 9 + frand01() * 10; }
    if (curX == X_AWAKE && t > E.sz && t > E.u) { int i = frand() > 0 ? 0 : 1; static const float SZ[3] = {.8f, 1.18f, 1.25f}; eyes[i].szT = SZ[(int)(frand01() * 2.99f)]; eyes[1 - i].szT = 1; sched(Rr(1.5f, 3.0f), EV_SZ); E.sz = t + 7 + frand01() * 8; }
    if (t > E.sl && t > E.u) { slowBlink(); E.sl = t + (34 - cr.sleepy * 14) + frand01() * 14; }
    if (t > E.fl && t > E.u) { int a = FLOUR[(int)(frand01() * 5.99f)], b = FLOUR[(int)(frand01() * 5.99f)]; if (cr.grumpy > .6f) { a = FLOUR[1 + (int)(frand01() * 2.99f) % 3]; b = a; } if (frand01() < .5f) { genExoPair(); goPose(PO_XA, PO_XA + 1, Rr(.04f, .2f)); } else goPose(a, b, Rr(.04f, .2f)); settle(Rr(1000, 1900)); E.fl = t + (3.4f + frand01() * 3.2f) / cr.energy; }
  }
  for (int i = 0; i < 2; i++) { static float closedT[2]; if (eyes[i].ot < .5f && birthVar < 0 && t > E.u) closedT[i] += dt; else closedT[i] = 0; if (closedT[i] > 1.4f) { eyes[i].ot = 1; closedT[i] = 0; } }   // red de seguridad: nunca se queda cerrado
  lookStep(t);
  for (int k = 0; k < NK; k++) Vv[k] += (Tt[k] - Vv[k]) * fminf(1.0f, dt * (k == K_ARC ? (birthVar >= 0 ? 2.6f : 6.0f) : 9.0f));
  Vcm = mix3(Vcm, Tcol, fminf(1.0f, dt * 8));
  popv += ((Tt[K_SCALE] - pop) * 170 - popv * 10) * dt; pop += popv * dt;
  for (int i = 0; i < 2; i++) {
    SEye& e = eyes[i]; float out = i ? 1.0f : -1.0f; (void)out;
    e.gvx += ((gx_t - e.gx) * e.k * 1.25f - e.gvx * e.d) * dt; e.gx += e.gvx * dt; e.gvy += ((gy_t - e.gy) * e.k * 1.25f - e.gvy * e.d) * dt; e.gy += e.gvy * dt;
    e.jn -= dt; if (e.jn < 0) { e.jx = Rr(-3, 3); e.jy = Rr(-2.2f, 2.2f); e.jn = Rr(.35f, 1.3f); }
    float sl = Vv[K_SLOSH], wx = sinf(t * (.9f + i * .37f) + e.ph) * 1.6f + cosf(t * 3.2f + e.ph) * sl * 7, wy = cosf(t * (1.1f + i * .29f) + e.ph) * 1.2f + sinf(t * 3.2f + e.ph * 2) * sl * 6;
    float tx = gx_t * .82f + e.jx + wx, ty = gy_t * .7f + e.jy + wy;
    e.pvx += ((tx - e.px) * e.kp - e.pvx * 11) * dt; e.px += e.pvx * dt; e.pvy += ((ty - e.py) * e.kp - e.pvy * 11) * dt; e.py += e.pvy * dt;
    e.szv += ((e.szT * Vv[K_PUP] * (1 + .07f * sinf(t * 1.3f + e.ph)) - e.sz) * 130 - e.szv * 10) * dt; e.sz += e.szv * dt;
    for (int k = 0; k < 12; k++) { e.v[k] += ((e.tgt[k] - e.p[k]) * e.k - e.v[k] * e.d) * dt; e.p[k] += e.v[k] * dt; }
    e.ov += ((e.ot - e.o) * 520 - e.ov * 28) * dt; e.o += e.ov * dt;
  }
  if (E.fb && t > E.fbn) { int i = (int)(frand01() * 10.99f); static int last = -1; if (i == last) i = (i + 1) % 11; last = i; fbBg = hexc(FBP[i][0]); fbEye = hexc(FBP[i][1]); fbPup = hexc(FBP[i][2]); E.fbn = t + Rr(2, 3.2f); popv += 3.5f; }
  if (E.part && t > E.pn) {
    E.pn = t + (E.part == 1 ? Rr(.18f, .38f) : Rr(1.1f, 1.8f));
    Part q = {};
    if (E.part == 1) { q.k = 1; q.x = frand() > 0 ? Rr(12, 52) : Rr(188, 228); q.y = Rr(120, 190); q.vx = Rr(-8, 8); q.vy = Rr(-38, -22); q.l = Rr(1.6f, 2.4f); q.s = Rr(9, 25); }
    else { int i = frand() > 0 ? 0 : 1; q.k = 2; q.x = 120 + (i ? 1 : -1) * cr.gap / 2 + Rr(-8, 8); q.y = 170; q.vy = 10; q.l = 2.2f; q.s = 6; }
    addPart(q);
  }
  for (int i = 0; i < NPARTS; i++) if (parts[i].on) { Part& q = parts[i]; q.a += dt; q.x += q.vx * dt; q.y += q.vy * dt; if (q.k == 2) q.vy += 90 * dt; if (q.a >= q.l) q.on = false; }
}

// ---------------- dibujo de un cuadro ----------------
static C3 batCol(float v) {
  static const float BS[5][4] = {{0, 236, 64, 64}, {.30f, 244, 128, 56}, {.55f, 240, 196, 70}, {.78f, 150, 214, 96}, {1, 72, 208, 138}};
  v = clampf(v, 0, 1); int i = 1; while (i < 4 && v > BS[i][0]) i++;
  float k = clampf((v - BS[i - 1][0]) / (BS[i][0] - BS[i - 1][0]), 0, 1);
  return {BS[i - 1][1] + (BS[i][1] - BS[i - 1][1]) * k, BS[i - 1][2] + (BS[i][2] - BS[i - 1][2]) * k, BS[i - 1][3] + (BS[i][3] - BS[i - 1][3]) * k};
}
static void closedEye(float cx, float cy, float rot, float flip, float w, float dir, C3 col, float br, float k, float lo, float t) {
  uint16_t c = c565(col); float dep = fmaxf(9.0f, 14.0f * br * fminf(1.1f, k)), cd = Vv[K_CD], ci = Vv[K_CI], kk = fminf(1.1f, k * .95f);
  const int N = 24; float px[N + 1], py[N + 1];
  for (int i = 0; i <= N; i++) {
    float u = -1 + 2.0f * i / N, x = u * w, y = dir * dep * cd * (1 - u * u) + 2 + ci * lo * x * .5f;
    toScreen(cx, cy, rot, flip, x, y, px[i], py[i]);
    float th = (7 + 8 * powf(sinf(3.14159f * (.5f + .5f * u)), .7f)) * kk;
    discAA(px[i], py[i], th * .5f, c);
  }
  if (Vv[K_SCR] > .5f) for (int q = -1; q <= 1; q++) { float x0 = lo * (w + 4), y0 = dir * dep * cd * 0 + 2 + ci * lo * w * .5f + q * 5, ax, ay, bx, by; toScreen(cx, cy, rot, flip, x0, y0, ax, ay); toScreen(cx, cy, rot, flip, x0 + lo * 9, y0 + q * 5 + 3, bx, by); lineAA(ax, ay, bx, by, 2.8f, c); }
}

static void renderStudio() {
  float t = St;
  if (birthVar >= 0) { float ms = (St - B.t0) * 1000; if (ms < PRE_MS || (!B.cDone && ms - PRE_MS < RU_T1)) { drawBirth(ms, B.dt); for (int i = 0; i < NPARTS; i++) if (parts[i].on && parts[i].k == 3) { Part& q = parts[i]; float u = q.a / q.l, L = sqrtf(q.vx * q.vx + q.vy * q.vy) + 1e-3f, nx = q.vx / L, ny = q.vy / L; setAlpha(fmaxf(0, 1 - u)); lineAA(q.x - nx * q.len, q.y - ny * q.len, q.x, q.y, q.w, c565(q.c)); setAlpha(1); } return; } }
  if (E.fb) fillAll(c565(fbBg));
  float lv = E.meter == 1 ? Vcharge : clampf(energy / 100.0f, .02f, 1);
  float bob = sinf(t * 1.7f) * 1.4f + (Vv[K_BREATH] > .5f ? sinf(t * 1.2f) * 2.2f : 0), ox = sinf(t * .6f) * 1.4f;
  for (int side = 0; side < 2; side++) {
    SEye& e = eyes[side]; float out = side ? 1.0f : -1.0f, lo = (cr.mir && side) ? -1.0f : out, flip = (cr.mir && side) ? -1.0f : 1.0f;
    float o = clampf(e.o, 0, 1.1f), arcK = clampf(Vv[K_ARC] * 2, 0, 1), f = (.07f + .93f * o) * (1 + sinf(t * 1.6f + e.ph) * .012f);
    C3 base = (side && cr.two) ? cr.c2 : cr.c1, col = mix3(base, Vcm, Vv[K_CMIX]);
    if (E.fb) col = fbEye;
    if (E.flash && fabsf(sinf(t * 9)) > .85f) col = {255, 255, 255};
    if (E.meter == 2 || E.meter == 1) col = E.meter == 2 ? batCol(lv) : col;
    col = mix3(col, {0, 0, 0}, Vv[K_DIM]);
    C3 colRaw = col; if (bsat >= 0) col = mix3({244, 244, 248}, col, bsat);
    C3 pc = E.fb ? fbPup : mix3(colRaw, {0, 0, 0}, .86f); if (bsat >= 0) pc = mix3({228, 228, 238}, pc, bsat);
    float sp = hypotf(e.gvx, e.gvy), ang = atan2f(e.gvy, e.gvx), str = sp > 2 ? 1 + fminf(.25f, sp * .006f) : 1.0f, k = pop * cr.scale * e.gene;
    float jx = sinf(t * 61 + e.ph) * Vv[K_JIT] * 2.4f, jy = cosf(t * 47 + e.ph) * Vv[K_JIT] * 1.6f, bo = -fabsf(sinf(t * 6 + side * .4f)) * Vv[K_BOUNCE] * 11;
    float sx = cosf(t * 3 + e.ph) * Vv[K_SLOSH] * 20, sy2 = sinf(t * 2.3f + e.ph * 2) * Vv[K_SLOSH] * 10;
    float cx = 120 + ox + out * cr.gap / 2 * (1 + (pop - 1) * .3f) + e.gx * .9f + jx + sx + E.bx;
    float cy = 134 + bob + e.gy * .9f + jy + bo + sy2 + E.by - Vv[K_LIFT] * 9 + (1 - f) * e.p[1] * .3f;
    float rot = (e.p[11] + Vv[K_TILT] * lo * -14 + Vv[K_EUPH] * sinf(t * 7 + e.ph) * 5) * 0.0174533f;
    EDraw D; memset(&D, 0, sizeof(D));
    for (int q = 0; q < 12; q++) D.p[q] = e.p[q];
    D.cx = cx; D.cy = cy; D.rot = rot; D.flip = flip; D.sang = ang; D.sstr = str; D.t = t; D.id = (float)side; D.sx = k; D.sy = k * f;
    D.cEye = c565(col); D.cPup = c565(pc);
    float mm = clampf(Vv[K_ARC], 0, 1);
    if (mm > .003f) { float br = 1 + sinf(t * 1.2f + e.ph) * .14f * Vv[K_BREATH]; D.morph = mm; D.bw = fminf(e.p[0] * k * (1.22f + sinf(t * 1.2f + e.ph) * .03f * Vv[K_BREATH]), cr.gap / 2 * .84f - 3); D.bdep = fmaxf(9.0f, 14.0f * br * fminf(1.1f, k)); D.bdir = Vv[K_DIR]; D.bcd = Vv[K_CD]; D.bci = Vv[K_CI]; D.blo = lo; D.bkk = fminf(1.1f, k * .95f); }
    float lim = e.p[0] * .38f; float pxl = clampf((cr.pd * e.p[0] * .2f + e.px) * (cr.mir && side ? -1.0f : 1.0f), -lim, lim), pyl = e.p[1] * .12f + e.py - Vv[K_CHEEK] * e.p[1] * .28f;
    bool rd = cr.pstyle == 1 || (cr.pstyle == 2 && cr.pd * (side ? -1 : 1) > 0); if (E.hasPso) rd = E.pso == 1;
    float prx = rd ? e.p[0] * .42f : e.p[0] * .52f, pry = rd ? e.p[0] * .42f : e.p[1] * .6f;
    float pv = 1 - clampf(mm * 1.6f, 0, 1);
    D.hasPup = pv > .02f; D.ppx = pxl; D.ppy = pyl; D.prx = prx * e.sz * pv; D.pry = pry * e.sz * f * pv;
    if (Vv[K_HEART] > .5f) { D.heart = true; D.hasPup = false; D.hs = prx * e.sz * (1 + sinf(t * 7) * .08f) * 1.3f; }
    float H_ = e.p[1] * k * f;
    if (Vv[K_LID] > .01f) { float top = -e.p[1] * k * f * 1.4f, bot = e.p[1] * k * f * 1.2f; D.lid = true; D.lidY = top + (bot - top) * Vv[K_LID]; D.lidK = Vv[K_SLANT] * 20 * lo / 90.0f; }
    if (Vv[K_CHEEK] > .02f) { D.cheek = true; D.chCy = H_ * (1.6f - Vv[K_CHEEK]); D.chRx = e.p[0] * k * 1.6f; D.chRy = H_ * 1.05f; }
    if (E.meter) { D.meter = true; D.mTop = -e.p[1] * k * f * 1.3f; D.mBot = e.p[1] * k * f * 1.1f; D.mLv = lv; D.mT = t; D.cDark = c565(mix3(col, {0, 0, 0}, .8f)); D.hasPup = false; if (E.meter == 1) { D.cEye = c565(mix3(col, {0, 0, 0}, 0)); } }
    renderEye(D);
    if (Vv[K_ARC] > .9f && Vv[K_SCR] > .5f) {                       // arrugas del bostezo
      float wq = fminf(e.p[0] * k * 1.22f, cr.gap / 2 * .84f - 3); uint16_t cc = c565(col);
      for (int q = -1; q <= 1; q++) { float x0 = lo * (wq + 4), y0 = 2 + Vv[K_CI] * lo * wq * .5f + q * 5, ax, ay, bx, by; toScreen(cx, cy, rot, flip, x0, y0, ax, ay); toScreen(cx, cy, rot, flip, x0 + lo * 9, y0 + q * 5 + 3, bx, by); lineAA(ax, ay, bx, by, 2.8f, cc); }
    }
  }
  for (int i = 0; i < NPARTS; i++) if (parts[i].on) {
    Part& q = parts[i]; float u = q.a / q.l;
    if (q.k == 1) { float sc = q.s * (u < .15f ? (u / .15f) * 1.25f : 1 + sinf(q.a * 9) * .07f) * (u > .75f ? (1 - u) / .25f : 1.0f); setAlpha(1); heartAA(q.x, q.y, sc * 0.55f * 1.15f, sc * 0.55f, rgb565(255, 42, 69)); }
    else if (q.k == 2) { float qs = q.s * (u > .8f ? (1 - u) / .2f : 1.0f); ellipseAA(q.x, q.y, qs * .8f, qs * 1.4f, rgb565(111, 214, 255)); }
    else if (q.k == 3) { float L = sqrtf(q.vx * q.vx + q.vy * q.vy) + 1e-3f, nx = q.vx / L, ny = q.vy / L; setAlpha(fmaxf(0, 1 - u)); lineAA(q.x - nx * q.len, q.y - ny * q.len, q.x, q.y, q.w, c565(q.c)); setAlpha(1); }
  }
}

// ============================================================================================
//  API del core (la usan el .ino y el simulador)
// ============================================================================================
void wake(uint32_t) { lastActivityS = St; if (curX == X_SLEEP || curX == X_DIMSLEEP) setExpr(X_AWAKE); }
bool isAsleep() { return curX == X_SLEEP || curX == X_DIMSLEEP; }
// la identidad de la criatura sale SOLO de la semilla (la misma semilla guardada = la misma criatura, al nacer y al reiniciar)
static void creatureFromSeed(uint32_t s) {
  uint32_t keep = rng; rng = s * 2654435761u + 12345u;
  fresh(true);
  for (int i = 0; i < 2; i++) { SEye& e = eyes[i]; e.bk = e.k = i ? Rr(120, 145) : Rr(105, 125); e.bd = e.d = i ? Rr(8.5f, 10) : Rr(7.5f, 9); e.kp = i ? Rr(170, 215) : Rr(190, 240); e.gene = Rr(.9f, 1.1f); e.ph = Rr(0, 6.3f); }
  rng = keep ^ (s >> 3);
}
void evNewCreature(uint32_t now, uint32_t seed) { (void)now; creatureFromSeed(seed); setPose(0, PO_REST); setPose(1, PO_REST); }
void evBirth(uint32_t now, int v) { (void)now; (void)v; lastActivityS = St; for (int i = 0; i < NPARTS; i++) parts[i].on = false; setExpr(X_BIRTH, true); }
static bool busy() { return birthVar >= 0; }
void evSet(uint8_t x, uint32_t now, uint16_t dur) { (void)now; (void)dur; if (busy()) return; lastActivityS = St; E.usb = false; E.holdPet = 0; static const uint8_t M[9] = {X_AWAKE, X_CURIOUS, X_HAPPY, X_SAD, X_ANGRY, X_LOVE, X_SURPRISE, X_SMUG, X_ANXIOUS}; setExpr(M[(x ? x - 1 : 0) % 9]); }
void evNextExpr(uint32_t now) { (void)now; if (busy()) return; lastActivityS = St; E.usb = false; int n = curX + 1; if (n == X_BIRTH) n++; if (n >= NEXPR) n = 0; setExpr(n); }
void evTheme(uint32_t now) { (void)now; if (busy()) return; fresh(false); setPose(0, PO_REST); setPose(1, PO_REST); }
void evPoke(uint32_t now) { (void)now; if (busy()) return; lastActivityS = St; setExpr(X_CURIOUS); }
void evWink() { if (busy()) return; lastActivityS = St; setExpr(X_WINK); }
void evPlay(uint32_t now) { (void)now; if (busy()) return; lastActivityS = St; setExpr(X_HYPE); }
void evUsb(uint32_t now, bool on) { (void)now; if (busy()) return; lastActivityS = St; E.usb = on; if (on) setExpr(X_ELECTRIC); else if (curX == X_CHARGING || curX == X_ELECTRIC) setExpr(X_AWAKE); }
void evLoud(uint32_t now) { (void)now; if (busy()) return; lastActivityS = St; setExpr(X_ANXIOUS); }
void evShake(uint32_t now, bool strong) { (void)now; if (busy()) return; lastActivityS = St; setExpr(strong ? X_ANGRY : X_DIZZY); }
void evFlick(uint32_t now) { (void)now; if (busy()) return; lastActivityS = St; setExpr(X_SAD); }
void evCold(uint32_t now, bool on) { (void)now; if (busy()) return; lastActivityS = St; if (on) setExpr(X_COLD); else if (curX == X_COLD) setExpr(X_AWAKE); }
void evThumbs(uint32_t now) { (void)now; if (busy()) return; lastActivityS = St; setExpr(X_BATCHECK); }
void evPet(uint32_t now, bool on) { (void)now; if (busy()) return; lastActivityS = St; if (on) { if (curX != X_LOVE) setExpr(X_LOVE); E.holdPet = 1; } else if (curX == X_LOVE) { E.holdPet = 0; E.ex = St - XDUR[X_LOVE] + 0.9f; } }
void evEmotion(uint32_t now) {
  (void)now; if (busy()) return; lastActivityS = St;
  static const uint8_t EMO[17] = {X_CURIOUS, X_HAPPY, X_LOVE, X_SURPRISE, X_SMUG, X_HYPE, X_FULL, X_PANT, X_ANGRY, X_SAD, X_ANXIOUS, X_COLD, X_DIZZY, X_YAWN, X_LOWBAT, X_WINK, X_ELECTRIC};
  static uint8_t idx = 0; if (isAsleep()) { setExpr(X_AWAKE); return; }
  int x = EMO[idx % 17]; idx = (idx + 1) % 17; E.usb = false; E.wasSleepy = false; setExpr(x);
}

void coreInit(uint32_t now, uint32_t seed) {
  rng ^= seed; St = 0; gNow = now; memset(evq, 0, sizeof(evq)); memset(parts, 0, sizeof(parts)); memset(&E, 0, sizeof(E)); pop = 1; popv = 0;
  E.b = 2.5f; E.sc = 1; E.fl = 4; E.w = 12; E.sz = 9; E.sl = 18; E.sleepPlan = 20; lastActivityS = 0;
  for (int i = 0; i < 2; i++) {
    SEye& e = eyes[i]; memset(&e, 0, sizeof(e));
    e.bk = e.k = i ? Rr(120, 145) : Rr(105, 125); e.bd = e.d = i ? Rr(8.5f, 10) : Rr(7.5f, 9); e.kp = i ? Rr(170, 215) : Rr(190, 240);
    e.o = e.ot = i ? 0.0f : 0.0f; e.sz = e.szT = 1; e.jn = Rr(.3f, 1); e.ph = Rr(0, 6.3f); e.gene = Rr(.9f, 1.1f);
  }
  creatureFromSeed(seed * 2246822519u + 374761393u);       // misma transformacion que usa doRebirth/evNewCreature
  for (int i = 0; i < 2; i++) { float tmp[12]; const float* P = restVec(i, tmp); for (int k = 0; k < 12; k++) { eyes[i].p[k] = P[k]; eyes[i].tgt[k] = P[k]; } }
  for (int k = 0; k < NK; k++) Vv[k] = 0; Vv[K_CD] = Vv[K_PUP] = Vv[K_SCALE] = Vv[K_DIR] = 1; tgDefault();
  setExpr(X_AWAKE, true);
  eyes[0].o = eyes[1].o = 0; eyes[0].ot = eyes[1].ot = 1;               // al encender "abre los ojos"
  if (birthSkip) { setExpr(X_HAPPY, true); sched(1.4f, EV_EXPR, X_AWAKE, 0, 0, false); }
  else evBirth(now, 0);
}

void coreFrame(uint32_t now, float dt) {
  gNow = now; DT = clampf(dt, 0.005f, 0.04f);
  stepEngine(DT);
  memset(fb, 0, (size_t)W * H * 2);
  dirtyY0 = H; dirtyY1 = -1;
  renderStudio();
}
