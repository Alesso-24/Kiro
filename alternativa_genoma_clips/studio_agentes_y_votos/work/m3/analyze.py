"""Analisis offline: porta motionFeed a Python y prueba metricas/umbrales alternativos sobre los mismos generadores."""
import sys, math, numpy as np
sys.argv=[sys.argv[0]]
import test_motion as T
FS=60.0; DT=1/FS
def lp(prev,x,tau): return prev+(x-prev)*(1-math.exp(-DT/tau))
def sim(sig, P):
    G=np.array([0,1,0.0]); sh=0; shm=0; ev=[]; fallT=0; falling=False; prevmag=1; cool={}
    up=0; wasup=False; down=np.array([0,1.0]); fb=0; flat=0
    def ready(k,i,ms):
        if k in cool and (i-cool[k])*1000/FS<ms: return False
        cool[k]=i; return True
    for i,a in enumerate(sig):
        G=G+(a-G)*(1-math.exp(-DT/0.25))
        mag=np.linalg.norm(a); dev=np.linalg.norm(a-G)
        sh_prev=sh
        if P.get('mode')=='mag':
            sh=lp(sh,abs(mag-1),P['tau'])
        else:
            sh=lp(sh,dev,0.14)
        sx,sy=-G[0],G[1]; n=math.hypot(sx,sy)
        if n>0.25: down=down+(np.array([sx,sy])/n-down)*0.25
        # caida/impacto
        if mag<P['fall_mag']:
            fallT+=DT
            if fallT>P['fall_t'] and not falling:
                falling=True
                if ready('f',i,800): ev.append((i,2))
        else:
            if falling and (dev>P['imp_after_fall_dev'] or mag>1.7) and ready('i',i,400): ev.append((i,3))
            falling=False; fallT=0
        gate = (P.get('gate') is None) or (sh_prev<P['gate'])
        if not falling and dev>P['imp_dev'] and mag>P['imp_mag'] and gate and ready('i',i,400): ev.append((i,3))
        if sh>P['strong']:
            if ready('s',i,1500): ev.append((i,1))
        elif sh>P['soft']:
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
        if G[2]>0.85 and sh<P['flat_sh']: flat+=DT
        else:
            if flat>1.5 and G[2]<0.5 and ready('p',i,4000): ev.append((i,7))
            flat=0
    return {w for (_,w) in ev}
BASE=dict(fall_mag=.38,fall_t=.09,imp_after_fall_dev=1.6,imp_dev=2.4,imp_mag=2.0,strong=2.0,soft=0.85,flat_sh=0.15)
def evaluate(P,seeds=20,verbose=True):
    res={}
    for (name,gen,dur,must,forbid,expect) in T.CASES:
        ok=0; seen=[0]*8
        for sd in range(seeds):
            sig=gen(np.random.default_rng(1000+sd),dur+T.PRE)
            got={w for w in sim(sig,P) } if False else None
            # solo eventos tras el lead-in: simular completo y filtrar
            g=simfull(sig,P)
            got={w for (i,w) in g if i>=int(T.PRE*FS)}
            for w in got: seen[w]+=1
            ok+=all(m in got for m in must) and not any(f in got for f in forbid)
        res[name]=(ok/seeds,seen)
    if verbose:
        for k,(r,s) in res.items(): print(f"{k:24s} {r*100:4.0f}%  {[EV for EV in s]}")
    return res
def simfull(sig,P):
    # version de sim() que devuelve lista (i,w)
    global sim
    return _sim_list(sig,P)
import inspect
src=inspect.getsource(sim).replace("def sim(","def _sim_list(").replace("return {w for (_,w) in ev}","return ev")
exec(src)
if __name__=="__main__":
    import json
    cands={
     "base":BASE,
     "A":{**BASE,'mode':'mag','tau':0.14,'soft':0.30,'strong':0.9,'imp_dev':1.5,'imp_mag':1.8,'gate':0.5},
    }
    for n,P in cands.items():
        print("==",n); evaluate(P)
