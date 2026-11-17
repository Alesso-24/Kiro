// Simulador de PC: compila EXACTAMENTE el mismo EyesCore.h que corre en la XIAO y lo expone como DLL.
#include <cmath>
#include "../firmware/Alloy_Eyes/EyesCore.h"
static uint16_t buf[240 * 240];
#define API extern "C" __declspec(dllexport)
API void sim_init(uint32_t seed) { fb = buf; coreInit(0, seed); }
API void sim_frame(uint32_t now, float dt) { coreFrame(now, dt); }
API uint16_t* sim_buf() { return buf; }
API int sim_expr() { return expr; }
API int sim_theme() { return awakeTheme; }
API int sim_asleep() { return deepSleep ? 2 : (isAsleep() ? 1 : 0); }
API int sim_usb() { return usb ? 1 : 0; }
API void sim_set_expr(int x, uint32_t now, uint32_t dur) { evSet((uint8_t)x, now, (uint16_t)dur); }
API void sim_next_expr(uint32_t now) { evNextExpr(now); }
API void sim_next_theme(uint32_t now) { evTheme(now); }
API void sim_poke(uint32_t now) { evPoke(now); }
API void sim_wink(uint32_t now) { wake(now); evWink(); }
API void sim_tap(uint32_t now) { evTap(now); }
API void sim_pet(uint32_t now, int on) { evPet(now, on != 0); }
API void sim_play(uint32_t now) { evPlay(now); }
API void sim_shake(uint32_t now, int strong) { evShake(now, strong != 0); }
API void sim_set_usb(uint32_t now, int on) { evUsb(now, on != 0); }
API void sim_loud(uint32_t now) { evLoud(now); }
API void sim_cold(uint32_t now, int on) { evCold(now, on != 0); }
API void sim_thumbs(uint32_t now) { evThumbs(now); }
API void sim_flick(uint32_t now) { evFlick(now); }
API void sim_look(int pal, int style) { G.pal = pal % NPALS; G.pupil = style % NPUPILS; }
API void sim_seed(uint32_t seed) { genomeFromSeed(seed); }
API void sim_creature(uint32_t now, uint32_t seed) { evNewCreature(now, seed); }
API void sim_genome(int* o) { o[0] = G.pal; o[1] = G.species; o[2] = G.pupil; o[3] = G.toneMode; }
API int sim_npal() { return NPALS; }
API int sim_nspecies() { return NSPECIES; }
API int sim_npupils() { return NPUPILS; }
API int sim_nclips() { return NCLIPS; }
API const char* sim_clip_name(int i) { return CLIPS[i].name; }
API int sim_clip_tag(int i) { return CLIPS[i].tag; }
API int sim_clip_dur(int i) { return CLIPS[i].dur; }
API void sim_clip_play(int i, uint32_t now, int mirror, float speed) { clipPlay(i, now, mirror != 0, speed); }
API void sim_motion(float ax, float ay, float az, uint32_t now, float dt) { motionFeed(ax, ay, az, now, dt); }
API void sim_motion_off() { mActive = false; }
API void sim_traits(float* o) { for (int i = 0; i < 6; i++) o[i] = G.trait[i]; }
API const char* sim_pal_name(int i) { return PALS[i].name; }
API const char* sim_species_name(int i) { return SPECIES[i].name; }
API const char* sim_pupil_name(int i) { return PUPILS[i].name; }
API int sim_nexpr() { return NEXPR; }
API void sim_batfull(uint32_t now) { evBatFull(now); }
API void sim_batlow(uint32_t now, int on) { evBatLow(now, on != 0); }
API void sim_gaze(float x, float y, uint32_t now, uint32_t hold) { setGaze(x, y, now, hold); }
API void sim_pupil(float l, float r) { puOv[0] = l; puOv[1] = r; }
API void sim_stats(float* out) { out[0] = energy; out[1] = fun; out[2] = affection; }
API void sim_timescale(float s) { timeScale = s; }
API const char* sim_name(int x) { return XNAME[x]; }
API int sim_dirty0() { return dirtyY0; }
API int sim_dirty1() { return dirtyY1; }
API int sim_ok() {
  for (int i = 0; i < NP; i++) if (!std::isfinite(cur[i]) || !std::isfinite(vel[i])) return 0;
  for (int e = 0; e < 2; e++) {
    const Eye& E = eye[e];
    if (!std::isfinite(E.ox) || !std::isfinite(E.oy) || !std::isfinite(E.sx) || !std::isfinite(E.sy) || !std::isfinite(E.px) || !std::isfinite(E.py)) return 0;
    if (fabsf(E.ox) > 60 || fabsf(E.oy) > 60 || fabsf(E.sx) > 1.0f || fabsf(E.sy) > 1.0f) return 0;
  }
  for (int c = 0; c < NCH; c++) if (!std::isfinite(clipAdd[c])) return 0;
  if (dirtyY0 < 0 || dirtyY1 > 239) return 0;
  return 1;
}
API int sim_nparts() { int n = 0; for (int i = 0; i < NPART; i++) n += parts[i].on; return n; }
API void sim_set_species(int i) { G.species = i % NSPECIES; }

// ---- exports nuevos (M2, solo lectura; no modifican el firmware) ----
// sim_motion_state(o): o[0..2]=mG (gravedad filtrada, g)  o[3]=mShake  o[4..5]=mDown (x der, y abajo)  o[6]=mRoll (rad)
//   o[7]=mFalling  o[8]=mWasUpside  o[9]=mActive  o[10..12]=ultima lectura mAx,mAy,mAz  o[13]=mUpsideT  o[14]=mFlatT  o[15]=mFaceDownT
API void sim_motion_state(float* o) {
  o[0] = mG[0]; o[1] = mG[1]; o[2] = mG[2]; o[3] = mShake; o[4] = mDown[0]; o[5] = mDown[1]; o[6] = mRoll;
  o[7] = mFalling ? 1.f : 0.f; o[8] = mWasUpside ? 1.f : 0.f; o[9] = mActive ? 1.f : 0.f;
  o[10] = mAx; o[11] = mAy; o[12] = mAz; o[13] = mUpsideT; o[14] = mFlatT; o[15] = mFaceDownT;
}
// sim_motion_cool(k): marca de tiempo (ms) del ultimo evento del tipo k (MC_SHAKE..MC_FACEDOWN); cambia al dispararse => deteccion de eventos
API uint32_t sim_motion_cool(int k) { return (k >= 0 && k < 8) ? mCool[k] : 0; }
// sim_clip_active(ids): escribe en ids[0..2] el id de clip de cada ranura activa (-1 si libre); devuelve cuantas hay activas
API int sim_clip_active(int* ids) { int n = 0; for (int i = 0; i < 3; i++) { ids[i] = slots[i].on ? slots[i].id : -1; n += slots[i].on; } return n; }
