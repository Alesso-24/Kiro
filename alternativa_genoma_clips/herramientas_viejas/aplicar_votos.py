#!/usr/bin/env python
"""Aplica recortes decididos por votacion (por NOMBRE, idempotente) sobre firmware/Alloy_Eyes/src/datos/*.txt y recompila.
Se corre DESPUES de integrate.py + curate.py (o solo, si los datos ya estan curados).  python tools/aplicar_votos.py"""
import glob, os, subprocess, sys
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
D = os.path.join(ROOT, "firmware", "Alloy_Eyes", "src", "datos")
COMP = os.path.join(ROOT, "tools", "studio_compile.py")
CUT = {
    "pu": ["Puntito", "Lucero", "Mitad y mitad", "Anillo", "Inclinada brillo",
           "Canica",
           "Redonda media", "Rendija girada",
           "Rendija", "Rendija gruesa"],  # ronda 3-4                      # ronda 1: brillos, doble color, anillo, punto
    "sp": ["Lagrimita", "Ciclope Tierno", "Mochi", "Ay Tropezado", "Cangrejito", "Cometa y Luna",
           "Cometa", "Ojo y Rendija", "Oraculo", "Altito", "Pluma"],  # ronda 1
    "pal": ["Helado de fresa", "Cafe con leche", "Durazno cielo"],                                  # ronda 1
}
for kind, names in CUT.items():
    low = {n.lower() for n in names}
    cut = 0
    for f in sorted(glob.glob(os.path.join(D, f"*.{kind}.txt"))):
        out, changed = [], False
        for l in open(f, encoding="utf-8").read().split("\n"):
            s = l.strip()
            if s and not s.startswith(("//", ";")) and s.split("|")[0].strip().lower() in low:
                cut += 1
                changed = True
                continue
            out.append(l)
        if changed:
            open(f, "w", encoding="utf-8", newline="\n").write("\n".join(out))
            subprocess.run([sys.executable, COMP, kind, f, f[: -len(f"{kind}.txt") - 1] + ".h"], check=True, capture_output=True)
    print(f"{kind}: {cut} cortadas")

# ---- regla de color (2026-11-16): fuera los "blancos con un toque de color" (saturacion baja y muy claros): parecen un filtro.
# Se queda el blanco casi puro (sat < 0.06) y los colores claramente definidos. Regla: 0.06 <= sat < 0.215 y valor >= 0.89.
import colorsys
def washed(eye_hex):
    h = eye_hex.lstrip("#"); r, g, b = [int(h[i:i + 2], 16) / 255 for i in (0, 2, 4)]
    _, s_, v_ = colorsys.rgb_to_hsv(r, g, b)
    return 0.06 <= s_ < 0.215 and v_ >= 0.89
cut = 0
for f in sorted(glob.glob(os.path.join(D, "*.pal.txt"))):
    out, changed = [], False
    for l in open(f, encoding="utf-8").read().split("\n"):
        st = l.strip()
        if st and not st.startswith(("//", ";")):
            p = [x.strip() for x in st.split("|")]
            if len(p) >= 3 and washed(p[2]):
                cut += 1; changed = True
                print("  paleta lavada fuera:", p[0], p[2])
                continue
        out.append(l)
    if changed:
        open(f, "w", encoding="utf-8", newline="\n").write("\n".join(out))
        subprocess.run([sys.executable, COMP, "pal", f, f[: -len(".pal.txt")] + ".h"], check=True, capture_output=True)
print(f"pal (regla de color): {cut} cortadas")

# ---- regla 2 (2026-11-16): las pastel palidas que quedan (0.215 <= sat < 0.30, valor >= 0.85) tiran a BLANCO: el color de ojo se mezcla 80 % con
# blanco (queda blanco limpio) y se conserva el color de la pupila; asi no se ven como un filtro de color.
def to_white(eye_hex):
    h = eye_hex.lstrip("#"); c = [int(h[i:i + 2], 16) for i in (0, 2, 4)]
    r, g, b = [x / 255 for x in c]
    _, s_, v_ = colorsys.rgb_to_hsv(r, g, b)
    if not (0.215 <= s_ < 0.30 and v_ >= 0.85):
        return None
    return "#%02X%02X%02X" % tuple(int(round(x + (255 - x) * 0.8)) for x in c)
conv = 0
for f in sorted(glob.glob(os.path.join(D, "*.pal.txt"))):
    out, changed = [], False
    for l in open(f, encoding="utf-8").read().split("\n"):
        st = l.strip()
        if st and not st.startswith(("//", ";")):
            p = [x.strip() for x in st.split("|")]
            if len(p) >= 3:
                nh = to_white(p[2])
                if nh:
                    print("  tira a blanco:", p[0], p[2], "->", nh)
                    p[2] = nh; l = " | ".join(p); conv += 1; changed = True
        out.append(l)
    if changed:
        open(f, "w", encoding="utf-8", newline="\n").write("\n".join(out))
        subprocess.run([sys.executable, COMP, "pal", f, f[: -len(".pal.txt")] + ".h"], check=True, capture_output=True)
print(f"pal (tiran a blanco): {conv} convertidas")
