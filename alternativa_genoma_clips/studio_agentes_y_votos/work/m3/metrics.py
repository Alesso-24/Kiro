import sys, math, numpy as np
sys.argv=[sys.argv[0]]
import test_motion as T
FS=60.0; DT=1/FS
def lpf(x,tau):
    k=1-math.exp(-DT/tau); y=np.zeros(len(x)); a=0
    for i,v in enumerate(x): a+=(v-a)*k; y[i]=a
    return y
def met(sig):
    mag=np.linalg.norm(sig,axis=1)
    G=np.zeros_like(sig); g=np.array([0,1.0,0]); k=1-math.exp(-DT/0.25)
    for i,a in enumerate(sig): g=g+(a-g)*k; G[i]=g
    dev=np.linalg.norm(sig-G,axis=1)
    jerk=np.r_[0,np.linalg.norm(np.diff(sig,axis=0),axis=1)]
    return dict(M0_dev=lpf(dev,.14), M1_magdev=lpf(np.abs(mag-1),.14), M3_jerk=lpf(jerk,.25), M4_magdev_sq=np.sqrt(lpf((mag-1)**2,.2)))
pre=int(T.PRE*FS)
print("caso | metrica: min/med/max de picos (20 semillas)")
for (name,gen,dur,must,forbid,exp) in T.CASES:
    acc={}
    for sd in range(20):
        sig=gen(np.random.default_rng(1000+sd),dur+T.PRE)
        for k,v in met(sig).items(): acc.setdefault(k,[]).append(v[pre:].max())
    print(f"{name:24s} "+"  ".join(f"{k}={min(v):.2f}/{np.median(v):.2f}/{max(v):.2f}" for k,v in acc.items()))
