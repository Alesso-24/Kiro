import ctypes, importlib.util, os, sys
from PIL import Image, ImageDraw
work = os.path.abspath("studio/work/a1")
spec = importlib.util.spec_from_file_location("viewer", os.path.join(work, "sim", "viewer.py"))
V = importlib.util.module_from_spec(spec); spec.loader.exec_module(V)
d = V.load()
u32 = ctypes.c_uint32
d.sim_clip_play.argtypes = [ctypes.c_int, u32, ctypes.c_int, ctypes.c_float]
d.sim_clip_name.restype = ctypes.c_char_p
tagsel = int(sys.argv[1]); mir = int(sys.argv[2]); look = int(sys.argv[3]) if len(sys.argv)>3 else 0
ids = [i for i in range(d.sim_nclips()) if d.sim_clip_tag(i) == tagsel and d.sim_clip_name(i).decode() not in ("salto","guino","meneo","asiente")]
nf = 14
sh = Image.new("RGB", (nf*72, len(ids)*84), (30,30,36)); dr = ImageDraw.Draw(sh)
for r,i in enumerate(ids):
    d.sim_init(77); d.sim_look(look,2); d.sim_set_expr(1, 0, 0)
    t=0
    for _ in range(90):
        t+=17; d.sim_frame(t, 1/60)
    dur = d.sim_clip_dur(i)+150
    d.sim_clip_play(i, t, mir, 1.0)
    step = dur/nf; t0=t
    for f in range(nf):
        target = t0 + step*(f+0.5)
        while t < target:
            t+=17; d.sim_frame(t, 1/60)
        sh.paste(V.round_img(d,1).resize((72,72), Image.LANCZOS), (f*72, r*84))
    dr.text((2, r*84+72), f"{d.sim_clip_name(i).decode()} ({d.sim_clip_dur(i)}ms)", fill=(220,220,220))
sh.save(f"studio/work/a1/out/a1_tag{tagsel}_m{mir}.png")
print(len(ids))
