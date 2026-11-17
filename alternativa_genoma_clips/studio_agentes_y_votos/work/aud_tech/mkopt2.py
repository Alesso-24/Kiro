import os, shutil
here = os.path.dirname(os.path.abspath(__file__))
for f in os.listdir(os.path.join(here, "opt")):
    sp = os.path.join(here, "opt", f)
    if os.path.isdir(sp): shutil.copytree(sp, os.path.join(here, "opt2", f), dirs_exist_ok=True)
    else: shutil.copy(sp, os.path.join(here, "opt2", f))
p = os.path.join(here, "opt2", "EyesCore.h")
s = open(p, encoding="utf-8", newline="").read()
def rep(old, new, cnt=1):
    global s
    assert s.count(old) == cnt, (s.count(old), old[:70])
    s = s.replace(old, new)
# 1) plot() actualiza las filas sucias con la fila realmente escrita
rep("""  uint16_t* p = &fb[y * W + x];
  *p = ai >= 32 ? col : blend565(*p, col, ai);
}""", """  uint16_t* p = &fb[y * W + x];
  *p = ai >= 32 ? col : blend565(*p, col, ai);
  if (y < dirtyY0) dirtyY0 = y;
  if (y > dirtyY1) dirtyY1 = y;
}""")
# 2) las primitivas ya no marcan la caja completa
rep("  markRows(y0, y1);\n  float ix = 1.0f / (rx * rx)", "  float ix = 1.0f / (rx * rx)")
rep("  markRows(ya, yb);\n  for (int y = ya; y <= yb; y++) {\n    float wy = y - y0;", "  for (int y = ya; y <= yb; y++) {\n    float wy = y - y0;")
rep("  markRows(ya, yb);\n  float Q[3]", "  float Q[3]")
# 3) renderEye: marca por renglon si dibujo algo
rep("  markRows(y0, y1);\n  float irx = 1.0f / rx", "  float irx = 1.0f / rx")
rep("      row[x] = ai >= 32 ? col : blend565(row[x], col, ai);\n    }\n  }\n}", "      row[x] = ai >= 32 ? col : blend565(row[x], col, ai);\n      drew = true;\n    }\n    if (drew) { if (y < dirtyY0) dirtyY0 = y; if (y > dirtyY1) dirtyY1 = y; }\n  }\n}")
rep("    float lb = botY - y + 0.5f;\n    if (lb <= 0) continue;\n    for (int x = xs;", "    float lb = botY - y + 0.5f;\n    if (lb <= 0) continue;\n    bool drew = false;\n    for (int x = xs;")
open(p, "w", encoding="utf-8", newline="").write(s)
print("opt2 listo")
