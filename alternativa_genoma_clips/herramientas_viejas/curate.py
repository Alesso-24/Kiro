#!/usr/bin/env python
"""Curaduria post-integracion (aplica los informes de auditoria AUD_arte y AUD_animacion).
Se ejecuta DESPUES de `integrate.py` (que regenera datos desde las entregas):
    python tools/integrate.py --agents p1,p2,p3,s1,s2,s3,u1,u2,a1,a2,a3,a4,a5,a6,m1 && python tools/curate.py
Opera sobre los .txt copiados en firmware/Alloy_Eyes/src/datos/ y recompila sus .h. Indices = numeracion de las hojas de
auditoria (base primero, luego p1,p2,p3 / s1,s2,s3 / u1,u2)."""
import glob, os, re, subprocess, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
D = os.path.join(ROOT, "firmware", "Alloy_Eyes", "src", "datos")
COMP = os.path.join(ROOT, "tools", "studio_compile.py")
REP = os.path.join(ROOT, "studio", "reportes")

PAL_CUT = {112, 134, 113, 170, 186, 192, 118, 117, 173, 178, 184, 191, 172, 163, 181, 180, 166, 167, 156, 177, 182, 187,
           161, 176, 179, 162, 99, 29, 77, 136, 120}
PAL_ADJ = {0: ("p", "#0F8F46"), 15: ("p", "#B8501E"), 12: ("p", "#8A4A00"), 10: ("p", "#0A8FB0"), 5: ("e", "#B24558"),
           111: ("e", "#A83B6B"), 132: ("e", "#A8285A"), 131: ("e", "#14704A"), 90: ("e", "#86A6F5"),
           185: ("p", "#2A0840"), 188: ("p", "#2E0A5A"), 151: ("p", "#12126A"), 174: ("p", "#3A1470"), 123: ("p", "#0A3A66")}
SP_CUT = {1, 6, 3, 22, 33, 5, 23, 30, 38, 31, 35, 44, 13, 25, 17, 27}
SP_ADJ = {14: "gap=1.1", 19: "tilt=0.18 gap=1.05", 54: "tilt=0.24 gap=1.15", 24: "sx=0.72 sy=0.78 gap=1.15",
          26: "sx=1.25 sy=1.12", 37: "tilt=0.0 c2=0.06", 59: "asx=0.28", 61: "sx=0.7 sy=1.3", 63: "ac4=0.14 ay=5 c2=0.1",
          60: "tilt=0.4", 62: "ay=-6"}
PU_CUT = {16, 8, 14, 15, 18, 11, 5, 20, 7, 26, 27, 29, 31, 28, 33, 30, 35, 34}
PU_ADJ = {3: {"prx": 5, "pry": 5}, 10: {"pry": 18}}
CLIP_CUT = ("mirada_flotante respiro_suave parpadeo_carino celos burla_alternada asentir_dos estirar_ojos salto guino "
            "acurrucarse@a1 meneo_cola carcajada sacudida_furia ojos_entrecerrados tic_nervioso mirar_al_suelo resignacion "
            "abandono hipo_de_llanto sudor_frio tembleque_ansiedad verguenza@a3 arrullo@a4 respira_dormido sueno_agitado "
            "sobresalto_despertar caida_de_sueno mira_estrella mareo_jugueton bostezo_apagado guino_al_usuario "
            "ojos_corazon_rapido ojos_espiral_breve").split()
RETAG5 = {"negar", "encoger_hombros"}


def lines_of(path):
    return open(path, encoding="utf-8").read().split("\n")


def ordered(kind_ext, base, agents):
    files = [os.path.join(D, base)] + [os.path.join(D, a) for a in agents]
    return [f for f in files if os.path.exists(f)]


def is_data(l):
    s = l.strip()
    return bool(s) and not s.startswith(("//", ";"))


def curate_table(kind, ext, base, agents, cut, adj_fn):
    files = ordered(ext, base, agents)
    idx = 0
    cut_count = 0
    for f in files:
        out = []
        for l in lines_of(f):
            if is_data(l):
                if idx in cut:
                    idx += 1
                    cut_count += 1
                    continue
                l = adj_fn(idx, l)
                idx += 1
            out.append(l)
        open(f, "w", encoding="utf-8", newline="\n").write("\n".join(out))
        subprocess.run([sys.executable, COMP, kind, f, f[: -len(ext)] + ".h"], check=True, capture_output=True)
    print(f"  {kind}: {idx} entradas, {cut_count} cortadas -> quedan {idx - cut_count}")


def pal_adj(i, l):
    if i in PAL_ADJ:
        parts = [p.strip() for p in l.split("|")]
        col = 3 if PAL_ADJ[i][0] == "p" else 2
        parts[col] = PAL_ADJ[i][1]
        l = " | ".join(parts)
    parts = [p.strip() for p in l.split("|")]
    # regla transversal: pupila casi negra -> oscuro saturado (no se lee como agujero)
    h = parts[3].lstrip("#")
    r, g, b = int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16)
    if max(r, g, b) < 36:
        parts[3] = "#%02X%02X%02X" % (min(255, r * 2 + 18), min(255, g * 2 + 10), min(255, b * 2 + 40))
        l = " | ".join(parts)
    return l


def sp_adj(i, l):
    if i in SP_ADJ:
        parts = [p.strip() for p in l.split("|")]
        kv = {}
        for t in " ".join(parts[2:]).split():
            k, _, v = t.partition("=")
            kv[k] = v
        for t in SP_ADJ[i].split():
            k, _, v = t.partition("=")
            kv[k] = v
        l = f"{parts[0]} | {parts[1]} | " + " ".join(f"{k}={v}" for k, v in kv.items())
    return l


def pu_adj(i, l):
    if i in PU_ADJ:
        parts = [p.strip() for p in l.split("|")]
        nums = parts[2].split()
        names = ["prx", "pry", "pdx", "pdy", "ptilt", "pc1", "pc2"]
        for k, v in PU_ADJ[i].items():
            nums[names.index(k)] = str(v)
        parts[2] = " ".join(nums)
        l = " | ".join(parts)
    return l


print("curando tablas...")
curate_table("pal", ".pal.txt", "pal_base.pal.txt", ["p1_calidas.pal.txt", "p2_frias.pal.txt", "p3_raras.pal.txt"], PAL_CUT, pal_adj)
curate_table("sp", ".sp.txt", "sp_base.sp.txt", ["s1_tiernas.sp.txt", "s2_elegantes.sp.txt", "s3_raras.sp.txt"], SP_CUT, sp_adj)
curate_table("pu", ".pu.txt", "pu_base.pu.txt", ["u1_simples.pu.txt", "u2_especiales.pu.txt"], PU_CUT, pu_adj)

# ---------------- clips ----------------
print("curando clips...")
cfiles = sorted(glob.glob(os.path.join(D, "*.clip.txt")))
agent_of = {f: os.path.basename(f).split("_")[0] for f in cfiles}
cut_all = {c for c in CLIP_CUT if "@" not in c}
cut_ag = {tuple(c.split("@")) for c in CLIP_CUT if "@" in c}
# bloques nuevos / ajustados del informe del supervisor
rep_txt = open(os.path.join(REP, "AUD_animacion.md"), encoding="utf-8").read() if os.path.exists(os.path.join(REP, "AUD_animacion.md")) else ""
blocks = {}
for code in re.findall(r"```[a-z]*\n(.*?)```", rep_txt, re.S):
    for m in re.finditer(r"(clip\s+(\S+)[^\n]*\n.*?\nend)", code, re.S):
        blocks[m.group(2)] = m.group(1)
print(f"  bloques de clips del informe: {len(blocks)}")
replaced, cutn = set(), 0
for f in cfiles:
    txt = open(f, encoding="utf-8").read()
    out = []
    for m in re.finditer(r"(?:(?<=\n)|^)(clip\s+(\S+)[^\n]*\n.*?\nend\s*)", txt, re.S):
        pass
    parts = re.split(r"(?m)^(?=clip\s)", txt)
    new = [parts[0]]
    for blk in parts[1:]:
        name = blk.split()[1]
        ag = agent_of[f]
        if name in cut_all or (name, ag) in cut_ag:
            cutn += 1
            continue
        if name in blocks:
            blk = blocks[name] + "\n"
            replaced.add(name)
        if name in RETAG5:
            blk = re.sub(r"expr=\d+", "expr=5", blk, count=1)
        # los clips terminan en reposo (salvo loop)
        if " loop" not in blk.split("\n")[0]:
            ls = blk.rstrip().split("\n")
            for k in range(len(ls) - 1, -1, -1):
                if re.match(r"^\s*\d+\s*:", ls[k]):
                    ls[k] += " sx=0 sy=0 ox=0 oy=0 tilt=0 ltl=0 ltr=0 lbl=0 lbr=0 ps=0 gx=0 gy=0"
                    break
            blk = "\n".join(ls) + "\n"
        new.append(blk)
    open(f, "w", encoding="utf-8", newline="\n").write("".join(new))
# clips nuevos del informe (nombres que no existian)
existing = set()
for f in cfiles:
    existing |= set(re.findall(r"(?m)^clip\s+(\S+)", open(f, encoding="utf-8").read()))
newblk = [b for n, b in blocks.items() if n not in existing and n not in replaced]
if newblk:
    nf = os.path.join(D, "aud_nuevos.clip.txt")
    open(nf, "w", encoding="utf-8", newline="\n").write("\n".join(newblk) + "\n")
    cfiles.append(nf)
    agent_of[nf] = "aud"
    # registrar en manifiestos
    open(os.path.join(D, "clips_all.h"), "a", encoding="utf-8").write('#include "aud_nuevos.h"\n')
    open(os.path.join(D, "clips_list.h"), "a", encoding="utf-8").write("CLIPLIST_aud_nuevos\n")
for f in cfiles:
    stem = os.path.basename(f)[:-len(".clip.txt")]
    prefix = stem[:28] if stem != "clips_base" else "base"
    r = subprocess.run([sys.executable, COMP, "clip", f, f[: -len(".clip.txt")] + ".h", "--prefix", prefix],
                       capture_output=True, text=True)
    if r.returncode:
        print("ERROR compilando", f, r.stdout, r.stderr)
        sys.exit(1)
print(f"  clips cortados: {cutn}, reemplazados por el informe: {len(replaced)}, nuevos: {len(newblk)}")
print("OK")
