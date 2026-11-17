#!/usr/bin/env python
"""Integra las entregas aprobadas del estudio en el proyecto principal.

  python tools/integrate.py --agents p1,p2,p3,s1,s2,s3,u1,u2,a1,a2,a3,a4,a5,a6,m1 [--dry]

Para cada agente: copia sus *.pal.txt / *.sp.txt / *.pu.txt / *.clip.txt a firmware/Alloy_Eyes/src/datos/ (con prefijo del
agente), los recompila con tools/studio_compile.py y REGENERA los manifiestos (pal_all.h, sp_all.h, pu_all.h, clips_all.h,
clips_list.h). Los datos base (*_base.*) siempre se conservan. Avisa de nombres duplicados."""
import glob, os, re, shutil, subprocess, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DATOS = os.path.join(ROOT, "firmware", "Alloy_Eyes", "src", "datos")
COMP = os.path.join(ROOT, "tools", "studio_compile.py")
args = sys.argv[1:]
agents = args[args.index("--agents") + 1].split(",") if "--agents" in args else []
dry = "--dry" in args

KINDS = {"pal": ".pal.txt", "sp": ".sp.txt", "pu": ".pu.txt", "clip": ".clip.txt"}
# limpia lo generado por integraciones anteriores (todo lo que no sea *_base.* ni manifiestos)
for f in glob.glob(os.path.join(DATOS, "*")):
    b = os.path.basename(f)
    if "_base." in b or b.startswith(("pal_all", "sp_all", "pu_all", "clips_all", "clips_list")):
        continue
    if not dry:
        os.remove(f)

inc = {"pal": ["pal_base.h"], "sp": ["sp_base.h"], "pu": ["pu_base.h"], "clip": ["clips_base.h"]}
lists = ["CLIPLIST_base"]
names = {"pal": {}, "sp": {}, "pu": {}, "clip": {}}
counts = {"pal": 0, "sp": 0, "pu": 0, "clip": 0}
for ag in agents:
    ddir = os.path.join(ROOT, "studio", "deliveries", ag)
    if not os.path.isdir(ddir):
        print(f"AVISO: no existe entrega de {ag}")
        continue
    for kind, ext in KINDS.items():
        for src in sorted(glob.glob(os.path.join(ddir, "*" + ext))):
            stem = os.path.basename(src)[: -len(ext)]
            stem = re.sub(r"[^A-Za-z0-9_]", "_", stem)
            tag = f"{ag}_{stem}" if not stem.startswith(ag) else stem
            tag = re.sub(r"_+", "_", tag)
            dst_txt = os.path.join(DATOS, tag + ext)
            dst_h = os.path.join(DATOS, tag + ".h")
            if not dry:
                shutil.copy(src, dst_txt)
            cmd = [sys.executable, COMP, kind, src, dst_h if not dry else os.devnull] + (["--prefix", tag[:28]] if kind == "clip" else [])
            r = subprocess.run(cmd, capture_output=True, text=True)
            if r.returncode:
                print(f"ERROR compilando {src}:\n{r.stdout}{r.stderr}")
                sys.exit(1)
            print("  ", r.stdout.strip())
            if kind == "clip":
                inc["clip"].append(tag + ".h")
                lists.append("CLIPLIST_" + tag[:28])
            else:
                inc[kind].append(tag + ".h")
            # duplicados de nombre
            for line in open(src, encoding="utf-8"):
                s = line.strip()
                if not s or s.startswith(("//", ";", "end")) or s[0].isdigit():
                    continue
                nm = s.split("|")[0].strip().lower() if kind != "clip" else (s.split()[1].lower() if s.startswith("clip ") else None)
                if nm:
                    counts[kind] += 1
                    if nm in names[kind]:
                        print(f"   AVISO duplicado ({kind}): '{nm}' en {tag} y {names[kind][nm]}")
                    names[kind][nm] = tag

if not dry:
    def w(fn, lines):
        open(os.path.join(DATOS, fn), "w", encoding="utf-8", newline="\n").write("".join(lines))
    w("pal_all.h", [f'#include "{f}"\n' for f in inc["pal"]])
    w("sp_all.h", [f'#include "{f}"\n' for f in inc["sp"]])
    w("pu_all.h", [f'#include "{f}"\n' for f in inc["pu"]])
    w("clips_all.h", [f'#include "{f}"\n' for f in inc["clip"]])
    w("clips_list.h", [f"{l}\n" for l in lists])
print("integrado:", {k: len(v) for k, v in names.items()}, "(sin contar los base)")
