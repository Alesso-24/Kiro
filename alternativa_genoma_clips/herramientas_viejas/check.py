#!/usr/bin/env python
"""Validador del estudio.  python tools/check.py <workdir> [--creatures N] [--seeds N] [--quick]

Compila el simulador del sandbox (con tus datos) y prueba:
  * 0 NaN / desbordes / filas fuera de pantalla: N semillas x todas las expresiones, todos los clips (normal y espejo),
    y 600 cuadros de movimiento aleatorio
  * genera hojas de contacto en <workdir>/out/ : criaturas.png, paletas.png, especies.png, pupilas.png, clips_tagN.png
Sale con codigo 1 si hay errores graves."""
import ctypes, importlib.util, os, sys, time
import numpy as np
from PIL import Image, ImageDraw

work = os.path.abspath(sys.argv[1])
args = sys.argv[2:]


def opt(name, default):
    return int(args[args.index(name) + 1]) if name in args else default


NCRE = opt("--creatures", 48)
NSEEDS = opt("--seeds", 120)

spec = importlib.util.spec_from_file_location("viewer", os.path.join(work, "sim", "viewer.py"))
V = importlib.util.module_from_spec(spec)
spec.loader.exec_module(V)
d = V.load()
u32 = ctypes.c_uint32
d.sim_creature.argtypes = [u32, u32]
d.sim_seed.argtypes = [u32]
d.sim_clip_play.argtypes = [ctypes.c_int, u32, ctypes.c_int, ctypes.c_float]
d.sim_clip_name.restype = ctypes.c_char_p
d.sim_pal_name.restype = ctypes.c_char_p
d.sim_species_name.restype = ctypes.c_char_p
d.sim_pupil_name.restype = ctypes.c_char_p
d.sim_auto.argtypes = [ctypes.c_int]
d.sim_motion.argtypes = [ctypes.c_float] * 3 + [u32, ctypes.c_float]
d.sim_set_species.argtypes = [ctypes.c_int]
d.sim_look.argtypes = [ctypes.c_int, ctypes.c_int]
npal, nsp, npu, ncl = d.sim_npal(), d.sim_nspecies(), d.sim_npupils(), d.sim_nclips()
print(f"datos: {npal} paletas, {nsp} especies, {npu} pupilas, {ncl} clips  (combinaciones base: {npal*nsp*npu:,})")
out = os.path.join(work, "out")
os.makedirs(out, exist_ok=True)
errors = []


def run(t, n, dt=1 / 60):
    for _ in range(n):
        t += int(dt * 1000 + 0.5)
        d.sim_frame(t, dt)
        if not d.sim_ok():
            return t, False
    return t, True


def img(scale=1):
    return V.round_img(d, scale)


# ---- 1) semillas x expresiones ----
t0 = time.time()
frames = 0
for seed in range(NSEEDS):
    d.sim_init(seed * 7919 + 1)
    d.sim_seed(seed * 7919 + 1)
    t, ok = run(0, 30)
    for x in range(1, d.sim_nexpr()):
        d.sim_set_expr(x, t, 0)
        t, ok = run(t, 40)
        frames += 40
        if not ok:
            errors.append(f"estado invalido: seed={seed} expr={x}")
            break
print(f"[1] {NSEEDS} semillas x {d.sim_nexpr()-1} expresiones: {frames} cuadros en {time.time()-t0:.1f}s (host)")

# ---- 2) todos los clips, normal y espejo ----
for i in range(ncl):
    for mir in (0, 1):
        d.sim_init(i * 13 + 5)
        t, ok = run(0, 90)
        d.sim_clip_play(i, t, mir, 1.0)
        t, ok = run(t, int(max(10, d.sim_clip_dur(i) / 16.7) + 20))
        if not ok:
            errors.append(f"clip invalido: {d.sim_clip_name(i).decode()} mirror={mir}")
print(f"[2] {ncl} clips probados (normal+espejo)")

# ---- 3) movimiento sintetico aleatorio ----
d.sim_init(3)
t, ok = run(0, 60)
rng = np.random.default_rng(1)
mot_ok = True
for k in range(600):
    t += 16
    d.sim_motion(float(rng.normal(0, 1.2)), float(rng.normal(1, 1.2)), float(rng.normal(0, 1.2)), t, 1 / 60)
    d.sim_frame(t, 1 / 60)
    if not d.sim_ok():
        errors.append("movimiento aleatorio rompe el estado")
        mot_ok = False
        break
d.sim_motion_off()
print("[3] movimiento aleatorio (600 cuadros):", "ok" if mot_ok else "FALLO")

# ---- 4) hoja de criaturas ----
cols = 8
rows = (NCRE + cols - 1) // cols
sheet = Image.new("RGB", (cols * 120, rows * 120), (30, 30, 36))
for i in range(NCRE):
    seed = 1000 + i * 977
    d.sim_init(seed)
    d.sim_auto(0)
    d.sim_seed(seed)
    d.sim_set_expr(1, 0, 0)
    t, ok = run(0, 150)
    sheet.paste(img(1).resize((120, 120), Image.LANCZOS), ((i % cols) * 120, (i // cols) * 120))
sheet.save(os.path.join(out, "criaturas.png"))
print("[4] out/criaturas.png")


# ---- 5) hojas de datos ----
def grid(n, apply, fname, label, cols=10, sz=96):
    rows = (n + cols - 1) // cols
    sh = Image.new("RGB", (cols * sz, rows * (sz + 12)), (30, 30, 36))
    dr = ImageDraw.Draw(sh)
    for i in range(n):
        d.sim_init(4242)
        d.sim_auto(0)
        d.sim_seed(4242)
        apply(i)
        d.sim_set_expr(1, 0, 0)
        run(0, 130)
        x, y = (i % cols) * sz, (i // cols) * (sz + 12)
        sh.paste(img(1).resize((sz, sz), Image.LANCZOS), (x, y))
        dr.text((x + 2, y + sz), label(i)[:16], fill=(210, 210, 210))
    sh.save(os.path.join(out, fname))
    print("   out/" + fname)


print("[5] hojas de datos:")
grid(npal, lambda i: d.sim_look(i, 5), "paletas.png", lambda i: f"{i} {d.sim_pal_name(i).decode()}")
grid(nsp, lambda i: d.sim_set_species(i), "especies.png", lambda i: f"{i} {d.sim_species_name(i).decode()}")
grid(npu, lambda i: d.sim_look(0, i), "pupilas.png", lambda i: f"{i} {d.sim_pupil_name(i).decode()}")

# ---- 6) tiras de clips ----
by = {}
for i in range(ncl):
    by.setdefault(int(d.sim_clip_tag(i)), []).append(i)
for tag, ids in by.items():
    nf = 12
    sh = Image.new("RGB", (nf * 80, len(ids) * 92), (30, 30, 36))
    dr = ImageDraw.Draw(sh)
    for r, i in enumerate(ids):
        d.sim_init(77)
        d.sim_auto(0)
        d.sim_seed(77)
        t, ok = run(0, 130)
        dur = max(300, d.sim_clip_dur(i)) + 100
        d.sim_clip_play(i, t, 0, 1.0)
        step = dur / nf
        for f in range(nf):
            t, ok = run(t, max(1, int(step / 16.7)))
            sh.paste(img(1).resize((80, 80), Image.LANCZOS), (f * 80, r * 92))
        dr.text((2, r * 92 + 80), f"{i} {d.sim_clip_name(i).decode()} ({d.sim_clip_dur(i)} ms)", fill=(210, 210, 210))
    sh.save(os.path.join(out, f"clips_tag{tag}.png"))
print(f"[6] tiras de clips por tag {sorted(by)} en out/")

if errors:
    print("\nERRORES GRAVES:")
    for e in errors[:40]:
        print("  -", e)
    sys.exit(1)
print("\nOK: sin errores graves.")
