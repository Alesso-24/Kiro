#!/usr/bin/env python
"""Compilador de datos del estudio Alloy: texto legible -> cabeceras C++ para el firmware.

  python studio_compile.py pal    entrada.pal.txt   salida.h
  python studio_compile.py sp     entrada.sp.txt    salida.h        (especies = anatomia de ojos)
  python studio_compile.py pu     entrada.pu.txt    salida.h        (estilos de pupila)
  python studio_compile.py clip   entrada.clip.txt  salida.h --prefix a1   (animaciones)

Formatos (ver firmware/Alloy_Eyes/CONTRATO_DATOS.md). Comentarios: lineas que empiezan con // o con ;
"""
import re, sys, os

RARITY = {"0": 0, "1": 1, "2": 2, "3": 3, "common": 0, "comun": 0, "uncommon": 1, "pococomun": 1, "raro": 2, "rare": 2,
          "mythic": 3, "mitico": 3}


def clean(lines):
    for n, l in enumerate(lines, 1):
        s = l.strip()
        if not s or s.startswith("//") or s.startswith(";"):
            continue
        yield n, s


def hex3(h):
    h = h.strip().lstrip("#")
    if h in ("-", ""):
        return None
    if len(h) != 6:
        raise ValueError(f"color invalido '{h}' (usa #RRGGBB)")
    return [int(h[i:i + 2], 16) for i in (0, 2, 4)]


def cname(s):
    return re.sub(r'[^A-Za-z0-9_ \-áéíóúñÁÉÍÓÚÑ]', '', s).strip()[:23].replace('"', '')


def comp_pal(src, dst):
    out = ["// generado por tools/studio_compile.py (pal) desde %s\n" % os.path.basename(src)]
    n = 0
    for ln, s in clean(open(src, encoding="utf-8").read().split("\n")):
        parts = [p.strip() for p in s.split("|")]
        if len(parts) < 4:
            raise SystemExit(f"{src}:{ln}: se esperan 'nombre | rareza | #ojo | #pupila [| #acento]'")
        name, rar, e, p = parts[0], parts[1].lower(), hex3(parts[2]), hex3(parts[3])
        a = hex3(parts[4]) if len(parts) > 4 else None
        if rar not in RARITY:
            raise SystemExit(f"{src}:{ln}: rareza '{rar}' invalida")
        if a is None:   # acento por defecto: mezcla clara del color de pupila
            a = [min(255, (p[i] + 255) // 2) for i in range(3)]
        out.append('{{%d,%d,%d},{%d,%d,%d},{%d,%d,%d},%d,"%s"},\n' % (*e, *p, *a, RARITY[rar], cname(name)))
        n += 1
    open(dst, "w", encoding="utf-8", newline="\n").write("".join(out))
    print(f"{n} paletas -> {dst}")


SP_KEYS = ["gap", "sx", "sy", "tilt", "c1", "c2", "c3", "c4", "c5", "asx", "atilt", "ac4", "aps", "ay"]
SP_DEF = {"gap": 1.0, "sx": 1.0, "sy": 1.0, "tilt": 0.0, "c1": 0.0, "c2": 0.0, "c3": 0.0, "c4": 0.0, "c5": 0.0,
          "asx": 0.0, "atilt": 0.0, "ac4": 0.0, "aps": 0.0, "ay": 0.0}
SP_RANGE = {"gap": (0.7, 1.4), "sx": (0.6, 1.5), "sy": (0.6, 1.5), "tilt": (-0.7, 0.7), "c1": (-0.4, 0.4),
            "c2": (-0.4, 0.4), "c3": (-0.4, 0.4), "c4": (-0.4, 0.4), "c5": (-0.4, 0.4), "asx": (-0.4, 0.4),
            "atilt": (-0.5, 0.5), "ac4": (-0.4, 0.4), "aps": (-0.6, 0.6), "ay": (-14, 14)}


def comp_sp(src, dst):
    out = ["// generado por tools/studio_compile.py (sp) desde %s\n" % os.path.basename(src)]
    n = 0
    for ln, s in clean(open(src, encoding="utf-8").read().split("\n")):
        parts = [p.strip() for p in s.split("|")]
        if len(parts) < 2:
            raise SystemExit(f"{src}:{ln}: se esperan 'nombre | rareza | clave=valor ...'")
        name, rar = parts[0], parts[1].lower()
        if rar not in RARITY:
            raise SystemExit(f"{src}:{ln}: rareza invalida")
        v = dict(SP_DEF)
        for kv in " ".join(parts[2:]).split():
            k, _, val = kv.partition("=")
            if k not in v:
                raise SystemExit(f"{src}:{ln}: clave desconocida '{k}' (validas: {', '.join(SP_KEYS)})")
            v[k] = float(val)
            lo, hi = SP_RANGE[k]
            if not lo <= v[k] <= hi:
                raise SystemExit(f"{src}:{ln}: {k}={v[k]} fuera de rango [{lo},{hi}]")
        out.append("{%s,%d,\"%s\"},\n" % (",".join("%.3ff" % v[k] for k in SP_KEYS), RARITY[rar], cname(name)))
        n += 1
    open(dst, "w", encoding="utf-8", newline="\n").write("".join(out))
    print(f"{n} especies -> {dst}")


PU_KINDS = {"none": 0, "glint": 1, "split": 2, "glintsplit": 3, "halfmoon": 4, "ring": 5}


def comp_pu(src, dst):
    out = ["// generado por tools/studio_compile.py (pu) desde %s\n" % os.path.basename(src)]
    n = 0
    for ln, s in clean(open(src, encoding="utf-8").read().split("\n")):
        parts = [p.strip() for p in s.split("|")]
        if len(parts) < 4:
            raise SystemExit(f"{src}:{ln}: se esperan 'nombre | rareza | prx pry pdx pdy ptilt pc1 pc2 | tipo'")
        name, rar = parts[0], parts[1].lower()
        nums = [float(x) for x in parts[2].split()]
        if len(nums) != 7:
            raise SystemExit(f"{src}:{ln}: se esperan 7 numeros (prx pry pdx pdy ptilt pc1 pc2)")
        kind = PU_KINDS.get(parts[3].lower())
        if kind is None:
            raise SystemExit(f"{src}:{ln}: tipo '{parts[3]}' invalido ({', '.join(PU_KINDS)})")
        prx, pry, pdx, pdy, pt, pc1, pc2 = nums
        if not (0 <= prx <= 24 and 0 <= pry <= 30 and -20 <= pdx <= 20 and -20 <= pdy <= 24 and -1.2 <= pt <= 1.2
                and -0.45 <= pc1 <= 0.45 and -0.45 <= pc2 <= 0.45):
            raise SystemExit(f"{src}:{ln}: valores fuera de rango")
        out.append("{%s,%d,%d,\"%s\"},\n" % (",".join("%.3ff" % x for x in nums), kind, RARITY[rar], cname(name)))
        n += 1
    open(dst, "w", encoding="utf-8", newline="\n").write("".join(out))
    print(f"{n} pupilas -> {dst}")


CH = ["sx", "sy", "ox", "oy", "tilt", "ltl", "ltr", "lbl", "lbr", "ps", "gx", "gy"]
CH_RANGE = {"sx": 0.5, "sy": 0.5, "ox": 24.0, "oy": 24.0, "tilt": 0.6, "ltl": 1.0, "ltr": 1.0, "lbl": 1.0, "lbr": 1.0,
            "ps": 1.0, "gx": 1.0, "gy": 1.0}
EV = {"none": 0, "heart": 1, "hearts": 2, "spark": 3, "sparks": 4, "drop": 5, "bolt": 6, "bolts": 7, "line": 8,
      "lines": 9, "dots": 10, "star": 11, "wink": 12, "blink": 13, "shake": 14, "sweat": 15, "snow": 16, "note": 17}
CF = {"mirror": 1, "loop": 2, "ease": 4, "linear": 8}


def comp_clip(src, dst, prefix):
    lines = list(clean(open(src, encoding="utf-8").read().split("\n")))
    clips = []
    cur = None
    for ln, s in lines:
        if s.startswith("clip "):
            toks = s.split()
            cur = {"name": toks[1], "flags": 0, "keys": [], "expr": 0}
            for t in toks[2:]:
                if t in CF:
                    cur["flags"] |= CF[t]
                elif t.startswith("expr="):
                    cur["expr"] = int(t[5:])
                else:
                    raise SystemExit(f"{src}:{ln}: opcion de clip desconocida '{t}' ({', '.join(CF)}, expr=N)")
            clips.append(cur)
        elif s == "end":
            cur = None
        else:
            if cur is None:
                raise SystemExit(f"{src}:{ln}: linea fuera de un 'clip ... end'")
            m = re.match(r"^(\d+)\s*(?:ms)?\s*:\s*(.*)$", s)
            if not m:
                raise SystemExit(f"{src}:{ln}: formato de keyframe 'T: canal=valor ...'")
            t = int(m.group(1))
            if t > 6000:
                raise SystemExit(f"{src}:{ln}: tiempo > 6000 ms")
            vals = {}
            ev = 0
            for kv in m.group(2).split():
                k, _, val = kv.partition("=")
                if k == "ev":
                    if val not in EV:
                        raise SystemExit(f"{src}:{ln}: evento '{val}' invalido ({', '.join(EV)})")
                    ev = EV[val]
                elif k == "lt":
                    vals["ltl"] = vals["ltr"] = float(val)
                elif k == "lb":
                    vals["lbl"] = vals["lbr"] = float(val)
                elif k in CH:
                    vals[k] = float(val)
                else:
                    raise SystemExit(f"{src}:{ln}: canal desconocido '{k}' ({', '.join(CH)}, lt, lb, ev)")
            cur["keys"].append((t, vals, ev))
    out = ["// generado por tools/studio_compile.py (clip) desde %s\n" % os.path.basename(src)]
    reg = []
    total_keys = 0
    for c in clips:
        if len(c["keys"]) < 2:
            raise SystemExit(f"clip {c['name']}: necesita >= 2 keyframes")
        ts = [k[0] for k in c["keys"]]
        if ts != sorted(ts) or len(set(ts)) != len(ts):
            raise SystemExit(f"clip {c['name']}: tiempos deben ser estrictamente crecientes")
        held = {k: 0.0 for k in CH}
        rows = []
        for t, vals, ev in c["keys"]:
            held.update(vals)
            q = []
            for k in CH:
                v = max(-1.0, min(1.0, held[k] / CH_RANGE[k]))
                q.append(int(round(v * 127)))
            rows.append("{%d,{%s},%d}" % (t, ",".join(map(str, q)), ev))
        ident = f"{prefix}_{re.sub(r'[^A-Za-z0-9_]', '_', c['name'])}"
        out.append(f"static const Key K_{ident}[] = {{" + ",".join(rows) + "};\n")
        reg.append('{"%s",K_%s,%d,%d,%d,%d},' % (cname(c["name"]), ident, len(rows), c["flags"], ts[-1], c["expr"]))
        total_keys += len(rows)
    out.append(f"#define CLIPLIST_{prefix} \\\n  " + " \\\n  ".join(reg) + "\n")
    open(dst, "w", encoding="utf-8", newline="\n").write("".join(out))
    print(f"{len(clips)} clips, {total_keys} keyframes -> {dst}")


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print(__doc__)
        sys.exit(1)
    kind, src, dst = sys.argv[1:4]
    prefix = sys.argv[sys.argv.index("--prefix") + 1] if "--prefix" in sys.argv else "x"
    {"pal": lambda: comp_pal(src, dst), "sp": lambda: comp_sp(src, dst), "pu": lambda: comp_pu(src, dst),
     "clip": lambda: comp_clip(src, dst, prefix)}[kind]()
