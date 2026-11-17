// arnes de prueba de Motion.h con gestos sinteticos (stubs de EyesCore)
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
const float TAU = 6.2831853f;
float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
enum { X_AWAKE = 1, X_CURIOUS, X_SLEEP, X_HAPPY, X_ANGRY, X_LOVE, X_CHARGING, X_ELECTRIC, X_DIMSLEEP, X_SAD,
  X_FULL, X_HYPE, X_LOWBAT, X_DIZZY, X_SURPRISE, X_SMUG, X_YAWN, X_ANXIOUS, X_COLD, X_BATCHECK };
struct Step { uint8_t x; uint16_t dur; };
struct Eye { float sx, sy; } eye[2];
float shakeAmp = 0; uint32_t lastActivity = 0; bool deepSleep = false; uint8_t expr = X_AWAKE;
bool isAsleep() { return deepSleep || expr == X_SLEEP || expr == X_DIMSLEEP; }
void wake(uint32_t) {}
void evShake(uint32_t, bool) {}
void evSet(uint8_t x, uint32_t, uint16_t) { expr = x; }
void play(uint32_t, uint8_t, Step a, Step b = {0, 0}, Step c = {0, 0}) { expr = a.x ? a.x : X_AWAKE; (void)b; (void)c; }
void setGaze(float, float, uint32_t, uint32_t) {}
const char* NAMES[] = {"sacudida_suave", "sacudida_fuerte", "caida", "impacto", "boca_abajo", "asiente", "m1_asiente", "arrullo", "mecido", "caminata", "giro", "tap", "doble_tap", "recogido", "puesto_en_mesa", "mareo_de_giro", "alivio", "susto_de_caida", "aterrizaje", "vibracion", "cosquillas_de_mov"};
int clipFind(const char* n) { for (int i = 0; i < 21; i++) if (!strcmp(NAMES[i], n)) return i; return -1; }
int clipPlay(int, uint32_t, bool = false, float = 1.0f) { return 0; }
std::vector<std::pair<uint32_t, int>> LOG;
void motionTrace(uint8_t w, uint32_t now) { LOG.push_back({now, w}); }
#define MOTION_TRACE
#include "Motion.h"

uint32_t T = 1000; float TS = 0;
unsigned rs = 12345; float rnd() { rs = rs * 1664525u + 1013904223u; return ((rs >> 8) & 0xFFFF) / 32768.0f - 1.0f; }
void feed(float x, float y, float z) { T += 16; TS += 1 / 60.f; motionFeed(x, y, z, T, 1 / 60.f); }
template <class F> void run(float secs, F f) { int n = (int)(secs * 60); float t0 = 0; for (int i = 0; i < n; i++) { float t = i / 60.f; float x, y, z; f(t, x, y, z); feed(x + 0.01f * rnd(), y + 0.01f * rnd(), z + 0.01f * rnd()); (void)t0; } }
void still(float s) { run(s, [](float, float& x, float& y, float& z) { x = 0; y = 1; z = 0; }); }
void reset() { memset(mCool, 0, sizeof mCool); }
void report(const char* name, const char* expect) {
  printf("%-22s esperado[%s] obtenido[", name, expect);
  for (auto& e : LOG) printf("%d@%.1f ", e.second, (e.first - 1000) / 1000.0f);
  printf("]\n"); LOG.clear();
}
#define TEST(name, expect, body) { LOG.clear(); still(2.5f); LOG.clear(); body; report(name, expect); still(4.0f); LOG.clear(); }
int main() {
  still(3);
  TEST("quieto 20s", "", still(20));
  TEST("inclinar lento 40deg", "25", {
    run(3, [](float t, float& x, float& y, float& z) { float a = 0.7f * t / 3; x = -sinf(a); y = cosf(a); z = 0; });
    run(2.5f, [](float, float& x, float& y, float& z) { float a = 0.7f; x = -sinf(a); y = cosf(a); z = 0; }); });
  still(3);
  TEST("arrullo 15deg 0.6Hz", "8 .. 26", run(9, [](float t, float& x, float& y, float& z) { float a = 0.26f * sinf(TAU * 0.6f * t); x = -sinf(a); y = cosf(a); z = 0; }));
  TEST("mecido 35deg 1.0Hz", "9", run(5, [](float t, float& x, float& y, float& z) { float a = 0.6f * sinf(TAU * 1.0f * t); x = -sinf(a); y = cosf(a); z = 0; }));
  TEST("mecer adelante-atras", "8", run(7, [](float t, float& x, float& y, float& z) { float a = 0.3f * sinf(TAU * 0.6f * t); x = 0; y = cosf(a); z = sinf(a); }));
  TEST("sacudida suave 4Hz", "0", run(1.2f, [](float t, float& x, float& y, float& z) { x = 1.6f * sinf(TAU * 4 * t); y = 1; z = 0; }));
  TEST("sacudida fuerte", "1", run(1.5f, [](float t, float& x, float& y, float& z) { x = 3.5f * sinf(TAU * 4 * t); y = 1 + 2.0f * sinf(TAU * 3 * t); z = 0; }));
  TEST("golpe unico 3g (impacto)", "3", { feed(3.2f, 1, 0); feed(0, 1, 0); still(0.5f); });
  TEST("caida 0.5s + golpe", "2 3", { run(0.5f, [](float, float& x, float& y, float& z) { x = 0.05f; y = 0.05f; z = 0.05f; }); feed(1.5f, 2.5f, 2); feed(0, 1, 0); still(0.5f); });
  TEST("caida 0.7s + atrapado", "2 19", { run(0.7f, [](float, float& x, float& y, float& z) { x = 0.05f; y = 0.05f; z = 0.05f; }); feed(0, 1.3f, 0); still(0.5f); });
  TEST("salto 0.2s + aterrizaje", "2 20", { run(0.2f, [](float, float& x, float& y, float& z) { x = 0.05f; y = 0.05f; z = 0.05f; }); feed(0, 2.2f, 0); feed(0, 1.2f, 0); still(0.5f); });
  TEST("caida larga + golpe medio", "2 21", { run(0.8f, [](float, float& x, float& y, float& z) { x = 0.05f; y = 0.05f; z = 0.05f; }); feed(0, 2.1f, 0); feed(0, 1.2f, 0); still(0.5f); });
  TEST("lanzar al aire", "2(thrown)", { run(0.12f, [](float, float& x, float& y, float& z) { x = 0; y = 2.4f; z = 0; }); run(0.5f, [](float, float& x, float& y, float& z) { x = 0.05f; y = 0.05f; z = 0.05f; }); feed(0, 1.2f, 0); still(0.5f); });
  TEST("tap", "14", { feed(0.9f, 1, 0); feed(0.2f, 1, 0); still(0.6f); });
  TEST("doble tap", "14 15", { feed(0.9f, 1, 0); feed(0.2f, 1, 0); still(0.3f); feed(-0.9f, 1, 0); feed(-0.2f, 1, 0); still(0.6f); });
  TEST("tap 3 veces lento", "14 14 14", { for (int i = 0; i < 3; i++) { feed(0.9f, 1, 0); feed(0.2f, 1, 0); still(1.0f); } });
  TEST("caminar 1.9Hz", "10.. 22", { run(7, [](float t, float& x, float& y, float& z) { x = 0.05f * sinf(TAU * 0.95f * t); y = 1 + 0.3f * sinf(TAU * 1.9f * t); z = 0.1f * sinf(TAU * 1.9f * t); }); still(3); });
  TEST("correr 3Hz", "11..", run(5, [](float t, float& x, float& y, float& z) { x = 0; y = 1 + 0.9f * sinf(TAU * 3.0f * t); z = 0.2f * sinf(TAU * 3.0f * t); }));
  TEST("giro 5rad/s 2s", "12 13", { run(2, [](float t, float& x, float& y, float& z) { float a = 5 * t; x = -sinf(a); y = cosf(a); z = 0; }); still(1.5f); });
  TEST("recoger (de pie, inclina)", "7", { run(0.6f, [](float t, float& x, float& y, float& z) { float a = 1.0f * t / 0.6f; x = -sinf(a) + 0.2f * rnd(); y = cosf(a) + 0.2f * rnd(); z = 0.15f * rnd(); }); run(1, [](float, float& x, float& y, float& z) { x = -0.84f + 0.1f * rnd(); y = 0.54f + 0.1f * rnd(); z = 0.1f * rnd(); }); });
  TEST("recoger recto hacia arriba", "7", { run(0.4f, [](float, float& x, float& y, float& z) { x = 0; y = 1.25f; z = 0; }); run(0.4f, [](float, float& x, float& y, float& z) { x = 0; y = 0.8f; z = 0; }); });
  TEST("puesto en mesa", "16", { run(2, [](float t, float& x, float& y, float& z) { x = 0.18f * sinf(TAU * 2.3f * t) + 0.1f * rnd(); y = 1 + 0.1f * sinf(TAU * 3.1f * t) + 0.1f * rnd(); z = 0.15f * sinf(TAU * 1.7f * t); }); feed(0, 1.5f, 0); feed(0, 0.9f, 0); still(1); });
  TEST("vibracion motor", "17", { for (int i = 0; i < 100; i++) feed(0, 1, (i & 1) ? 0.18f : -0.18f); });
  TEST("cosquillas (temblor)", "18", { float s = 0; for (int i = 0; i < 180; i++) { s += 0.3f * (rnd() - s); feed(1.6f * s, 1 + 0.3f * rnd(), 0.3f * rnd()); } });
  TEST("de cabeza 3s", "4, 5 al volver", { run(3, [](float, float& x, float& y, float& z) { x = 0; y = -1; z = 0; }); });
  TEST("boca abajo 3s", "6", run(3, [](float, float& x, float& y, float& z) { x = 0; y = 0; z = -1; }));
  TEST("plano 30s", "23 24", run(30, [](float, float& x, float& y, float& z) { x = 0; y = 0; z = 1; }));
  // robustez
  { LOG.clear(); float bad[] = {NAN, INFINITY, -INFINITY, 1e30f, -1e30f};
    for (int i = 0; i < 200; i++) { float v = bad[i % 5]; motionFeed(v, bad[(i + 1) % 5], v, T += 16, (i % 3) ? NAN : -1.0f); }
    bool ok = true; float* f[] = {&mG[0], &mG[1], &mG[2], &mShake, &mRoll, &mLeanX, &mLeanY, &mTiltFace, &mAlt, &mHF, &mMagVar, &mMagPk, &mSpinRate, &mSpinAcc, &mRest[0], &mDown[0], &mDown[1]};
    for (auto p : f) if (!(*p == *p) || fabsf(*p) > 1e6f) ok = false;
    printf("NaN/inf: estado %s, eventos=%d\n", ok ? "finito" : "CORRUPTO", (int)LOG.size()); LOG.clear(); }
  { unsigned long long cnt = 0; for (int i = 0; i < 20000; i++) { motionFeed(3 * rnd(), 3 * rnd(), 3 * rnd(), T += 16, 1 / 60.f); }
    bool ok = true; float* f[] = {&mG[0], &mG[1], &mG[2], &mShake, &mRoll, &mAlt, &mHF, &mSpinRate}; for (auto p : f) if (!(*p == *p)) ok = false;
    printf("ruido 20000 cuadros: %s, eventos=%d\n", ok ? "ok" : "NaN", (int)LOG.size()); (void)cnt; }
  return 0;
}
