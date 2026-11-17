#!/usr/bin/env python
"""Banco de pruebas de MOVIMIENTO de Alloy (M3).
    python test_motion.py [--motion RUTA/Motion.h] [--seeds 20] [--out reporte_auto.md]

* Copia Motion.h (por defecto el del proyecto) al sandbox INYECTANDO un contador en motionReact (no se toca el original),
  compila la DLL (sim/eyes_sim.cpp del sandbox, con exports extra: sim_slot_clip, sim_ev_count, sim_mstate) y
  corre ~21 casos sinteticos a 60 Hz, N semillas de ruido cada uno.
* Eventos = llamadas a motionReact(what): 0 sacudida suave, 1 fuerte, 2 inicio caida, 3 impacto, 4 de cabeza,
  5 vuelve normal, 6 boca abajo, 7 recogido.  Tambien registra cambios de sim_expr y clips activos (slots).
* Ejes: x der, y ARRIBA, z hacia el espectador (de pie (0,1,0), plano (0,0,1), de cabeza (0,-1,0)).
* Veredicto por caso: PASA si en >= 95% de las semillas salen todos los eventos "debe" y ninguno de "prohibe".
* Cada caso tiene 1.5 s de lead-in en la pose inicial (el filtro converge); solo se cuentan eventos posteriores.
"""
import ctypes, hashlib, os, subprocess, sys, argparse
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
PROJ = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
ap = argparse.ArgumentParser()
ap.add_argument("--motion", default=os.path.join(PROJ, "firmware", "Alloy_Eyes", "Motion.h"))
ap.add_argument("--seeds", type=int, default=20)
ap.add_argument("--out", default=os.path.join(HERE, "reporte_auto.md"))
args, _ = ap.parse_known_args()

EV = ["shake_suave", "shake_fuerte", "caida", "impacto", "de_cabeza", "vuelve_normal", "boca_abajo", "recogido"]
FS = 60.0
DT = 1 / FS
PRE = 1.5


def build():
    fw = os.path.join(HERE, "firmware", "Alloy_Eyes")
    src = open(args.motion, encoding="utf-8").read()
    pat = "void motionReact(uint8_t what, float strength, uint32_t now) {"
    assert pat in src, "firma de motionReact no encontrada: ajusta el parche de instrumentacion"
    src = src.replace(pat, "int mEvLog[16]; float mEvStr[16];\n" + pat +
                      "\n  if (what < 16) { mEvLog[what]++; mEvStr[what] = strength; }", 1)
    if "void motionReset()" in src: src = "#define M3_HAS_RESET 1" + chr(10) + src
    open(os.path.join(fw, "Motion.h"), "w", encoding="utf-8").write(src)
    h = hashlib.md5((src + open(os.path.join(HERE, "sim", "eyes_sim.cpp"), encoding="utf-8").read()
                     + open(os.path.join(fw, "EyesCore.h"), encoding="utf-8").read()).encode()).hexdigest()[:8]
    bdir = os.path.join(HERE, "sim", "build")
    os.makedirs(bdir, exist_ok=True)
    dll = os.path.join(bdir, f"m3_{h}.dll")
    if not os.path.exists(dll):
        r = subprocess.run(["g++", "-O2", "-shared", "-static-libgcc", "-static-libstdc++", "-o", dll,
                            os.path.join(HERE, "sim", "eyes_sim.cpp")], capture_output=True, text=True)
        if r.returncode:
            print(r.stderr)
            sys.exit(1)
    d = ctypes.CDLL(dll)
    u32, f32 = ctypes.c_uint32, ctypes.c_float
    d.sim_init.argtypes = [u32]
    d.sim_frame.argtypes = [u32, f32]
    d.sim_motion.argtypes = [f32, f32, f32, u32, f32]
    d.sim_ev_count.argtypes = [ctypes.c_int]
    d.sim_ev_strength.argtypes = [ctypes.c_int]
    d.sim_ev_strength.restype = f32
    d.sim_slot_clip.argtypes = [ctypes.c_int]
    d.sim_clip_name.argtypes = [ctypes.c_int]
    d.sim_clip_name.restype = ctypes.c_char_p
    d.sim_mstate.argtypes = [ctypes.POINTER(f32)]
    return d


# ------------------------------------------------------------------ generadores (N,3) en g a 60 Hz
def tvec(sec):
    return np.arange(int(sec * FS)) * DT


def grav(th, pitch=0.0):
    """th = balanceo (+ = lado derecho abajo)."""
    th = np.asarray(th, float)
    pitch = np.broadcast_to(np.asarray(pitch, float), th.shape)
    return np.stack([-np.sin(th) * np.cos(pitch), np.cos(th) * np.cos(pitch), np.sin(pitch)], axis=-1)


def ramp(t, t0, t1):
    return np.clip((t - t0) / (t1 - t0), 0, 1)


def smooth(x):
    return x * x * (3 - 2 * x)


def noise(rng, n, sig=None):
    return rng.normal(0, rng.uniform(0.02, 0.05) if sig is None else sig, (n, 3))


def unit(rng):
    v = rng.normal(size=3)
    return v / np.linalg.norm(v)


def up(n):
    return np.tile([0, 1, 0.0], (n, 1))


def flat(n):
    return np.tile([0, 0, 1.0], (n, 1))


def c_still_up(rng, dur):
    n = len(tvec(dur)); return up(n) + noise(rng, n)


def c_still_flat(rng, dur):
    n = len(tvec(dur)); return flat(n) + noise(rng, n)


def c_slow_tilt(rng, dur):
    t = tvec(dur); th = np.deg2rad(60) * smooth(ramp(t, PRE, dur - 0.5))
    return grav(th) + noise(rng, len(t))


def c_rock(freq):
    def f(rng, dur):
        t = tvec(dur); amp = np.deg2rad(rng.uniform(15, 20)); ph = rng.uniform(0, 6.28)
        e = ramp(t, PRE, PRE + 0.5)
        th = amp * np.sin(2 * np.pi * freq * t + ph) * e
        thdd = -amp * (2 * np.pi * freq) ** 2 * np.sin(2 * np.pi * freq * t + ph) * e
        s = grav(th); s[:, 0] += 0.08 * thdd / 9.81      # aceleracion tangencial (radio ~8 cm)
        return s + noise(rng, len(t))
    return f


def c_shake(freq, amp):
    def f(rng, dur):
        t = tvec(dur); d = unit(rng); d[1] += 0.3; d /= np.linalg.norm(d)
        env = ramp(t, PRE, PRE + 0.15) * (1 - ramp(t, dur - 0.65, dur - 0.5))
        osc = amp * np.sin(2 * np.pi * freq * t + rng.uniform(0, 6.28)) * env
        th = np.deg2rad(rng.uniform(5, 15)) * np.sin(np.pi * freq * t) * env     # la mano tambien inclina
        return grav(th) + osc[:, None] * d[None, :] + noise(rng, len(t))
    return f


def c_freefall(rng, dur):
    n = len(tvec(dur)); s = up(n)
    ff = rng.uniform(0.30, 0.60); i0 = int((PRE + 0.2) * FS); i1 = i0 + int(ff * FS)
    s[i0:i1] = unit(rng)[None, :] * rng.uniform(0.0, 0.18)
    peak = rng.uniform(3, 6); k = int(rng.integers(2, 4))
    d = np.array([0, 1.0, 0]) + 0.3 * rng.normal(size=3); d /= np.linalg.norm(d)
    for j in range(k):
        s[i1 + j] = d * peak * (1 - 0.35 * j)
    i2 = i1 + k
    s[i2:i2 + 6] += rng.normal(size=(6, 3)) * 0.8 * np.exp(-np.arange(6) / 2)[:, None]
    return s + noise(rng, n)


def c_tap(extra, w=2):
    def f(rng, dur):
        n = len(tvec(dur)); s = up(n); i = int((PRE + 0.5) * FS)
        d = unit(rng); d[1] = abs(d[1]) + 1; d /= np.linalg.norm(d)
        for j in range(w):
            s[i + j] += d * extra * (1 - 0.4 * j)
        return s + noise(rng, n)
    return f


def c_inverted(rng, dur):
    t = tvec(dur); th = np.pi * smooth(ramp(t, PRE + 0.3, PRE + 0.8))
    s = grav(th); s[:, 0] += 0.4 * np.sin(np.pi * ramp(t, PRE + 0.3, PRE + 0.8)) * rng.choice([-1, 1])
    return s + noise(rng, len(t))


def c_inverted_return(rng, dur):
    t = tvec(dur)
    th = np.pi * smooth(ramp(t, PRE + 0.3, PRE + 0.8)) * (1 - smooth(ramp(t, PRE + 3.5, PRE + 4.0)))
    return grav(th) + noise(rng, len(t))


def c_facedown(rng, dur):
    t = tvec(dur); p = -np.pi / 2 * smooth(ramp(t, PRE + 0.2, PRE + 0.7))      # de pie -> boca abajo sobre la mesa
    return np.stack([np.zeros(len(t)), np.cos(p), np.sin(p)], 1) + noise(rng, len(t))


def c_pickup(rng, dur):
    t = tvec(dur); n = len(t)
    u = ramp(t, PRE + 3.0, PRE + 3.8); p = (np.pi / 2) * smooth(u)
    s = np.stack([np.zeros(n), np.sin(p), np.cos(p)], 1)
    s += np.sin(np.pi * u)[:, None] * rng.normal(0, 0.25, 3)[None, :]
    return s + noise(rng, n)


def c_walk(freq, amp, run=False):
    def f(rng, dur):
        t = tvec(dur); ph = rng.uniform(0, 6.28); w = 2 * np.pi * freq
        v = amp * np.sin(w * t + ph) + (0.35 * amp * np.sin(2 * w * t + 2 * ph) if run else 0)
        th = np.deg2rad(14 if run else 8) * np.sin(0.5 * w * t + ph)
        s = grav(th)
        s[:, 1] += v
        s[:, 2] += 0.5 * amp * np.sin(0.5 * w * t + ph)
        s[:, 0] += 0.3 * amp * np.sin(0.5 * w * t)
        if run:
            s[:, 1] += 0.5 * amp * np.maximum(0, np.sin(w * t + ph)) ** 6      # impacto del pie
        return s + noise(rng, len(t))
    return f


def c_motor(rng, dur):
    t = tvec(dur); n = len(t); f0 = rng.uniform(80, 200)
    ts = t + rng.normal(0, 0.0004, n)                     # alias de f0 a 60 Hz + jitter del reloj
    return up(n) + 0.2 * np.sin(2 * np.pi * f0 * ts)[:, None] * unit(rng)[None, :] + noise(rng, n)


def c_noise(sig, base=up):
    def f(rng, dur):
        n = len(tvec(dur)); return base(n) + noise(rng, n, sig)
    return f


# nombre, generador, duracion tras el lead-in, debe, prohibe, esperado
CASES = [
    ("quieto_de_pie",        c_still_up,          8, [],    [0, 1, 2, 3, 4, 5, 6, 7], "nada"),
    ("quieto_plano",         c_still_flat,        8, [],    [0, 1, 2, 3, 4, 5, 6, 7], "nada (sin 'recogido')"),
    ("inclinacion_0_60",     c_slow_tilt,         8, [],    [0, 1, 2, 3, 4, 6, 7],    "nada brusco (solo efecto continuo)"),
    ("mecer_0.5Hz_20deg",    c_rock(0.5),         8, [],    [0, 1, 2, 3, 4, 6, 7],    "arrullo: sin sacudida/caida"),
    ("mecer_1Hz_20deg",      c_rock(1.0),         8, [],    [0, 1, 2, 3, 4, 6, 7],    "arrullo: sin sacudida/caida"),
    ("sacudida_suave",       c_shake(2.5, 1.0),   4, [0],   [2, 3],                   "shake_suave, sin caida/impacto"),
    ("sacudida_fuerte",      c_shake(5.0, 3.0),   4, [1],   [2, 3],                   "shake_fuerte, sin caida ni impacto espurios"),
    ("caida_libre+impacto",  c_freefall,          3, [2, 3], [0, 1],                  "caida (0.3-0.6 s |a|~0) e impacto"),
    ("golpe_seco_2g",        c_tap(2.0, 2),       3, [3],   [0, 1, 2],                "impacto (tap +2 g, 2 muestras)"),
    ("golpe_debil_1g (info)", c_tap(1.0, 2),      3, [],    [0, 1, 2],                "informativo: no debe ser sacudida/caida"),
    ("de_cabeza_sostenido",  c_inverted,          5, [4],   [0, 1, 2, 3],             "de_cabeza tras ~1.2 s"),
    ("de_cabeza_y_vuelta",   c_inverted_return,   8, [4, 5], [0, 1, 2, 3],            "de_cabeza y luego vuelve_normal"),
    ("boca_abajo_sostenido", c_facedown,          5, [6],   [0, 1, 2, 3],             "boca_abajo tras ~1 s"),
    ("recoger_desde_plano",  c_pickup,            6, [7],   [0, 1, 2],                "recogido"),
    ("caminar_1.8Hz_0.4g",   c_walk(1.8, 0.4),    8, [],    [0, 1, 2, 3, 4, 6, 7],    "nada"),
    ("correr_2.8Hz_1.2g",    c_walk(2.8, 1.2, True), 8, [], [1, 2, 3],                "como mucho shake_suave; no fuerte/caida/impacto"),
    ("vibracion_motor_60Hz", c_motor,             8, [],    [0, 1, 2, 3, 4, 6, 7],    "nada (alias de 80-200 Hz)"),
    ("ruido_puro_0.05g",     c_noise(0.05),       8, [],    [0, 1, 2, 3, 4, 6, 7],    "nada"),
    ("ruido_fuerte_0.3g",    c_noise(0.30),       8, [],    [0, 1, 2, 3, 4, 6, 7],    "nada (sensor ruidoso/EMI)"),
    ("ruido_fuerte_plano",   c_noise(0.30, flat), 8, [],    [0, 1, 2, 3, 4, 6],       "nada (plano, sensor ruidoso)"),
]


def run_case(d, sig, seed):
    d.sim_init(seed); d.sim_motion_reset(); d.sim_ev_reset()
    n = len(sig); t = 0
    events = []; exprs = []; clips = []; prev = [0] * 16; lastx = -1; lastslot = [-1] * 3
    shake = np.zeros(n); mag = np.linalg.norm(sig, axis=1); dev = np.zeros(n)
    buf = (ctypes.c_float * 6)()
    for i in range(n):
        t = int(round((i + 1) * 1000.0 / FS))
        a = sig[i]
        d.sim_motion(float(a[0]), float(a[1]), float(a[2]), t, DT)
        d.sim_frame(t, DT)
        d.sim_mstate(buf); shake[i] = buf[0]
        dev[i] = np.linalg.norm(a - np.array([buf[1], buf[2], buf[3]]))
        for w in range(8):
            c = d.sim_ev_count(w)
            for _ in range(c - prev[w]):
                events.append((i, w, d.sim_ev_strength(w)))
            prev[w] = c
        x = d.sim_expr()
        if x != lastx:
            exprs.append((i, x)); lastx = x
        for s in range(3):
            c = d.sim_slot_clip(s)
            if c != lastslot[s] and c >= 0:
                clips.append((i, d.sim_clip_name(c).decode()))
            lastslot[s] = c
    return events, exprs, clips, shake, mag, dev


def main():
    d = build()
    clipnames = [d.sim_clip_name(i).decode() for i in range(d.sim_nclips())]
    pre = int(PRE * FS)
    rows = []
    for (name, gen, dur, must, forbid, expect) in CASES:
        ok = 0; seen = [0] * 8; lat = [[] for _ in range(8)]
        sh = []; mn = []; mx = []; dv = []; clipset = {}; exprset = {}
        for sd in range(args.seeds):
            rng = np.random.default_rng(1000 + sd)
            sig = gen(rng, dur + PRE)
            ev, ex, cl, shake, mag, dev = run_case(d, sig, 7 + sd)
            got = set()
            for (i, w, s) in ev:
                if i >= pre:
                    got.add(w); lat[w].append((i - pre) / FS)
            for w in got:
                seen[w] += 1
            ok += all(m in got for m in must) and not any(f in got for f in forbid)
            sh.append(shake[pre:].max()); mn.append(mag[pre:].min()); mx.append(mag[pre:].max()); dv.append(dev[pre:].max())
            for (i, c) in cl:
                if i >= pre: clipset[c] = clipset.get(c, 0) + 1
            for (i, x) in ex:
                if i >= pre: exprset[x] = exprset.get(x, 0) + 1
        st = lambda a: (min(a), float(np.median(a)), max(a))
        rows.append(dict(name=name, expect=expect, rate=ok / args.seeds, seen=seen, lat=lat, clips=clipset,
                         exprs=exprset, shake=st(sh), magmin=st(mn), magmax=st(mx), dev=st(dv)))
    write(rows, clipnames)


def write(rows, clipnames):
    N = args.seeds
    L = [f"# Resultados automaticos de test_motion.py ({N} semillas/caso)", f"Motion.h probado: `{args.motion}`", "",
         "Eventos: " + ", ".join(f"{i}={n}" for i, n in enumerate(EV)), "",
         "Clips usados por Motion.h y si existen en los datos: " + ", ".join(
             f"{n}={'si' if n in clipnames else 'NO'}" for n in ["sacudida_suave", "sacudida_fuerte", "caida", "impacto", "boca_abajo", "asiente"]),
         "", "| caso | esperado | observado (semillas con el evento) | tasa OK | veredicto |", "|---|---|---|---|---|"]
    for r in rows:
        seen = ", ".join(f"{EV[w]} {r['seen'][w]}/{N}" for w in range(8) if r["seen"][w]) or "(nada)"
        L.append(f"| {r['name']} | {r['expect']} | {seen} | {r['rate']*100:.0f}% | {'PASA' if r['rate'] >= 0.95 else 'FALLA'} |")
    f3 = lambda t: " / ".join(f"{x:.2f}" for x in t)
    L += ["", "## Estadisticas de senal (min / mediana / max sobre semillas)", "",
          "| caso | mShake pico | min abs(a) | max abs(a) | dev pico |", "|---|---|---|---|---|"]
    for r in rows:
        L.append(f"| {r['name']} | {f3(r['shake'])} | {f3(r['magmin'])} | {f3(r['magmax'])} | {f3(r['dev'])} |")
    L += ["", "## Latencias (mediana desde inicio de la senal) y clips/expresiones observados", ""]
    for r in rows:
        lt = ", ".join(f"{EV[w]}={np.median(v):.2f}s" for w, v in enumerate(r["lat"]) if v)
        L.append(f"- {r['name']}: lat [{lt}] ; clips {r['clips'] or '-'} ; expr(id:cambios) {r['exprs'] or '-'}")
    open(args.out, "w", encoding="utf-8").write("\n".join(L) + "\n")
    print("\n".join(L))


if __name__ == "__main__":
    main()
