import ctypes, importlib.util, os, sys
from PIL import Image, ImageDraw
work = os.path.abspath(sys.argv[1])
names = sys.argv[3:]  # optional clip name filter
outname = sys.argv[2]
spec = importlib.util.spec_from_file_location("viewer", os.path.join(work, "sim", "viewer.py"))
V = importlib.util.module_from_spec(spec); spec.loader.exec_module(V)
d = V.load()
u32 = ctypes.c_uint32
d.sim_seed.argtypes = [u32]
d.sim_clip_play.argtypes = [ctypes.c_int, u32, ctypes.c_int, ctypes.c_float]
d.sim_clip_name.restype = ctypes.c_char_p
n = d.sim_nclips()
ids = [i for i in range(n) if d.sim_clip_tag(i) == 3 and (not names or d.sim_clip_name(i).decode() in names)]
NF, SZ = 16, 76
sh = Image.new("RGB", (NF * SZ, len(ids) * (SZ + 12)), (30, 30, 36))
dr = ImageDraw.Draw(sh)


def run(t, k):
    for _ in range(k):
        t += 17
        d.sim_frame(t, 1 / 60)
    return t


for r, i in enumerate(ids):
    for seed in range(3):
        d.sim_init(4242 + seed*31); d.sim_seed(4242 + seed*31)
        d.sim_set_expr(1, 0, 0); t = run(0, 75)
        d.sim_clip_play(i, t, 0, 1.0)
        dur = d.sim_clip_dur(i) + 150
        step = dur / NF
        fr = []
        bad = False
        tt = t
        for f in range(NF):
            tt = run(tt, max(1, int(step / 17)))
            if d.sim_expr() != 1: bad = True
            fr.append(V.round_img(d, 1).resize((SZ, SZ), Image.LANCZOS))
        if not bad: break
    for f, im in enumerate(fr):
        sh.paste(im, (f * SZ, r * (SZ + 12)))
    dr.text((2, r * (SZ + 12) + SZ), f"{d.sim_clip_name(i).decode()} {d.sim_clip_dur(i)}ms seed{seed}", fill=(220, 220, 220))
sh.save(os.path.join(work, "out", outname))
print("ok", len(ids))
