#!/usr/bin/env python
"""Pagina para evaluar animaciones: catalogo/anims.html  (expresiones + clips, en 2 criaturas favoritas lado a lado).
  python tools/encuesta_anim.py     Cada item: voto + nota. Exporta JSON."""
import ctypes, glob, importlib.util, json, os, sys
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
spec = importlib.util.spec_from_file_location("viewer", os.path.join(ROOT, "sim", "viewer.py"))
V = importlib.util.module_from_spec(spec); spec.loader.exec_module(V)
d = V.load()
u32 = ctypes.c_uint32
d.sim_seed.argtypes = [u32]; d.sim_auto.argtypes = [ctypes.c_int]; d.sim_set_species.argtypes = [ctypes.c_int]
d.sim_look.argtypes = [ctypes.c_int, ctypes.c_int]
d.sim_clip_play.argtypes = [ctypes.c_int, u32, ctypes.c_int, ctypes.c_float]
for f in ("sim_pal_name", "sim_species_name", "sim_pupil_name", "sim_clip_name", "sim_expr_name"):
    if hasattr(d, f): getattr(d, f).restype = ctypes.c_char_p


def find(fn, n, name):
    for i in range(n):
        if fn(i).decode() == name: return i
    raise SystemExit("no existe: " + name)


npal, nsp, npu, ncl = d.sim_npal(), d.sim_nspecies(), d.sim_npupils(), d.sim_nclips()
CRE = [("Hojas de otono", "Frijolito", "Redonda grande"), ("Chicle explotado", "Veleta", "Redonda")]
CRE = [(find(d.sim_pal_name, npal, a), find(d.sim_species_name, nsp, b), find(d.sim_pupil_name, npu, c)) for a, b, c in CRE]
TAGS = ["idle", "alegria", "enojo", "triste/miedo", "sueno", "curiosidad", "energia/sistema", "reaccion a movimiento"]
IMG = os.path.join(ROOT, "catalogo", "anims")
os.makedirs(IMG, exist_ok=True)
for f in glob.glob(os.path.join(IMG, "*")): os.remove(f)


def run(t, n):
    for _ in range(n):
        t += 17
        d.sim_frame(t, 1 / 60)
    return t


def setup(k, seed):
    p, s, u = CRE[k]
    d.sim_init(seed); d.sim_auto(0); d.sim_seed(seed); d.sim_look(p, u); d.sim_set_species(s)
    d.sim_reset_state()
    d.sim_simbatt(0); d.sim_force(-1, -1, -1, -1); d.sim_force_love(-1)
    d.sim_birth_off()
    d.sim_set_expr(1, 0, 0)
    return run(0, 100)


def save(frs, path):
    frs[0].save(path, save_all=True, append_images=frs[1:], duration=33, loop=0, quality=85, method=4)


items = []
XN = {d.sim_expr_name(x).decode(): x for x in range(1, d.sim_nexpr())}
d.sim_set_usb.argtypes = [u32, ctypes.c_int]; d.sim_cold.argtypes = [u32, ctypes.c_int]; d.sim_pet.argtypes = [u32, ctypes.c_int]
d.sim_set_energy.argtypes = [ctypes.c_float]; d.sim_batlow.argtypes = [u32, ctypes.c_int]; d.sim_shake.argtypes = [u32, ctypes.c_int]; d.sim_timescale.argtypes = [ctypes.c_float]
for f in ("sim_tap", "sim_poke", "sim_loud", "sim_thumbs", "sim_flick", "sim_play", "sim_batfull", "sim_next_theme"):
    getattr(d, f).argtypes = [u32]


def X(name):
    return lambda t: d.sim_set_expr(XN[name], t, 0)


def seq(*names, gap=1800):                       # emocion tras emocion
    return [(500 + k * gap, X(n)) for k, n in enumerate(names)], 500 + len(names) * gap + 600


SCEN = []                                       # (grupo, nombre, eventos[(ms, fn(t))], duracion_ms, timescale)
def add(g, n, ev, dur, ts=1.0, auto=0, pre=0, energy=None): SCEN.append((g, n, ev, dur, ts, auto, pre, energy))


d.sim_slowblink.argtypes = [u32]; d.sim_force_love.argtypes = [ctypes.c_int]; d.sim_force.argtypes = [ctypes.c_int] * 4; d.sim_simbatt.argtypes = [ctypes.c_int]; d.sim_birth.argtypes = [u32, ctypes.c_int]; d.sim_emotion.argtypes = [u32]
BN = ["completo"]
for v, nm in enumerate(BN):
    add("Nacimiento (al encender)", f"Ruleta de ojos al conectar ({nm})", [(300, (lambda vv: (lambda t: d.sim_birth(t, vv)))(v))], 35000)
add("Parpadeo lento", "Parpadeo lento: cierra, espera y mira alrededor (segun personalidad)", [(500, lambda t: d.sim_slowblink(t))], 8500)
add("Botones", "Azul: emocion por un momento (3 pulsaciones)", [(500, lambda t: d.sim_emotion(t)), (4500, lambda t: d.sim_emotion(t)), (8500, lambda t: d.sim_emotion(t))], 12500)
add("Botones", "Verde: acariciar (mantener)", [(500, lambda t: d.sim_pet(t, 1)), (4500, lambda t: d.sim_pet(t, 0))], 7500)
for ty, nm in enumerate(["llenado (ojos recipiente)", "ondas de energia", "ronroneo"]):
    add("Carga", f"Le conectan el cargador, tipo {ty + 1}: {nm}", [(100, (lambda a: (lambda t: d.sim_force(a, -1, -1, -1)))(ty)), (300, lambda t: d.sim_set_usb(t, 1)), (13500, lambda t: d.sim_batfull(t)), (17500, lambda t: d.sim_set_usb(t, 0))], 20000, 5, energy=15)
for k in range(6):
    add("Cara llena (6 paletas)", f"Cara llena, paleta {k + 1}", [(50, (lambda a: (lambda t: d.sim_force(-1, a, -1, -1)))(k)), (200, X("cara-llena")), (3400, X("despierto"))], 5200)
ANGN = ["clasico", "furia", "ceño frio", "berrinche", "cejas", "vapor", "fuego", "chevrones", "hervir (se va enrojeciendo)"]
for v, c in ((0, 0), (1, 1), (2, 2), (3, 3), (4, 0), (5, 2), (6, 0), (7, 1), (8, 4)):
    add("Enojado (9 formas)", f"Enojado: {ANGN[v]}", [(50, (lambda a, b: (lambda t: d.sim_force(-1, -1, a, b)))(v, c)), (200, X("enojado")), (4200, X("despierto"))], 6000)
for k, nm in enumerate(["feliz con sus colores", "ojos cerrados felices", "clasico rosa", "corazones en los ojos"]):
    add("Amor / acariciar (4 variantes)", f"Amor, variante {k + 1}: {nm}", [(50, (lambda a: (lambda t: d.sim_force_love(a)))(k)), (200, X("amor")), (4200, X("despierto"))], 6000)
add("Botones", "Azul: emociones EN ORDEN (6 pulsaciones)", [(500 + 3900 * q, lambda t: d.sim_emotion(t)) for q in range(6)], 25500)
add("Carga", "Conectar cable, cargar y volver a la normalidad (x25 rapido)", [(500, lambda t: d.sim_set_usb(t, 1)), (11000, lambda t: d.sim_batfull(t)), (17000, lambda t: d.sim_set_usb(t, 0))], 20000, 25, energy=20)
add("Carga", "Bateria baja: cansado y pide carga (se ve un buen rato)", [(500, lambda t: d.sim_batlow(t, 1)), (22000, lambda t: d.sim_batlow(t, 0))], 24000, auto=1, energy=12)
add("Carga", "Boton: ver bateria al 90%", [(500, lambda t: d.sim_thumbs(t))], 6500, energy=90)
add("Carga", "Boton: ver bateria al 45%", [(500, lambda t: d.sim_thumbs(t))], 6500, energy=45)
add("Carga", "Boton: ver bateria al 12%", [(500, lambda t: d.sim_thumbs(t))], 6500, energy=12)
add("Sueno", "Aburrimiento: se queda quieto un buen rato", [], 24000, auto=1, pre=34000)
add("Sueno", "Mucho rato sin hacerle caso: bosteza y se duerme", [(14000, lambda t: d.sim_tap(t))], 18000, auto=1, pre=93000)
add("Sueno", "Bostezo y se duerme, luego lo tocas", [(500, X("bostezo")), (3300, X("dormido")), (8000, lambda t: d.sim_tap(t))], 12000)
add("Sueno", "Dormido-tenue a dormido a despertar", [(500, X("dormido-tenue")), (4000, X("dormido")), (8500, lambda t: d.sim_poke(t))], 12000)
add("Reaccion", "Agitar suave", [(500, lambda t: d.sim_shake(t, 0))], 6000)
add("Reaccion", "Agitar fuerte (mareo y enojo)", [(500, lambda t: d.sim_shake(t, 1))], 8500)
add("Reaccion", "Ruido fuerte (ansioso)", [(500, lambda t: d.sim_loud(t))], 6500)
add("Reaccion", "Frio (tirita) y se le quita", [(500, lambda t: d.sim_cold(t, 1)), (5500, lambda t: d.sim_cold(t, 0))], 8500)
add("Reaccion", "Desprecio (flick)", [(500, lambda t: d.sim_flick(t))], 6000)
add("Cariño", "Tocar una vez", [(500, lambda t: d.sim_tap(t))], 4500)
add("Cariño", "Tocar tres veces: amor", [(500, lambda t: d.sim_tap(t)), (1100, lambda t: d.sim_tap(t)), (1700, lambda t: d.sim_tap(t))], 7000)
add("Cariño", "Acariciar (mantener)", [(500, lambda t: d.sim_pet(t, 1)), (4500, lambda t: d.sim_pet(t, 0))], 7500)
add("Cariño", "Jugar (hype)", [(500, lambda t: d.sim_play(t))], 7000)
add("Cariño", "Piquete", [(500, lambda t: d.sim_poke(t))], 4500)
add("Cambio", "Cambiar de look (mismo bicho, otros colores)", [(500, lambda t: d.sim_next_theme(t))], 4500)
for nm, names in (("feliz a triste a enojado", ("feliz", "triste", "enojado")), ("despierto a sorprendido a curioso", ("sorprendido", "curioso", "despierto")),
                  ("amor a orgulloso a hype", ("amor", "orgulloso", "hype")), ("ansioso a frio a bostezo", ("ansioso", "frio", "bostezo"))):
    ev, dur = seq(*names); add("Transiciones entre emociones", nm, ev, dur)
for n in XN:                                    # cada expresion: entra, se queda, vuelve a despierto
    add("Expresiones", n, [(500, X(n)), (3400, X("despierto"))], 5200)

items = []
for g, nm, ev, dur, ts, auto, pre, en in SCEN:
    hf = nm.startswith('Ruleta')
    files = []
    for k in (0, 1):
        t = setup(k, 31 + k)
        d.sim_timescale(1.0)
        if en is not None: d.sim_set_energy(float(en))
        if pre:
            d.sim_auto(auto); t = run(t, int(pre / 17))
        d.sim_timescale(ts); d.sim_auto(auto)
        frs = [V.round_img(d, 1).resize((170, 170))]
        t0, pend = t, sorted(ev, key=lambda a: a[0])
        while t - t0 < dur:
            while pend and t - t0 >= pend[0][0]:
                pend.pop(0)[1](t)
            if hf:
                t += 20; d.sim_frame(t, 1 / 50)
            else:
                t = run(t, 2)
            frs.append(V.round_img(d, 1).resize((170, 170)))
        d.sim_timescale(1.0)
        fn = f"s{len(items):03d}_{k}.webp"
        frs[0].save(os.path.join(IMG, fn), save_all=True, append_images=frs[1:], duration=(20 if hf else 33), loop=0, quality=85, method=4); files.append("anims/" + fn)
    items.append({"id": "esc:" + g + ":" + nm, "grupo": g, "nombre": nm, "imgs": files})
    print(len(items), nm)

html = open(os.path.join(ROOT, "tools", "anim_template.html"), encoding="utf-8").read().replace("__ITEMS__", json.dumps(items))
open(os.path.join(ROOT, "catalogo", "anims.html"), "w", encoding="utf-8").write(html)
print("OK", len(items), "items,", sum(os.path.getsize(f) for f in glob.glob(os.path.join(IMG, "*"))) // 1024, "KB")
