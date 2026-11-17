import ctypes, importlib.util, os, sys
from PIL import Image, ImageDraw
work = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("viewer", os.path.join(work, "sim", "viewer.py"))
V = importlib.util.module_from_spec(spec); spec.loader.exec_module(V)
d = V.load(); d.sim_look.argtypes = [ctypes.c_int, ctypes.c_int]
d.sim_pupil_name.restype = ctypes.c_char_p
d.sim_seed.argtypes = [ctypes.c_uint32]
FIRST, N = 8, 14
cols = [("pal a", 1, 0), ("pal b", 1, 7), ("curioso", 2, 3), ("orgulloso", 16, 5), ("sorprend", 15, 9),
        ("ansioso", 18, 11), ("triste", 10, 13), ("hype", 12, 2)]
sz = 84
sh = Image.new("RGB", (len(cols) * sz + 110, N * sz), (30, 30, 36)); dr = ImageDraw.Draw(sh)
for r in range(N):
    dr.text((4, r * sz + 36), f"{r} {d.sim_pupil_name(FIRST+r).decode()}", fill=(230, 230, 230))
    for c, (nm, ex, pal) in enumerate(cols):
        d.sim_init(4242); d.sim_seed(4242)
        d.sim_look(pal, FIRST + r); d.sim_set_expr(ex, 0, 0)
        t = 0
        for _ in range(140):
            t += 17; d.sim_frame(t, 1/60)
        sh.paste(V.round_img(d, 1).resize((sz, sz), Image.LANCZOS), (110 + c * sz, r * sz))
        if r == 0: dr.text((110 + c * sz + 4, 2), nm, fill=(255, 255, 120))
os.makedirs(os.path.join(work, "out"), exist_ok=True)
sh.save(os.path.join(work, "out", "u1_expresiones.png"))
