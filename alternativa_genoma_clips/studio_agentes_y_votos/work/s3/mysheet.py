import ctypes, importlib.util, os, sys
from PIL import Image, ImageDraw
work = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("viewer", os.path.join(work, "sim", "viewer.py"))
V = importlib.util.module_from_spec(spec); spec.loader.exec_module(V)
d = V.load()
d.sim_set_species.argtypes=[ctypes.c_int]; d.sim_species_name.restype=ctypes.c_char_p
d.sim_seed.argtypes=[ctypes.c_uint32]
EX=[0,3,4,9,10,11,13,14]
first=int(sys.argv[1]) if len(sys.argv)>1 else 6
n=d.sim_nspecies()
rows=range(first,n)
S=0.5; w=int(240*S)
sheet=Image.new("RGB",(w*len(EX)+90,w*len(rows)),(20,20,24))
dr=ImageDraw.Draw(sheet)
for r,i in enumerate(rows):
    for c,x in enumerate(EX):
        d.sim_init(7); d.sim_look(r*3+1+c%1,r%5)
        d.sim_set_species(i)
        t=0; t=V.run_frames(d,t,30)
        d.sim_set_expr(x,t,6000); t=V.run_frames(d,t,70)
        sheet.paste(V.round_img(d,1).resize((w,w)),(90+c*w,r*w))
    dr.text((4,r*w+w//2),d.sim_species_name(i).decode()+f" #{i}",fill=(255,255,255))
sheet.save(os.path.join(work,"out","s3_hoja.png")); print(sheet.size)
