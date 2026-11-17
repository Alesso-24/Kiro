// Motion.h - capa de MOVIMIENTO (acelerometro). Sin hardware: el firmware (MPU) o el simulador la alimentan con motionFeed().
//
// CONVENCION de ejes (en g): x = derecha de la pantalla, y = ARRIBA de la pantalla, z = hacia el espectador.
// El sensor lee la aceleracion que se opone a la gravedad:  de pie = (0,+1,0) ; boca arriba en la mesa = (0,0,+1);
// boca abajo = (0,0,-1); de cabeza = (0,-1,0); pantalla girada a la derecha (lado derecho abajo) = (-1,0,0).
//
// Efectos CONTINUOS (siempre): gravedad -> los ojos/pupilas resbalan hacia "abajo" y la cara se ladea (mLeanX/mLeanY/mTiltFace).
// Eventos DISCRETOS: sacudida suave/fuerte, caida libre, golpe/impacto, de cabeza, boca abajo, recogerlo.
// M1 (diseno de reacciones) puede ampliar motionReact(); M3 prueba con senales sinteticas.
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
uint32_t mCool[8];                    // enfriamientos por tipo de evento
enum { MC_SHAKE, MC_FALL, MC_IMPACT, MC_UPSIDE, MC_PICKUP, MC_FACEDOWN, MC_OTHER1, MC_OTHER2 };

bool mReady(int k, uint32_t now, uint32_t ms) { if (now - mCool[k] < ms && mCool[k]) return false; mCool[k] = now ? now : 1; return true; }

// ---- reacciones: aqui se conecta cada gesto con clips/expresiones (M1 las afina) ----
void motionReact(uint8_t what, float strength, uint32_t now) {
  // what: 0 sacudida suave 1 sacudida fuerte 2 inicio de caida 3 impacto 4 de cabeza 5 vuelve a posicion normal
  //       6 boca abajo 7 recogido
  int c;
  switch (what) {
    case 0: evShake(now, false); c = clipFind("sacudida_suave"); if (c >= 0) clipPlay(c, now); break;
    case 1: evShake(now, true); c = clipFind("sacudida_fuerte"); if (c >= 0) clipPlay(c, now); break;
    case 2: play(now, 2, {X_ANXIOUS, 1500}, {0, 0}); c = clipFind("caida"); if (c >= 0) clipPlay(c, now); break;
    case 3: for (int e = 0; e < 2; e++) { eye[e].sx = 0.30f; eye[e].sy = -0.34f; }
            shakeAmp = 4; if (strength > 2.5f || mFalling) play(now, 3, {X_DIZZY, 1500}, {X_CURIOUS, 800}, {0, 0});
            c = clipFind("impacto"); if (c >= 0) clipPlay(c, now); break;
    case 4: play(now, 2, {X_SAD, 3200}, {0, 0}); c = clipFind("boca_abajo"); if (c >= 0) clipPlay(c, now); break;
    case 5: play(now, 2, {X_HAPPY, 1300}, {0, 0}); break;
    case 6: evSet(X_SLEEP, now, 0); break;
    case 7: play(now, 2, {X_CURIOUS, 1500}, {0, 0}); c = clipFind("asiente"); if (c >= 0) clipPlay(c, now); break;
  }
}

void motionFeed(float ax, float ay, float az, uint32_t now, float dt) {
  mActive = true;
  mAx = ax; mAy = ay; mAz = az;
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

  // --- caida libre e impacto ---
  if (mag < 0.38f) {
    mFallT += dt;
    if (mFallT > 0.09f && !mFalling) { mFalling = true; if (mReady(MC_FALL, now, 800)) motionReact(2, mag, now); }
  } else {
    if (mFalling && (dev > 1.6f || mag > 1.7f) && mReady(MC_IMPACT, now, 400)) motionReact(3, dev, now);
    mFalling = false; mFallT = 0;
  }
  if (!mFalling && dev > 2.4f && mag > 2.0f && mReady(MC_IMPACT, now, 400)) motionReact(3, dev, now);
  // --- sacudidas ---
  if (mShake > 2.0f) { if (mReady(MC_SHAKE, now, 1500)) motionReact(1, mShake, now); }
  else if (mShake > 0.85f) { if (mReady(MC_SHAKE, now, 1500)) motionReact(0, mShake, now); }
  // --- de cabeza ---
  bool upside = mDown[1] < -0.65f && fabsf(mG[2]) < 0.6f;
  if (upside) {
    mUpsideT += dt;
    if (mUpsideT > 1.2f && !mWasUpside) { mWasUpside = true; if (mReady(MC_UPSIDE, now, 3000)) motionReact(4, 1, now); }
  } else {
    if (mWasUpside && mDown[1] > 0.5f) { mWasUpside = false; motionReact(5, 1, now); }
    mUpsideT = 0;
  }
  // --- boca abajo / plano ---
  if (mG[2] < -0.8f) { mFaceDownT += dt; if (mFaceDownT > 1.0f && mReady(MC_FACEDOWN, now, 10000)) motionReact(6, 1, now); } else mFaceDownT = 0;
  if (mG[2] > 0.85f && mShake < 0.15f) mFlatT += dt;
  else { if (mFlatT > 1.5f && mG[2] < 0.5f && mReady(MC_PICKUP, now, 4000)) motionReact(7, 1, now); mFlatT = 0; }
  mStillT = mShake < 0.06f ? mStillT + dt : 0;
}
