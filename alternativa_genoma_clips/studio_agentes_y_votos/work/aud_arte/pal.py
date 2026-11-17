import re,glob,colorsys,math
rows=[]
for f in ['pal_base.pal.txt','p1_calidas.pal.txt','p2_frias.pal.txt','p3_raras.pal.txt']:
    for l in open(f,encoding='utf8'):
        if l.startswith('//') or '|' not in l: continue
        p=[x.strip() for x in l.split('|')]
        rows.append((p[0],p[1],p[2],p[3],p[4] if len(p)>4 else ''))
print(len(rows))
def rgb(h): return tuple(int(h[i:i+2],16) for i in (1,3,5))
def lum(c):
    def f(v):
        v/=255; return v/12.92 if v<=.03928 else ((v+.055)/1.055)**2.4
    return .2126*f(c[0])+.7152*f(c[1])+.0722*f(c[2])
def cr(a,b):
    la,lb=lum(a),lum(b); 
    if la<lb: la,lb=lb,la
    return (la+.05)/(lb+.05)
out=[]
for i,(n,r,e,p,a) in enumerate(rows):
    E,P=rgb(e),rgb(p)
    out.append((i,n,e,p,round(cr(E,P),2),round(lum(E),3),round(lum(P),3)))
for o in out: print(*o)
print('--- dup')
for i in range(len(rows)):
  for j in range(i+1,len(rows)):
    d=math.dist(rgb(rows[i][2]),rgb(rows[j][2]))+math.dist(rgb(rows[i][3]),rgb(rows[j][3]))
    if d<60: print(i,rows[i][0],j,rows[j][0],round(d))
