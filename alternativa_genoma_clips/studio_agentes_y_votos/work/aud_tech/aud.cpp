// Arnes de auditoria tecnica (AUD_tecnico). Compila el MISMO core (instrumentado con contadores) en un ejecutable nativo.
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>
#include <random>
#include <string>
#include <cstdint>
#ifdef _WIN32
#include <windows.h>
#endif
#include "EyesCore.h"
#undef min
#undef max

static std::atomic<unsigned long long> g_prog(0);
static uint16_t g_buf[240 * 240 + 64];
static uint32_t T = 0;          // reloj global (ms), monotono
static std::string g_fail;
static int g_nfail = 0;
static double g_maxframe_us = 0;

static void watchdog() {
  unsigned long long last = ~0ULL;
  for (;;) {
    std::this_thread::sleep_for(std::chrono::seconds(8));
    unsigned long long p = g_prog.load();
    if (p == last) { fprintf(stderr, "HANG detectado (sin progreso 8 s)\n"); fflush(stderr); _exit(3); }
    last = p;
  }
}
static bool fin(float v) { return std::isfinite(v); }
// validador estricto (mas que sim_ok)
static const char* validate() {
  for (int i = 0; i < NP; i++) { if (!fin(cur[i]) || !fin(vel[i]) || !fin(tgt[i])) return "cur/vel/tgt no finito"; if (fabsf(cur[i]) > 1000) return "cur enorme"; }
  for (int e = 0; e < 2; e++) {
    for (int i = 0; i < NF; i++) if (!fin(fcur[e][i]) || !fin(ftgt[e][i])) return "fcur";
    const float* p = (const float*)&eye[e];
    for (int i = 0; i < 14; i++) if (!fin(p[i])) return "eye no finito";
    if (fabsf(eye[e].ox) > 21 || fabsf(eye[e].oy) > 21) return "eye ox/oy fuera de +-20";
    if (fabsf(eye[e].sx) > 0.46f || fabsf(eye[e].sy) > 0.46f) return "eye sx/sy fuera de clamp";
  }
  for (int c = 0; c < NCH; c++) if (!fin(clipAdd[c]) || fabsf(clipAdd[c]) > 4.0f * CH_RANGE[c]) { static char mb[200]; snprintf(mb, sizeof mb, "clipAdd[%d]=%g slots:%d/%d/%d on=%d%d%d", c, clipAdd[c], slots[0].id, slots[1].id, slots[2].id, slots[0].on, slots[1].on, slots[2].on); return mb; }
  for (int i = 0; i < NPART; i++) if (parts[i].on) { const Part& p = parts[i]; if (!fin(p.x) || !fin(p.y) || !fin(p.vx) || !fin(p.vy) || !fin(p.life) || fabsf(p.x) > 1e4f || fabsf(p.y) > 1e4f) return "particula"; if (p.col > 6 || p.type > PT_LINE) return "particula col/type"; }
  for (int i = 0; i < 8; i++) if (!fin(presence[i])) return "presence";
  if (expr < 1 || expr >= NEXPR) return "expr fuera de rango";
  if (seqLen > 3 || seqIdx >= 3) return "seq fuera";
  for (int i = 0; i < 3; i++) if (slots[i].on && (slots[i].id < 0 || slots[i].id >= NCLIPS || !fin(slots[i].speed))) return "slot invalido";
  if (!fin(energy) || !fin(fun) || !fin(affection) || energy < 0 || energy > 100) return "stats";
  if (!fin(shakeAmp) || !fin(gx) || !fin(gy) || !fin(beat)) return "shake/gaze/beat";
  if (dirtyY0 < 0 || dirtyY1 > H - 1) return "dirty fuera";
  if (g_oob) return "escritura de pixel fuera de pantalla";
  if (mActive) { for (int i = 0; i < 3; i++) if (!fin(mG[i])) return "mG"; if (!fin(mShake) || !fin(mRoll) || !fin(mDown[0]) || !fin(mDown[1]) || !fin(mLeanX) || !fin(mLeanY) || !fin(mTiltFace)) return "motion state"; }
  return nullptr;
}
static void fail(const char* what, const char* detail) {
  g_nfail++;
  if (g_nfail <= 25) fprintf(stderr, "FALLO: %s | %s\n", what, detail);
}
static bool frame(float dt, int tag1 = 0, int tag2 = 0) {
  T += (uint32_t)(dt * 1000.0f + 0.5f); if (!(dt > 0.0005f && dt < 5)) T += 16;
  auto a = std::chrono::steady_clock::now();
  coreFrame(T, dt);
  double us = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - a).count();
  if (us > g_maxframe_us) g_maxframe_us = us;
  g_prog++;
  const char* v = validate();
  if (v) { char b[200]; snprintf(b, sizeof b, "T=%u tag=%d/%d expr=%d", T, tag1, tag2, expr); fail(v, b); return false; }
  return true;
}
static void run(int n, float dt = 1.0f / 60, int a = 0, int b = 0) { for (int i = 0; i < n; i++) if (!frame(dt, a, b)) { g_oob = 0; return; } }
static void resetWorld(uint32_t seed) {
  rng = 12345; fb = g_buf;
  memset(presence, 0, sizeof presence); memset(acc, 0, sizeof acc);
  shakeAmp = 0; beat = 0; energy = 85; fun = 75; affection = 40; usb = lowBat = deepSleep = fullBat = cold = false; petting = false;
  nextBlink = T + 2500; nextWander = 0; gazeUntil = 0; nextIdleAct = T + 9000; nextForm = T + 1500; mActive = false; autoOn = true;
  gx = gy = tgx = tgy = pgx = pgy = 0; puOv[0] = puOv[1] = -1; timeScale = 1;
  coreInit(T, seed);
  genomeFromSeed(seed);
}

// ---------------------------------------------------------------- tests
static void t_seeds(int s0, int s1, int autoOn_, int nfr) {
  unsigned long long frames = 0;
  for (int s = s0; s < s1; s++) {
    uint32_t seed = (uint32_t)s * 7919u + 1u;
    resetWorld(seed); autoOn = autoOn_ != 0;
    run(30, 1.0f / 60, s, 0);
    for (int x = 1; x < NEXPR; x++) { evSet(x, T, 0); run(nfr, 1.0f / 60, s, x); frames += nfr; }
    // y las mismas expresiones con una secuencia corta de 2 pasos (dur>0)
    for (int x = 1; x < NEXPR; x += 3) { evSet(x, T, 700); run(60, 1.0f / 60, s, 100 + x); frames += 60; }
  }
  printf("seeds %d..%d auto=%d: %llu cuadros, fallos=%d, maxframe_host=%.0f us, spawnfail=%llu dirbad=%llu trunc=%llu oob=%llu\n", s0, s1, autoOn_, frames, g_nfail, g_maxframe_us, g_spawnfail, g_dirbad, g_trunc, g_oob);
}
static void t_storm(int s0, int s1, int nfr) {
  std::mt19937 R(777);
  unsigned long long frames = 0;
  for (int s = s0; s < s1; s++) {
    uint32_t seed = (uint32_t)s * 104729u + 7u;
    resetWorld(seed);
    for (int f = 0; f < nfr; f++) {
      if (R() % 9 == 0) {
        switch (R() % 20) {
          case 0: evTap(T); break; case 1: evPet(T, R() & 1); break; case 2: evPlay(T); break;
          case 3: evShake(T, R() & 1); break; case 4: evLoud(T); break; case 5: evCold(T, R() & 1); break;
          case 6: evThumbs(T); break; case 7: evFlick(T); break; case 8: evUsb(T, R() & 1); break;
          case 9: evBatFull(T); break; case 10: evBatLow(T, R() & 1); break; case 11: evPoke(T); break;
          case 12: evNewCreature(T, R()); break; case 13: evTheme(T); break; case 14: evNextExpr(T); break;
          case 15: wake(T); break; case 16: evWink(); break; case 17: evSet(1 + R() % (NEXPR - 1), T, R() % 3 ? 800 : 0); break;
          case 18: clipPlay(R() % NCLIPS, T, R() & 1, 0.2f + (R() % 300) / 100.0f); break;
          case 19: timeScale = (R() & 1) ? 20.0f : 1.0f; break;
        }
      }
      if (!frame(1.0f / 60, s, f)) break;
      frames++;
    }
  }
  printf("storm seeds %d..%d: %llu cuadros, fallos=%d, spawnfail=%llu dirbad=%llu peakmaxframe=%.0f us\n", s0, s1, frames, g_nfail, g_spawnfail, g_dirbad, g_maxframe_us);
}
static void t_clips() {
  const float speeds[3] = {0.2f, 1.0f, 3.0f};
  const int exprs[3] = {X_AWAKE, X_SLEEP, X_ANGRY};
  int bad = 0, stuck = 0, runs = 0;
  unsigned long long evExpect[3] = {0, 0, 0}, evGot[3] = {0, 0, 0};
  double maxAbs[NCH] = {0};
  for (int i = 0; i < NCLIPS; i++) {
    const Clip& c = CLIPS[i];
    int nev = 0; for (int k = 0; k < c.n; k++) if (c.keys[k].ev) nev++;
    for (int mir = 0; mir < 2; mir++) for (int si = 0; si < 3; si++) for (int xe = 0; xe < 3; xe++) {
      resetWorld(1000 + i); autoOn = false;
      evSet(exprs[xe], T, 0); run(40, 1.0f / 60, i, 0);
      g_emit = 0; int before = g_nfail;
      clipPlay(i, T, mir, speeds[si]);
      int nfr = (int)(std::min(8000.0f, (c.dur + 100.0f) / speeds[si] / 16.67f) + 20);
      if (c.flags & 2) nfr = 300;
      bool wasOn = true; int endedAt = -1;
      for (int f = 0; f < nfr; f++) {
        if (!frame(1.0f / 60, i, si)) break;
        for (int ch = 0; ch < NCH; ch++) maxAbs[ch] = std::max(maxAbs[ch], (double)fabsf(clipAdd[ch]));
        bool on = false; for (int k = 0; k < 3; k++) on |= slots[k].on;
        if (wasOn && !on) { endedAt = f; wasOn = false; }
      }
      runs++;
      if (g_nfail > before) bad++;
      if (!(c.flags & 2)) {
        bool on = false; for (int k = 0; k < 3; k++) on |= slots[k].on;
        if (on) { stuck++; if (stuck < 6) fprintf(stderr, "clip no termina: %s speed=%.1f\n", c.name, speeds[si]); }
        if (mir == 0 && xe == 0) { evExpect[si] += nev; evGot[si] += g_emit; }
      }
    }
  }
  printf("clips: %d clips x mirror x 3 velocidades x 3 expresiones = %d corridas; invalidas=%d, no terminan=%d\n", NCLIPS, runs, bad, stuck);
  for (int si = 0; si < 3; si++) printf("  eventos (ev!=0) esperados vs emitidos a velocidad %.1f (no-loop, sin espejo): %llu vs %llu\n", speeds[si], evExpect[si], evGot[si]);
  printf("  |clipAdd| maximo por canal (1 clip):"); for (int ch = 0; ch < NCH; ch++) printf(" %.2f", maxAbs[ch]); printf("\n");
}
static void t_triple() {
  std::mt19937 R(4242);
  int bad = 0, trials = 0; double maxAbs[NCH] = {0};
  for (int i = 0; i < NCLIPS; i++) for (int rep = 0; rep < 12; rep++) {
    resetWorld(900 + i * 13 + rep); autoOn = false;
    evSet(1 + R() % (NEXPR - 1), T, 0); run(30, 1.0f / 60, i, rep);
    int before = g_nfail;
    int ids[3] = {i, (int)(R() % NCLIPS), (int)(R() % NCLIPS)};
    for (int k = 0; k < 3; k++) clipPlay(ids[k], T + k * 10, R() & 1, (rep & 1) ? 3.0f : 0.2f + (R() % 200) / 100.0f);
    if (rep % 3 == 0) clipPlay(R() % NCLIPS, T + 40, R() & 1, 1.0f);       // 4.o clip: desaloja el mas viejo
    for (int f = 0; f < 140; f++) { if (!frame(1.0f / 60, i, rep)) break; for (int ch = 0; ch < NCH; ch++) maxAbs[ch] = std::max(maxAbs[ch], (double)fabsf(clipAdd[ch])); }
    trials++; if (g_nfail > before) bad++;
  }
  printf("triple: %d pruebas de 3-4 clips simultaneos, invalidas=%d, |clipAdd| max:", trials, bad);
  for (int ch = 0; ch < NCH; ch++) printf(" %.2f", maxAbs[ch]); printf("\n");
}
static void t_motion() {
  const float bad[] = {NAN, INFINITY, -INFINITY, 1e30f, -1e30f, FLT_MAX, -FLT_MAX, 1e-30f, 0.0f, 31.9f, -31.9f, 32.0f, 100.0f, FLT_MIN, 1.0f};
  const int nb = sizeof(bad) / sizeof(bad[0]);
  const float dts[] = {0.0f, -1.0f, NAN, INFINITY, 1e9f, 1e-9f, 0.0005f, 0.1f, 0.5f, 1.0f / 60};
  const int nd = sizeof(dts) / sizeof(dts[0]);
  int bads = 0; std::mt19937 R(99);
  resetWorld(5); autoOn = false;
  run(60);
  // 1) todas las combinaciones de valores extremos en (ax,ay,az) y dt
  for (int a = 0; a < nb; a++) for (int b = 0; b < nb; b++) for (int c = 0; c < nb; c++) for (int d = 0; d < nd; d++) {
    for (int rep = 0; rep < 3; rep++) { T += 16; motionFeed(bad[a], bad[b], bad[c], T, dts[d]); }
    char buf[120]; snprintf(buf, sizeof buf, "a=%g b=%g c=%g dt=%g", bad[a], bad[b], bad[c], dts[d]);
    if (!frame(1.0f / 60, 0, 0)) { bads++; fprintf(stderr, "  ^ tras %s\n", buf); resetWorld(5); autoOn = false; mActive = false; }
    g_prog++;
  }
  printf("motion: %d combinaciones extremas (ax,ay,az,dt) x3 cuadros: estados invalidos=%d\n", nb * nb * nb * nd, bads);
  // 2) ruido gaussiano fuerte + saltos de reloj
  int b2 = 0;
  for (int s = 0; s < 40; s++) {
    resetWorld(77 + s); autoOn = false; std::normal_distribution<float> N(0, 1);
    for (int f = 0; f < 3000; f++) {
      float sc = (f / 500) % 2 ? 6.0f : 1.2f;
      uint32_t nowv = (f % 400 == 399) ? T - 5000 : T;          // reloj que retrocede
      motionFeed(N(R) * sc, 1 + N(R) * sc, N(R) * sc, nowv, 1.0f / 60);
      if (!frame(1.0f / 60, s, f)) { b2++; break; }
    }
  }
  printf("motion: 40 corridas x3000 cuadros de ruido (sigma 1.2 y 6 g) con saltos de reloj atras: invalidas=%d\n", b2);
}
static bool g_nonan=false;
static void t_dt() {
  const float dts[] = {0.0f, -1.0f, 1e-9f, 0.0005f, 0.004f, 0.04f, 0.05f, 1.0f, 3.0f, 100.0f, 1e9f, INFINITY, NAN};
  for (float dt : dts) {
    if (g_nonan && dt != dt) continue;
    resetWorld(31); autoOn = true; evSet(X_HYPE, T, 0);
    int ok = 0, n = 300; int before = g_nfail;
    for (int f = 0; f < n; f++) { coreFrame(T += 16, dt); g_prog++; if (!validate()) ok++; else { g_oob = 0; } }
    // tras volver a dt normal, se recupera?
    int rec = 0; for (int f = 0; f < 120; f++) { coreFrame(T += 16, 1.0f / 60); g_prog++; if (!validate()) rec++; }
    printf("dt=%-10g: %3d/%d cuadros validos con ese dt; tras 120 cuadros normales validos=%d/120 (DT=%g)\n", dt, ok, n, rec, DT);
    (void)before;
  }
  g_nfail = 0;
}
static void t_wrap() {
  // reloj cerca de 2^32: contar eventos espontaneos (parpadeos, formas, idle) en 40 s antes y despues del desborde
  auto probe = [&](uint32_t start, const char* name) {
    resetWorld(123); autoOn = true; T = start; coreInit(T, 123); genomeFromSeed(123);
    nextBlink = T + 2500; nextIdleAct = T + 9000; nextForm = T + 1500; nextClip = T + 4000; nextWander = T;
    lastActivity = T;
    int blinks = 0, forms = 0, clips = 0, exprChanges = 0; uint8_t pe = expr; bool prevBlink = false; uint8_t pm = formMode; uint32_t nf = nextForm;
    long long ticks = 0;
    for (int f = 0; f < 60 * 120; f++) {
      T += 17; coreFrame(T, 1.0f / 60); ticks++;
      bool b = eye[0].blinkT > 0; if (b && !prevBlink) blinks++; prevBlink = b;
      if (nextForm != nf) { forms++; nf = nextForm; }
      for (int k = 0; k < 3; k++) if (slots[k].on && slots[k].t0 == T - 17) {}
      if (expr != pe) { exprChanges++; pe = expr; }
      (void)pm;
    }
    int act = 0; for (int k = 0; k < 3; k++) act += slots[k].on;
    printf("  %-34s start=%10u: 120 s -> parpadeos=%d cambios de forma=%d cambios de expr=%d  nextClip-T=%d\n", name, start, blinks, forms, exprChanges, (int)(nextClip - T));
    (void)clips; (void)act;
  };
  probe(1000, "arranque normal");
  probe(4294967296u - 60000u, "cruza 2^32 a los 60 s");
  probe(4294967296u - 1000u, "cruza 2^32 al 1 s");
  probe(4294967296u - 20000u, "cruza 2^32 a los 20 s");
  // precision del tiempo float: cuantos cuadros seguidos comparten el mismo 't' a distintas edades de funcionamiento
  printf("  precision de t = now*0.001f (float 24 bits) a 60 fps:\n");
  const double ages_h[] = {0.1, 1, 4.66, 12, 24, 72, 240, 1000, 1193};
  for (double h : ages_h) {
    uint32_t base = (uint32_t)(h * 3600e3); int same = 0, tot = 600, distinct = 0; float lastT = -1;
    for (int f = 0; f < tot; f++) { uint32_t now = base + (uint32_t)(f * 16.667); float t = now * 0.001f; if (t == lastT) same++; else distinct++; lastT = t; }
    float ulp = nextafterf(base * 0.001f, 1e30f) - base * 0.001f;
    printf("    edad %7.2f h: t ulp=%.4f s, cuadros con t repetido=%d/%d (%.0f%%)\n", h, ulp, same, tot, 100.0 * same / tot);
  }
}
static void t_trunc(int nseeds) {
  // mide cuantas veces el ojo llega al borde del tramo analitico (posible recorte de la forma) y su pose
  std::vector<int> byExpr(NEXPR, 0);
  unsigned long long frames = 0, tot = 0;
  for (int s = 0; s < nseeds; s++) {
    resetWorld(s * 31 + 5); autoOn = true; run(20);
    for (int x = 1; x < NEXPR; x++) {
      evSet(x, T, 0);
      for (int f = 0; f < 60; f++) { unsigned long long b = g_trunc; frame(1.0f / 60); frames++; if (g_trunc > b) { byExpr[x]++; tot += g_trunc - b; } }
    }
  }
  printf("trunc: %llu cuadros; pixeles de ojo dibujados justo en el borde del tramo analitico: %llu; cuadros afectados por expresion:", frames, tot);
  for (int x = 1; x < NEXPR; x++) printf(" %d:%d", x, byExpr[x]);
  printf("\n");
}

static void t_extdiff(int nseeds) {
  static uint16_t b1[240 * 240];
  std::vector<unsigned long long> diffE(NEXPR, 0), frE(NEXPR, 0); std::vector<int> worst(NEXPR, 0);
  for (int s = 0; s < nseeds; s++) {
    resetWorld(s * 31 + 5); autoOn = true; run(20);
    for (int x = 1; x < NEXPR; x++) {
      evSet(x, T, 0);
      for (int f = 0; f < 60; f++) {
        frame(1.0f / 60);
        if (f % 4) continue;
        memcpy(b1, g_buf, sizeof b1); float old = g_extk; g_extk = 1.7f; memset(g_buf, 0, sizeof b1); renderFrame(T); g_extk = old;
        int d = 0; for (int i = 0; i < 240 * 240; i++) d += g_buf[i] != b1[i];
        frE[x]++; if (d) { diffE[x]++; worst[x] = std::max(worst[x], d); }
        memcpy(g_buf, b1, sizeof b1);
      }
    }
  }
  printf("extdiff (ojo recortado por la cota analitica: render con cota x1.03 vs cota 1.7): cuadros muestreados con diferencia / total, y peor caso en pixeles, por expresion\n");
  unsigned long long tf = 0, td = 0;
  for (int x = 1; x < NEXPR; x++) { printf("  %2d %-13s %5llu/%5llu worst=%d px\n", x, XNAME[x], diffE[x], frE[x], worst[x]); tf += frE[x]; td += diffE[x]; }
  printf("  TOTAL %llu/%llu cuadros con recorte (%.2f%%)\n", td, tf, 100.0 * td / tf);
}
static void t_extimg(int nseeds, int xsel) {
  static uint16_t b1[240 * 240]; int best = 0; static uint16_t bA[240 * 240], bB[240 * 240]; int bs = -1;
  for (int s = 0; s < nseeds; s++) {
    resetWorld(s * 31 + 5); autoOn = true; run(20); evSet(xsel, T, 0);
    for (int f = 0; f < 60; f++) {
      frame(1.0f / 60); if (f % 4) continue;
      memcpy(b1, g_buf, sizeof b1); float old = g_extk; g_extk = 1.7f; memset(g_buf, 0, sizeof b1); renderFrame(T); g_extk = old;
      int d = 0; for (int i = 0; i < 240 * 240; i++) d += g_buf[i] != b1[i];
      if (d > best) { best = d; bs = s; memcpy(bA, b1, sizeof b1); memcpy(bB, g_buf, sizeof b1); }
      memcpy(g_buf, b1, sizeof b1);
    }
  }
  printf("worst diff=%d seed idx=%d\n", best, bs);
  FILE* fo = fopen("extimg.raw", "wb"); fwrite(bA, 2, 240 * 240, fo); fwrite(bB, 2, 240 * 240, fo); fclose(fo);
}
static void t_extm(int nseeds) {
  static uint16_t bref[240 * 240], bc[240 * 240];
  const float ms[] = {0.5f, 0.7f, 0.9f, 1.1f, 1.4f};
  unsigned long long dif[5] = {0}, tot = 0, px[5] = {0}, pxref = 0; int worst[5] = {0};
  for (int s = 0; s < nseeds; s++) {
    resetWorld(s * 31 + 5); autoOn = true; run(20);
    for (int x = 1; x < NEXPR; x++) {
      evSet(x, T, 0);
      for (int f = 0; f < 60; f++) {
        frame(1.0f / 60); if (f % 3) continue;
        float om = g_extm;
        g_extm = 3.0f; g_eyepx = 0; memset(g_buf, 0, sizeof bref); renderFrame(T); memcpy(bref, g_buf, sizeof bref); pxref += g_eyepx;
        for (int k = 0; k < 5; k++) {
          g_extm = ms[k]; g_eyepx = 0; memset(g_buf, 0, sizeof bc); renderFrame(T); px[k] += g_eyepx;
          int d = 0; for (int i = 0; i < 240 * 240; i++) d += g_buf[i] != bref[i];
          if (d) { dif[k]++; worst[k] = std::max(worst[k], d); }
        }
        g_extm = om; tot++;
      }
    }
  }
  printf("extm: %llu cuadros muestreados; referencia m=3.0\n", tot);
  for (int k = 0; k < 5; k++) printf("  m=%.1f: cuadros con diferencia=%llu (%.2f%%) peor=%d px; pixeles evaluados vs m=0.5: %+.1f%%\n", ms[k], dif[k], 100.0 * dif[k] / tot, worst[k], 100.0 * ((double)px[k] / px[0] - 1.0));
}
static void t_spi(int nfr) {
  printf("modelo SPI (31.25 MHz, 16 bit): enviado actual = union(filas sucias cuadro, filas sucias cuadro previo) x 240 col; ajustado = bbox real de pixeles no negros (cuadro + previo)\n");
  printf("  expr            filas enviadas  SPI ms  fps<=  | filas bbox real | SPI ms (solo filas) | SPI ms (filas x cols) | ahorro filas / filas+cols\n");
  double SPIHZ = 31.25e6;
  for (int x = 1; x < NEXPR; x++) {
    resetWorld(2024); autoOn = true; evSet(x, T, 0); run(90);
    int py0 = H, py1 = -1, tr0 = H, tr1 = -1, tc0 = 240, tc1 = -1;
    double sA = 0, sB = 0, sC = 0; int n = 0;
    for (int f = 0; f < nfr; f++) {
      T += 17; updateAnimation(T); memset(fb, 0, 240 * 240 * 2); dirtyY0 = H; dirtyY1 = -1; renderFrame(T);
      int r0 = H, r1 = -1, c0 = 240, c1 = -1;
      for (int y = 0; y < 240; y++) for (int xx = 0; xx < 240; xx++) if (fb[y * 240 + xx]) { r0 = std::min(r0, y); r1 = std::max(r1, y); c0 = std::min(c0, xx); c1 = std::max(c1, xx); }
      int y0 = std::min(dirtyY0, py0), y1 = std::max(dirtyY1, py1); if (dirtyY1 < 0) { y0 = py0; y1 = py1; }
      int rowsA = y1 >= y0 ? y1 - y0 + 1 : 0;
      int ty0 = std::min(r0, tr0), ty1 = std::max(r1, tr1), tx0 = std::min(c0, tc0), tx1 = std::max(c1, tc1);
      int rowsB = ty1 >= ty0 ? ty1 - ty0 + 1 : 0, colsB = tx1 >= tx0 ? tx1 - tx0 + 1 : 0;
      sA += rowsA; sB += rowsB; sC += (double)rowsB * colsB / 240.0; n++;
      py0 = dirtyY0 <= dirtyY1 ? dirtyY0 : H; py1 = dirtyY1; tr0 = r0; tr1 = r1; tc0 = c0; tc1 = c1; g_prog++;
    }
    auto ms = [&](double rows) { return rows * 240 * 16 / SPIHZ * 1000.0 + 0.25; };
    double a = sA / n, b = sB / n, c = sC / n;
    printf("  %2d %-13s %6.1f  %6.2f  %5.1f  | %6.1f | %6.2f | %6.2f | %4.0f%% / %4.0f%%\n", x, XNAME[x], a, ms(a), std::min(60.0, 1000 / ms(a)), b, ms(b), ms(c), 100 * (1 - ms(b) / ms(a)), 100 * (1 - ms(c) / ms(a)));
  }
}
// ---------------------------------------------------------------- rendimiento
static void bench(const char* name, int expr_, int nparts, int partType, int nclips, bool tone, int species, int pupil, bool bigEyes, int nfr) {
  resetWorld(2024); autoOn = false;
  G.species = species; G.pupil = pupil; G.toneMode = tone ? 1 : 0;
  evSet(expr_, T, 0);
  run(90);
  for (int k = 0; k < nclips; k++) clipPlay(k * 17 % NCLIPS, T, k & 1, 0.5f);
  std::vector<double> us; us.reserve(nfr);
  g_eyepx = g_eyedraw = g_ell = g_line = g_tri = g_plot = 0;
  double dirtyRows = 0;
  for (int f = 0; f < nfr; f++) {
    // mantener el pool de particulas lleno con el tipo pedido
    if (nparts) for (int i = 0; i < NPART; i++) if (i < nparts) { Part& p = parts[i]; if (!p.on) { p = {(float)(40 + (i * 37) % 160), (float)(30 + (i * 53) % 170), 0, 0, 5.0f, 5.0f, 10.0f, 0.3f * i, 1.0f, (uint8_t)(partType < 0 ? i % 6 : partType), (uint8_t)(i % 5), 1}; } p.life = 5; }
    if (nclips) { for (int k = 0; k < 3; k++) if (!slots[k].on) clipPlay((k * 17 + f) % NCLIPS, T, k & 1, 0.5f); }
    T += 17;
    updateAnimation(T);
    if (bigEyes) { cur[P_RX] = 74; cur[P_RY] = 92; }
    memset(fb, 0, sizeof(uint16_t) * W * H);
    dirtyY0 = H; dirtyY1 = -1;
    auto a = std::chrono::steady_clock::now();
    renderFrame(T);
    us.push_back(std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - a).count());
    dirtyRows += (dirtyY1 >= dirtyY0) ? dirtyY1 - dirtyY0 + 1 : 0;
    g_prog++;
  }
  std::sort(us.begin(), us.end());
  double sum = 0; for (double v : us) sum += v;
  printf("%-44s render host: med %6.0f us p95 %6.0f max %6.0f | por cuadro: eyePx=%7.0f eyeDrawn=%7.0f ellPx=%6.0f linePx=%6.0f triPx=%6.0f | filas=%.0f\n", name,
         us[us.size() / 2], us[us.size() * 95 / 100], us.back(), (double)g_eyepx / nfr, (double)g_eyedraw / nfr, (double)g_ell / nfr, (double)g_line / nfr, (double)g_tri / nfr, dirtyRows / nfr);
}
static void t_perf(int nfr) {
  int spB = 0, spMax = 0; float mx = 0;
  for (int i = 0; i < NSPECIES; i++) { float a = SPECIES[i].sx * SPECIES[i].sy; if (a > mx) { mx = a; spMax = i; } } (void)spB;
  int puRing = 0, puGlint2 = 0, puMax = 0; float pm = 0;
  for (int i = 0; i < NPUPILS; i++) { if (PUPILS[i].kind == 5) puRing = i; if (PUPILS[i].kind == 3) puGlint2 = i; float a = PUPILS[i].prx * PUPILS[i].pry; if (a > pm) { pm = a; puMax = i; } }
  printf("species mas grande: #%d %s (sx=%.2f sy=%.2f) | pupila anillo #%d, brillo+2tonos #%d, mayor #%d\n", spMax, SPECIES[spMax].name, SPECIES[spMax].sx, SPECIES[spMax].sy, puRing, puGlint2, puMax);
  bench("A nominal despierto, sin extras", X_AWAKE, 0, 0, 0, false, 0, 0, false, nfr);
  bench("B cara llena (disco 240 filas)", X_FULL, 0, 0, 0, false, 0, 0, false, nfr);
  bench("C despierto, especie max, pupila brillo+2t", X_AWAKE, 0, 0, 0, true, spMax, puGlint2, false, nfr);
  bench("D C + ojos al clamp (74x92)", X_AWAKE, 0, 0, 0, true, spMax, puGlint2, true, nfr);
  bench("E D + 3 clips", X_AWAKE, 0, 0, 3, true, spMax, puGlint2, true, nfr);
  bench("F E + 56 particulas MIXTAS", X_AWAKE, 56, -1, 3, true, spMax, puGlint2, true, nfr);
  bench("G E + 56 chispas (PT_SPARK size10)", X_AWAKE, 56, PT_SPARK, 3, true, spMax, puGlint2, true, nfr);
  bench("H E + 56 corazones", X_AWAKE, 56, PT_HEART, 3, true, spMax, puGlint2, true, nfr);
  bench("I 25 particulas mixtas (uso realista alto)", X_AWAKE, 25, -1, 1, false, 0, 0, false, nfr);
  bench("J mareado (espirales) nominal", X_DIZZY, 0, 0, 0, false, 0, 0, false, nfr);
  bench("K dormido (arcos) nominal", X_SLEEP, 0, 0, 0, false, 0, 0, false, nfr);
  bench("L enojado nominal", X_ANGRY, 0, 0, 0, false, 0, 0, false, nfr);
  bench("M pupila anillo, dos tonos, nominal", X_AWAKE, 0, 0, 0, true, 0, puRing, false, nfr);
}
// por expresion: filas sucias / pixeles evaluados (para la cuenta de SPI y de render)
static void t_perf_expr(int nfr) {
  printf("por expresion (criatura nominal sem. 2024, auto off): eyePx evaluados, filas sucias, host us\n");
  for (int x = 1; x < NEXPR; x++) {
    resetWorld(2024); autoOn = false; evSet(x, T, 0); run(90);
    g_eyepx = g_eyedraw = g_ell = g_line = g_tri = 0; double rows = 0, us = 0, maxus = 0; int minr = 999, maxr = 0; double colsum = 0;
    for (int f = 0; f < nfr; f++) {
      T += 17; updateAnimation(T); memset(fb, 0, 240 * 240 * 2); dirtyY0 = H; dirtyY1 = -1;
      auto a = std::chrono::steady_clock::now(); renderFrame(T);
      double u = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - a).count(); us += u; maxus = std::max(maxus, u);
      int r = dirtyY1 >= dirtyY0 ? dirtyY1 - dirtyY0 + 1 : 0; rows += r; minr = std::min(minr, r); maxr = std::max(maxr, r);
      // columnas realmente no negras en las filas sucias
      int cmin = 240, cmax = -1; for (int y = dirtyY0; y <= dirtyY1; y++) for (int xx = 0; xx < 240; xx++) if (fb[y * 240 + xx]) { cmin = std::min(cmin, xx); cmax = std::max(cmax, xx); }
      colsum += cmax >= cmin ? cmax - cmin + 1 : 0;
      g_prog++;
    }
    printf("  %2d %-14s eyePx=%7.0f tri/ell/line=%5.0f/%5.0f/%5.0f filas avg %5.1f [%3d..%3d] cols avg %5.1f host avg %5.0f max %5.0f us\n", x, XNAME[x], (double)g_eyepx / nfr,
           (double)g_tri / nfr, (double)g_ell / nfr, (double)g_line / nfr, rows / nfr, minr, maxr, colsum / nfr, us / nfr, maxus);
  }
}

int main(int argc, char** argv) {
  std::thread(watchdog).detach();
  fb = g_buf;
  std::string m = argc > 1 ? argv[1] : "";
  if (m == "info") {
    printf("NPALS=%d NSPECIES=%d NPUPILS=%d NCLIPS=%d NEXPR=%d sizeof(Pal)=%zu Species=%zu Pupil=%zu Clip=%zu Key=%zu Part=%zu Genome=%zu Eye=%zu\n", NPALS, NSPECIES, NPUPILS, NCLIPS, NEXPR, sizeof(Pal), sizeof(Species), sizeof(PupilStyle), sizeof(Clip), sizeof(Key), sizeof(Part), sizeof(Genome), sizeof(Eye));
    int keys = 0, maxn = 0, noev = 0; for (int i = 0; i < NCLIPS; i++) { keys += CLIPS[i].n; maxn = std::max(maxn, (int)CLIPS[i].n); }
    printf("claves totales=%d (prom %.1f/clip, max %d)\n", keys, (double)keys / NCLIPS, maxn); (void)noev;
    printf("RAM globales del core: POSE=%zu cur/vel/tgt=%zu fcur/ftgt=%zu parts=%zu nph=%zu slots=%zu seq=%zu eye=%zu motion=~%zu\n", sizeof POSE, 3 * sizeof cur, 2 * sizeof fcur, sizeof parts, sizeof nph, sizeof slots, sizeof seq, sizeof eye, sizeof(mCool) + 400);
    // validacion de datos
    int bad = 0;
    for (int i = 0; i < NCLIPS; i++) {
      const Clip& c = CLIPS[i];
      if (c.n < 2 || c.n > 12) { printf("clip %s n=%d fuera de 2..12\n", c.name, c.n); bad++; }
      for (int k = 1; k < c.n; k++) if (c.keys[k].t <= c.keys[k - 1].t) { printf("clip %s: t no creciente en clave %d\n", c.name, k); bad++; break; }
      if (c.dur != c.keys[c.n - 1].t) { printf("clip %s: dur=%d != ultima clave %d\n", c.name, c.dur, c.keys[c.n - 1].t); bad++; }
      if (c.tag > 7) { printf("clip %s tag %d\n", c.name, c.tag); bad++; }
      for (int k = 0; k < c.n; k++) if (c.keys[k].ev > 17) { printf("clip %s ev %d\n", c.name, c.keys[k].ev); bad++; }
      for (int j = 0; j < i; j++) if (!strcmp(c.name, CLIPS[j].name)) { printf("clip duplicado: %s (#%d y #%d)\n", c.name, j, i); }
      if ((c.flags & 2) && c.dur == 0) { printf("clip loop sin dur: %s\n", c.name); bad++; }
      for (int ch = 0; ch < 12; ch++) if (c.keys[c.n - 1].v[ch] != 0 && !(c.flags & 2)) { printf("clip %s no termina en reposo (canal %d = %d)\n", c.name, ch, c.keys[c.n - 1].v[ch]); bad++; break; }
    }
    for (int i = 0; i < NPALS; i++) if (PALS[i].rarity > 3) { printf("paleta %d rareza %d\n", i, PALS[i].rarity); bad++; }
    for (int i = 0; i < NSPECIES; i++) if (SPECIES[i].rarity > 3) { printf("especie %d rareza\n", i); bad++; }
    for (int i = 0; i < NPUPILS; i++) { if (PUPILS[i].rarity > 3 || PUPILS[i].kind > 5) { printf("pupila %d rareza/kind\n", i); bad++; } }
    int cnt[4][3] = {{0}}; for (int i = 0; i < NPALS; i++) cnt[PALS[i].rarity][0]++; for (int i = 0; i < NSPECIES; i++) cnt[SPECIES[i].rarity][1]++; for (int i = 0; i < NPUPILS; i++) cnt[PUPILS[i].rarity][2]++;
    printf("rarezas (pal/sp/pu): "); for (int r = 0; r < 4; r++) printf("[%d: %d/%d/%d] ", r, cnt[r][0], cnt[r][1], cnt[r][2]); printf("\n");
    int tagc[8] = {0}; for (int i = 0; i < NCLIPS; i++) tagc[CLIPS[i].tag]++; printf("clips por tag:"); for (int t = 0; t < 8; t++) printf(" %d:%d", t, tagc[t]); printf("\n");
    printf("clip ultimo (fallback director) = #%d %s tag=%d\n", NCLIPS - 1, CLIPS[NCLIPS - 1].name, CLIPS[NCLIPS - 1].tag);
    printf("validacion de datos: problemas=%d\n", bad);
    // nombres que Motion.h necesita
    const char* need[] = {"sacudida_suave", "sacudida_fuerte", "caida", "impacto", "boca_abajo", "asiente", "m1_asiente", "recogido", "arrullo", "mecido", "caminata", "giro", "mareo_de_giro", "tap", "doble_tap", "puesto_en_mesa", "vibracion", "cosquillas_de_mov", "alivio", "aterrizaje", "susto_de_caida"};
    for (const char* n : need) if (clipFind(n) < 0) printf("FALTA clip pedido por Motion.h: %s\n", n);
    return 0;
  }
  if (m == "seeds") { t_seeds(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), argc > 5 ? atoi(argv[5]) : 24); return g_nfail ? 1 : 0; }
  if (m == "storm") { t_storm(atoi(argv[2]), atoi(argv[3]), atoi(argv[4])); return g_nfail ? 1 : 0; }
  if (m == "clips") { t_clips(); return g_nfail ? 1 : 0; }
  if (m == "triple") { t_triple(); return g_nfail ? 1 : 0; }
  if (m == "motion") { t_motion(); return g_nfail ? 1 : 0; }
  if (m == "dt") { g_nonan = argc > 2; t_dt(); return 0; }
  if (m == "wrap") { t_wrap(); return 0; }
  if (m == "extdiff") { t_extdiff(atoi(argv[2])); return 0; }
  if (m == "extimg") { t_extimg(atoi(argv[2]), atoi(argv[3])); return 0; }
  if (m == "extm") { t_extm(atoi(argv[2])); return 0; }
  if (m == "spi") { t_spi(argc > 2 ? atoi(argv[2]) : 200); return 0; }
  if (m == "trunc") { t_trunc(atoi(argv[2])); return 0; }
  if (m == "perf") { t_perf(argc > 2 ? atoi(argv[2]) : 600); return 0; }
  if (m == "perfx") { t_perf_expr(argc > 2 ? atoi(argv[2]) : 300); return 0; }
  fprintf(stderr, "modo?\n");
  return 2;
}
