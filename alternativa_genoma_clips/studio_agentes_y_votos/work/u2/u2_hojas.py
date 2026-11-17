import ctypes, importlib.util, os, sys
from PIL import Image, ImageDraw
work = r"C:\Users\jordi\OneDrive\Escritorio\Alloy\studio\work\u2"
spec = importlib.util.spec_from_file_location("viewer", os.path.join(work, "sim", "viewer.py"))
V = importlib.util.module_from_spec(spec); spec.loader.exec_module(V)
d = V.load(); u32 = ctypes.c_uint32
d.sim_seed.argtypes=[u32]; d.sim_look.argtypes=[ctypes.c_int]*2; d.sim_pupil_name.restype=ctypes.c_char_p
SZ=104; first=8
def snap(pal, pu, ex):
    d.sim_init(4242); d.sim_seed(4242); d.sim_look(pal, pu); d.sim_set_expr(ex,0,0)
    t=0
    for _ in range(110):
        t+=17; d.sim_frame(t,1/60)
    return V.round_img(d,1).resize((SZ,SZ),Image.LANCZOS)
def sheet(cfgs, fname):
    n=14; sh=Image.new("RGB",(len(cfgs)*SZ+110,n*SZ),(30,30,36)); dr=ImageDraw.Draw(sh)
    for r in range(n):
        pu=first+r; dr.text((2,r*SZ+45),f"{pu} {d.sim_pupil_name(pu).decode()}"[:18],fill=(220,220,220))
        for c,(p,e) in enumerate(cfgs): sh.paste(snap(p,pu,e),(110+c*SZ,r*SZ))
    sh.save(os.path.join(work,"out",fname))
sheet([(p,1) for p in (0,2,4,6,8,10,12,14)],"u2_looks.png")
sheet([(5,e) for e in (3,4,5,9,11,13,15,16)],"u2_expr.png")
