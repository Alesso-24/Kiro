# Genera aud/EyesCore.h (INSTRUMENTADO, misma logica) a partir de src_orig/. Solo agrega contadores y comprobaciones.
import os, shutil, sys
src = os.path.join(os.path.dirname(os.path.abspath(__file__)), "src_orig")
dst = os.path.join(os.path.dirname(os.path.abspath(__file__)), "aud")
os.makedirs(dst, exist_ok=True)
for f in ("Data.h", "Motion.h"):
    shutil.copy(os.path.join(src, f), os.path.join(dst, f))
shutil.copytree(os.path.join(os.path.dirname(src), "sketch", "Alloy_Eyes", "src"), os.path.join(dst, "src"), dirs_exist_ok=True)
s = open(os.path.join(src, "EyesCore.h"), encoding="utf-8").read()
def rep(old, new):
    global s
    assert s.count(old) == 1, (s.count(old), old)
    s = s.replace(old, new)
rep("const int W = 240, H = 240;", "const int W = 240, H = 240;\nunsigned long long g_oob=0,g_plot=0,g_eyepx=0,g_eyedraw=0,g_ell=0,g_line=0,g_tri=0,g_trunc=0,g_spawnfail=0,g_emit=0,g_dirbad=0,g_chbad=0,g_peakparts=0,g_hasP=0;")
rep("  if (ai <= 0) return;\n  uint16_t* p = &fb[y * W + x];", "  if (ai <= 0) return;\n  g_plot++;\n  if ((unsigned)x >= (unsigned)W || (unsigned)y >= (unsigned)H) { g_oob++; return; }\n  uint16_t* p = &fb[y * W + x];")
rep("      float dx = x - cx, f = dx * dx * ix + dy2 - 1.0f;", "      g_ell++;\n      float dx = x - cx, f = dx * dx * ix + dy2 - 1.0f;")
rep("      float t = (wx * vx + wy * vy) * il2;", "      g_line++;\n      float t = (wx * vx + wy * vy) * il2;")
rep("      float m = 1e9f;", "      g_tri++;\n      float m = 1e9f;")
rep("      float u = xr * irx, v = yr * iry, u2 = u * u, v2 = v * v;", "      g_eyepx++;\n      float u = xr * irx, v = yr * iry, u2 = u * u, v2 = v * v;")
rep("      if (ai <= 0) continue;\n      row[x] = ai >= 32 ? col : blend565(row[x], col, ai);", "      if (ai <= 0) continue;\n      g_eyedraw++;\n      if (x < 0 || x >= W) { g_oob++; continue; }\n      if ((x == xs && xs > x0) || (x == xe && xe < x1)) g_trunc++;\n      row[x] = ai >= 32 ? col : blend565(row[x], col, ai);")
rep("  bool hasP = D.prx > 0.5f && D.pry > 0.5f;", "  bool hasP = D.prx > 0.5f && D.pry > 0.5f; if (hasP) g_hasP++;")
rep("      parts[i] = {x, y, vx, vy, life, life, size, rot, spin, type, col, 1};\n      return;\n    }", "      parts[i] = {x, y, vx, vy, life, life, size, rot, spin, type, col, 1};\n      return;\n    }\n  g_spawnfail++;")
rep("void clipEmit(uint8_t ev, bool mir) {", "void clipEmit(uint8_t ev, bool mir) {\n  g_emit++;")
rep("  clipPlay(pick, now, frand() > 0, 0.9f + 0.25f * frand01());", "  if (CLIPS[pick].tag >= 6) g_dirbad++;\n  clipPlay(pick, now, frand() > 0, 0.9f + 0.25f * frand01());")
rep("  const Key* K = c.keys; int n = c.n;", "  const Key* K = c.keys; int n = c.n;\n  if (n < 1 || ch < 0 || ch >= 12) g_chbad++;")
rep("uint16_t* fb = nullptr;", "uint16_t* fb = nullptr; float g_extk=1.03f, g_extm=0.5f;")
rep("float ext = 1.03f + 0.5f * (", "float ext = g_extk + g_extm * (")
open(os.path.join(dst, "EyesCore.h"), "w", encoding="utf-8").write(s)
print("ok")
