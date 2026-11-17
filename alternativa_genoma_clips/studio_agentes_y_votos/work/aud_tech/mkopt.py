import os, shutil
here = os.path.dirname(os.path.abspath(__file__))
src = os.path.join(here, "src_orig")
data = os.path.join(here, "sketch", "Alloy_Eyes", "src")
for d in ("base", "opt"):
    os.makedirs(d, exist_ok=True)
    for f in ("Data.h", "Motion.h", "EyesCore.h"):
        shutil.copy(os.path.join(src, f), os.path.join(here, d, f))
    shutil.copytree(data, os.path.join(here, d, "src"), dirs_exist_ok=True)
p = os.path.join(here, "opt", "EyesCore.h")
s = open(p, encoding="utf-8", newline="").read().replace(chr(13)+chr(10), chr(10))
def rep(old, new):
    global s
    assert s.count(old) == 1, (s.count(old), old[:60])
    s = s.replace(old, new)
# ---- OPT-C1: ellipseAA con tramo por renglon
rep("""  for (int y = y0; y <= y1; y++) {
    float dy = y - cy, dy2 = dy * dy * iy;
    for (int x = x0; x <= x1; x++) {
      float dx = x - cx, f = dx * dx * ix + dy2 - 1.0f;""",
"""  float feE = 0.5f / sc;
  for (int y = y0; y <= y1; y++) {
    float dy = y - cy, dy2 = dy * dy * iy;
    float rem = 1.0f + feE - dy2;
    if (rem <= 0) continue;
    float hw = rx * sqrtf(rem);
    int xs = max(x0, (int)floorf(cx - hw) - 1), xe = min(x1, (int)ceilf(cx + hw) + 1);
    for (int x = xs; x <= xe; x++) {
      float dx = x - cx, f = dx * dx * ix + dy2 - 1.0f;""")
# ---- OPT-C2: triAA con tramo por renglon
rep("""  for (int y = ya; y <= yb; y++)
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
}""",
"""  float Q[3], iQ[3], th = 0.5f + grow;
  for (int i = 0; i < 3; i++) { Q[i] = ey[i] * il[i]; iQ[i] = (Q[i] > 1e-6f || Q[i] < -1e-6f) ? 1.0f / Q[i] : 0.0f; }
  for (int y = ya; y <= yb; y++) {
    float xlo = -1e8f, xhi = 1e8f; bool empty = false;
    for (int i = 0; i < 3; i++) {
      float P = (ex[i] * (y - py[i]) + ey[i] * px[i]) * il[i] + th;
      if (Q[i] > 1e-6f) { float v = P * iQ[i]; if (v < xhi) xhi = v; }
      else if (Q[i] < -1e-6f) { float v = P * iQ[i]; if (v > xlo) xlo = v; }
      else if (P <= 0) empty = true;
    }
    if (empty || xlo > xhi) continue;
    int xs = max(xa, (int)floorf(xlo) - 1), xe = min(xb, (int)ceilf(xhi) + 1);
    for (int x = xs; x <= xe; x++) {
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
}""")
# ---- OPT-B: renderEye: parpados primero + compuerta de pupila
rep("""  float psc = 0.5f * sqrtf(D.prx * D.pry), pfe = 0.5f / (psc + 1e-3f);""",
"""  float psc = 0.5f * sqrtf(D.prx * D.pry), pfe = 0.5f / (psc + 1e-3f);
  float pR2 = 1e18f;                  // radio (al cuadrado) que contiene seguro a la pupila: fuera de el no se evalua
  if (hasP) {
    float Aq = 1.0f - D.pc1, Bq = 1.0f + D.pc1;
    if (Aq > 0.1f && Bq > 0.1f) {
      float lim = 1.0f + pfe;
      float pam = sqrtf((lim + D.pc2 * D.pc2 / (4 * Bq)) / Aq) * (D.prx + 1e-3f);
      float pbm = (fabsf(D.pc2) + sqrtf(D.pc2 * D.pc2 + 4 * Bq * lim)) / (2 * Bq) * (D.pry + 1e-3f);
      float R = sqrtf(pam * pam + pbm * pbm) + 1.5f;
      pR2 = R * R;
    }
  }""")
rep("""    float pdy = y - D.pcy;
    for (int x = xs; x <= xe; x++, xr += ca, yr -= sa, topY += k) {
      float u = xr * irx, v = yr * iry, u2 = u * u, v2 = v * v;
      float f = u2 + v2 - 1 - c1 * (u2 - v2) - c2x * u * v - c3 * u - c4 * v - c5 * u * (u2 - 3 * v2);
      if (f >= fe) continue;
      float a = f <= -fe ? 1.0f : 0.5f - f * sc;
      float lt = y - topY + 0.5f;
      if (lt <= 0) continue;
      if (lt < 1) a *= lt;
      float lb = botY - y + 0.5f;
      if (lb <= 0) continue;
      if (lb < 1) a *= lb;""",
"""    float pdy = y - D.pcy, pdy2 = pdy * pdy;
    float lb = botY - y + 0.5f;
    if (lb <= 0) continue;
    for (int x = xs; x <= xe; x++, xr += ca, yr -= sa, topY += k) {
      float lt = y - topY + 0.5f;
      if (lt <= 0) continue;
      float u = xr * irx, v = yr * iry, u2 = u * u, v2 = v * v;
      float f = u2 + v2 - 1 - c1 * (u2 - v2) - c2x * u * v - c3 * u - c4 * v - c5 * u * (u2 - 3 * v2);
      if (f >= fe) continue;
      float a = f <= -fe ? 1.0f : 0.5f - f * sc;
      if (lt < 1) a *= lt;
      if (lb < 1) a *= lb;""")
rep("""      if (hasP) {
        float pdx = x - D.pcx;""", """      if (hasP && (x - D.pcx) * (x - D.pcx) + pdy2 <= pR2) {
        float pdx = x - D.pcx;""")
open(p, "w", encoding="utf-8", newline="").write(s)
print("opt listo")
