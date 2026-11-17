import ctypes, importlib.util, os, sys
from PIL import Image, ImageDraw
work=os.path.dirname(os.path.abspath(__file__))
spec=importlib.util.spec_from_file_location("viewer",os.path.join(work,"sim","viewer.py"))
V=importlib.util.module_from_spec(spec); spec.loader.exec_module(V); d=V.load()
d.sim_species_name.restype=ctypes.c_char_p; d.sim_set_species.argtypes=[ctypes.c_int]
d.sim_look.argtypes=[ctypes.c_int,ctypes.c_int]; d.sim_seed.argtypes=[ctypes.c_uint32]
first=int(sys.argv[1]) if len(sys.argv)>1 else 6
exprs=[1,2,4,5,6,10,12,15,16,18]
sz=88
sh=Image.new("RGB",(len(exprs)*sz+110,18*sz),(30,30,36)); dr=ImageDraw.Draw(sh)
for k in range(18):
    s=first+k
    dr.text((2,k*sz+40),d.sim_species_name(s).decode(),fill=(220,220,220))
    for j,x in enumerate(exprs):
        d.sim_init(4242); d.sim_seed(4242); d.sim_look((k*3)%16,k%8); d.sim_set_species(s)
        t=0
        for _ in range(20): t+=16; d.sim_frame(t,1/60)
        d.sim_set_expr(x,t,0)
        for _ in range(70): t+=16; d.sim_frame(t,1/60)
        sh.paste(V.round_img(d,1).resize((sz,sz),Image.LANCZOS),(110+j*sz,k*sz))
sh.save(os.path.join(work,"out","s2_expresiones.png"))
