"""Port a Python de la logica PROPUESTA para Motion.h (mismos nombres) + evaluacion con los generadores de test_motion.py"""
import sys, math, numpy as np
sys.argv=[sys.argv[0]]
import test_motion as T
FS=60.0; DT=1/FS
P0=dict(soft=0.50, imp_delay=6, imp_confirm_sh=0.7, strong=1.55, rev_big=0.35, rev_soft=3, rev_strong=4, imp_dev=1.5, imp_mag=1.8, imp_gate=0.5,
        imp_block_shake=1.0, pick_g=0.70, fall_mag=0.38, fall_t=0.09)
def sim(sig,P):
    G=np.array([0,1,0.0]); sh=0.0; ev=[]; cool={}; fallT=0; falling=False
    down=np.array([0,1.0]); up=0; wasup=False; fb=0; flat=0; wasflat=False
    lastbig=None; revs=[]; block=-1; cand=None
    def ready(k,i,ms):
        if k in cool and (i-cool[k])*1000/FS<ms: return False
        cool[k]=i; return True
    for i,a in enumerate(sig):
        G=G+(a-G)*(1-math.exp(-DT/0.25))
        mag=np.linalg.norm(a); d=a-G; dev=np.linalg.norm(d)
        shp=sh; sh=sh+(dev-sh)*(1-math.exp(-DT/0.14))
        if dev>P['rev_big']:
            if lastbig is not None and np.dot(d,lastbig)<0: revs.append(i)
            lastbig=d
        revs=[r for r in revs if i-r<0.8*FS]
        sx,sy=-G[0],G[1]; n=math.hypot(sx,sy)
        if n>0.25: down=down+(np.array([sx,sy])/n-down)*0.25
        if mag<P['fall_mag']:
            fallT+=DT
            if fallT>P['fall_t'] and not falling:
                falling=True
                if ready('f',i,800): ev.append((i,2)); block=i+int(P['imp_block_shake']*FS)
        else:
            if falling and (dev>1.6 or mag>1.7) and ready('i',i,400): ev.append((i,3)); block=i+int(P['imp_block_shake']*FS)
            falling=False; fallT=0
        if not falling and cand is None and dev>P['imp_dev'] and mag>P['imp_mag'] and shp<P['imp_gate']:
            cand=i
        if cand is not None and i>=cand+P['imp_delay']:      # confirma 100 ms despues: un golpe aislado, no el inicio de una sacudida
            if sh<P['imp_confirm_sh'] and len(revs)<3 and ready('i',i,400):
                ev.append((i,3)); block=i+int(P['imp_block_shake']*FS)
            cand=None
        if i>=block:
            if sh>P['strong'] and len(revs)>=P['rev_strong']:
                if ready('s',i,1500): ev.append((i,1))
            elif sh>P['soft'] and len(revs)>=P['rev_soft']:
                if ready('s',i,1500): ev.append((i,0))
        upside = down[1]<-0.65 and abs(G[2])<0.6
        if upside:
            up+=DT
            if up>1.2 and not wasup:
                wasup=True
                if ready('u',i,3000): ev.append((i,4))
        else:
            if wasup and down[1]>0.5: wasup=False; ev.append((i,5))
            up=0
        if G[2]<-0.8:
            fb+=DT
            if fb>1.0 and ready('b',i,10000): ev.append((i,6))
        else: fb=0
        if G[2]>0.85 and sh<0.15: flat+=DT
        elif G[2]>0.85: pass
        else: flat=0 if not wasflat else flat
        if flat>1.5: wasflat=True
        if wasflat and (G[2]<P['pick_g'] or (G[2]<0.85 and sh>0.3)):
            wasflat=False; flat=0
            if ready('p',i,4000): ev.append((i,7))
        elif wasflat and G[2]<0.5: wasflat=False
    return ev
def evaluate(P,seeds=20):
    out=[]
    for (name,gen,dur,must,forbid,expect) in T.CASES:
        ok=0; seen=[0]*8
        for sd in range(seeds):
            sig=gen(np.random.default_rng(1000+sd),dur+T.PRE)
            got={w for (i,w) in sim(sig,P) if i>=int(T.PRE*FS)}
            for w in got: seen[w]+=1
            ok+=all(m in got for m in must) and not any(f in got for f in forbid)
        out.append((name,ok/seeds,seen))
    return out
if __name__=="__main__":
    for name,r,seen in evaluate(P0): print(f"{name:24s} {r*100:4.0f}% {seen}")
