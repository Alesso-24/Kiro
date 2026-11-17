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
//  Forma del ojo: elipse + armonicos (huevo, gota, triangulo, torcido...) + parpados rectos + pupila
//  Frontera:  u^2+v^2 - 1 - c1(u^2-v^2) - 2 c2 u v - c3 u - c4 v - c5 u(u^2-3v^2) = 0
//    c1 aplasta en diagonal | c2 inclina | c3 abulta hacia adentro (+) | c4 gota/huevo (+ ancho abajo)
//    c5 triangulo (punta hacia afuera).  c2,c3,c5 se espejean solos entre el ojo izq. y el der.
// ============================================================================================
struct EyeDraw {
  float cx, cy, rx, ry, ang;
  float c[5];                         // (heredado, ya no dibuja: el contorno viene de la pose de boceto)
  float lidT, lidB, slope, inner;     // parpados (fracciones de 2*ry), pendiente, +1 ojo izq / -1 der
  float pcx, pcy, prx, pry, pang, pc1, pc2;   // pupila (elipse girada con armonicos propios)
  uint16_t cEye, cPup, cAcc;
  uint8_t pkind;
  bool toneOn;
  float tc, ts, toneOff;
  float pz[9];                        // pose de boceto: base plana, angulo cresta, alto cresta, ancho cresta, alto punta, ancho punta, inclinacion, gancho, muesca
  float t, seed; bool flip;           // tiempo (el borde respira) y espejo
};

// ---- contorno de "boceto": huevo con base plana + cresta/punta inclinada (ver studio/estilo_tinta/alloy_v4.html) ----
const int NPT = 72;
float ptsX[NPT], ptsY[NPT];
static void buildOutline(const EyeDraw& D) {
  float fl = D.pz[0], ca = D.pz[1], ch = D.pz[2], cw = fmaxf(D.pz[3], 0.02f), th = D.pz[4], tw = fmaxf(D.pz[5], 0.01f), ln = D.pz[6], hk = D.pz[7], nt = D.pz[8];
  float cA = cosf(D.ang), sA = sinf(D.ang), tt = D.t;
  for (int i = 0; i < NPT; i++) {
    float a = TAU * i / NPT, sn = sinf(a), cs = cosf(a);
    float r = 1.0f + 0.028f * sinf(3 * a + tt * 1.3f + D.seed * 2) + 0.022f * sinf(5 * a - tt * 0.9f + D.seed) + 0.015f * sinf(2 * a + tt * 0.7f);
    if (ch > 0.01f || th > 0.01f) {
      float d = a - ca; d = atan2f(sinf(d), cosf(d));
      float d2 = a - ca - hk * 0.5f; d2 = atan2f(sinf(d2), cosf(d2));
      r += ch * expf(-d * d / cw) + th * 0.6f * expf(-d2 * d2 / tw) * (1.0f + 0.07f * sinf(tt * 2.3f + D.seed));
      if (nt > 0.01f) { float dn = a - ca - 0.55f; dn = atan2f(sinf(dn), cosf(dn)); r -= nt * expf(-dn * dn / 0.02f); }
    }
    float x = cs * D.rx * r, y = sn * D.ry * r;
    if (y > 0) y = D.ry * powf(sn, 1.0f - fl * 0.7f) * r;
    x += -y * ln * 0.55f;
    if (D.flip) x = -x;
    ptsX[i] = D.cx + x * cA - y * sA; ptsY[i] = D.cy + x * sA + y * cA;
  }
}
float covRow[W];

void renderEye(const EyeDraw& D) {
  float rx = D.rx, ry = D.ry;
  if (rx < 1 || ry < 1) return;
  buildOutline(D);
  float xmin = 1e9f, xmax = -1e9f, ymin = 1e9f, ymax = -1e9f;
  for (int i = 0; i < NPT; i++) { xmin = fminf(xmin, ptsX[i]); xmax = fmaxf(xmax, ptsX[i]); ymin = fminf(ymin, ptsY[i]); ymax = fmaxf(ymax, ptsY[i]); }
  float cx = D.cx, cy = D.cy;
  float exT = 1.0f + 1.2f * (1.0f - clampf(D.lidT * 10.0f, 0, 1)), exB = 1.0f + 0.4f * (1.0f - clampf(D.lidB * 10.0f, 0, 1));   // sin parpado, la cresta del boceto no se recorta
  float botY = cy + ry * exB - D.lidB * 2 * ry, topBase = cy - ry * exT + D.lidT * 2 * ry, k = D.slope * ry * D.inner / rx;
  int y0 = max(0, (int)floorf(ymin) - 1), y1 = min(H - 1, (int)ceilf(ymax) + 1);
  int x0 = max(0, (int)floorf(xmin) - 1), x1 = min(W - 1, (int)ceilf(xmax) + 1);
  markRows(y0, y1);
  float ca = cosf(D.ang), sa = sinf(D.ang);
  bool hasP = D.prx > 0.5f && D.pry > 0.5f;
  float iprx = 1.0f / (D.prx + 1e-3f), ipry = 1.0f / (D.pry + 1e-3f);
  float pca = cosf(D.pang), psa = sinf(D.pang);
  float psc = 0.5f * sqrtf(D.prx * D.pry), pfe = 0.5f / (psc + 1e-3f);
  for (int x = x0; x <= x1; x++) covRow[x] = 0;
  for (int y = y0; y <= y1; y++) {
    // cobertura del renglon: 2 sub-renglones, cruces con el contorno (par-impar)
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
    float dy = y - cy, pdy = y - D.pcy;
    uint16_t* row = fb + y * W;
    for (int x = x0; x <= x1; x++) {
      float a = covRow[x]; if (a <= 0) continue; covRow[x] = 0; if (a > 1) a = 1;
      float dx = x - cx;
      float topY = topBase + k * dx;
      float lt = y - topY + 0.5f;
      if (lt <= 0) continue;
      if (lt < 1) a *= lt;
      float lb = botY - y + 0.5f;
      if (lb <= 0) continue;
      if (lb < 1) a *= lb;
      uint16_t col = D.cEye;
      if (D.toneOn) { float u = (dx * ca + dy * sa) / rx, v = (-dx * sa + dy * ca) / ry; if ((u * D.tc + v * D.ts) > D.toneOff) col = D.cAcc; }
      if (hasP) {
        float pdx = x - D.pcx;
        float pa = (pdx * pca + pdy * psa) * iprx, pb = (-pdx * psa + pdy * pca) * ipry;
        float pa2 = pa * pa, pb2 = pb * pb;
        float fp = pa2 + pb2 - 1 - D.pc1 * (pa2 - pb2) - D.pc2 * pb;
        if (fp < pfe) {
          bool ok = true;
          uint16_t pc = D.cPup;
          if (D.pkind == 4) { if (pb > 0.30f) ok = false; }
          else if (D.pkind == 5) { if (pa2 + pb2 < 0.28f) ok = false; }
          if (ok) {
            if ((D.pkind & 2) && D.pkind < 4 && pb > 0.05f) pc = D.cAcc;
            col = fp <= -pfe ? pc : blend565(col, pc, (int)((0.5f - fp * psc) * 32.0f + 0.5f));
          }
        }
      }
      int ai = (int)(a * (float)gAlpha + 0.5f);
      if (ai <= 0) continue;
      row[x] = ai >= 32 ? col : blend565(row[x], col, ai);
    }
  }
}

// ---- ojo cerrado: trazo de pincel (grueso al centro, extremos redondos, inclinable) ----
void brushArc(float cx, float cy, float halfW, float dep, float dir, float incl, float side, float th, uint16_t col) {
  const int N = 22;
  for (int i = 0; i <= N; i++) {
    float u = -1.0f + 2.0f * i / N, x = u * halfW;
    float y = dir * dep * (1.0f - u * u) + 2.0f + incl * side * x * 0.5f;
    float w = (0.46f + 0.54f * powf(sinf(3.14159f * (0.5f + 0.5f * u)), 0.7f)) * th;
    discAA(cx + x, cy + y, w * 0.5f, col);
  }
}

// ============================================================================================
//  EXPRESIONES = poses (todos los parametros viajan con resortes -> morph fluido)
// ============================================================================================
enum : uint8_t {
  X_AWAKE = 1, X_CURIOUS, X_SLEEP, X_HAPPY, X_ANGRY, X_LOVE, X_CHARGING, X_ELECTRIC, X_DIMSLEEP, X_SAD,
  X_FULL, X_HYPE, X_LOWBAT, X_DIZZY, X_SURPRISE, X_SMUG, X_YAWN, X_ANXIOUS, X_COLD, X_BATCHECK, NEXPR
};
const char* const XNAME[NEXPR] = {"", "despierto", "curioso", "dormido", "feliz", "enojado", "amor", "cargando",
                                  "electrico", "dormido-tenue", "triste", "cara-llena", "hype", "bateria-baja",
                                  "mareado", "sorprendido", "orgulloso", "bostezo", "ansioso", "frio", "ver-bateria"};

enum { P_GAP, P_RX, P_RY, P_TILT, P_EGG, P_LIDT, P_LIDB, P_SLOPE, P_PRX, P_PRY, P_PDX, P_PDY, P_OPEN, P_ARCDIR,
       P_ARCW, P_TH, P_Y, P_ER, P_EG, P_EB, P_PR, P_PG, P_PB,
       P_C1, P_C2, P_C3, P_C4, P_C5, P_PTILT, P_PC1, P_PC2, P_WOB, P_FTILT, NP };
const int NBASE = P_PB + 1;   // columnas de POSES; el resto viene de SHAPES

// GAP  RX  RY  TILT  EGG(sin uso) LIDT LIDB SLOPE PRX PRY PDX PDY OPEN ARCDIR ARCW TH   Y   ER  EG  EB   PR  PG  PB
const float POSES[NEXPR][NBASE] = {
  {0},
  {40, 38, 60, 0.07f, 0, 0, 0, 0,     5.5f, 17, 15, 11, 1, 1, 34, 3.5f, 0,   245, 208, 192,  46, 190, 70},    // 1 despierto
  {44, 42, 62, 0.05f, 0, 0, 0, 0,    15, 26, 10, 13, 1, 1, 34, 3.5f, -3,   250, 250, 250,  50, 140, 235},     // 2 curioso
  {40, 38, 60, 0, 0, 0.5f, 0.5f, 0,   0, 0, 0, 0,   0, 1, 34, 3.6f, 22,   255, 255, 255,  255, 255, 255},     // 3 dormido
  {40, 38, 60, 0, 0, 0.5f, 0.5f, 0,   0, 0, 0, 0,   0, -1, 33, 4.2f, 6,   255, 255, 255,  255, 255, 255},     // 4 feliz
  {42, 38, 56, -0.12f, 0, 0.24f, 0, 0.65f, 10, 12, 12, 12, 1, 1, 34, 3.5f, 4,   235, 55, 48,  255, 255, 255},    // 5 enojado
  {43, 40, 58, 0.05f, 0, 0, 0.40f, 0, 0, 0, 0, 0,   1, 1, 34, 3.5f, 4,    245, 105, 215,  255, 60, 90},       // 6 amor
  {40, 36, 54, 0.0f, 0, 0, 0, 0,      0, 0, 0, 0,   1, 1, 34, 3.5f, 0,    245, 195, 20,   0, 215, 235},       // 7 cargando
  {42, 34, 48, 0.0f, 0, 0, 0, 0,      0, 0, 0, 0,   1, 1, 34, 3.5f, 0,    248, 190, 20,   0, 215, 235},       // 8 electrico
  {40, 38, 60, 0, 0, 0.5f, 0.5f, 0,   0, 0, 0, 0,   0, 1, 34, 4.0f, 22,   235, 120, 30,  235, 120, 30},       // 9 dormido tenue
  {46, 40, 50, 0.0f, 0, 0.52f, 0, -0.40f, 0, 0, 0, 0,   1, 1, 34, 3.5f, 12,    220, 30, 45,  90, 190, 170},    // 10 triste
  {42, 40, 58, 0.0f, 0, 0, 0.42f, 0,      15, 20, 0, 9, 1, 1, 34, 3.5f, -14,   255, 232, 96,   50, 135, 55},       // 11 cara llena
  {42, 38, 52, 0.22f, 0, 0, 0.18f, 0, 14, 14, 8, 10, 1, 1, 34, 3.5f, 0,   255, 120, 0,    255, 255, 255},    // 12 hype
  {42, 36, 42, 0.0f, 0, 0.18f, 0, 0,  6.5f, 3.8f, 6, 6, 1, 1, 34, 3.5f, 8,   248, 195, 20,  0, 215, 235},     // 13 bateria baja
  {42, 38, 56, 0.0f, 0, 0.06f, 0.04f, 0,  0, 0, 0, 0,   1, 1, 34, 3.5f, 0,    250, 200, 60,   40, 200, 220},   // 14 mareado
  {46, 46, 66, 0.0f, 0, 0, 0, 0,      3.5f, 6, 6, 8, 1, 1, 34, 3.5f, -2,  245, 208, 192,  46, 190, 70},       // 15 sorprendido
  {42, 38, 58, 0.0f, 0, 0.34f, 0, 0.12f,  13, 21, 0, 14, 1, 1, 34, 3.5f, 4,  252, 200, 50,  225, 120, 50},   // 16 orgulloso
  {40, 36, 66, 0.0f, 0, 0.30f, 0.12f, 0,  4, 8, 6, 14, 1, 1, 34, 3.5f, 2,  245, 208, 192,  46, 190, 70},      // 17 bostezo
  {43, 40, 66, 0.0f, 0, 0, 0, 0,      4, 7, 5, 5, 1, 1, 34, 3.5f, -1,   245, 208, 192,  46, 190, 70},         // 18 ansioso (ojos enormes, pupila minima)
  {40, 38, 50, 0.0f, 0, 0.26f, 0.18f, 0,  5, 8, 8, 8, 1, 1, 34, 3.5f, 5,   190, 225, 255,  60, 120, 220},      // 19 frio (entrecerrados, azulados)
  {41, 37, 56, 0.0f, 0, 0.40f, 0, 0,    0, 0, 0, 0,  1, 1, 34, 3.5f, 2,   250, 195, 30,   0, 215, 235},       // 20 ver bateria (el ojo es un medidor)
};
//  C1     C2     C3     C4     C5    PTILT   PC1    PC2   WOB   FTILT
const float SHAPES[NEXPR][NP - NBASE] = {
  {0},
  {0,     0,     0.06f, 0.12f, 0,    0,      0,     0.18f, 0.00f, 0},        // 1 despierto (leve huevo)
  {-0.04f, 0.04f, 0.14f, 0.06f, 0,   0,      0,     0.10f, 0.00f, 0.07f},    // 2 curioso (ladea la cara)
  {0, 0, 0, 0, 0,                    0, 0, 0, 0, 0},                          // 3 dormido
  {0, 0, 0, 0, 0,                    0, 0, 0, 0, 0},                          // 4 feliz
  {0.10f, 0.06f, 0,    -0.08f, 0.17f, 0.40f, 0.20f, 0,    0.00f, 0},         // 5 enojado (hoja afilada)
  {-0.08f, 0,   0.04f, -0.12f, 0,    0,      0,     0,    0.04f, 0},         // 6 amor (domo regordete)
  {0,     0,     0,    0.10f, 0,     0,      0,     0,    0.02f, 0},         // 7 cargando
  {0,     0,     0,    0.04f, 0,     0,      0,     0,    0.05f, 0},         // 8 electrico (tiembla)
  {0, 0, 0, 0, 0,                    0, 0, 0, 0, 0},                          // 9 dormido tenue
  {0,     0.14f, -0.18f, 0.30f, 0,   0,      0,     0,    0.00f, 0.04f},     // 10 triste (cae hacia afuera)
  {0,     0,     0,    0.05f, 0,     0,      0,     0.10f, 0.00f, 0},        // 11 cara llena
  {-0.12f, 0,    0.10f, 0.06f, 0.14f, 0,     0,     0,    0.06f, 0},         // 12 hype (gordito saltarin)
  {0.14f, 0,     0,    0.26f, 0,     0,      0.10f, 0,    0.00f, 0},         // 13 bateria baja (se derrite)
  {0,     0,     0,    0.04f, 0,     0,      0,     0,    0.16f, 0},         // 14 mareado (gelatina que ondula)
  {-0.10f, 0,    0,    0.04f, 0,     0,      0,     0,    0.00f, 0},         // 15 sorprendido (redondo)
  {0,     0,     0.04f, 0.20f, -0.10f, 0,    0,     0.14f, 0.00f, -0.05f},   // 16 orgulloso
  {0.10f, 0,     0,    0.10f, 0,     0,      0,     0,    0.00f, 0},         // 17 bostezo
  {-0.06f, 0,    0.02f, 0.02f, 0,    0,      0,     0,    0.12f, 0},         // 18 ansioso
  {0.08f, 0,     0,    0.12f, 0,     0,      0,     0,    0.00f, 0},         // 19 frio
  {0,     0,     0,    0.04f, 0,     0,      0,     0,    0.00f, 0},         // 20 ver bateria
};

#include "Data.h"
// ---- GENOMA: una semilla de 32 bits define una criatura unica (paleta x especie x pupila x tonos x personalidad x variaciones) ----
struct Genome {
  uint32_t seed;
  uint16_t pal, species, pupil;
  uint8_t toneMode;        // 0 ninguno, 1 ojo de dos tonos (linea de corte con color de acento)
  float toneAng, toneOff;
  float trait[6];          // 0 energia 1 timidez 2 gruñon 3 sueño 4 curiosidad 5 cariño (0..1)
  float jit[8];            // variaciones continuas unicas (-1..1)
};
Genome G;
#define awakeTheme (G.pal)
#define pStyle (G.pupil)
float POSE[NEXPR][NP];   // tabla completa (se arma en coreInit)

// ============================================================================================
//  Estado de la animacion
// ============================================================================================
float cur[NP], vel[NP], tgt[NP];
uint8_t expr = X_AWAKE, prevExpr = 0;
uint32_t exprStart = 0;

struct Eye {
  float ox, oy, ovx, ovy, sx, svx, sy, svy, px, py, pvx, pvy, blinkT, blinkTot;
};
Eye eye[2];
float gx = 0, gy = 0, tgx = 0, tgy = 0, pgx = 0, pgy = 0;
uint32_t gazeUntil = 0, nextWander = 0, nextBlink = 2500, nextIdleAct = 9000, nextForm = 1500;
float shakeAmp = 0, beat = 0, beatPhase = 0, slosh = 0;
int8_t birthVar = -1;                       // >=0 mientras nace: se ignoran botones, eventos, clips y particulas ajenas
bool birthSpawn = false, birthPlay = false;
uint32_t gNow = 0, nextBored = 0, nextAsk = 0, usbSince = 0;
uint8_t boredIdx = 0;
uint8_t chargeType = 0, fullPal = 0, angryVar = 0, angryCol = 0, loveVar = 0;      // variantes elegidas al entrar a cada expresion
int8_t forceLoveVar = -1, forceChargeType = -1, forceFullPal = -1, forceAngryVar = -1, forceAngryCol = -1;   // (pruebas del simulador)
bool birthSkip = false;                                                 // true = ya hay criatura guardada: se salta el nacimiento
bool simBatt = false; float simT = 0;                                  // bateria simulada mientras no hay sensor: 3 min bajando + 1 min vacia, luego llena
const float SIM_DRAIN = 180.0f, SIM_HOLD = 60.0f;
// CARA LLENA: {color de la cara, ojo, pupila}
static const uint8_t FULLP[6][9] = {{250, 205, 20, 255, 232, 96, 50, 135, 55}, {40, 200, 170, 170, 255, 235, 20, 60, 110}, {255, 120, 170, 255, 205, 225, 110, 30, 130},
                                    {255, 140, 40, 255, 215, 150, 150, 40, 30}, {90, 130, 255, 200, 220, 255, 255, 205, 60}, {170, 230, 90, 235, 255, 170, 20, 100, 100}};
// ENOJADO: rojos y naranjas
static const uint8_t ANGC[6][3] = {{235, 55, 48}, {205, 28, 52}, {255, 100, 25}, {255, 148, 20}, {172, 22, 38}, {255, 72, 72}};
float presence[8];
enum { XT_CHARGE, XT_FULL, XT_SPIRAL, XT_GLOW, XT_X4, XT_X5, XT_X6, XT_X7 };
float puOv[2] = {-1, -1};       // tamano de pupila forzado por ojo (<0 = automatico)

// formas irregulares por ojo (cambian solas cada pocos segundos; nunca iguales)
enum { F_SC, F_AX, F_AY, F_ANG, F_C1, F_C2, F_C3, F_C4, F_C5, F_FX, F_FY, F_PS, F_PT, F_FL, F_CA, F_CH, F_CW, F_TH, F_TW, F_LN, F_HK, F_NT, NF };
float fcur[2][NF], ftgt[2][NF];
uint8_t formMode = 0;
uint32_t settleAt = 0;          // las deformaciones son momentos: tras un rato 'se acomodan' a la forma de reposo
float formAmp = 1;
float nph[2][12][4];            // fases/frecuencias del ruido organico

// poses de los bocetos: AX, AY, flat, ca, ch, cw, th, tw, lean, hook, notch
enum { PZ_HUEVO, PZ_BANDERA, PZ_PICO, PZ_TORRE, PZ_AGACHADO, PZ_COLA, PZ_GANCHO, PZ_CABECITA, PZ_FURIAL, PZ_FURIAR, NPZ };
const float POSEV[NPZ][11] = {
  {1.000f, 1.000f, .12f, -1.57f, 0.00f, .30f, 0.00f, .050f, 0.00f, 0.00f, 0.00f},
  {1.050f, 0.870f, .20f, -1.90f, 0.55f, .35f, 0.50f, .040f, 0.25f, 0.30f, 0.00f},
  {0.950f, 0.913f, .20f, -1.30f, 0.45f, .25f, 0.90f, .030f, 0.30f, 0.00f, 0.00f},
  {0.850f, 1.087f, .25f, -1.20f, 0.50f, .40f, 1.10f, .035f, 0.45f, 0.20f, 0.25f},
  {1.200f, 0.696f, .35f, -1.57f, 0.15f, .50f, 0.25f, .050f, 0.00f, 0.00f, 0.00f},
  {0.950f, 0.957f, .20f, -2.00f, 0.50f, .30f, 0.95f, .035f, -0.40f, -0.20f, 0.00f},
  {1.000f, 0.957f, .25f, -1.80f, 0.45f, .30f, 0.80f, .040f, -0.35f, -0.70f, 0.30f},
  {0.900f, 1.043f, .25f, -1.00f, 0.35f, .20f, 0.60f, .050f, 0.10f, 0.50f, 0.00f},
  {0.950f, 0.913f, .20f, -2.10f, 0.45f, .25f, 0.90f, .030f, -0.30f, 0.00f, 0.00f},
  {0.950f, 0.913f, .20f, -1.04f, 0.45f, .25f, 0.90f, .030f, 0.30f, 0.00f, 0.00f}};
void poseSet(float* f, int pz, float k = 1.0f) {   // mezcla desde el huevo hacia la pose (k 0..1)
  const float* P = POSEV[pz]; const float* H0 = POSEV[PZ_HUEVO];
  float v[11]; for (int i = 0; i < 11; i++) v[i] = H0[i] + (P[i] - H0[i]) * k;
  f[F_AX] = v[0]; f[F_AY] = v[1]; f[F_FL] = v[2]; f[F_CA] = v[3]; f[F_CH] = v[4]; f[F_CW] = v[5]; f[F_TH] = v[6]; f[F_TW] = v[7]; f[F_LN] = v[8]; f[F_HK] = v[9]; f[F_NT] = v[10];
}
bool styleMir = false;           // espejo: los picos del ojo derecho miran hacia el lado contrario (se decide por criatura)
float baseF[2][NF]; uint32_t baseKey = 0xFFFFFFFFu;
static void makeBase() {        // forma de reposo PROPIA de cada criatura (muchas combinaciones base): pose de boceto parcial por ojo, asimetria y ladeo
  uint32_t key = (uint32_t)G.pal * 10007u + (uint32_t)G.species * 131u + (uint32_t)G.pupil * 7u + 1u;
  if (key == baseKey) return;
  baseKey = key; uint32_t h = key * 2654435761u + 12345u;
  auto rnd = [&h]() { h = h * 1664525u + 1013904223u; return ((h >> 8) & 0xFFFF) / 65535.0f; };
  static const uint8_t BP[8] = {PZ_HUEVO, PZ_BANDERA, PZ_PICO, PZ_TORRE, PZ_AGACHADO, PZ_COLA, PZ_GANCHO, PZ_CABECITA};
  int pa = BP[(int)(rnd() * 7.99f)], pb = rnd() < 0.45f ? pa : BP[(int)(rnd() * 7.99f)];
  float ka = 0.35f + 0.45f * rnd(), kb = 0.35f + 0.45f * rnd(), asy = 0.82f + 0.36f * rnd();
  for (int e = 0; e < 2; e++) {
    float* f = baseF[e]; for (int i = 0; i < NF; i++) f[i] = 0;
    f[F_SC] = f[F_AX] = f[F_AY] = f[F_PS] = 1;
    poseSet(f, e ? pb : pa, e ? kb : ka);
    if (e) { f[F_AX] *= asy; f[F_AY] *= asy; }
    f[F_ANG] = (rnd() - 0.5f) * 0.20f;
  }
}
void formReset(float* f) {
  makeBase();
  const float* B = (f == ftgt[1] || f == fcur[1]) ? baseF[1] : baseF[0];
  for (int i = 0; i < NF; i++) f[i] = (i >= F_FL || i == F_AX || i == F_AY || i == F_ANG) ? B[i] : 0;
  f[F_SC] = f[F_PS] = 1;
}
void chooseForm(uint32_t now) {
  nextForm = now + 3200 + (uint32_t)(frand01() * 4800);
  settleAt = now + 900 + (uint32_t)(frand01() * 900);
  float r = frand01();
  uint8_t m = r < 0.18f ? 0 : r < 0.84f ? 7 : r < 0.92f ? 1 : 4;
  if (m == formMode && m != 7) m = 7;
  formMode = m;
  int big = frand() > 0 ? 1 : 0;
  static const uint8_t FLO[6] = {PZ_BANDERA, PZ_PICO, PZ_TORRE, PZ_COLA, PZ_GANCHO, PZ_CABECITA};
  int pa = FLO[(int)(frand01() * 5.99f)], pb = FLO[(int)(frand01() * 5.99f)];
  for (int e = 0; e < 2; e++) {
    float* f = ftgt[e];
    formReset(f);
    switch (m) {
      case 7: poseSet(f, e ? (styleMir ? pa : pb) : pa); f[F_ANG] = frand() * 0.04f; break;
      case 1: f[F_ANG] = frand() * 0.14f; f[F_FX] = frand() * 5; f[F_FY] = frand() * 7; f[F_PT] = frand() * 0.5f; f[F_SC] = 1.0f + frand() * 0.08f; break;
      case 4: f[F_SC] = (e == big) ? 1.2f : 0.80f; f[F_FY] = frand() * 5; break;
    }
  }
  float pr = frand01();     // tamanos de pupila: iguales grandes / iguales chicas / una grande y otra chica
  if (pr >= 0.30f) {
    int bigp = frand() > 0 ? 1 : 0;
    for (int e = 0; e < 2; e++) {
      ftgt[e][F_PS] = pr < 0.50f ? 1.45f + 0.15f * frand() : pr < 0.62f ? 0.85f + 0.08f * frand() : (e == bigp ? 1.65f : 0.70f);
      ftgt[e][F_PT] += frand() * 0.4f;
    }
  }
  for (int e = 0; e < 2; e++)    // la amplitud depende de la expresion
    for (int i = 0; i < NF; i++) {
      float base = (i == F_SC || i == F_AX || i == F_AY || i == F_PS) ? 1.0f : 0.0f;
      if (i >= F_FL) base = POSEV[PZ_HUEVO][i == F_FL ? 2 : i == F_CA ? 3 : i == F_CH ? 4 : i == F_CW ? 5 : i == F_TH ? 6 : i == F_TW ? 7 : i == F_LN ? 8 : i == F_HK ? 9 : 10];
      ftgt[e][i] = base + (ftgt[e][i] - base) * (i >= F_FL ? fminf(1.0f, formAmp * 1.6f) : formAmp);
    }
}

// --- particulas ---
enum { PT_HEART, PT_SPARK, PT_DOT, PT_DROP, PT_BOLT, PT_LINE };
struct Part { float x, y, vx, vy, life, max, size, rot, spin; uint8_t type, col, on; };
const int NPART = 56;
Part parts[NPART];
void spawn(uint8_t type, float x, float y, float vx, float vy, float life, float size, uint8_t col, float spin = 0, float rot = 0) {
  if (type == PT_SPARK) return;      // sin estrellitas (a Alessandro no le gustan)
  if (birthVar >= 0 && !birthSpawn) return;      // mientras nace solo salen los efectos del nacimiento
  for (int i = 0; i < NPART; i++)
    if (!parts[i].on) {
      parts[i] = {x, y, vx, vy, life, life, size, rot, spin, type, col, 1};
      return;
    }
}
float acc[5];
uint16_t cAccC = 0xFFFF;      // color de acento (segundo tono del ojo / pupila dividida)

// ---------------------------------------------------------------- genoma
static uint32_t grng = 1;
static float gr() { grng = grng * 1664525u + 1013904223u; return ((grng >> 8) & 0xFFFF) / 65535.0f; }
static uint8_t rarPal(int i) { return PALS[i].rarity; }
static uint8_t rarSp(int i) { return SPECIES[i].rarity; }
static uint8_t rarPu(int i) { return PUPILS[i].rarity; }
// elige por rareza: comun 60%, poco comun 25%, raro 12%, mitico 3% (baja de nivel si ese nivel esta vacio)
static int pickTier(int n, uint8_t (*rar)(int)) {
  float x = gr();
  int tier = x < 0.60f ? 0 : x < 0.85f ? 1 : x < 0.97f ? 2 : 3;
  for (; tier >= 0; tier--) {
    int cnt = 0;
    for (int i = 0; i < n; i++) if (rar(i) == tier) cnt++;
    if (cnt) {
      int k = (int)(gr() * cnt) % cnt;
      for (int i = 0; i < n; i++) if (rar(i) == tier && k-- == 0) return i;
    }
  }
  return (int)(gr() * n) % n;
}
void genomeFromSeed(uint32_t seed) {
  grng = seed * 2654435761u + 12345u;
  for (int i = 0; i < 4; i++) gr();
  G.seed = seed;
  G.pal = pickTier(NPALS, rarPal); G.species = pickTier(NSPECIES, rarSp); G.pupil = pickTier(NPUPILS, rarPu);
  // reglas de exclusion (auditoria): pupila diminuta en ojo grande = "muerto"; pupila partida/anillo + dos tonos = ilegible
  for (int tries = 0; tries < 6; tries++) {
    const PupilStyle& pu = PUPILS[G.pupil];
    const Species& sp = SPECIES[G.species];
    if (pu.prx + pu.pry < 11.0f && sp.sx * sp.sy > 1.02f) G.pupil = pickTier(NPUPILS, rarPu); else break;
  }
  int pr = PALS[G.pal].rarity;
  float pt = pr >= 2 ? 0.55f : (pr == 1 ? 0.22f : 0.07f);
  (void)pt; G.toneMode = 0; (void)gr(); G.toneAng = gr() * 6.2832f; G.toneOff = -0.2f + 0.5f * gr();
  { uint8_t k = PUPILS[G.pupil].kind; if (k == 2 || k == 3 || k == 5) G.toneMode = 0; }   // dos tonos + pupila partida/anillo = ilegible
  for (int i = 0; i < 6; i++) G.trait[i] = 0.5f * (gr() + gr());
  {   // cada color y cada forma tiene su caracter: calidos = energicos/carinosos, frios = timidos/sonolientos, oscuros = gruñones, verdes = curiosos
    const Pal& pl = PALS[G.pal]; const Species& sp = SPECIES[G.species];
    float r = pl.e[0] / 255.0f, g = pl.e[1] / 255.0f, b = pl.e[2] / 255.0f;
    float mx = fmaxf(r, fmaxf(g, b)), mn = fminf(r, fminf(g, b)), sat = mx > 0 ? (mx - mn) / mx : 0, val = mx, hue = 0, dl = mx - mn;
    if (dl > 0.001f) { if (mx == r) hue = 60 * fmodf((g - b) / dl + 6, 6); else if (mx == g) hue = 60 * ((b - r) / dl + 2); else hue = 60 * ((r - g) / dl + 4); }
    float warm = (hue < 70 || hue > 320) ? 1 : 0, cool = (hue >= 170 && hue <= 290) ? 1 : 0, green = (hue >= 70 && hue < 170) ? 1 : 0;
    float pastel = (sat < 0.45f && val > 0.8f) ? 1 : 0, dark = val < 0.6f ? 1 : 0, pink = (hue >= 300 && hue <= 345) ? 1 : 0, red = (hue < 20 || hue > 345) ? 1 : 0;
    float area = sp.sx * sp.sy, quirk = fabsf(sp.asx) + fabsf(sp.atilt) + fabsf(sp.ay) * 0.05f;
    G.trait[0] += 0.18f * warm - 0.12f * cool + 0.10f * sat - 0.10f * dark + 0.08f * (area > 1.05f) + 0.4f * quirk;
    G.trait[1] += 0.20f * cool - 0.10f * warm + 0.10f * (1 - sat) + 0.12f * (area < 0.95f);
    G.trait[2] += 0.20f * dark + 0.12f * red - 0.15f * pastel + 0.5f * fmaxf(0.0f, -sp.tilt);
    G.trait[3] += 0.15f * dark + 0.10f * cool - 0.05f * warm + 0.08f * (sp.sx > sp.sy * 0.8f);
    G.trait[4] += 0.20f * green + 0.10f * (sat > 0.7f) + 0.12f * (sp.sy > sp.sx * 1.5f) + 0.10f * (area > 1.05f) + 0.5f * quirk;
    G.trait[5] += 0.20f * pastel + 0.20f * pink + 0.05f * warm + 0.10f * (area > 1.05f);
    for (int i = 0; i < 6; i++) G.trait[i] = clampf(G.trait[i], 0.02f, 1.0f);
  }
  for (int i = 0; i < 8; i++) G.jit[i] = gr() * 2 - 1;
}

// ---------------------------------------------------------------- reproductor de clips (capas aditivas, Catmull-Rom)
const int NCH = 12;
const float CH_RANGE[NCH] = {0.5f, 0.5f, 24, 24, 0.6f, 1, 1, 1, 1, 1, 1, 1};
float clipAdd[NCH];
struct Slot { int16_t id; uint32_t t0; float speed; bool mirror; uint8_t evi; bool on; };
Slot slots[3];
uint32_t nextClip = 4000;
bool autoOn = true;           // false = sin acciones espontaneas ni clips del director (pruebas)

int clipFind(const char* name) {
  for (int i = 0; i < NCLIPS; i++) if (strcmp(CLIPS[i].name, name) == 0) return i;
  return -1;
}
int clipPlay(int id, uint32_t now, bool mirror = false, float speed = 1.0f) {
  if (id < 0 || id >= NCLIPS || birthVar >= 0) return -1;
  int sl = -1;
  for (int i = 0; i < 3; i++) if (!slots[i].on) { sl = i; break; }
  if (sl < 0) { sl = 0; for (int i = 1; i < 3; i++) if (slots[i].t0 < slots[sl].t0) sl = i; }
  slots[sl].id = id; slots[sl].t0 = now; slots[sl].speed = speed < 0.2f ? 0.2f : speed;
  slots[sl].mirror = mirror && (CLIPS[id].flags & 1); slots[sl].evi = 0; slots[sl].on = true;
  return sl;
}
static float chanAt(const Clip& c, int ch, float tms) {
  const Key* K = c.keys; int n = c.n;
  if (tms <= K[0].t) return K[0].v[ch];
  if (tms >= K[n - 1].t) return K[n - 1].v[ch];
  int i = 0;
  while (i < n - 2 && tms >= K[i + 1].t) i++;
  float t0 = K[i].t, t1 = K[i + 1].t, u = (tms - t0) / (t1 - t0);
  float p1 = K[i].v[ch], p2 = K[i + 1].v[ch];
  if (c.flags & 8) return p1 + (p2 - p1) * u;                       // linear
  float p0 = i > 0 ? K[i - 1].v[ch] : p1, p3 = i + 2 < n ? K[i + 2].v[ch] : p2;
  float u2 = u * u, u3 = u2 * u;
  return 0.5f * ((2 * p1) + (-p0 + p2) * u + (2 * p0 - 5 * p1 + 4 * p2 - p3) * u2 + (-p0 + 3 * p1 - 3 * p2 + p3) * u3);
}
void clipEmit(uint8_t ev, bool mir) {
  float s = mir ? -1.0f : 1.0f, cx = 120, cy = 108;
  switch (ev) {
    case 1: spawn(PT_HEART, cx + s * 18, cy - 50, s * 6, -26, 1.4f, 9, 2); break;
    case 2: for (int i = 0; i < 5; i++) spawn(PT_HEART, cx + (i - 2) * 16, cy - 40, (i - 2) * 7, -24 - 8 * frand01(), 1.5f, 8 + 4 * frand01(), 2); break;
    case 3: spawn(PT_SPARK, cx + s * 45, cy - 30, 0, -8, 0.9f, 6, 4, 2); break;
    case 4: for (int i = 0; i < 6; i++) { float a = TAU * i / 6; spawn(PT_SPARK, cx + cosf(a) * 40, cy + sinf(a) * 38, cosf(a) * 50, sinf(a) * 46, 0.9f, 5 + 3 * frand01(), 4, 2); } break;
    case 5: spawn(PT_DROP, cx + s * 52, cy + 4, 0, 8, 1.2f, 8, 5); break;
    case 6: spawn(PT_BOLT, cx + s * 30, cy, s * 60, -60, 0.35f, 6, 3, 0, -0.8f); break;
    case 7: for (int i = 0; i < 8; i++) { float a = TAU * i / 8; spawn(PT_BOLT, cx + cosf(a) * 28, cy + sinf(a) * 30, cosf(a) * 110, sinf(a) * 100, 0.32f, 5, 3, 0, a); } break;
    case 8: spawn(PT_LINE, cx, cy - 70, 0, -30, 0.5f, 10, 4, 0, -1.5708f); break;
    case 9: { int n = 3 + (int)(frand01() * 3.99f); for (int i = 0; i < n; i++) { float a = -1.5708f + (i - (n - 1) * 0.5f) * 0.40f + frand() * 0.16f; float L = 7 + 8 * frand01(), v = 28 + 22 * frand01(); spawn(PT_LINE, cx + cosf(a) * (58 + 8 * frand01()), cy - 12 + sinf(a) * (66 + 8 * frand01()), cosf(a) * v, sinf(a) * v, 0.38f + 0.35f * frand01(), L, 4, 0, a); } } break;
    case 10: for (int i = 0; i < 4; i++) spawn(PT_DOT, cx + (i - 1.5f) * 22, cy + 40, (i - 1.5f) * 12, -30 - 10 * frand01(), 0.5f, 2.4f, 0); break;
    case 11: spawn(PT_SPARK, cx, cy - 62, 0, -10, 1.0f, 10, 6, 1.5f); break;
    case 12: { Eye& E = eye[mir ? 0 : 1]; E.blinkTot = E.blinkT = 0.5f; } break;
    case 13: for (int e = 0; e < 2; e++) eye[e].blinkTot = eye[e].blinkT = 0.23f; break;
    case 14: shakeAmp = 3; break;
    case 15: spawn(PT_DROP, cx + s * 66, cy - 34, s * 4, 4, 1.0f, 5.5f, 4); break;
    case 16: for (int i = 0; i < 3; i++) spawn(PT_SPARK, cx + (i - 1) * 40, cy - 70, 0, 14, 1.2f, 4, 4, 1); break;
    case 17: spawn(PT_SPARK, cx + s * 50, cy - 20, s * 10, -22, 1.1f, 7, 6, 1); break;
  }
}
void clipUpdate(uint32_t now) {
  for (int c = 0; c < NCH; c++) clipAdd[c] = 0;
  for (int si = 0; si < 3; si++) {
    Slot& sl = slots[si];
    if (!sl.on) continue;
    const Clip& c = CLIPS[sl.id];
    float tms = (now - sl.t0) * sl.speed;
    if (c.flags & 2) { if (c.dur > 0) tms = fmodf(tms, (float)c.dur); }
    else if (tms > c.dur + 30) { sl.on = false; continue; }
    for (int ch = 0; ch < NCH; ch++) {
      float v = chanAt(c, ch, tms) / 127.0f * CH_RANGE[ch];
      if (sl.mirror) {
        if (ch == 2 || ch == 4 || ch == 10) v = -v;                  // ox, tilt, gx
        int cc = ch == 5 ? 6 : ch == 6 ? 5 : ch == 7 ? 8 : ch == 8 ? 7 : ch;   // izq <-> der
        clipAdd[cc] += v;
      } else clipAdd[ch] += v;
    }
    while (sl.evi < c.n && c.keys[sl.evi].t <= tms) { if (c.keys[sl.evi].ev) clipEmit(c.keys[sl.evi].ev, sl.mirror); sl.evi++; }
  }
}
// el "director" elige animaciones de idle segun la personalidad de la criatura
static float clipWeight(int i) {
  switch (CLIPS[i].tag) {
    case 0: return 1.0f;
    case 1: return 0.5f + G.trait[5] + G.trait[0];
    case 2: return 0.1f + 0.8f * G.trait[2];
    case 3: return 0.1f + 0.8f * G.trait[1];
    case 4: return 0.1f + 0.8f * G.trait[3];
    case 5: return 0.5f + G.trait[4];
    default: return 0.0f;     // 6 energia/sistema y 7 reaccion: solo por evento
  }
}
void directorClip(uint32_t now) {
  nextClip = now + (uint32_t)((2200 + 4200 * frand01()) / (0.55f + G.trait[0]));
  float tot = 0;
  for (int i = 0; i < NCLIPS; i++) tot += clipWeight(i);     // dos pasadas: sin arreglos en la pila (el core 0 tiene 2 KB)
  if (tot <= 0) return;
  float r = frand01() * tot * 0.9999f;
  int pick = -1;
  for (int i = 0; i < NCLIPS; i++) { float w = clipWeight(i); if (w <= 0) continue; pick = i; r -= w; if (r <= 0) break; }
  if (pick >= 0) clipPlay(pick, now, frand() > 0, 0.9f + 0.25f * frand01());
}

// --- secuencias ---
struct Step { uint8_t x; uint16_t dur; };
Step seq[3];
uint8_t seqLen = 0, seqIdx = 0;
uint32_t stepEnd = 0;

// --- tamagotchi ---
bool usb = false, lowBat = false, deepSleep = false, fullBat = false, cold = false;
float energy = 85, fun = 75, affection = 40;
float timeScale = 1;
uint32_t lastActivity = 0, lastTap = 0;
uint8_t tapCount = 0;
bool petting = false;
uint32_t petLast = 0;

uint8_t baseExpr() {
  if (usb && !fullBat) return X_CHARGING;
  if (energy < 18 || lowBat) return X_LOWBAT;
  if (cold) return X_COLD;
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
  if (birthVar >= 0 && !birthPlay) return;       // nada interrumpe el nacimiento
  seq[0] = a; seq[1] = b; seq[2] = c;
  seqLen = n; seqIdx = 0;
  startStep(now);
}

// ============================================================================================
//  Acciones / eventos (los dispara el firmware con botones/sensores y el simulador con el teclado)
// ============================================================================================
void wake(uint32_t now) {
  lastActivity = now;
  if (deepSleep) { deepSleep = false; play(now, 2, {X_SURPRISE, 700}, {0, 0}); return; }
  if (expr == X_SLEEP || expr == X_DIMSLEEP) play(now, 2, {X_CURIOUS, 1100}, {0, 0});
}
bool isAsleep() { return deepSleep || expr == X_SLEEP || expr == X_DIMSLEEP; }
void evSet(uint8_t x, uint32_t now, uint16_t dur) {
  lastActivity = now; deepSleep = false;
  if (dur) play(now, 2, {x, dur}, {0, 0}); else play(now, 1, {x, 0});
}
void evTap(uint32_t now) {
  bool slept = isAsleep();
  wake(now);
  if (slept) return;
  if (now - lastTap > 1800) tapCount = 0;
  lastTap = now;
  fun = clampf(fun + 2, 0, 100);
  for (int e = 0; e < 2; e++) { eye[e].sx = 0.12f; eye[e].sy = -0.14f; }
  if (++tapCount >= 3) { tapCount = 0; affection = clampf(affection + 10, 0, 100); play(now, 2, {X_LOVE, 3400}, {0, 0}); }
}
void evPet(uint32_t now, bool on) {
  if (on) {
    if (isAsleep()) { wake(now); return; }
    lastActivity = now; petLast = now;
    if (!petting) { petting = true; play(now, 2, {X_LOVE, 1600}, {0, 0}); }
    else if (expr != X_LOVE) play(now, 2, {X_LOVE, 1600}, {0, 0});
    affection = clampf(affection + 8 * DT, 0, 100);
    fun = clampf(fun + 6 * DT, 0, 100);
  } else petting = false;
}
void evPlay(uint32_t now) {
  if (isAsleep()) { wake(now); return; }
  lastActivity = now;
  fun = clampf(fun + 22, 0, 100);
  energy = clampf(energy - 3, 0, 100);
  play(now, 2, {X_HYPE, 2800}, {0, 0});
}
void evShake(uint32_t now, bool strong) {
  if (birthVar >= 0) return;
  slosh = strong ? 1.0f : 0.55f;            // "agitalo y se marea y se enoja contigo"
  if (isAsleep()) { wake(now); return; }
  lastActivity = now;
  shakeAmp = strong ? 7.0f : 3.0f;
  if (strong) { fun = clampf(fun - 4, 0, 100); play(now, 3, {X_DIZZY, 1800}, {X_ANGRY, 1800}, {0, 0}); }
  else play(now, 2, {X_CURIOUS, 1800}, {0, 0});
}
void evLoud(uint32_t now) {                          // "en un lugar ruidoso se pone ansioso"
  if (isAsleep()) { wake(now); return; }
  lastActivity = now;
  play(now, 2, {X_ANXIOUS, 3200}, {0, 0});
}
void evCold(uint32_t now, bool on) { cold = on; lastActivity = now; if (on && !usb && !isAsleep()) play(now, 1, {0, 0}); }
void evThumbs(uint32_t now) {                        // pulgar arriba = ver nivel de bateria
  lastActivity = now;
  if (isAsleep()) { wake(now); return; }
  play(now, 2, {X_BATCHECK, 4200}, {0, 0});
}
void evFlick(uint32_t now) {                         // "dale un golpecito de desprecio y se pone triste"
  if (isAsleep()) { wake(now); return; }
  lastActivity = now; fun = clampf(fun - 15, 0, 100);
  play(now, 2, {X_SAD, 3600}, {0, 0});
}
void evUsb(uint32_t now, bool on) {
  usb = on; lastActivity = now; deepSleep = false; fullBat = false; usbSince = now;
  if (on) play(now, 2, {X_ELECTRIC, 1900}, {0, 0});     // le meten el cargador: ojos enormes + destello de los propios ojos -> carga (sin rayitas encima)
  else play(now, 2, {X_HAPPY, 1000}, {0, 0});
}
void evBatFull(uint32_t now) {
  if (usb) { fullBat = true; energy = 100; play(now, 2, {X_HAPPY, 2200}, {0, 0}); }       // lleno: feliz y vuelve a la normalidad (no se duerme)
}
void evBatLow(uint32_t now, bool on) { lowBat = on; if (!usb) play(now, 1, {0, 0}); }
void evPoke(uint32_t now) {
  wake(now);
  for (int e = 0; e < 2; e++) { eye[e].sx = 0.34f; eye[e].sy = -0.30f; eye[e].svx = eye[e].svy = 0; }
  if (expr == X_AWAKE) play(now, 2, {X_SURPRISE, 700}, {0, 0});
}
void evWink() { eye[1].blinkTot = eye[1].blinkT = 0.5f; }
void newLook() {                                     // otra criatura: genoma nuevo
  rng = rng * 1664525u + 1013904223u;
  genomeFromSeed(rng);
}
void evNewCreature(uint32_t now, uint32_t seed) {
  genomeFromSeed(seed); wake(now);
  play(now, 2, {X_SURPRISE, 800}, {0, 0});
  for (int i = 0; i < 8; i++) { float a = TAU * i / 8; spawn(PT_SPARK, 120 + cosf(a) * 36, 112 + sinf(a) * 34, cosf(a) * 60, sinf(a) * 55, 1.0f, 6, 4, 2); }
}
void evTheme(uint32_t now) { wake(now); newLook(); }

uint16_t cEye, cPup;
uint16_t pcols[10];
static uint8_t heartCol() { return 2; }               // los corazones son siempre rojos
// ---------------- NACIMIENTO = RULETA: pasan muchas combinaciones de ojos de izquierda a derecha, cada vez mas rapido;
// al final cierra los ojos y los abre con la criatura elegida (la que se queda hasta que se desconecte) ----------------
const int NBIRTH = 1;
uint32_t birthT0 = 0, birthPick = 0; float birthDur = 4700, birthPrevU = 0;
Genome finalG; bool birthSwap = false, birthRestored = false, birthSnap = false; int birthCombo = -1;
static float sstep(float a, float b, float x) { float k = clampf((x - a) / (b - a), 0, 1); return k * k * (3 - 2 * k); }
static float grow(float x) { if (x <= 0) return 0; if (x >= 1) return 1; return 1.0f - expf(-6.0f * x) * cosf(11.0f * x); }   // sube con rebote
void setGaze(float x, float y, uint32_t now, uint32_t holdMs);
// ---- cierre lento de ojos + miradas segun la personalidad ----
static float eyeShut(float ms, float c, float h, float o) {   // 0 abierto .. 1 cerrado (al abrir rebota un poco)
  if (ms <= 0) return 0;
  if (ms < c) return sstep(0, c, ms);
  if (ms < c + h) return 1;
  float x = (ms - c - h) / o;
  return x >= 1 ? 0.0f : 1.0f - grow(x);
}
struct LookPlan { uint32_t t0; uint8_t n, i; uint16_t step; float gx[6], gy[6]; };
LookPlan lookPlan = {0, 0, 0, 500, {0}, {0}};
void planLooks(uint32_t start) {         // mira para varios lados; el estilo depende de la personalidad
  const float* T = G.trait;
  float w[5] = {0.05f + 1.6f * T[4] * T[4], 0.05f + 1.6f * T[1] * T[1], 0.05f + 1.6f * T[0] * T[0], 0.05f + 1.6f * T[3] * T[3], 0.05f + 1.2f * T[2] * T[2]};
  float tot = w[0] + w[1] + w[2] + w[3] + w[4], r = frand01() * tot;
  int k = 0;
  while (k < 4 && r > w[k]) { r -= w[k]; k++; }
  static const float P[5][6][2] = {
    {{-1, -0.2f}, {1, -0.3f}, {0.2f, -1}, {-0.7f, 0.4f}, {0.8f, 0.3f}, {0, 0}},          // curiosa: mira ancho y alto
    {{-0.5f, 0.7f}, {0.4f, 0.6f}, {-0.3f, 0.8f}, {0, 0.2f}, {0, 0}, {0, 0}},             // timida: miradas bajas y cortas
    {{-1, 0}, {1, 0.1f}, {-0.8f, -0.6f}, {0.9f, -0.5f}, {-0.4f, 0.5f}, {0, 0}},          // energica: rapida
    {{-0.5f, 0.3f}, {0.5f, 0.2f}, {0, 0.5f}, {0, 0}, {0, 0}, {0, 0}},                    // dormilona: lenta
    {{-1, 0.2f}, {1, 0.2f}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}};                            // grunona: de reojo
  static const uint8_t N[5] = {6, 4, 6, 4, 3};
  lookPlan.n = N[k]; lookPlan.i = 0; lookPlan.t0 = start;
  for (int q = 0; q < 6; q++) { lookPlan.gx[q] = P[k][q][0] * (frand() > 0 ? 1.0f : -1.0f); lookPlan.gy[q] = P[k][q][1]; }
  float st = 850.0f - 450.0f * T[0] - 200.0f * T[4] + 300.0f * T[3];
  if (k == 2) st *= 0.7f;
  lookPlan.step = (uint16_t)clampf(st, 260, 1100);
}
void lookTick(uint32_t now) {
  if (!lookPlan.n || now < lookPlan.t0 + (uint32_t)lookPlan.i * lookPlan.step) return;
  setGaze(lookPlan.gx[lookPlan.i], lookPlan.gy[lookPlan.i], now, lookPlan.step + 200);
  if (++lookPlan.i >= lookPlan.n) lookPlan.n = 0;
}
uint32_t slowT0 = 0, nextSlow = 40000; float slowH = 2000;
const float SB_C = 550, SB_O = 750;
void evSlowBlink(uint32_t now) {         // cierra los ojos despacio, los mantiene un par de segundos, los abre y mira alrededor
  slowT0 = now; slowH = 1800.0f + 700.0f * frand01();
  planLooks(now + (uint32_t)(SB_C + slowH + SB_O));
}
void slowAdd(uint32_t now) {
  if (!slowT0) return;
  float ms = (float)(now - slowT0);
  if (ms > SB_C + slowH + SB_O) { slowT0 = 0; return; }
  float k = eyeShut(ms, SB_C, slowH, SB_O);
  clipAdd[1] -= 0.93f * k; clipAdd[0] += 0.30f * k;
}
void slowTgt(uint32_t now) {
  if (!slowT0) return;
  float k = eyeShut((float)(now - slowT0), SB_C, slowH, SB_O);
  tgt[P_OPEN] = k > 0.97f ? 0.0f : 1.0f;      // cerrado del todo = arco de ojo cerrado
}

// ---- NACIMIENTO = RULETA EN EL SITIO: los ojos casi no viajan (asi no hay rasgado de pantalla); la combinacion cambia cada vez mas
// rapido (de 1.5 a 30 por segundo, ~11 s) mientras rayas finas cruzan de izquierda a derecha. Al final frena, cierra, abre y mira. ----
// ---- PRELUDIO: rayas que giran en circulo y se aceleran muchisimo mientras crece el punto negro; del punto nacen los ojos de nube blancos;
// esos mismos ojos de nube aceleran, cambian de forma y toman color hasta ser los ojos de verdad ----
const float PR_W = 7000, PR_D = 0, PR_R = 380, PR_B = 3000, PR_S = 1300, PR_H = 800, PR_A = 0;
const float PRE = PR_W + PR_D + PR_R + PR_B + PR_S + PR_H + PR_A;       // ~11.5 s
struct OLine { float r, th, w, age; bool on; };
const int NOL = 64;
OLine olines[NOL]; float olAcc = 0, armPh = 0, armFl = 0;
struct Thread { float a0, k, len, rate, spd, df, wa, wf, wp, th, al, ph, fl, pin; };
const int NTH = 12;
Thread thr[NTH];
float birthShapeF = 1.0f, dotKick = 0.0f, dotR = 3.0f, eyeFadeA = 1.0f, spasm = 0.0f, bulX = 0.0f, bulY = 0.0f, feed = 0.0f; bool birthWhite = false;
float cloudSf = 0; int cloudPA = 1, cloudPB = 2;
float cloudRX = 34, cloudRY = 55, cloudGap = 38, cloudOffX = 0, cloudOffY = 0, cloudA = 1, cloudBump = 1, cloudSat = 0, cloudPup = 0;
bool cloudDone = false;
static void blobAA(float cx, float cy, float rx, float ry, float t, float seed, uint16_t col, float bump = 1.0f) {   // mancha organica (gelatina); bump 1 = nube con bultos, 0 = elipse lisa
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
const float RU_T1 = 15000, RU_C = 550, RU_H = 2000, RU_O = 750;
bool birthLooked = false;
float birthNextSwap = 0, birthLastSwap = 0, birthSwapDur = 600, streakAcc = 0; uint8_t birthStage = 0;
static float speedNorm(float interval) { return clampf(logf(650.0f / interval) / logf(650.0f / 33.0f), 0, 1); }
void evBirth(uint32_t now, int v) {
  (void)v;
  finalG = G;                            // la criatura elegida (ya sorteada)
  birthVar = 0; birthT0 = now; birthPrevU = 0; birthRestored = false; birthCombo = -1; birthLooked = false; slowT0 = 0;
  birthNextSwap = 900; birthLastSwap = -1e6f; birthShapeF = 0; birthWhite = false; dotKick = 0; birthSwapDur = 600; streakAcc = 0; birthStage = 0;
  birthDur = PRE + RU_T1 + RU_C + RU_H + RU_O + 2300.0f;
  memset(olines, 0, sizeof(olines)); olAcc = 0; armPh = 0; armFl = 0;
  for (int a = 0; a < NTH; a++)                    // hilos irregulares: cada uno con su largo, curvatura, velocidad y ondulacion
    thr[a] = {TAU * a / NTH + 0.25f * frand(), 2.7f + 0.9f * frand01(), 0.72f + 0.28f * frand01(), 0.95f + 0.12f * frand01(), 0.85f + 0.30f * frand01(),
              4.0f + 1.6f * frand01(), 0.015f + 0.02f * frand01(), 1.0f, TAU * frand01(), 2.2f + 0.6f * frand01(), 0.80f + 0.15f * frand01(), 0.0f, TAU * frand01()};
  cloudRX = 34; cloudRY = 55; cloudGap = 38; cloudOffX = cloudOffY = 0; cloudA = 1; dotR = 3; spasm = 0; bulX = bulY = feed = 0; cloudDone = false; cloudBump = 1; cloudSat = 0; cloudPup = 0; cloudSf = 0;   // (restaurado: reinicio de la nube en cada nacimiento)
  lastActivity = now; deepSleep = false;
  memset(parts, 0, sizeof(parts)); memset(slots, 0, sizeof(slots)); memset(clipAdd, 0, sizeof(clipAdd)); slosh = 0; shakeAmp = 0;
  birthPlay = true; play(now, 3, {X_AWAKE, (uint16_t)birthDur}, {X_HAPPY, 1500}, {0, 0}); birthPlay = false;
}
struct SpawnOK { SpawnOK() { birthSpawn = true; } ~SpawnOK() { birthSpawn = false; } };
void birthAdd(uint32_t now) {            // justo despues de clipUpdate
  if (birthVar < 0) return;
  SpawnOK sok;
  float ms = (float)(now - birthT0);
  if (ms >= birthDur) { if (!birthRestored) G = finalG; birthVar = -1; birthSnap = true; return; }
  if (ms >= PRE - PR_A && ms < PRE) clipAdd[9] -= 1.0f;     // ojos blancos SIN pupila al nacer (despues la pupila aparece sola)
  if (ms < PRE) return;                    // preludio: lo dibuja birthFx
  ms -= PRE;
  if (ms < RU_T1) {
    clipAdd[9] -= 0.7f * (1.0f - sstep(4500.0f, 9000.0f, ms));                               // la pupila va apareciendo
    float x = ms / RU_T1, itv;
    if (x < 0.80f) itv = 650.0f * powf(33.0f / 650.0f, x / 0.80f);           // acelera geometricamente: sigue y sigue
    else if (x < 0.90f) itv = 33.0f;                                          // a tope
    else itv = 33.0f * powf(420.0f / 33.0f, (x - 0.90f) / 0.10f);             // frena como un rodillo que se asienta
    float sn = speedNorm(itv);
    if (ms >= birthNextSwap) {                                                // combinacion nueva (tiempos algo irregulares: como animado a mano)
      float jit = 0.80f + 0.40f * frand01();
      if (x > 0.15f && x < 0.75f && frand01() < 0.04f) jit = 2.2f;           // duda un instante
      birthSwapDur = fmaxf(33.0f, itv * jit);
      birthLastSwap = ms; birthNextSwap = ms + birthSwapDur;
      if (!cloudDone) {                                                        // con cada cambio brusco la nube da un pasito: otra forma, menos bultos, mas color, asoma la pupila
        float cf = clampf(x / 0.30f, 0, 1), va = 0.06f + 0.50f * cf;
        cloudRX = 34.0f * (1 + va * frand()); cloudRY = 55.0f * (1 + 0.7f * va * frand()); cloudGap = 38.0f * (1 + 0.35f * va * frand());
        cloudBump = 1.0f - cf; cloudSat = powf(x, 0.5f); cloudPup = 0.0f; cloudSf = clampf(0.25f + 1.5f * cf, 0, 1); cloudPA = 1 + (int)(frand01() * 6.99f); cloudPB = 1 + (int)(frand01() * 6.99f);
        if (x >= 0.30f) cloudDone = true;                                      // en este cambio pasa a los ojos de verdad (un corte mas de la ruleta)
      }
      int bp = (int)(frand01() * (NPALS - 0.01f));
      if (x > 0.30f) {                                                         // cada vez mas colorido: elige la mas saturada entre varias
        int tries = 1 + (int)(3.0f * x);
        for (int q = 0; q < tries; q++) {
          int c = (int)(frand01() * (NPALS - 0.01f));
          const uint8_t* a = PALS[c].e; const uint8_t* b = PALS[bp].e;
          if (fmaxf(a[0], fmaxf(a[1], a[2])) - fminf(a[0], fminf(a[1], a[2])) > fmaxf(b[0], fmaxf(b[1], b[2])) - fminf(b[0], fminf(b[1], b[2]))) bp = c;
        }
      }
      G.pal = bp; G.species = (int)(frand01() * (NSPECIES - 0.01f)); G.pupil = (int)(frand01() * (NPUPILS - 0.01f));
      { float sf = clampf(0.3f + 1.1f * x, 0, 1); for (int e = 0; e < 2; e++) { int pz = 1 + (int)(frand01() * 6.99f); poseSet(fcur[e], pz, sf); poseSet(ftgt[e], pz, sf); } }   // cada paso de la ruleta = otra silueta de boceto
      birthSnap = true;
    }
    float tc = clampf((ms - birthLastSwap) / birthSwapDur, 0, 1), e = (1 - tc) * (1 - tc);
    float A = x < 0.90f ? 55.0f - 30.0f * clampf(x / 0.80f, 0, 1) : 25.0f + 40.0f * ((x - 0.90f) / 0.10f);   // entra desde la izquierda y se asienta
    clipAdd[2] -= A * e; cloudOffX = -A * e;
    clipAdd[0] += 0.14f * e; clipAdd[1] -= 0.06f * e;                                                         // pequeno estiron al entrar
    clipAdd[1] -= 0.08f;
    float tens = x < 0.90f ? x * x : 0.0f;
    clipAdd[4] += 0.10f * tens * sinf(TAU * (1.0f + 2.5f * x) * ms * 0.001f);
    clipAdd[3] += (1.5f + 2.5f * tens) * sinf(TAU * 2.0f * ms * 0.001f); cloudOffY = (1.5f + 2.5f * tens) * sinf(TAU * 2.0f * ms * 0.001f);
    float rate = (x < 0.93f) ? 2.5f + 55.0f * sn : 0.0f;                      // rayas de velocidad de izquierda a derecha
    streakAcc += rate * DT;
    while (streakAcc >= 1.0f) {
      streakAcc -= 1.0f;
      spawn(PT_LINE, -30.0f + 40.0f * frand01(), 56.0f + 128.0f * frand01(), 1900.0f + 2300.0f * frand01() * (0.4f + sn), 0.0f,
            0.13f + 0.12f * frand01(), 16.0f + 36.0f * frand01() * (0.4f + sn), frand01() < 0.65f ? 0 : 4, 0, 0);
    }
  } else {
    float m = ms - RU_T1, k = eyeShut(m, RU_C, RU_H, RU_O);
    clipAdd[1] -= 0.93f * k; clipAdd[0] += 0.30f * k;
    if (m > RU_C + 400 && !birthRestored) { G = finalG; birthRestored = true; birthSnap = true; for (int e = 0; e < 2; e++) { formReset(fcur[e]); formReset(ftgt[e]); } }   // cerrados: cambia por el elegido
    float mo = m - (RU_C + RU_H + 150);                                       // al abrir: rayitas sobrias que salen en tandas desiguales
    static const float ST[5] = {0, 70, 150, 250, 370};
    while (birthStage < 5 && mo >= ST[birthStage]) {
      int n = 2 + (int)(frand01() * 1.99f);
      for (int q = 0; q < n; q++) {
        float a = TAU * frand01(), v = 110.0f + 120.0f * frand01();
        spawn(PT_LINE, 120 + cosf(a) * (58 + 8 * frand01()), 118 + sinf(a) * (54 + 8 * frand01()), cosf(a) * v, sinf(a) * v,
              0.30f + 0.25f * frand01(), 7.0f + 15.0f * frand01(), frand01() < 0.6f ? 0 : 4, 0, a);
      }
      birthStage++;
    }
    if (m >= RU_C + RU_H + RU_O * 0.6f && !birthLooked) { birthLooked = true; planLooks(now); }
  }
}
void birthTgt(uint32_t now) {            // justo despues de setTargets
  if (birthVar < 0) { birthShapeF = 1.0f; eyeFadeA = 1.0f; if (birthSnap) { for (int i = 0; i < NP; i++) if (i != P_OPEN) { cur[i] = tgt[i]; vel[i] = 0; } birthSnap = false; } return; }
  float ms = (float)(now - birthT0) - PRE;
  float x = ms > 0 ? clampf(ms / RU_T1, 0, 1) : 0.0f;
  birthShapeF = powf(x, 0.9f);                                          // las formas se van animando
  float f = (ms > 0 && cloudDone) ? 1.0f : 0.0f;                         // corte (sin desvanecer) de la nube a los ojos de verdad
  cloudA = 1.0f - f;
  tgt[P_OPEN] = 1.0f;
  if (ms < 0) tgt[P_OPEN] = 0.0f;
  eyeFadeA = (ms < 0) ? 0.0f : ((ms < RU_T1 && !cloudDone) ? 0.0f : 1.0f);
  if (ms >= 0) {                                                        // de opaco (blanco/gris) a muy colorido
    float sat = powf(x, 0.8f);
    static const float WE[3] = {244, 244, 248}, WP[3] = {228, 228, 238};
    for (int c = 0; c < 3; c++) { tgt[P_ER + c] = WE[c] + (tgt[P_ER + c] - WE[c]) * sat; tgt[P_PR + c] = WP[c] + (tgt[P_PR + c] - WP[c]) * sat; }
    if (!birthWhite) { birthWhite = true; birthSnap = true; }
  }
  if (ms >= RU_T1 && eyeShut(ms - RU_T1, RU_C, RU_H, RU_O) > 0.97f) tgt[P_OPEN] = 0.0f;   // cerrados: arco
  if (birthSnap) { for (int i = 0; i < NP; i++) if (i != P_OPEN) { cur[i] = tgt[i]; vel[i] = 0; } birthSnap = false; }
}
static void fillAll(uint16_t col) { uint32_t v = (uint32_t)col | ((uint32_t)col << 16); uint32_t* q = (uint32_t*)fb; for (int i = 0; i < W * H / 2; i++) q[i] = v; }   // relleno rapido
static void drawCloudEyes(float t, float x, float alpha, float offx, float offy, float rx, float ry, float gap, uint16_t tint, uint16_t core, float bump, float pupR, uint16_t pupCol) {
  float tw = t * (1.8f + 3.0f * x), cy = 122.0f + offy;
  for (int lobe = 0; lobe < 2; lobe++) {
    float cx = 120 + (lobe ? gap : -gap) + offx, seed = lobe * 2.3f;
    setAlpha(alpha);
    if (cloudSf > 0.04f) {                       // silueta de boceto (en vez de la nube redonda)
      float f[NF]; poseSet(f, lobe ? cloudPB : cloudPA, cloudSf);
      EyeDraw Q; memset(&Q, 0, sizeof(Q));
      Q.cx = cx; Q.cy = cy; Q.rx = rx * f[F_AX]; Q.ry = ry * f[F_AY] * 0.9f; Q.ang = 0; Q.inner = lobe ? -1.0f : 1.0f;
      Q.pz[0] = f[F_FL]; Q.pz[1] = f[F_CA]; Q.pz[2] = f[F_CH]; Q.pz[3] = f[F_CW]; Q.pz[4] = f[F_TH]; Q.pz[5] = f[F_TW]; Q.pz[6] = f[F_LN]; Q.pz[7] = f[F_HK]; Q.pz[8] = f[F_NT];
      Q.t = tw; Q.seed = (float)lobe; Q.flip = styleMir && lobe == 1; Q.cEye = tint; Q.cPup = tint; Q.cAcc = tint;
      renderEye(Q);
    } else blobAA(cx, cy, rx, ry, tw, seed, tint, bump);
    if (cloudSf < 0.3f) blobAA(cx - 0.05f * rx, cy - 0.04f * ry, rx * 0.55f, ry * 0.55f, tw * 1.1f, seed + 1.0f, core, bump);
    if (pupR > 0.8f) ellipseAA(cx + (lobe ? -3.0f : 3.0f), cy + 9.0f, pupR * 0.85f, pupR * 1.15f, pupCol);
  }
}
void birthFx(uint32_t now) {             // dibujo del preludio y de los ojos de nube
  if (birthVar < 0) return;
  float ms = (float)(now - birthT0), t = ms * 0.001f;
  uint16_t light = rgb565(215, 220, 236), cream = rgb565(250, 226, 216), white = rgb565(255, 255, 255), pale = rgb565(255, 240, 245), dk = rgb565(40, 40, 52), lite = rgb565(238, 240, 250);
  if (ms >= PRE) {                                                         // ojos de nube que aceleran, cambian de forma y toman color
    float x = clampf((ms - PRE) / RU_T1, 0, 1);
    if (cloudA > 0.02f && ms - PRE < RU_T1) {
      float sat = cloudSat;                                                // escalonado: cambia solo en cada paso
      const uint8_t* pe = PALS[G.pal].e; const uint8_t* pp = PALS[G.pal].p;
      uint16_t tint = rgb565((uint8_t)(250 + (pe[0] - 250) * sat), (uint8_t)(226 + (pe[1] - 226) * sat), (uint8_t)(216 + (pe[2] - 216) * sat));
      uint16_t pcol = rgb565((uint8_t)(70 + (pp[0] - 70) * sat), (uint8_t)(70 + (pp[1] - 70) * sat), (uint8_t)(80 + (pp[2] - 80) * sat));
      drawCloudEyes(t, x, cloudA, cloudOffX, cloudOffY, cloudRX, cloudRY, cloudGap, tint, white, cloudBump, cloudPup, pcol);
    }
    setAlpha(1);
    return;
  }
  float tb = ms - (PR_W + PR_D + PR_R);
  uint16_t bg = 0;                                                          // fondo NEGRO (invertido a pedido de Alessandro)
  setAlpha(1); fillAll(bg);
  float x = clampf(ms / PR_W, 0, 1);
  if (ms < PR_W + PR_D) {                                                  // rios de hilos que fluyen en espiral y convergen en el puntito; nunca frenan
    dotKick *= expf(-DT * 6.0f);
    float g = clampf(ms / (PR_W + PR_D), 0, 1);
    dotR = 3.0f + 23.0f * powf(g, 1.5f);                                   // la particula crece conforme se alimenta
    const int NS = 26;
    static float PW[NS + 1], U8[NS + 1]; static bool tab = false;
    if (!tab) { tab = true; for (int q = 0; q <= NS; q++) { float u = (float)q / NS; PW[q] = powf(1 - u, 1.3f); U8[q] = powf(u, 0.8f); } }
    float w = 1.0f + 33.0f * x * x, fv = 0.5f + 13.0f * x * x;             // giro y flujo aceleran sin parar
    for (int a = 0; a < NTH; a++) {
      Thread& h = thr[a];
      h.ph += w * h.spd * DT; h.fl += fv * h.spd * DT;
      float ro = dotR + (108.0f - dotR) * powf(fmaxf(0.0f, 1.0f - x * h.rate), 0.9f) * h.len;   // todo se va metiendo: los bordes quedan vacios
      if (ro < dotR + 1.5f) continue;
      {                                                                   // cada trazo que llega a la particula la empuja y la alimenta
        float pin = fmodf(h.df - h.fl, 1.0f); if (pin < 0) pin += 1.0f;
        if (pin > h.pin + 0.5f) {
          float ai = h.a0 + h.ph + h.k + h.wa * sinf(TAU * (h.wf - 0.35f * t) + h.wp), mg = 2.4f * (1.0f - 0.65f * x) * h.al;
          bulX += cosf(ai) * mg; bulY += sinf(ai) * mg; feed += 0.055f * (1.0f - 0.5f * x);
        }
        h.pin = pin;
      }
      float px = 0, py = 0;
      for (int q = 0; q <= NS; q++) {
        float u = (float)q / NS, r = dotR + (ro - dotR) * PW[q];
        float ang = h.a0 + h.ph + h.k * U8[q] + h.wa * sinf(TAU * (h.wf * u - 0.35f * t) + h.wp);   // curvatura propia + ondulacion organica
        float xx = 120 + r * cosf(ang), yy = 118 + r * sinf(ang);
        if (q > 0) {
          float um = u - 0.5f / NS, ph = fmodf(um * h.df - h.fl, 1.0f); if (ph < 0) ph += 1.0f;   // los trazos fluyen hacia el centro
          if (ph < 0.52f) { setAlpha(h.al * (0.55f + 0.40f * u)); lineAA(px, py, xx, yy, h.th * (0.8f + 0.4f * u), lite); }
        }
        px = xx; py = yy;
      }
    }
    spasm *= expf(-DT * 8.0f); bulX *= expf(-DT * 7.0f); bulY *= expf(-DT * 7.0f); feed *= expf(-DT * 6.0f);
    bulX = clampf(bulX, -7.0f, 7.0f); bulY = clampf(bulY, -7.0f, 7.0f); feed = fminf(feed, 0.45f);
    if (frand01() < DT * (0.8f + 10.0f * x * x)) spasm += (0.35f + 0.45f * frand01()) * (frand() > 0 ? 1.0f : -0.6f);   // espasmos
    float rr = dotR * (1 + 0.10f * sinf(TAU * (2.5f + 8.0f * x) * t)) * (1 + feed);                                    // crece con cada bocado
    float sxs = 1 + 0.45f * spasm + 0.05f * fabsf(bulX), sys = 1 - 0.30f * spasm + 0.05f * fabsf(bulY);
    setAlpha(1); blobAA(120 + bulX + 2.0f * spasm * sinf(t * 40.0f), 118 + bulY + 1.6f * spasm * cosf(t * 33.0f), rr * sxs, rr * sys, t * (2.5f + 7.0f * x), 0.7f, white);   // particula-nube blanca
  } else if (ms < PR_W + PR_D + PR_R) {                                    // el punto da un golpe: onda y asoma lo blanco
    float u = (ms - PR_W - PR_D) / PR_R, R = dotR + 175.0f * powf(u, 0.55f);
    {                                                                        // EXPLOTA: aro de onda de choque un poco irregular y rayos desiguales
      setAlpha((1 - u) * (1 - u) * 0.95f);
      float lx = 0, ly = 0;
      for (int q = 0; q <= 40; q++) {
        float a = TAU * q / 40, rg = R * (1 + 0.07f * sinf(3 * a + 1.7f + 6 * u) + 0.05f * sinf(5 * a + 4.1f - 4 * u));
        float xx = 120 + cosf(a) * rg, yy = 118 + sinf(a) * rg;
        if (q > 0) lineAA(lx, ly, xx, yy, 2.8f, white);
        lx = xx; ly = yy;
      }
      for (int q = 0; q < 20; q++) {
        float h1 = sinf(q * 12.9898f) * 43758.5453f; h1 -= floorf(h1);
        float h2 = sinf(q * 78.233f) * 12345.678f; h2 -= floorf(h2);
        float a = TAU * q / 20 + (h1 - 0.5f) * 0.35f, dl = h2 * 0.25f;
        if (u <= dl) continue;
        float r0 = R * (0.15f + 0.35f * h1), r1 = R * (0.55f + 0.38f * h2);
        setAlpha((1 - u) * (0.55f + 0.40f * h2));
        lineAA(120 + cosf(a) * r0, 118 + sinf(a) * r0, 120 + cosf(a) * r1, 118 + sinf(a) * r1, 1.5f + 1.8f * h1, white);
      }
    }
    setAlpha(1); if (u < 0.6f) blobAA(120, 118, dotR * (1 - u / 0.6f) + 1.0f, dotR * (1 - u / 0.6f) + 1.0f, t * 9.0f, 0.7f, white);   // la nube blanca se encoge al explotar
    blobAA(120, 118, 2.0f + 6.0f * u, 2.0f + 6.0f * u, t * 1.8f, 0.5f, white);
  } else {                                                                 // del punto negro nace la nube blanca y se parte en dos ojos de nube
    const float RX0 = 60.0f, RY0 = 49.0f;
    float tw = t * 1.8f;
    float u = clampf(tb / PR_B, 0, 1), r = 5.0f + 51.0f * grow(u), k = r / 56.0f;
    k *= 1.0f + 0.04f * sinf(TAU * 3.0f * t) + 0.10f * expf(-tb / 1400.0f) * sinf(TAU * 2.4f * tb * 0.001f);
    float e = sstep(0, 1, (tb - PR_B) / PR_S);
    float breathe = tb > PR_B + PR_S ? 2.5f * sinf(TAU * 1.8f * t) : 0.0f;
    float rxl = (RX0 * k) * (1 - e) + 34.0f * e, ryl = RY0 * k + 6.0f * e + breathe, d = 38.0f * e;
    float CY = 118.0f + 4.0f * sstep(0, 0.6f, u);                            // nace en el punto y baja a su sitio
    float dr = dotR * (1 - sstep(0, 0.7f, u));                              // el punto negro se encoge mientras nace lo blanco
    if (dr > 0.5f) { setAlpha(1); ellipseAA(120, 118, dr, dr, 0); }
    for (int lobe = 0; lobe < 2; lobe++) {
      float cx = 120 + (lobe ? d : -d), seed = lobe * 2.3f;
      setAlpha(1);     blobAA(cx, CY, rxl, ryl, tw, seed, cream);
      blobAA(cx - 0.05f * rxl, CY - 0.04f * ryl, rxl * 0.55f, ryl * 0.55f, tw * 1.1f, seed + 1.0f, white);
      if (d < 0.5f) break;
    }
  }
  setAlpha(1);
}
const int NEMO = 14;
const uint8_t EMO[NEMO] = {X_CURIOUS, X_HAPPY, X_LOVE, X_SURPRISE, X_SMUG, X_HYPE, X_FULL, X_ANGRY, X_SAD, X_ANXIOUS, X_COLD, X_DIZZY, X_YAWN, X_LOWBAT};
uint8_t emoIdx = 0;
void evEmotion(uint32_t now) {           // boton azul: pasa a la SIGUIENTE emocion de la lista (en orden), por un momento
  if (isAsleep()) { wake(now); return; }
  lastActivity = now;
  uint8_t x = EMO[emoIdx % NEMO]; emoIdx = (uint8_t)((emoIdx + 1) % NEMO);
  play(now, 2, {x, 3400}, {0, 0});
}
uint8_t cycleIdx = 0;
const uint8_t CYCLE[19] = {X_CURIOUS, X_HAPPY, X_ANGRY, X_LOVE, X_SAD, X_HYPE, X_LOWBAT, X_SURPRISE, X_SMUG,
                           X_DIZZY, X_ELECTRIC, X_CHARGING, X_YAWN, X_ANXIOUS, X_COLD, X_BATCHECK, X_DIMSLEEP, X_FULL, X_SLEEP};
void evNextExpr(uint32_t now) {
  uint8_t x = CYCLE[cycleIdx];
  cycleIdx = (cycleIdx + 1) % 19;
  evSet(x, now, (x == X_SLEEP || x == X_DIMSLEEP || x == X_CHARGING) ? 4500 : 3400);
}
void setGaze(float x, float y, uint32_t now, uint32_t holdMs) {
  tgx = clampf(x, -1, 1); tgy = clampf(y, -1, 1);
  gazeUntil = now + holdMs;
}
#include "Motion.h"

// ============================================================================================
//  Animacion
// ============================================================================================


void setTargets() {
  for (int i = 0; i < NP; i++) tgt[i] = POSE[expr][i];
  const Pal& PL = PALS[G.pal];
  const PupilStyle& PU = PUPILS[G.pupil];
  bool usesPal = (expr == X_AWAKE || expr == X_YAWN || expr == X_SURPRISE || expr == X_ANXIOUS || expr == X_SLEEP ||
                  expr == X_HAPPY || expr == X_DIMSLEEP || expr == X_LOWBAT ||
                  expr == X_CURIOUS || expr == X_SMUG || expr == X_HYPE || expr == X_DIZZY || expr == X_SAD || expr == X_COLD || expr == X_CHARGING || expr == X_ELECTRIC || (expr == X_LOVE && loveVar != 2));
  if (usesPal) {                       // la criatura usa SU paleta (los arcos de dormir/feliz tambien)
    float dim = (expr == X_DIMSLEEP || expr == X_LOWBAT) ? 0.8f : (expr == X_SAD || expr == X_COLD) ? 0.88f : 1.0f;
    for (int j = 0; j < 3; j++) { tgt[P_ER + j] = PL.e[j] * dim; tgt[P_PR + j] = PL.p[j]; }
  }
  if (expr == X_AWAKE || expr == X_YAWN || expr == X_LOWBAT || expr == X_CURIOUS || expr == X_SMUG || expr == X_HYPE || expr == X_DIZZY || expr == X_SAD || expr == X_COLD || (expr == X_CHARGING && chargeType == 1) || (expr == X_LOVE && loveVar == 0)) {
    tgt[P_PRX] = PU.prx; tgt[P_PRY] = PU.pry; tgt[P_PDX] = PU.pdx; tgt[P_PDY] = PU.pdy; tgt[P_PTILT] = PU.ptilt;
  } else if (expr == X_SURPRISE || expr == X_ANXIOUS || expr == X_ELECTRIC) {
    float f = expr == X_SURPRISE ? 0.45f : 0.55f;
    tgt[P_PRX] = fmaxf(2.5f, PU.prx * f); tgt[P_PRY] = fmaxf(4.0f, PU.pry * f); tgt[P_PDX] = PU.pdx * 0.5f; tgt[P_PDY] = PU.pdy * 0.7f;
    tgt[P_PTILT] += PU.ptilt;
  } else if (tgt[P_PRX] > 0.5f) tgt[P_PTILT] += PU.ptilt;
  if (tgt[P_PRX] > 0.5f) { tgt[P_PC1] += PU.pc1; tgt[P_PC2] += PU.pc2; }
  cAccC = rgb565(PL.a[0], PL.a[1], PL.a[2]);
  if (expr == X_LOWBAT) { tgt[P_LIDT] = 0.40f; tgt[P_LIDB] = 0.08f; tgt[P_Y] += 4; }   // cansado: parpados pesados, pupila normal
  if (expr == X_BATCHECK) {            // los ojos son el medidor: se llenan de abajo hacia arriba y cambian rojo -> amarillo -> verde
    float lvl = clampf(energy / 100.0f, 0.02f, 1);
    float u = clampf((gNow - exprStart) / 1100.0f, 0, 1); u = 1 - (1 - u) * (1 - u);
    float sh = lvl * u, r, g, b;
    tgt[P_LIDT] = 0.86f * (1.0f - sh) + 0.012f * sinf(TAU * 1.1f * gNow * 0.001f); tgt[P_SLOPE] = 0.05f * sinf(TAU * 0.8f * gNow * 0.001f);   // recipiente: la superficie ondula
    { static const float BS[5][4] = {{0, 236, 64, 64}, {0.30f, 244, 128, 56}, {0.55f, 240, 196, 70}, {0.78f, 150, 214, 96}, {1, 72, 208, 138}};   // rojo > naranja > amarillo > lima > esmeralda (sin tonos pantano)
      int i = 1; while (i < 4 && sh > BS[i][0]) i++;
      float k = clampf((sh - BS[i - 1][0]) / (BS[i][0] - BS[i - 1][0]), 0, 1);
      r = BS[i - 1][1] + (BS[i][1] - BS[i - 1][1]) * k; g = BS[i - 1][2] + (BS[i][2] - BS[i - 1][2]) * k; b = BS[i - 1][3] + (BS[i][3] - BS[i - 1][3]) * k; }
    tgt[P_ER] = r; tgt[P_EG] = g; tgt[P_EB] = b;
  }
  float tt = gNow * 0.001f;
  if (expr == X_CHARGING) {             // 3 tipos de carga
    float lvl = clampf(energy / 100.0f, 0.02f, 1);
    if (chargeType == 0) { tgt[P_LIDT] = 0.86f * (1.0f - lvl) + 0.012f * sinf(TAU * 1.1f * tt); tgt[P_SLOPE] = 0.05f * sinf(TAU * 0.8f * tt); tgt[P_PRX] = tgt[P_PRY] = 0; }   // ojos-recipiente que se llenan
    else if (chargeType == 1) { float pl = 0.5f + 0.5f * sinf(TAU * 0.9f * tt); tgt[P_RX] *= 1.08f + 0.07f * pl; tgt[P_RY] *= 1.08f + 0.07f * pl; for (int c = 0; c < 3; c++) tgt[P_ER + c] += (255.0f - tgt[P_ER + c]) * 0.30f * pl; }   // respira y brilla                                                                                              // ondas de energia
    else { tgt[P_OPEN] = 0; tgt[P_ARCDIR] = -1; tgt[P_Y] += 2.5f * sinf(TAU * 0.7f * tt); }                                                                            // ronroneo: ojos cerrados felices
  }
  if (expr == X_ELECTRIC) {             // le meten el cargador: ojos enormes y un destello DE LOS PROPIOS OJOS (sin nada encima)
    tgt[P_RX] *= 1.22f; tgt[P_RY] *= 1.28f;
    float ft = (gNow - exprStart) * 0.001f, w = fmaxf(0.0f, 1.0f - ft / 0.45f);
    if (ft > 0.55f) w = fmaxf(w, 0.7f * (1.0f - (ft - 0.55f) / 0.30f));
    for (int c = 0; c < 3; c++) tgt[P_ER + c] += (255.0f - tgt[P_ER + c]) * w;
  }
  if (expr == X_LOVE) {                 // 0 feliz con sus colores, 1 ojos cerrados felices, 2 clasico rosa, 3 corazones en los ojos
    if (loveVar == 1) { tgt[P_OPEN] = 0; tgt[P_ARCDIR] = -1; }
    if (loveVar != 2) tgt[P_Y] += -3.0f * fabsf(sinf(TAU * 1.3f * tt));
  }
  if (expr == X_FULL) {
    const uint8_t* fp = FULLP[fullPal];
    tgt[P_ER] = fp[3]; tgt[P_EG] = fp[4]; tgt[P_EB] = fp[5]; tgt[P_PR] = fp[6]; tgt[P_PG] = fp[7]; tgt[P_PB] = fp[8];
  }
  if (expr == X_ANGRY) {                // 9 formas de mostrar el enojo; colores rojos/naranjas
    const uint8_t* ac = ANGC[angryCol % 6];
    float ur = clampf((gNow - exprStart) / 2600.0f, 0, 1);
    tgt[P_ER] = ac[0]; tgt[P_EG] = ac[1]; tgt[P_EB] = ac[2];
    switch (angryVar % 9) {
      case 1: tgt[P_SLOPE] = 0.90f; tgt[P_PRX] *= 0.6f; tgt[P_PRY] *= 0.6f; break;                                  // furia: mas inclinado, pupila diminuta
      case 2: tgt[P_SLOPE] = 0.80f; tgt[P_RY] *= 0.86f; break;                                                       // ceño frio: angostos y quieto
      case 3: tgt[P_Y] += -5.0f * fabsf(sinf(TAU * 2.0f * tt)); break;                                              // berrinche: salta
      case 4: tgt[P_SLOPE] = 0.10f; tgt[P_RY] *= 0.94f; break;                                                       // cejas: ojos redondos, las cejas hacen el enojo
      case 5: tgt[P_SLOPE] = 0.70f; tgt[P_RY] *= 0.80f; tgt[P_RX] *= 1.05f; break;                                  // vapor por las orejas
      case 6: {                                                                                                       // fuego: ojos-llama que parpadean
        const uint8_t* fc = ANGC[(angryCol % 3 == 0) ? 2 : (angryCol % 3 == 1 ? 3 : 0)];
        tgt[P_ER] = fc[0]; tgt[P_EG] = fc[1] + 35.0f * sinf(TAU * 7.0f * tt); tgt[P_EB] = fc[2];
        tgt[P_SLOPE] = 0.20f; tgt[P_RX] *= 0.74f; tgt[P_RY] *= 1.12f; tgt[P_GAP] *= 1.14f; tgt[P_C1 + 3] = -0.24f; tgt[P_WOB] = 0.14f; tgt[P_PRX] = tgt[P_PRY] = 0; } break;
      case 8:                                                                                                         // hervir: se va enrojeciendo y apretando
        for (int c = 0; c < 3; c++) tgt[P_ER + c] = PL.e[c] * (1 - ur) + ac[c] * ur;
        tgt[P_SLOPE] = 0.20f + 0.60f * ur; tgt[P_RY] *= 1.0f - 0.12f * ur; break;
      default: break;                                                                                                 // 0 clasico, 7 chevrones (se dibujan aparte)
    }
  }
}

void updateStats(uint32_t now) {
  float dt = DT * timeScale;
  bool sleeping = isAsleep();
  if (usb) energy = clampf(energy + 1.6f * dt, 0, 100);
  else energy = clampf(energy - (sleeping ? -0.35f : 0.07f) * dt, 0, 100);
  fun = clampf(fun - (sleeping ? 0.01f : 0.10f) * dt, 0, 100);
  affection = clampf(affection - 0.03f * dt, 0, 100);
  if (simBatt) {                        // bateria simulada (sin sensor): 100 -> 3 % en 3 min, 1 min vacia, vuelve a 100 %
    if (usb) { simT = (100.0f - energy) / 97.0f * SIM_DRAIN; }
    else { simT += dt; if (simT >= SIM_DRAIN + SIM_HOLD) simT = 0; energy = simT < SIM_DRAIN ? 100.0f - 97.0f * (simT / SIM_DRAIN) : 3.0f; }
  }
  if (usb && energy >= 99.9f && !fullBat && now - usbSince > 14000) evBatFull(now);
}

// al entrar a una expresion: "patada" de gelatina + rafaga de particulas
void onEnter(uint8_t x, uint32_t now) {
  (void)now;
  if (x == X_CHARGING) {                // tipo de carga al azar segun la personalidad: 0 llenado, 1 ondas de energia, 2 ronroneo
    const float* T = G.trait;
    float w0 = 1.0f, w1 = 0.5f + T[0], w2 = 0.4f + T[5] + 0.6f * T[3], r = frand01() * (w0 + w1 + w2);
    chargeType = r < w0 ? 0 : (r < w0 + w1 ? 1 : 2);
    if (forceChargeType >= 0) chargeType = (uint8_t)forceChargeType;
  }
  if (x == X_LOVE) { loveVar = 3; if (forceLoveVar >= 0) loveVar = (uint8_t)forceLoveVar; }
  if (x == X_FULL) { fullPal = (uint8_t)(frand01() * 5.99f); if (forceFullPal >= 0) fullPal = (uint8_t)forceFullPal; }
  if (x == X_ANGRY) {
    angryVar = (uint8_t)(frand01() * 8.99f); angryCol = (uint8_t)(frand01() * 5.99f);
    if (forceAngryVar >= 0) angryVar = (uint8_t)forceAngryVar;
    if (forceAngryCol >= 0) angryCol = (uint8_t)forceAngryCol;
  }
  float cxm = 120, cym = 112;
  for (int e = 0; e < 2; e++) {
    Eye& E = eye[e];
    switch (x) {
      case X_HYPE:     E.sx = -0.12f; E.sy = 0.34f; break;
      case X_LOVE:     E.sx = 0.14f; E.sy = -0.14f; break;
      case X_SURPRISE: E.sx = 0.30f; E.sy = 0.34f; break;
      case X_ANGRY:    E.sx = -0.18f; E.sy = 0.12f; break;
      case X_SAD:      E.sx = 0.16f; E.sy = -0.26f; break;
      case X_HAPPY:    E.sx = -0.08f; E.sy = 0.22f; break;
      case X_CURIOUS:  E.sx = 0.12f; E.sy = 0.10f; break;
      case X_ELECTRIC: E.sx = 0.45f; E.sy = 0.40f; break;
      case X_DIZZY:    E.sx = 0.2f; E.sy = -0.1f; break;
      case X_SMUG:     E.sx = 0.10f; E.sy = -0.12f; break;
      case X_ANXIOUS:  E.sx = 0.22f; E.sy = 0.26f; break;
      case X_BATCHECK: E.sx = 0.10f; E.sy = 0.10f; break;
      default: break;
    }
  }
  for (int e = 0; e < 2; e++) { eye[e].sx *= 1.45f; eye[e].sy *= 1.45f; }     // entradas mas exageradas
  if (x == X_DIZZY) slosh = fmaxf(slosh, 0.8f);
  if (x == X_ANGRY) { static const float SH[9] = {4.0f, 6.5f, 1.2f, 3.5f, 3.0f, 2.5f, 3.0f, 4.0f, 1.0f}; shakeAmp = SH[angryVar % 9]; }
  if (x == X_ELECTRIC) shakeAmp = 8.0f;     // le metieron el cargador: sacudida fuerte
  if (x == X_LOVE) for (int i = 0; i < 9; i++) {
      float a = -1.5708f + (i - 4) * 0.30f + frand() * 0.1f;
      spawn(PT_HEART, cxm + (i - 4) * 9, cym - 26, cosf(a) * (52 + 30 * frand01()), sinf(a) * (58 + 36 * frand01()), 1.9f + 1.0f * frand01(), 9.0f + 16.0f * frand01(), heartCol());
    }
  if (x == X_HYPE || x == X_HAPPY) for (int i = 0; i < 8; i++) {
      float a = TAU * i / 8 + frand() * 0.3f;
      spawn(PT_SPARK, cxm + cosf(a) * 40, cym + sinf(a) * 36, cosf(a) * 55, sinf(a) * 50, 0.9f + 0.4f * frand01(), 5 + 5 * frand01(), x == X_HYPE ? 4 : 6, frand() * 3);
    }
  if (x == X_SURPRISE || x == X_ANXIOUS) { int n = 3 + (int)(frand01() * 3.99f); for (int i = 0; i < n; i++) {
      float a = -1.5708f + (i - (n - 1) * 0.5f) * 0.40f + frand() * 0.16f; float L = 7 + 8 * frand01(), v = 28 + 22 * frand01();
      spawn(PT_LINE, cxm + cosf(a) * (58 + 8 * frand01()), cym - 12 + sinf(a) * (66 + 8 * frand01()), cosf(a) * v, sinf(a) * v, 0.38f + 0.35f * frand01(), L, 4, 0, a);
    } }
}

void updateAnimation(uint32_t now) {
  float t = now * 0.001f;
  clipUpdate(now);
  birthAdd(now);
  slowAdd(now);
  lookTick(now);
  // --- secuencias y estado base ---
  if (stepEnd && now >= stepEnd) {
    if (seqIdx + 1 < seqLen) { seqIdx++; startStep(now); }
    else stepEnd = 0;
  }
  bool seqDone = (stepEnd == 0);
  if (petting && now - petLast > 250) petting = false;
  uint32_t idle = now - lastActivity;
  // (sueno profundo con pantalla sin dibujar eliminado: dormido sigue mostrando los arcos respirando)
  if (seqDone && seq[seqIdx].x == 0) {
    uint8_t b = baseExpr();
    if (expr != b && expr != X_SLEEP && expr != X_DIMSLEEP) { expr = b; exprStart = now; }
    if ((!usb || fullBat) && (expr == X_AWAKE || expr == X_LOWBAT) && (idle > 100000 || (energy < 6 && !usb && !simBatt)))
      play(now, 2, {X_YAWN, 1700}, {X_SLEEP, 0});        // se duerme solo, con bostezo
    if (autoOn && (expr == X_AWAKE || expr == X_LOWBAT) && idle > 35000 && now >= nextBored && NCLIPS > 0) {    // aburrimiento antes de dormir
      static const char* const BORED[6] = {"mirada_vacio", "suspiro_triste", "mirada_de_lado", "parpados_pesados", "pensar", "cabeceo"};
      nextBored = now + 7000 + (uint32_t)(frand01() * 6000);
      clipPlay(clipFind(BORED[boredIdx++ % 6]), now, frand() > 0);
    }
    if (autoOn && expr == X_LOWBAT && !usb && now >= nextAsk && NCLIPS > 0) {     // pide carga de varias maneras (cansancio)
      static const char* const ASK[3] = {"hambre_bateria", "tambaleo_sin_energia", "apagado_lento"};
      nextAsk = now + 8000 + (uint32_t)(frand01() * 6000);
      clipPlay(clipFind(ASK[(uint8_t)(frand01() * 2.99f)]), now, frand() > 0);
    }
    if (autoOn && birthVar < 0 && slowT0 == 0 && expr == X_AWAKE && now >= nextSlow && now - lastActivity > 5000) {   // parpadeo lento con emocion
      evSlowBlink(now); nextSlow = now + 24000 + (uint32_t)(frand01() * 26000);
    }
    if (autoOn && birthVar < 0 && slowT0 == 0 && lookPlan.n == 0 && expr == X_AWAKE && now >= nextIdleAct) {          // a veces hace algo por su cuenta
      nextIdleAct = now + 3000 + (uint32_t)(frand01() * 5500);
      const float* T = G.trait;
      float w[7] = {0.10f + 0.22f * T[4], 0.08f + 0.20f * T[5], 0.05f + 0.18f * T[2], 0.04f + 0.10f * T[0], 0.04f + 0.18f * T[3], 0.06f, 0.02f + 0.14f * T[1]};
      float tot = 0.45f, r = frand01() * 1.0f;      // el resto del tiempo simplemente esta tranquilo
      float acc = 0;
      for (int k = 0; k < 7; k++) { acc += w[k]; tot = acc; if (r < acc) {
        switch (k) {
          case 0: play(now, 2, {X_CURIOUS, 1600}, {0, 0}); break;
          case 1: play(now, 2, {X_HAPPY, 1500}, {0, 0}); break;
          case 2: play(now, 2, {X_SMUG, 2100}, {0, 0}); break;
          case 3: play(now, 2, {X_SURPRISE, 900}, {0, 0}); break;
          case 4: play(now, 2, {X_YAWN, 1700}, {0, 0}); break;
          case 5: evWink(); break;
          default: play(now, 2, {X_ANXIOUS, 1300}, {0, 0}); break;
        } break; } }
      (void)tot;
    }
  }
  if (autoOn && seqDone && birthVar < 0 && slowT0 == 0 && lookPlan.n == 0 && expr == X_AWAKE && now >= nextClip && NCLIPS > 0) directorClip(now);
  if (expr != prevExpr) { onEnter(expr, now); prevExpr = expr; }

  // --- objetivos de la pose ---
  setTargets();
  birthTgt(now);
  slowTgt(now);
  if (shakeAmp > 0.05f) shakeAmp *= powf(0.02f, DT);
  slosh *= expf(-DT / 1.7f);
  float pulse = 0.5f - 0.5f * cosf(TAU * fmodf(t, 0.6f) / 0.6f);
  float ph = fmodf(t, 0.85f) / 0.85f;       // latido (amor) y compas (hype)
  float thump = (ph < 0.13f ? sinf(ph / 0.13f * 3.14159f) : 0) + 0.6f * ((ph > 0.24f && ph < 0.37f) ? sinf((ph - 0.24f) / 0.13f * 3.14159f) : 0);
  beat = (expr == X_LOVE) ? 0.07f * thump : (expr == X_HYPE ? 0.09f * pulse : 0);
  float bp = fmodf(t, 0.6f);
  if (expr == X_HYPE && bp < beatPhase) {
    for (int e = 0; e < 2; e++) { eye[e].sy += 0.16f; eye[e].sx -= 0.07f; }
    spawn(PT_SPARK, 120 + frand() * 70 * (frand() > 0 ? 1 : -1), 70 + frand() * 30, frand() * 20, -20 - 20 * frand01(), 0.8f, 4 + 4 * frand01(), 4, frand() * 3);
  }
  beatPhase = bp;
  if (expr == X_CURIOUS && now - exprStart < 300) tgt[P_Y] += -6.0f * sinf(3.14159f * (now - exprStart) / 300.0f);
  if (expr == X_SLEEP || expr == X_DIMSLEEP) tgt[P_Y] += 2.5f * sinf(TAU * t / 3.0f);
  if (expr == X_LOWBAT) tgt[P_Y] += 1.5f * sinf(TAU * t / 2.4f);
  if (expr == X_HAPPY) tgt[P_Y] += -2.5f * fabsf(sinf(TAU * t * 1.4f));
  if (expr == X_YAWN) {
    float u = clampf((now - exprStart) / 1500.0f, 0, 1);
    float k = sinf(3.14159f * u);
    tgt[P_LIDT] = 0.22f + 0.30f * k; tgt[P_LIDB] = 0.05f + 0.22f * k; tgt[P_RY] = 66 + 6 * k; tgt[P_RX] = 36 + 5 * k;
    if (k > 0.6f) { tgt[P_OPEN] = 1.0f - 0.9f * (k - 0.6f) / 0.4f; tgt[P_ARCDIR] = 1; tgt[P_Y] -= 4; tgt[P_RX] += 5; }   // el bostezo se cierra en curva caida (no sonrisa) y se estira
  }

  // --- resortes (morph) ---
  for (int i = 0; i < NP; i++) {
    bool color = (i >= P_ER && i <= P_PB);
    float k = color ? 150.0f : (i == P_OPEN ? 220.0f : 190.0f);
    float c = color ? 24.0f : (i == P_OPEN ? 26.0f : 16.0f);
    float a = k * (tgt[i] - cur[i]) - c * vel[i];
    vel[i] += a * DT;
    cur[i] += vel[i] * DT;
  }
  for (int j = 0; j < 3; j++) { cur[P_ER + j] = clampf(cur[P_ER + j], 0, 255); cur[P_PR + j] = clampf(cur[P_PR + j], 0, 255); }
  cEye = rgb565((uint8_t)cur[P_ER], (uint8_t)cur[P_EG], (uint8_t)cur[P_EB]);
  cPup = rgb565((uint8_t)cur[P_PR], (uint8_t)cur[P_PG], (uint8_t)cur[P_PB]);
  pcols[0] = cEye; pcols[1] = cPup; pcols[2] = rgb565(255, 60, 85); pcols[3] = rgb565(0, 215, 235);
  pcols[4] = rgb565(255, 255, 255); pcols[5] = rgb565(55, 185, 165); pcols[6] = rgb565(255, 225, 90);
  pcols[7] = rgb565(255, 160, 195); pcols[8] = cPup; pcols[9] = cAccC;

  // --- extras con fundido ---
  float tp[8] = {0};
  tp[XT_CHARGE] = 0;                       // (antes: barra cian entre los ojos; se leia como una pupila unida)
  tp[XT_FULL] = expr == X_FULL; tp[XT_SPIRAL] = expr == X_DIZZY; tp[XT_GLOW] = expr == X_DIMSLEEP || (expr == X_CHARGING && chargeType == 2) || (expr == X_LOVE && loveVar == 1);
  for (int i = 0; i < 8; i++) presence[i] += (tp[i] - presence[i]) * (1.0f - expf(-((i == XT_FULL) ? 5.0f : 9.0f) * DT));

  // --- formas irregulares ---
  formAmp = (expr == X_AWAKE || expr == X_CURIOUS || expr == X_SMUG || expr == X_LOWBAT || expr == X_SURPRISE || expr == X_YAWN) ? 1.0f : 0.35f;
  if (settleAt && now >= settleAt) {      // se acomoda: vuelve a la forma de reposo (los resortes dan el rebote)
    settleAt = 0; formMode = 0;
    for (int e = 0; e < 2; e++) formReset(ftgt[e]);
  }
  if (now >= nextForm && birthVar < 0) chooseForm(now);
  if (birthVar < 0 && (expr == X_ANGRY || expr == X_SAD || expr == X_CURIOUS)) {   // poses fijas de la emocion (picos de boceto)
    for (int e = 0; e < 2; e++) { float* f = ftgt[e]; float keep[NF]; memcpy(keep, f, sizeof(keep));
      int pz = expr == X_ANGRY ? (e ? PZ_FURIAR : PZ_FURIAL) : expr == X_SAD ? (e ? PZ_COLA : PZ_GANCHO) : (e ? PZ_PICO : PZ_CABECITA);
      if (styleMir && e == 1 && expr != X_ANGRY) pz = expr == X_SAD ? PZ_GANCHO : PZ_CABECITA;
      poseSet(f, pz); f[F_FX] = keep[F_FX]; f[F_FY] = keep[F_FY]; f[F_PS] = keep[F_PS]; f[F_PT] = keep[F_PT]; f[F_SC] = keep[F_SC]; f[F_ANG] = keep[F_ANG]; }
    settleAt = 0;
  }
  float kf = 1.0f - expf(-3.2f * DT);
  for (int e = 0; e < 2; e++) for (int i = 0; i < NF; i++) fcur[e][i] += (ftgt[e][i] - fcur[e][i]) * kf;

  // --- mirada ---
  bool anx = (expr == X_ANXIOUS);
  if (now >= nextWander && now >= gazeUntil) {
    nextWander = now + (anx ? 220 + (uint32_t)(frand01() * 260) : 900 + (uint32_t)(frand01() * 2600));
    float nx = frand01() < 0.3f ? 0 : frand() * (anx ? 1.0f : 0.8f), ny = frand01() < 0.3f ? 0 : frand() * 0.6f;
    if (fabsf(nx - tgx) + fabsf(ny - tgy) > 0.7f) for (int e = 0; e < 2; e++) eye[e].sx += 0.07f;
    tgx = nx; tgy = ny;
  }
  float kg = 1.0f - expf(-18.0f * DT);
  gx += (tgx - gx) * kg; gy += (tgy - gy) * kg;
  float dgx = (gx - pgx) / DT, dgy = (gy - pgy) / DT;
  pgx = gx; pgy = gy;

  // --- parpadeo ---
  bool canBlink = birthVar < 0 && slowT0 == 0 && cur[P_OPEN] > 0.9f && expr != X_DIZZY && expr != X_YAWN && expr != X_BATCHECK;
  if (canBlink && now >= nextBlink) {
    nextBlink = now + 2400 + (uint32_t)(frand01() * 3200);
    if (expr == X_LOWBAT) nextBlink = now + 1300 + (uint32_t)(frand01() * 900);
    if (anx) nextBlink = now + 700 + (uint32_t)(frand01() * 600);
    for (int e = 0; e < 2; e++) eye[e].blinkTot = eye[e].blinkT = 0.23f;
    if (frand() > 0.65f) nextBlink = now + 360;
  }

  // --- gelatina por ojo ---
  float shake = shakeAmp * sinf(TAU * 14.0f * t);
  if (expr == X_COLD) shake += 1.9f * sinf(TAU * 19.0f * t);       // tiritando
  if (anx) shake += 0.9f * sinf(TAU * 11.0f * t);
  for (int e = 0; e < 2; e++) {
    Eye& E = eye[e];
    if (E.blinkT > 0) E.blinkT -= DT;
    float php = e * 3.14159f, inner = e ? -1.0f : 1.0f;
    float k = e ? 120.0f : 140.0f;
    float gxe = gx + clipAdd[10], gye = gy + clipAdd[11];
    float tx = gxe * 9 + shake + (mActive ? mLeanX : 0.0f), ty = gye * 7 + (mActive ? mLeanY : 0.0f);
    if (expr == X_DIZZY) { float a = TAU * 0.9f * t + php; tx += 9 * cosf(a); ty += 7 * sinf(a); }
    if (expr == X_HYPE) { ty += 4 * sinf(TAU * 3.0f * t + php); tx += 22 * sinf(TAU * 0.85f * t + php * 0.2f); }
    if (slosh > 0.01f) { float w = TAU * 1.1f * t + php * 0.25f; tx += 30 * slosh * sinf(w); ty += 14 * slosh * sinf(1.3f * w + 1); }
    if (expr == X_ANGRY) { static const float JT[9] = {1.3f, 2.2f, 0.3f, 1.0f, 1.4f, 1.0f, 1.6f, 1.8f, 1.0f}; tx += JT[angryVar % 9] * frand(); ty += 0.8f * JT[angryVar % 9] * frand();
      if (angryVar == 8) shakeAmp = fmaxf(shakeAmp, 0.8f + 4.0f * clampf((gNow - exprStart) / 2600.0f, 0, 1)); }
    if (expr == X_SAD) tx += 2.5f * sinf(TAU * t / 3.2f);
    float pvx0 = E.ovx, pvy0 = E.ovy;
    E.ovx += (k * (tx - E.ox) - 8 * E.ovx) * DT;
    E.ovy += (k * (ty - E.oy) - 8 * E.ovy) * DT;
    E.ox += E.ovx * DT; E.oy += E.ovy * DT;
    if (E.ox > 30) { E.ox = 30; E.ovx *= -0.45f; E.sx -= 0.10f; } else if (E.ox < -30) { E.ox = -30; E.ovx *= -0.45f; E.sx -= 0.10f; }     // rebota en el borde
    if (E.oy > 24) { E.oy = 24; E.ovy *= -0.45f; E.sy -= 0.10f; } else if (E.oy < -24) { E.oy = -24; E.ovy *= -0.45f; E.sy -= 0.10f; }
    float ax = (E.ovx - pvx0) / DT, ay = (E.ovy - pvy0) / DT;
    float sxt = 0.0009f * (fabsf(E.ovx + dgx * 90) - 0.5f * fabsf(E.ovy + dgy * 90));
    float syt = 0.0009f * (fabsf(E.ovy + dgy * 90) - 0.5f * fabsf(E.ovx + dgx * 90));
    if (expr == X_DIZZY) { sxt += 0.10f * sinf(TAU * 1.5f * t + php); syt -= 0.10f * sinf(TAU * 1.5f * t + php); }
    if (expr == X_SAD) syt -= 0.04f * (0.5f + 0.5f * sinf(TAU * t / 3.2f));
    sxt = clampf(sxt, -0.2f, 0.25f); syt = clampf(syt, -0.2f, 0.25f);
    E.svx += (210 * (sxt - E.sx) - 9 * E.svx) * DT; E.sx += E.svx * DT;
    E.svy += (210 * (syt - E.sy) - 9 * E.svy) * DT; E.sy += E.svy * DT;
    E.sx = clampf(E.sx, -0.4f, 0.45f); E.sy = clampf(E.sy, -0.4f, 0.45f);

    float ps = fcur[e][F_PS] * (puOv[e] >= 0 ? puOv[e] : 1.0f);
    float prx = fminf(cur[P_PRX] * ps, 0.62f * cur[P_RX]), pry = fminf(cur[P_PRY] * ps, 0.60f * cur[P_RY]);
    float lx = fmaxf(1.0f, cur[P_RX] - prx - 2), ly = fmaxf(1.0f, cur[P_RY] - pry - 2);
    float ptx = inner * cur[P_PDX] + gxe * 0.55f * lx + (mActive ? mDown[0] * 0.5f * lx : 0.0f);
    float pty = cur[P_PDY] + gye * 0.45f * ly + (mActive ? (mDown[1] - 1.0f) * 0.4f * ly : 0.0f);
    E.pvx += (230 * (ptx - E.px) - 16 * E.pvx - 0.30f * ax) * DT;
    E.pvy += (230 * (pty - E.py) - 16 * E.pvy - 0.30f * ay) * DT;
    E.px += E.pvx * DT; E.py += E.pvy * DT;
    float nd = (E.px * E.px) / (lx * lx) + (E.py * E.py) / (ly * ly);
    if (nd > 1.0f) { float s = 1.0f / sqrtf(nd); E.px *= s; E.py *= s; }
  }

  // --- emisores de particulas ---
  float dt = DT;
  if (expr == X_LOVE) {
    static const float RT[4] = {5.0f, 3.4f, 4.2f, 6.0f};
    acc[0] += dt * RT[loveVar & 3];
    while (acc[0] >= 1) { acc[0] -= 1; float sd = frand() > 0 ? 1.0f : -1.0f, lift = loveVar == 1 ? 0.65f : 1.0f;
      spawn(PT_HEART, 120 + sd * (66 + 44 * frand01()), 120 + 30 * frand(), sd * (6 + 10 * frand01()) + frand() * 6, (-30 - 24 * frand01()) * lift, 1.9f + 1.0f * frand01(), 9.0f + 16.0f * frand01(), heartCol()); }
  }
  if (expr == X_HAPPY) {
    acc[1] += dt * 5.0f;
    while (acc[1] >= 1) { acc[1] -= 1; float sd = frand() > 0 ? 1.0f : -1.0f;
      spawn(PT_SPARK, 120 + sd * (35 + 50 * frand01()), 70 + 60 * frand01(), 0, -6, 0.9f, 4 + 5 * frand01(), frand() > 0.5f ? 4 : 6, frand() * 2, frand() * 3); }
  }
  if (expr == X_SAD) {
    acc[2] += dt;
    if (acc[2] >= 1.1f) { acc[2] = 0; static int sd = 0; sd ^= 1;
      float cxd = 120 + (sd ? 1 : -1) * (cur[P_GAP] + 4), cyd = 124 + cur[P_Y] - 8;
      spawn(PT_DROP, cxd + (sd ? 12 : -12), cyd + 6, 0, 8, 1.25f, 10.5f, 5); }
  }
  if (expr == X_ANXIOUS) {      // gotas de sudor frio
    acc[4] += dt;
    if (acc[4] >= 0.7f) { acc[4] = 0; float sd = frand() > 0 ? 1.0f : -1.0f;
      spawn(PT_DROP, 120 + sd * (70 + 8 * frand01()), 70 + 20 * frand01(), sd * 6, 4, 1.0f, 5.5f, 4); }
  }
  if (expr == X_COLD) {         // copitos / chispitas de frio
    acc[4] += dt * 3.0f;
    while (acc[4] >= 1) { acc[4] -= 1; float sd = frand() > 0 ? 1.0f : -1.0f;
      spawn(PT_SPARK, 120 + sd * (50 + 40 * frand01()), 60 + 60 * frand01(), sd * 4, 10 + 8 * frand01(), 1.1f, 3.5f + 3 * frand01(), 4, frand() * 2, frand() * 3); }
  }
  if (expr == X_ANGRY && angryVar == 5) {          // vapor que sale por los lados
    acc[1] += dt * 8.0f;
    while (acc[1] >= 1) { acc[1] -= 1; float sd = frand() > 0 ? 1.0f : -1.0f;
      spawn(PT_DOT, 120 + sd * (74.0f + 16.0f * frand01()), 92.0f + 14.0f * frand(), sd * 10.0f + 8.0f * frand(), -38.0f - 22.0f * frand01(), 0.9f + 0.6f * frand01(), 6.0f + 6.0f * frand01(), 4); }
  }
  // --- actualizar particulas ---
  for (int i = 0; i < NPART; i++) {
    Part& p = parts[i];
    if (!p.on) continue;
    p.life -= dt;
    if (p.life <= 0) {
      if (p.type == PT_DROP)      // salpicadura
        for (int k = 0; k < 4; k++) spawn(PT_DOT, p.x, p.y, (k - 1.5f) * 22 + frand() * 6, -45 - 25 * frand01(), 0.4f, 2.2f, p.col);
      p.on = 0; continue;
    }
    p.x += p.vx * dt; p.y += p.vy * dt; p.rot += p.spin * dt;
    if (p.type == PT_HEART) { p.vx *= (1 - 0.5f * dt); p.vy *= (1 - 0.6f * dt); }
    else if (p.type == PT_DROP) p.vy += 230 * dt;
    else if (p.type == PT_DOT) p.vy += 260 * dt;
  }
}

// ============================================================================================
//  Dibujo
// ============================================================================================
void drawParticles() {
  for (int i = 0; i < NPART; i++) {
    const Part& p = parts[i];
    if (!p.on) continue;
    float u = 1.0f - p.life / p.max;                       // 0..1
    float a = fminf(1.0f, fminf(u / 0.12f, (1.0f - u) / 0.35f));
    if (p.type == PT_HEART) a = fminf(1.0f, fminf(u / 0.05f, (1.0f - u) / 0.12f));      // corazones: solidos, con presencia
    if (a <= 0.02f) continue;
    setAlpha(a);
    uint16_t c = pcols[p.col];
    switch (p.type) {
      case PT_HEART: { float sc = u < 0.2f ? 0.3f + 0.85f * (u / 0.2f) : 1.15f - 0.15f * fminf(1.0f, (u - 0.2f) / 0.25f);   // aparecen con rebote
                       sc *= 1.0f + 0.07f * sinf(u * 28.0f + p.rot);
                       heartAA(p.x, p.y, p.size * 1.15f * sc, p.size * sc, c); } break;
      case PT_SPARK: sparkAA(p.x, p.y, p.size * (0.5f + 0.5f * sinf(3.14159f * u)), p.rot, c); break;
      case PT_DOT: discAA(p.x, p.y, p.size, c); break;
      case PT_DROP: { float s = 0.8f + 0.3f * u; discAA(p.x, p.y, p.size * s, c);
                      triAA(p.x - p.size * 0.85f * s, p.y - 2, p.x + p.size * 0.85f * s, p.y - 2, p.x, p.y - p.size * 2.8f * s, c, 0.2f); } break;
      case PT_BOLT: { float ca = cosf(p.rot), sa = sinf(p.rot); lineAA(p.x - ca * p.size, p.y - sa * p.size, p.x + ca * p.size, p.y + sa * p.size, 2.2f, c); } break;
      case PT_LINE: { float ca = cosf(p.rot), sa = sinf(p.rot); lineAA(p.x, p.y, p.x + ca * p.size, p.y + sa * p.size, 2.6f, c); } break;
    }
  }
  setAlpha(1);
}

static inline float sc0(const float* F) { return F[F_SC] * (1.0f + beat); }

void renderFrame(uint32_t now) {
  float t = now * 0.001f;
  styleMir = ((G.pal * 7 + G.species * 3 + G.pupil) & 1) != 0;
  uint16_t cB = pcols[3];

  if (presence[XT_FULL] > 0.02f) {          // disco amarillo de CARA LLENA
    float r = 170.0f * clampf(presence[XT_FULL], 0, 1);
    uint16_t cFace = rgb565(FULLP[fullPal][0], FULLP[fullPal][1], FULLP[fullPal][2]);
    if (r > 150) { for (int i = 0; i < W * H; i++) fb[i] = cFace; markRows(0, H - 1); }
    else discAA(120, 120, r, cFace);
  }

  float open = clampf(cur[P_OPEN], 0, 1);
  float wob = cur[P_WOB], amp = 0.05f + wob;
  Species SP = SPECIES[G.species];      // forma de reposo SUAVE (votos de Alessandro: ojos redondeados); lo irregular se hace en movimiento
  { const float KS = 0.40f, KB = 0.60f;
    SP.c1 *= KS; SP.c2 *= KS; SP.c3 *= KS; SP.c4 *= KS; SP.c5 *= KS; SP.tilt *= KS; SP.atilt *= KS; SP.ac4 *= KS; SP.asx *= KS;
    SP.aps *= 0.6f; SP.ay *= 0.5f; SP.sx = 1 + (SP.sx - 1) * KB; SP.sy = 1 + (SP.sy - 1) * KB; SP.gap = 1 + (SP.gap - 1) * KB; }
  if (birthVar >= 0 && birthShapeF < 0.999f) { float f = birthShapeF; SP.c1 *= f; SP.c2 *= f; SP.c3 *= f; SP.c4 *= f; SP.c5 *= f; SP.tilt *= f; SP.atilt *= f; SP.ac4 *= f; SP.asx *= f; SP.aps *= f; SP.ay *= f;
    SP.sx = 1 + (SP.sx - 1) * f; SP.sy = 1 + (SP.sy - 1) * f; SP.gap = 1 + (SP.gap - 1) * f; }
  float gapMul = SP.gap * (1.0f + 0.05f * G.jit[0]);
  float ftilt = cur[P_FTILT] + 0.012f * sinf(t * 0.7f) + clipAdd[4] + (mActive ? mTiltFace : 0.0f);
  float fca = cosf(ftilt), fsa = sinf(ftilt);
  for (int e = 0; e < 2; e++) {
    const Eye& E = eye[e];
    float side = e ? 1.0f : -1.0f, inner = e ? -1.0f : 1.0f;
    float blink = 0;
    if (E.blinkT > 0) { float u = 1.0f - E.blinkT / E.blinkTot; blink = u < 0.5f ? u * 2 : (1 - u) * 2; }
    const float* F = fcur[e];
    float spx = SP.sx * (1.0f + 0.06f * G.jit[1]) * (e ? 1.0f + SP.asx : 1.0f - SP.asx), spy = SP.sy * (1.0f + 0.06f * G.jit[2]);
    float rxE = clampf(cur[P_RX] * F[F_AX] * spx * sc0(F) * (1 + E.sx + clipAdd[0]), 8, 74);
    float gapE = fmaxf(cur[P_GAP] * gapMul, rxE * (1.03f + 0.5f * (fabsf(F[F_C3]) + fabsf(cur[P_C3]) + fabsf(F[F_C5]) + fabsf(cur[P_C5]) + fabsf(F[F_C2]) + fabsf(cur[P_C2]) + fabsf(SP.c2) + fabsf(SP.c3) + fabsf(SP.c5))) + 3.5f);
    float cx0 = side * gapE + E.ox + F[F_FX] + clipAdd[2], cy0 = cur[P_Y] + E.oy + F[F_FY] + clipAdd[3] + (e ? SP.ay : -SP.ay);
    float cx = 120 + cx0 * fca - cy0 * fsa, cy = 124 + cx0 * fsa + cy0 * fca;   // inclina toda la cara
    float sc = F[F_SC] * (1.0f + beat);

    if (open > 0.04f) {
      EyeDraw D;
      D.cx = cx; D.cy = cy;
      D.rx = rxE; D.c[0] = D.c[1] = D.c[2] = D.c[3] = D.c[4] = 0;
      D.ry = clampf(cur[P_RY] * F[F_AY] * spy * sc * (1 + E.sy + clipAdd[1]), 8, 92) * (1.0f - 0.86f * blink);
      D.ang = side * (cur[P_TILT] + F[F_ANG] + SP.tilt + 0.04f * G.jit[3]) + (e ? SP.atilt : 0.0f) + ftilt + clampf(E.ovx * 0.0012f, -0.2f, 0.2f);
      float sp = t * (1.0f + 3.0f * wob);   // armonicos = pose + forma irregular del momento + ruido organico
      for (int k = 0; k < 5; k++) {
        float nz = amp * (0.6f * sinf(sp * nph[e][k][0] + nph[e][k][1]) + 0.4f * sinf(sp * nph[e][k][2] + nph[e][k][3]));
        float spc = k == 0 ? SP.c1 : k == 1 ? SP.c2 : k == 2 ? SP.c3 : k == 3 ? SP.c4 + (e ? SP.ac4 : -SP.ac4) : SP.c5;
        D.c[k] = clampf(cur[P_C1 + k] + F[F_C1 + k] + spc + nz, -0.45f, 0.45f);
      }
      float closing = 1.0f - open;
      float lt = clampf(cur[P_LIDT] + (e ? clipAdd[6] : clipAdd[5]), 0, 0.88f), lb = clampf(cur[P_LIDB] + (e ? clipAdd[8] : clipAdd[7]), 0, 0.88f);
      if (lt + lb > 0.84f) { float k = 0.84f / (lt + lb); lt *= k; lb *= k; }   // nunca tapan todo el ojo por sumar capas
      D.lidT = lt + (0.5f - lt) * fmaxf(blink * 0.22f, closing);
      D.lidB = lb + (0.5f - lb) * fmaxf(blink * 0.22f, closing);
      D.slope = cur[P_SLOPE]; D.inner = inner;
      { bool flat = (expr == X_BATCHECK || (expr == X_CHARGING && chargeType == 0) || expr == X_FULL);
        float pk = flat ? 0.0f : 1.0f;
        D.pz[0] = F[F_FL]; D.pz[1] = F[F_CA]; D.pz[2] = F[F_CH] * pk; D.pz[3] = F[F_CW]; D.pz[4] = F[F_TH] * pk; D.pz[5] = F[F_TW]; D.pz[6] = F[F_LN]; D.pz[7] = F[F_HK]; D.pz[8] = F[F_NT] * pk;
        D.t = t * (1.0f + 3.0f * wob); D.seed = (float)e; D.flip = styleMir && e == 1; }
      D.pcx = cx + E.px; D.pcy = cy + E.py;
      float ps = F[F_PS] * (puOv[e] >= 0 ? puOv[e] : 1.0f) * (1.0f + 0.04f * sinf(TAU * 0.5f * t + e * 2)) * (1.0f + clipAdd[9]) * (e ? 1.0f + SP.aps : 1.0f - SP.aps);
      D.prx = fminf(cur[P_PRX] * ps, 0.62f * D.rx) * (1 - blink * 0.3f); D.pry = fminf(cur[P_PRY] * ps, 0.60f * D.ry) * (1 - blink * 0.3f);
      D.pang = side * (cur[P_PTILT] + F[F_PT]) + ftilt;
      D.pc1 = cur[P_PC1] + 0.10f * sinf(t * 1.3f + nph[e][5][1]); D.pc2 = cur[P_PC2] + 0.12f * sinf(t * 0.9f + nph[e][6][1]);
      D.cEye = cEye; D.cPup = cPup; D.cAcc = cAccC; D.pkind = PUPILS[G.pupil].kind;
      D.toneOn = G.toneMode == 1; D.tc = cosf(G.toneAng); D.ts = sinf(G.toneAng); D.toneOff = G.toneOff;
      setAlpha(clampf((open - 0.10f) / 0.15f, 0, 1) * eyeFadeA * ((expr == X_ANGRY && angryVar == 7) ? 0.0f : 1.0f));      // el ojo se desvanece ANTES de que aparezca el arco
      if (expr == X_BATCHECK || (expr == X_CHARGING && chargeType == 0)) {   // el ojo es un RECIPIENTE: se ve el contorno vacio y lo que falta
        EyeDraw Gh = D; Gh.lidT = Gh.lidB = 0.0f; Gh.pkind = 0; Gh.prx = Gh.pry = 0; Gh.toneOn = false;
        Gh.cEye = rgb565(52, 56, 72); Gh.cPup = Gh.cEye;
        renderEye(Gh);
      }
      renderEye(D);
      if (expr == X_LOVE && loveVar == 3) {                             // corazones en lugar de pupilas
        float hs = fminf(D.rx, D.ry) * 0.46f * (1 + 0.12f * sinf(TAU * 2.4f * t + e));
        setAlpha(clampf((open - 0.10f) / 0.15f, 0, 1)); heartAA(cx, cy + 4, hs * 1.15f, hs, pcols[2]);
      }
      setAlpha(1);

      if (false && presence[XT_SPIRAL] > 0.05f) {
        float spin = -TAU * 2.0f * t, rr = fminf(D.rx, D.ry) * 0.85f;
        setAlpha(presence[XT_SPIRAL]);
        for (int i = 0; i < 44; i++) {
          float a = spin + i * 0.52f, r = rr * i / 44.0f;
          float px = cx + r * cosf(a), py = cy + r * sinf(a);
          discAA(px, py, 2.6f, cPup);
        }
        setAlpha(1);
      }
      if (expr == X_ANGRY && angryVar == 7) {                          // chevrones: > < en lugar de ojos
        const uint8_t* ac = ANGC[angryCol % 6]; uint16_t cc = rgb565(ac[0], ac[1], ac[2]);
        float w = 1.0f + 0.06f * sinf(TAU * 6.0f * t + e);
        float x0 = cx - inner * 26.0f, x1 = cx + inner * 24.0f * w;
        setAlpha(1);
        lineAA(x0, cy - 34.0f, x1, cy + 2.0f, 12.0f, cc); lineAA(x1, cy + 2.0f, x0, cy + 38.0f, 12.0f, cc);
        discAA(x0, cy - 34.0f, 6.0f, cc); discAA(x0, cy + 38.0f, 6.0f, cc); discAA(x1, cy + 2.0f, 6.0f, cc);
      }
      if (expr == X_ANGRY && angryVar == 4) {                          // cejas enojadas
        const uint8_t* ac = ANGC[angryCol % 6]; uint16_t cc = rgb565(ac[0], ac[1], ac[2]);
        float topY = cy - D.ry * 1.0f - 6.0f + 1.5f * sinf(TAU * 7.0f * t);
        float xi = cx + inner * D.rx * 0.65f, xo = cx - inner * D.rx * 0.65f;
        setAlpha(1);
        lineAA(xi, topY + 14.0f, xo, topY - 14.0f, 8.0f, cc); discAA(xi, topY + 14.0f, 4.0f, cc); discAA(xo, topY - 14.0f, 4.0f, cc);
      }
      bool veinOn = expr == X_ANGRY && ((angryVar <= 3 && e == (angryVar & 1)) || (angryVar == 8 && gNow - exprStart > 1500));
      if (veinOn) {   // marca de enojo en la esquina
        float pl = 0.5f + 0.5f * sinf(TAU * 3.2f * t);
        setAlpha(clampf((cur[P_SLOPE] - 0.3f) * 1.6f, 0, 1));
        veinAA(cx + (e ? 44.0f : -44.0f), cy - D.ry * 0.88f, 11.0f + 2.5f * pl, pcols[2]);
        setAlpha(1);
      }
    }

    if (open < 0.10f && !(birthVar >= 0 && (gNow - birthT0) < PRE)) {      // ojo cerrado = arco (solo cuando el ojo ya no se ve: sin superposicion)
      float a = clampf((0.10f - open) / 0.08f, 0, 1);
      float dir = cur[P_ARCDIR];
      float incl = (expr == X_DIMSLEEP) ? 0.50f : (expr == X_SLEEP) ? 0.35f : (expr == X_YAWN) ? 0.60f : (birthVar >= 0) ? 0.25f : 0.18f;
      float dep = 13.0f * (expr == X_YAWN ? 1.4f : (expr == X_SLEEP || expr == X_DIMSLEEP) ? 0.55f : 1.0f);
      float aw = fminf(cur[P_ARCW], gapE * 0.9f);
      if (presence[XT_GLOW] > 0.05f) {
        float g = presence[XT_GLOW] * a * (0.8f + 0.2f * sinf(TAU * t / 2.0f));
        for (int k = 3; k >= 1; k--) { setAlpha(g * 0.13f); brushArc(cx, cy, aw, dep, dir, incl, side, cur[P_TH] * 2.3f + 5.0f * k, cEye); }
        setAlpha(a * (0.55f + 0.2f * g));
      } else setAlpha(a);
      brushArc(cx, cy, aw, dep, dir, incl, side, cur[P_TH] * 2.3f, cEye);
      setAlpha(1);
    }
  }

  if (presence[XT_CHARGE] > 0.03f) {      // barra cian de CARGANDO
    float pl = 0.5f - 0.5f * cosf(TAU * fmodf(t, 0.9f) / 0.9f);
    setAlpha(presence[XT_CHARGE] * (0.78f + 0.22f * pl));
    ellipseAA(120, 124 + cur[P_Y], 38 + 3 * pl, 10 + 2 * pl, cB);
    setAlpha(1);
  }
  if (false && presence[XT_SPIRAL] > 0.05f) {      // estrellitas orbitando (desactivado)
    setAlpha(presence[XT_SPIRAL]);
    for (int i = 0; i < 4; i++) { float a = TAU * 0.9f * t + i * 1.5708f; sparkAA(120 + 62 * cosf(a), 38 + 10 * sinf(a), 6.0f, t * 2 + i, cEye); }
    setAlpha(1);
  }
  drawParticles();
  birthFx(gNow);
}

// ============================================================================================
//  API del core
// ============================================================================================
void coreInit(uint32_t now, uint32_t seed) {
  rng ^= seed;
  for (int x = 0; x < NEXPR; x++) {
    for (int i = 0; i < NBASE; i++) POSE[x][i] = POSES[x][i];
    for (int i = NBASE; i < NP; i++) POSE[x][i] = SHAPES[x][i - NBASE];
  }
  for (int e = 0; e < 2; e++) {
    for (int k = 0; k < 12; k++) {
      nph[e][k][0] = 0.7f + 1.2f * frand01(); nph[e][k][1] = TAU * frand01();
      nph[e][k][2] = 1.9f + 1.6f * frand01(); nph[e][k][3] = TAU * frand01();
    }
    formReset(fcur[e]); formReset(ftgt[e]);
  }
  memset(parts, 0, sizeof(parts));
  genomeFromSeed(seed * 2246822519u + 374761393u);     // identidad fija: la misma semilla guardada = la misma criatura
  memset(slots, 0, sizeof(slots)); memset(clipAdd, 0, sizeof(clipAdd)); nextClip = now + 4000;
  expr = X_AWAKE; prevExpr = 0;
  setTargets();
  for (int i = 0; i < NP; i++) { cur[i] = tgt[i]; vel[i] = 0; }
  cur[P_OPEN] = 0;                                 // al encender "abre los ojos"
  for (int e = 0; e < 2; e++) eye[e] = Eye();
  lastActivity = now;
  if (birthSkip) play(now, 2, {X_HAPPY, 1400}, {0, 0});   // criatura guardada: no vuelve a nacer, solo abre los ojos contenta
  else evBirth(now, (int)(birthPick % NBIRTH));            // nace con una animacion elegida entre varias
}

void coreFrame(uint32_t now, float dt) {
  gNow = now;
  DT = clampf(dt, 0.005f, 0.04f);
  updateStats(now);
  updateAnimation(now);
  memset(fb, 0, (size_t)W * H * 2);
  dirtyY0 = H; dirtyY1 = -1;
  if (!deepSleep) renderFrame(now); else markRows(0, 0);
}
