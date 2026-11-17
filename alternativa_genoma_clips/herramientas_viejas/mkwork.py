#!/usr/bin/env python
"""Crea un sandbox de trabajo aislado para un agente:  python tools/mkwork.py <nombre>
Copia firmware/Alloy_Eyes (sin .ino) y sim/ a studio/work/<nombre>/ . Cada agente trabaja SOLO ahi."""
import os, shutil, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
name = sys.argv[1]
dst = os.path.join(ROOT, "studio", "work", name)
if os.path.exists(dst):
    shutil.rmtree(dst)
os.makedirs(os.path.join(dst, "firmware"))
shutil.copytree(os.path.join(ROOT, "firmware", "Alloy_Eyes"), os.path.join(dst, "firmware", "Alloy_Eyes"),
                ignore=shutil.ignore_patterns("*.ino"))
os.makedirs(os.path.join(dst, "sim"))
for f in ("eyes_sim.cpp", "viewer.py"):
    shutil.copy(os.path.join(ROOT, "sim", f), os.path.join(dst, "sim", f))
os.makedirs(os.path.join(dst, "out"), exist_ok=True)
os.makedirs(os.path.join(ROOT, "studio", "deliveries", name), exist_ok=True)
print("sandbox listo:", dst)
print("entregas en  :", os.path.join(ROOT, "studio", "deliveries", name))
