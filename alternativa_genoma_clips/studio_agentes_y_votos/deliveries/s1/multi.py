import ctypes, importlib.util, os, sys
from PIL import Image, ImageDraw
work=os.path.abspath(sys.argv[1]); exprs=[int(x) for x in sys.argv[2].split(',')]; out=sys.argv[3]
lo=int(sys.argv[4]); hi=int(sys.argv[5])
spec=importlib.util.spec_from_file_location("viewer",os.path.join(work,"sim","viewer.py"))
V=importlib.util.module_from_spec(spec); spec.loader.exec_module(V); d=V.load()
d.sim_set_species.argtypes=[ctypes.c_int]; d.sim_look.argtypes=[ctypes.c_int,ctypes.c_int]
d.sim_species_name.restype=ctypes.c_char_p; d.sim_name.restype=ctypes.c_char_p
sz=96; n=hi-lo
sh=Image.new("RGB",(len(exprs)*sz+90,n*sz),(30,30,36)); dr=ImageDraw.Draw(sh)
for r,s in enumerate(range(lo,hi)):
    dr.text((2,r*sz+40),d.sim_species_name(s).decode()[:14],fill=(230,230,230))
    for c,x in enumerate(exprs):
        d.sim_init(4242); d.sim_seed(4242); d.sim_look(0,5); d.sim_set_species(s)
        d.sim_set_expr(x,0,0); t=0
        for _ in range(120):
            t+=17; d.sim_frame(t,1/60)
        sh.paste(V.round_img(d,1).resize((sz,sz)),(90+c*sz,r*sz))
sh.save(out)
