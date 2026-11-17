#line 1 "C:\\Users\\jordi\\OneDrive\\Escritorio\\Alloy\\studio\\work\\aud_tech\\sketch_fix\\Alloy_Eyes\\Motion.h"
// Motion.h - capa de MOVIMIENTO (acelerometro).  v2 (M1, 2026-11-14)
// Sin hardware: el firmware (MPU) o el simulador la alimentan con motionFeed(ax,ay,az,now,dt) a ~60 Hz.
//
// CONVENCION de ejes (en g): x = derecha de la pantalla, y = ARRIBA de la pantalla, z = hacia el espectador.
// El sensor lee la aceleracion que se opone a la gravedad:  de pie = (0,+1,0) ; boca arriba en la mesa = (0,0,+1);
// boca abajo = (0,0,-1); de cabeza = (0,-1,0); pantalla girada a la derecha (lado derecho abajo) = (-1,0,0).
//
// Efectos CONTINUOS (siempre): gravedad -> los ojos/pupilas resbalan hacia "abajo" y la cara se ladea (mLeanX/mLeanY/mTiltFace).
// Eventos DISCRETOS (motionReact): ver la tabla de codigos abajo; todos pasan por enfriamientos (mReady) y por
// detectores con histeresis/duracion minima para evitar falsos positivos. El guion completo (umbrales, duraciones,
// enfriamientos, falsos positivos) esta en studio/deliveries/m1/guion_reacciones.md.
//
// CAMBIOS v2 (resumen): entrada saneada (NaN/inf/dt), sacudida exige inversiones de signo (ya no basta un golpe),
// impacto no se dispara en plena sacudida, ventana de aterrizaje (aterrizaje / susto / alivio / impacto),
// tap + doble tap (pico corto con quietud previa), recogido y puesto-en-mesa (quietud previa + cambio de orientacion /
// golpecito de asentamiento), mecer/arrullar (oscilacion lenta de la gravedad en cualquier eje), caminar/correr
// (oscilacion vertical ritmica), giro rapido + mareo de giro (velocidad angular de la gravedad en el plano de pantalla),
// vibracion (alternancia cuadro a cuadro), cosquillas (temblor irregular), aburrimiento plano, sueno por quietud larga.
//
// Todo es aritmetica de float; sin dependencias de hardware. Requiere de EyesCore.h: evShake, play, wake, isAsleep,
// setGaze, clipFind, clipPlay, clampf, lastActivity, deepSleep, eye[], shakeAmp, TAU, X_*.
#pragma once

float mAx = 0, mAy = 1, mAz = 0;     // ultima lectura (g)
float mG[3] = {0, 1, 0};              // gravedad filtrada (g)
float mDown[2] = {0, 1};              // "abajo" en coordenadas de pantalla (x der, y abajo)
float mShake = 0;                     // energia de sacudida suavizada (g)
float mRoll = 0;                      // balanceo (rad): + = lado derecho de la pantalla hacia abajo
float mLeanX = 0, mLeanY = 0, mTiltFace = 0;   // efecto continuo listo para usar
bool mActive = false;                 // true cuando alguien alimenta lecturas
float mFallT = 0, mUpsideT = 0, mFlatT = 0, mFaceDownT = 0, mStillT = 0;
bool mFalling = false, mWasUpside = false;

// ---- enfriamientos por tipo de evento ----
uint32_t mCool[24];
enum { MC_SHAKE, MC_FALL, MC_IMPACT, MC_UPSIDE, MC_PICKUP, MC_FACEDOWN, MC_OTHER1, MC_OTHER2,
       MC_SHAKE2, MC_TAP, MC_TAP2, MC_ROCK, MC_WALK, MC_SPIN, MC_DIZZY, MC_VIB, MC_TICKLE, MC_PLACE, MC_LAND,
       MC_BORED, MC_ARRIVE, MC_TILT, MC_SLEEPY, MC_RETURN };
bool mReady(int k, uint32_t now, uint32_t ms) { if (now - mCool[k] < ms && mCool[k]) return false; mCool[k] = now ? now : 1; return true; }

// ---- estado de los detectores nuevos ----
float mDirX = 1;                      // lado del ultimo golpe/giro/balanceo (+1 derecha, -1 izquierda): decide el "mirror" del clip
float mMagVar = 0;                    // |mag-1| suavizado (0.4 s): ~0 = movimiento casi estatico (mecer), alto = aceleraciones (caminar)
float mMagPk = 1;                     // pico reciente de |a| (para distinguir "lanzado" de "se cayo")
bool mThrown = false;                 // la caida libre empezo tras una aceleracion fuerte
float mLandT = -1, mLandPk = 0, mFallDur = 0;   // ventana de aterrizaje (-1 = inactiva)
float mPJx = 0, mPJy = 0, mPJz = 0, mPJm = 0;   // salto (a - a_prev) anterior
float mAlt = 0.4f, mHF = 0, mVibT = 0, mTickleT = 0;
float mRevT = 0; int mRevN = 0; int8_t mRevS[3] = {0, 0, 0};   // inversiones de signo por eje (sacudida)
float mShakeHiT = 0;                  // tiempo seguido con mShake alto
// tap
uint8_t mTapSt = 0, mTapN = 0; float mTapT = 0, mTapPk = 0, mTapTail = 0, mTapAge = 9, mTapDX = 0;
// recoger / poner en mesa / quietud
float mRest[3] = {0, 1, 0}; float mRestOK = 0, mLiftT = 0;
float mMovT = 0, mBumpAge = 9, mHeldAtBump = 0;
uint8_t mFlatSt = 0; bool mStillFired = false;
float mTiltT = 0; bool mTiltFired = false;
uint8_t mImpSt = 0; float mImpT = 0, mImpPk = 0;   // golpe pendiente de confirmar
int8_t mSpinDir = 0;

// oscilador generico: cuenta medias-oscilaciones consecutivas de una senal (extremos con histeresis)
struct MOsc { float ext, prevExt, tSince, halfT, per, amp; int8_t dir; uint8_t n; };
MOsc mOscG, mOscR[3];                 // marcha (aceleracion vertical) y mecer (ejes x,y,z)
void mOscConfirm(MOsc& o, float e, float h, float tMin, float tMax) {
  float pp = fabsf(e - o.prevExt), ht = o.tSince;
  o.prevExt = e; o.tSince = 0;
  if (pp >= 1.6f * h && ht >= tMin && ht <= tMax) {
    if (o.n > 0 && fabsf(ht - o.halfT) < 0.5f * o.halfT + 0.05f) { if (o.n < 250) o.n++; o.halfT += (ht - o.halfT) * 0.5f; o.amp += (0.5f * pp - o.amp) * 0.5f; }
    else { o.n = 1; o.halfT = ht; o.amp = 0.5f * pp; }
    o.per = 2.0f * o.halfT;
  } else o.n = 0;
}
void mOscFeed(MOsc& o, float s, float dt, float h, float tMin, float tMax) {
  o.tSince += dt;
  if (o.tSince > tMax * 2.2f) { o.n = 0; o.tSince = tMax * 2.2f; }
  if (o.dir == 0) { o.ext = s; o.prevExt = s; o.dir = 1; return; }
  if (o.dir > 0) {
    if (s > o.ext) o.ext = s;
    else if (o.ext - s > h) { mOscConfirm(o, o.ext, h, tMin, tMax); o.dir = -1; o.ext = s; }
  } else {
    if (s < o.ext) o.ext = s;
    else if (s - o.ext > h) { mOscConfirm(o, o.ext, h, tMin, tMax); o.dir = 1; o.ext = s; }
  }
}
void mOscClear(MOsc& o) { o.n = 0; o.dir = 0; o.tSince = 0; }

bool mGaitOn = false; uint8_t mGaitMax = 0; bool mRockOn = false, mRockSlept = false; float mRockT = 0, mRockLost = 0;
float mSpinSum = 0;   // giro neto acumulado (rad, con signo): el ruido y los vaivenes se cancelan, un giro real crece
float mSpinPrev = 0, mSpinRate = 0, mSpinAcc = 0, mSpinT = 0, mSpinQuiet = 0; bool mSpinHave = false, mSpinFired = false;

float mClean(float v, float fb) { return (v > -32.0f && v < 32.0f) ? v : fb; }   // NaN e inf caen al valor de respaldo

// ---- utilidades de reaccion ----
void mClip(const char* a, uint32_t now, bool mir = false, float spd = 1.0f, const char* b = 0) {
  int c = clipFind(a);
  if (c < 0 && b) c = clipFind(b);
  if (c >= 0) clipPlay(c, now, mir, spd);
}
// despierta si dormia (devuelve true si ya estaba despierto)
bool mWake(uint32_t now) {
  if (isAsleep()) { wake(now); return false; }
  lastActivity = now; return true;
}
void mBegin(uint32_t now) { lastActivity = now; deepSleep = false; }

// ---- reacciones: aqui se conecta cada gesto con clips/expresiones ----
// what: 0 sacudida suave   1 sacudida fuerte   2 inicio de caida (strength=|a|)   3 impacto (strength=g del golpe)
//       4 de cabeza        5 vuelve a posicion normal   6 boca abajo en la mesa   7 recogido
//       8 arrullo (mecer suave y lento)   9 mecido (balanceo grande/rapido)   10 caminata (strength=periodo de paso, s)
//       11 correr          12 giro rapido en el plano   13 mareo tras girar   14 tap   15 doble tap
//       16 puesto en la mesa   17 vibracion de motor   18 cosquillas (temblor irregular)   19 alivio (lo atraparon)
//       20 aterrizaje suave   21 susto tras caida larga   22 llegada (acaba de caminar)   23 aburrido plano (mira arriba)
//       24 quietud larga -> bostezo y sueno   25 inclinado largo (curioso)   26 se duerme mecido
void motionReact(uint8_t what, float strength, uint32_t now) {
#ifdef MOTION_TRACE
  motionTrace(what, now);                // gancho solo para pruebas en PC (arnes de M1); en el firmware no existe
#endif
  bool mir = mDirX < 0;
  bool was;
  switch (what) {
    case 0: was = isAsleep(); evShake(now, false); if (!was) mClip("sacudida_suave", now, mir); break;
    case 1: was = isAsleep(); evShake(now, true);  if (!was) mClip("sacudida_fuerte", now, mir); break;
    case 2: mBegin(now);
            if (mThrown) { play(now, 2, {X_HYPE, 1300}, {0, 0}); mClip("recogido", now, false, 1.2f); }      // lanzado al aire: "wheee"
            else { play(now, 2, {X_ANXIOUS, 1500}, {0, 0}); mClip("caida", now, mir); }
            break;
    case 3: mBegin(now);
            for (int e = 0; e < 2; e++) { eye[e].sx = 0.30f; eye[e].sy = -0.34f; }
            shakeAmp = 4;
            if (strength > 2.5f || mFallDur > 0.4f) play(now, 3, {X_DIZZY, 1500}, {X_CURIOUS, 800}, {0, 0});
            mClip("impacto", now, mir); break;
    case 4: mBegin(now); play(now, 3, {X_DIZZY, 1800}, {X_SAD, 2200}, {0, 0}); mClip("mareo_de_giro", now, mir, 0.7f); break;   // de cabeza
    case 5: mBegin(now); play(now, 2, {X_HAPPY, 1300}, {0, 0}); mClip("alivio", now, mir); break;
    case 6: mClip("boca_abajo", now, mir); evSet(X_SLEEP, now, 0); break;                              // boca abajo: se hace el dormido
    case 7: if (!mWake(now)) { mClip("recogido", now); break; }
            play(now, 2, {X_CURIOUS, 1500}, {0, 0}); mClip("recogido", now); break;
    case 8: if (isAsleep()) break;
            lastActivity = now; play(now, 2, {X_DIMSLEEP, 2600}, {0, 0}); mClip("arrullo", now, mir); break;
    case 9: if (!mWake(now)) break;
            play(now, 2, {X_HAPPY, 1800}, {0, 0}); mClip("mecido", now, mir); break;
    case 10: if (!mWake(now)) break;
             { float s = clampf(0.5f / (strength > 0.2f ? strength : 0.5f), 0.7f, 1.8f); mClip("caminata", now, mir, s); } break;
    case 11: if (!mWake(now)) break;
             if (expr != X_HYPE) play(now, 2, {X_HYPE, 2400}, {0, 0});
             { float s = clampf(0.5f / (strength > 0.2f ? strength : 0.33f), 0.9f, 1.9f); mClip("caminata", now, mir, s); } break;
    case 12: if (!mWake(now)) break;
             mClip("giro", now, mir); break;
    case 13: mBegin(now); play(now, 2, {X_DIZZY, 2200}, {0, 0}); mClip("mareo_de_giro", now, mir); break;
    case 14: if (!mWake(now)) break;
             mClip("tap", now, mir); break;
    case 15: if (!mWake(now)) break;
             play(now, 2, {X_CURIOUS, 1400}, {0, 0}); mClip("doble_tap", now, mir); break;
    case 16: if (!mWake(now)) break;
             play(now, 2, {X_HAPPY, 1000}, {0, 0}); mClip("puesto_en_mesa", now); break;
    case 17: if (!mWake(now)) break;
             play(now, 2, {X_SURPRISE, 500}, {X_HAPPY, 1200}, {0, 0}); mClip("vibracion", now, mir); break;
    case 18: if (!mWake(now)) break;
             play(now, 2, {X_HAPPY, 2000}, {0, 0}); mClip("cosquillas_de_mov", now, mir); break;
    case 19: mBegin(now); play(now, 2, {X_HAPPY, 1300}, {0, 0}); mClip("alivio", now, mir); break;
    case 20: mBegin(now); play(now, 2, {X_SURPRISE, 450}, {0, 0}); mClip("aterrizaje", now, mir); break;
    case 21: mBegin(now); play(now, 2, {X_ANXIOUS, 2200}, {0, 0}); mClip("susto_de_caida", now, mir); break;
    case 22: if (isAsleep()) break;
             mClip("m1_asiente", now, false, 1.0f, "asiente"); break;
    case 23: if (isAsleep()) break;
             setGaze(0.0f, -0.75f, now, 2500); play(now, 2, {X_CURIOUS, 2200}, {0, 0}); break;
    case 24: if (isAsleep()) break;
             play(now, 2, {X_YAWN, 1700}, {X_SLEEP, 0}); break;
    case 25: if (isAsleep()) break;
             play(now, 2, {X_CURIOUS, 1800}, {0, 0}); break;
    case 26: if (isAsleep()) break;
             play(now, 2, {X_YAWN, 1300}, {X_SLEEP, 0}); break;
  }
}

void motionFeed(float ax, float ay, float az, uint32_t now, float dt) {
  // ---- 0) entrada saneada: NaN/inf -> ultima lectura buena; dt invalido -> 1/60 s ----
  ax = mClean(ax, mAx); ay = mClean(ay, mAy); az = mClean(az, mAz);
  if (!(dt > 0.0005f)) dt = 1.0f / 60.0f;
  if (dt > 0.1f) dt = 0.1f;
  mActive = true;
  float pax = mAx, pay = mAy, paz = mAz, shakePrev = mShake, stillPrev = mStillT;   // valores ANTES de integrar este cuadro
  mAx = ax; mAy = ay; mAz = az;

  // ---- 1) filtros base ----
  float k = 1.0f - expf(-dt / 0.25f);
  mG[0] += (ax - mG[0]) * k; mG[1] += (ay - mG[1]) * k; mG[2] += (az - mG[2]) * k;
  float mag = sqrtf(ax * ax + ay * ay + az * az);
  float dx = ax - mG[0], dy = ay - mG[1], dz = az - mG[2];
  float dev = sqrtf(dx * dx + dy * dy + dz * dz);
  mShake += (dev - mShake) * (1.0f - expf(-dt / 0.14f));
  float sx = -mG[0], sy = mG[1], n = sqrtf(sx * sx + sy * sy);
  if (n > 0.25f) { mDown[0] += (sx / n - mDown[0]) * 0.25f; mDown[1] += (sy / n - mDown[1]) * 0.25f; mRoll = atan2f(sx, sy); }
  mLeanX = mDown[0] * 10.0f; mLeanY = (mDown[1] - 1.0f) * 8.0f;
  mTiltFace = clampf(mRoll, -1.2f, 1.2f) * 0.30f;
  mMagVar += (fabsf(mag - 1.0f) - mMagVar) * (1.0f - expf(-dt / 0.4f));
  mMagPk -= dt * 6.0f; if (mMagPk < 1.0f) mMagPk = 1.0f; if (mag > mMagPk) mMagPk = mag;
  if (fabsf(dx) > 0.5f) mDirX = dx > 0 ? 1.0f : -1.0f;
  mStillT = mShake < 0.06f ? mStillT + dt : 0;
  if (mShake > 0.10f && !isAsleep()) { if (mMovT > 0.5f) lastActivity = now; }   // llevarlo en la mano cuenta como actividad (no se duerme)

  // salto cuadro a cuadro: alternancia (vibracion) y energia de alta frecuencia
  float jx = ax - pax, jy = ay - pay, jz = az - paz, jm = sqrtf(jx * jx + jy * jy + jz * jz);
  { float kk = 1.0f - expf(-dt / 0.3f);
    if (jm > 0.03f && mPJm > 0.03f) mAlt += ((jx * mPJx + jy * mPJy + jz * mPJz) < 0 ? 1.0f - mAlt : -mAlt) * kk;
    else mAlt += (0.4f - mAlt) * kk;
    mHF += (jm - mHF) * kk; }
  mPJx = jx; mPJy = jy; mPJz = jz; mPJm = jm;

  // inversiones de signo de la parte dinamica (una sacudida de verdad cambia de sentido; un golpe, no)
  mRevT += dt;
  { float d3[3] = {dx, dy, dz};
    for (int i = 0; i < 3; i++) {
      if (fabsf(d3[i]) > 0.35f) {
        int8_t s = d3[i] > 0 ? 1 : -1;
        if (mRevS[i] != 0 && s != mRevS[i]) { mRevN++; mRevT = 0; }
        mRevS[i] = s;
      }
    }
    if (mRevT > 0.7f) { mRevN = 0; mRevS[0] = mRevS[1] = mRevS[2] = 0; mRevT = 0; } }

  // ---- 2) caida libre, ventana de aterrizaje e impacto ----
  if (mag < 0.38f) {
    mFallT += dt;
    if (mFallT > 0.09f && !mFalling) {
      mFalling = true; mThrown = mMagPk > 1.7f; mLandT = -1;
      if (mReady(MC_FALL, now, 800)) motionReact(2, mag, now);
    }
  } else {
    if (mFalling) { mFalling = false; mFallDur = mFallT; mLandT = 0; mLandPk = dev > fabsf(mag - 1.0f) ? dev : fabsf(mag - 1.0f); }
    mFallT = 0;
  }
  if (mLandT >= 0) {                    // tras la caida: espera el golpe hasta 0.22 s y clasifica
    mLandT += dt; if (dev > mLandPk) mLandPk = dev;
    if (mLandPk > 2.4f || mLandT > 0.22f) {
      float pk = mLandPk; mLandT = -1;
      if (pk > 2.4f) { if (mReady(MC_IMPACT, now, 400)) motionReact(3, pk, now); }
      else if (pk > 1.6f) { if (mReady(MC_LAND, now, 600)) motionReact(mFallDur < 0.30f ? 20 : 21, pk, now); }
      else if (mFallDur > 0.12f) { if (mReady(MC_LAND, now, 600)) motionReact(19, pk, now); }
    }
  }
  // golpe sin caida previa: pico fuerte desde la calma; se confirma 0.12 s despues si NO continua como sacudida
  if (mImpSt == 0) {
    if (!mFalling && mLandT < 0 && dev > 2.4f && mag > 2.0f && shakePrev < 0.9f) { mImpSt = 1; mImpT = 0; mImpPk = dev; }
  } else {
    mImpT += dt; if (mImpT < 0.05f && dev > mImpPk) mImpPk = dev;
    if (mImpT >= 0.12f) { mImpSt = 0; if (mRevN < 3 && mShake < 1.3f && mReady(MC_IMPACT, now, 400)) { mRestOK = 0; motionReact(3, mImpPk, now); } }
  }
  if (mFalling || mLandT >= 0) mRestOK = 0;                              // lo que cae no "fue recogido"

  // ---- 3) tap / doble tap: pico corto (<~0.1 s) tras quietud, que no continua en movimiento ----
  mTapAge += dt; if (mTapAge > 9.0f) mTapAge = 9.0f;
  if (mTapN == 1 && mTapAge > 0.75f) mTapN = 0;
  if (mTapSt == 0) {
    bool calm = stillPrev > 0.25f || (mTapN == 1 && mTapAge < 0.6f && shakePrev < 0.22f);
    if (calm && !mFalling && mLandT < 0 && dev > 0.45f && dev < 2.4f) { mTapSt = 1; mTapT = 0; mTapPk = dev; mTapTail = 0; mTapDX = dx; }
  } else {
    mTapT += dt;
    if (mTapT < 0.05f) { if (dev > mTapPk) { mTapPk = dev; mTapDX = dx; } }
    else if (dev > mTapTail) mTapTail = dev;
    if (mTapT > 0.05f && mTapTail > 0.40f) mTapSt = 0;                  // sigue moviendose: no era un tap
    else if (mTapT >= 0.14f) {
      mTapSt = 0;
      if (mShake < 0.30f && mTapPk < 2.4f) {
        if (fabsf(mTapDX) > 0.15f) mDirX = mTapDX > 0 ? 1.0f : -1.0f;
        bool dbl = (mTapN == 1 && mTapAge > 0.12f && mTapAge < 0.7f);
        mTapN = dbl ? 0 : 1; mTapAge = 0;
        if (dbl) { if (mReady(MC_TAP2, now, 1000)) motionReact(15, mTapPk, now); }
        else if (mReady(MC_TAP, now, 220)) motionReact(14, mTapPk, now);
      }
    }
  }

  // ---- 4) sacudidas (energia sostenida + inversiones de sentido) ----
  bool gaitLock = mGaitOn;
  bool spinning = mSpinAcc > 1.0f;                                       // girando (se mantiene 0.4 s tras frenar)
  float thrSoft = gaitLock ? 1.10f : 0.85f;                              // correr no es sacudir
  if (mShake > 0.60f) mShakeHiT += dt; else mShakeHiT = 0;
  if (mRevN >= 3 && mShakeHiT > 0.12f && !spinning) {
    if (mShake > 2.0f) { if (mReady(MC_SHAKE2, now, 1800)) { mCool[MC_SHAKE] = now ? now : 1; motionReact(1, mShake, now); } }
    else if (mShake > thrSoft) { if (mReady(MC_SHAKE, now, 1500)) motionReact(0, mShake, now); }
  }

  // ---- 5) de cabeza / boca abajo / plano ----
  bool upside = mDown[1] < -0.65f && fabsf(mG[2]) < 0.6f;
  if (upside) {
    mUpsideT += dt;
    if (mUpsideT > 1.2f && !mWasUpside) { mWasUpside = true; if (mReady(MC_UPSIDE, now, 3000)) motionReact(4, 1, now); }
  } else {
    if (mWasUpside && mDown[1] > 0.5f) { mWasUpside = false; motionReact(5, 1, now); }
    mUpsideT = 0;
  }
  if (mG[2] < -0.8f && mShake < 0.3f) { mFaceDownT += dt; if (mFaceDownT > 1.0f && mReady(MC_FACEDOWN, now, 10000)) motionReact(6, 1, now); } else mFaceDownT = 0;
  if (mG[2] > 0.85f && mShake < 0.15f) {                                 // plano boca arriba, quieto mucho tiempo
    mFlatT += dt;
    if (mFlatT > 12.0f && mFlatSt < 1) { mFlatSt = 1; if (mReady(MC_BORED, now, 20000)) motionReact(23, 1, now); }
    if (mFlatT > 28.0f && mFlatSt < 2) { mFlatSt = 2; if (mReady(MC_SLEEPY, now, 20000)) motionReact(24, 1, now); }
  } else { mFlatT = 0; mFlatSt = 0; }

  // ---- 6) quietud / recogerlo / ponerlo en la mesa ----
  if (mStillT > 40.0f && !mStillFired) { mStillFired = true; if (mReady(MC_SLEEPY, now, 20000)) motionReact(24, 1, now); }
  if (mStillT < 1.0f) mStillFired = false;
  if (mShake > 0.05f) mMovT += dt; else if (mStillT > 0.3f) mMovT = 0;
  // recoger: venia quieto >0.8 s y ahora se inclina >25 grados o sube con aceleracion sostenida
  if (mStillT > 0.8f) { mRest[0] = mG[0]; mRest[1] = mG[1]; mRest[2] = mG[2]; mRestOK = 0.7f; }
  else if (mRestOK > 0) mRestOK -= dt;
  if (mag > 1.10f) mLiftT += dt; else mLiftT = 0;
  if (mRestOK > 0 && !mFalling && mLandT < 0 && !(mRevN >= 3 && mShake > 0.85f)) {
    float na = sqrtf(mG[0] * mG[0] + mG[1] * mG[1] + mG[2] * mG[2]), nb = sqrtf(mRest[0] * mRest[0] + mRest[1] * mRest[1] + mRest[2] * mRest[2]);
    float cs = (na > 0.3f && nb > 0.3f) ? (mG[0] * mRest[0] + mG[1] * mRest[1] + mG[2] * mRest[2]) / (na * nb) : 1.0f;
    if (((cs < 0.90f && mShake > 0.10f) || mLiftT > 0.10f) && mReady(MC_PICKUP, now, 3000)) { mRestOK = 0; motionReact(7, 1, now); }
  }
  // poner en mesa: venia en movimiento >0.8 s, golpecito suave de asentamiento y luego quietud con un eje alineado
  mBumpAge += dt; if (mBumpAge > 9.0f) mBumpAge = 9.0f;
  if (mMovT > 0.8f && dev > 0.18f && dev < 2.4f && !mFalling && mLandT < 0 && !mGaitOn && mSpinAcc < 1.0f) { mBumpAge = 0; mHeldAtBump = mMovT; }
  if (mStillT > 0.45f && mStillT - dt <= 0.45f) {
    bool aligned = fabsf(mG[0]) > 0.9f || fabsf(mG[1]) > 0.9f || fabsf(mG[2]) > 0.9f;
    if (mBumpAge < 2.0f && mHeldAtBump > 0.8f && aligned && mReady(MC_PLACE, now, 2500)) motionReact(16, 1, now);
    mHeldAtBump = 0;
  }

  // ---- 7) marcha / correr (oscilacion de la aceleracion vertical, 1.5-3.5 pasos/s) ----
  { float gn = sqrtf(mG[0] * mG[0] + mG[1] * mG[1] + mG[2] * mG[2]);
    float v = gn > 0.3f ? (dx * mG[0] + dy * mG[1] + dz * mG[2]) / gn : 0.0f;
    if (mFalling) mOscClear(mOscG); else mOscFeed(mOscG, v, dt, 0.07f, 0.12f, 0.62f);
    bool gait = mOscG.n >= 6 && mMagVar > 0.045f && mShake < 1.2f;
    if (gait) {
      if (mOscG.n > mGaitMax) mGaitMax = mOscG.n;
      float per = mOscG.per; bool run = per < 0.40f && mOscG.amp > 0.45f;
      uint32_t gap = (uint32_t)clampf(2000.0f * per, 600.0f, 1500.0f);
      if (!mGaitOn) { mGaitOn = true; mCool[MC_WALK] = 0; }
      if (mReady(MC_WALK, now, gap)) motionReact(run ? 11 : 10, per, now);
    } else if (mGaitOn && mOscG.n < 2) {                                 // se detuvo
      mGaitOn = false;
      if (mGaitMax >= 12 && mReady(MC_ARRIVE, now, 4000)) motionReact(22, 1, now);
      mGaitMax = 0;
    }
  }

  // ---- 8) mecer / arrullar (balanceo lento de la gravedad en cualquier eje, casi sin aceleracion neta) ----
  { float as[3] = {ax, ay, az};
    int bi = -1; uint8_t bn = 0;
    for (int i = 0; i < 3; i++) {
      if (mFalling) mOscClear(mOscR[i]); else mOscFeed(mOscR[i], as[i], dt, 0.05f, 0.25f, 1.6f);
      if (mOscR[i].n > bn && mOscR[i].amp >= 0.09f) { bn = mOscR[i].n; bi = i; }
    }
    bool rock = bi >= 0 && bn >= 4 && mMagVar < 0.10f && mShake < 0.45f && !mGaitOn;
    if (rock) {
      mRockLost = 0; mRockT += dt;
      float amp = mOscR[bi].amp, per = mOscR[bi].per;
      bool calm = amp < 0.35f && per > 1.1f;
      if (!mRockOn) { mRockOn = true; mRockT = 0; mRockSlept = false; mCool[MC_ROCK] = 0; }
      if (calm) {
        if (mReady(MC_ROCK, now, 5000)) motionReact(8, amp, now);
        if (mRockT > 6.0f && !mRockSlept) { mRockSlept = true; motionReact(26, 1, now); }   // se duerme en tus brazos
      } else if (mReady(MC_ROCK, now, 3500)) motionReact(9, amp, now);
    } else if (mRockOn) {
      mRockLost += dt;
      if (mRockLost > 1.0f) { mRockOn = false; mRockT = 0; mRockSlept = false; }
    }
  }

  // ---- 9) giro rapido en el plano de la pantalla (velocidad angular de la gravedad) y mareo posterior ----
  { float hxy = sqrtf(ax * ax + ay * ay);
    float dr = 0;
    if (hxy > 0.5f && fabsf(mag - 1.0f) < 0.35f) {
      float r = atan2f(-ax, ay);
      if (mSpinHave) {
        dr = r - mSpinPrev;
        float PI_ = TAU * 0.5f;
        if (dr > PI_) dr -= TAU; else if (dr < -PI_) dr += TAU;
        float rate = clampf(dr / dt, -40.0f, 40.0f);
        mSpinRate += (rate - mSpinRate) * (1.0f - expf(-dt / 0.07f));
      }
      mSpinPrev = r; mSpinHave = true;
    } else { mSpinHave = false; mSpinRate *= 0.85f; }
    float ar = fabsf(mSpinRate);
    if (ar > 3.0f) {
      int8_t sd = mSpinRate > 0 ? 1 : -1;
      if (mSpinDir != 0 && sd != mSpinDir) { mSpinAcc = 0; mSpinSum = 0; mSpinT = 0; }    // un vaiven (mecer/sacudir) no es un giro: solo cuenta avance en un sentido
      mSpinDir = sd;
      mSpinT += dt; mSpinQuiet = 0; mSpinSum += dr; mSpinAcc = fabsf(mSpinSum);
      if (mSpinT > 0.15f && mSpinAcc > 2.4f && !mSpinFired && mReady(MC_SPIN, now, 1500)) {
        mSpinFired = true; mDirX = mSpinRate > 0 ? 1.0f : -1.0f; motionReact(12, ar, now);
      }
    } else if (ar < 1.2f) {
      mSpinT = 0; mSpinQuiet += dt;
      if (mSpinQuiet > 0.4f) {
        if (mSpinAcc > 6.0f && mReady(MC_DIZZY, now, 3000)) motionReact(13, mSpinAcc, now);
        mSpinAcc = 0; mSpinSum = 0; mSpinFired = false; mSpinDir = 0;
      }
    }
  }

  // ---- 10) inclinado largo (30-83 grados, quieto 1.5 s) -> curioso ----
  { float ar = fabsf(mRoll);
    if (ar > 0.55f && ar < 1.45f && mShake < 0.08f && mG[2] < 0.5f && mG[2] > -0.5f) {
      mTiltT += dt;
      if (mTiltT > 1.5f && !mTiltFired) { mTiltFired = true; if (mReady(MC_TILT, now, 8000)) motionReact(25, 1, now); }
    } else { mTiltT = 0; if (ar < 0.3f) mTiltFired = false; }
  }

  // ---- 11) vibracion de motor (alternancia cuadro a cuadro estable) y cosquillas (temblor irregular) ----
  if (mAlt > 0.90f && mHF > 0.08f && mHF < 1.5f && mShake < 1.0f && !mFalling) mVibT += dt;
  else mVibT = mVibT > dt * 2 ? mVibT - dt * 2 : 0;
  if (mVibT > 0.6f && mReady(MC_VIB, now, 4000)) { mVibT = 0; motionReact(17, mHF, now); }
  bool tick = mShake > 0.22f && mShake < 0.80f && mAlt < 0.88f && !mFalling && !mRockOn
              && mOscG.n < 3 && mOscR[0].n < 3 && mOscR[1].n < 3 && mOscR[2].n < 3 && mSpinAcc < 1.0f && mVibT < 0.2f;
  if (tick) mTickleT += dt; else mTickleT = mTickleT > dt * 2 ? mTickleT - dt * 2 : 0;   // fuga lenta: el temblor real es irregular
  if (mTickleT > 1.4f && mReady(MC_TICKLE, now, 6000)) { mTickleT = 0; motionReact(18, mShake, now); }
}
