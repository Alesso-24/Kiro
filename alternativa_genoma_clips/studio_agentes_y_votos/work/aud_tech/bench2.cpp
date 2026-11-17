#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <cstdint>
#include "EyesCore.h"
#undef min
#undef max
static uint16_t buf[240*240];
static uint32_t T=0;
struct Scn { const char* name; int expr; int np; int ptype; int nclips; bool tone; int sp; int pu; bool big; };
int main(int argc,char**argv){
  const char* tag = argv[1]; int nfr = argc>2?atoi(argv[2]):400; int dump = argc>3?atoi(argv[3]):40;
  fb=buf;
  int spMax=26, puG=34, puRing=35;
  std::vector<Scn> S = {
    {"A nominal despierto",X_AWAKE,0,0,0,false,0,0,false},
    {"C especie max+pupila brillo2t",X_AWAKE,0,0,0,true,spMax,puG,false},
    {"D C+ojos clamp 74x92",X_AWAKE,0,0,0,true,spMax,puG,true},
    {"E D+3 clips",X_AWAKE,0,0,3,true,spMax,puG,true},
    {"F E+56 part mixtas",X_AWAKE,56,-1,3,true,spMax,puG,true},
    {"G E+56 chispas",X_AWAKE,56,PT_SPARK,3,true,spMax,puG,true},
    {"H E+56 corazones",X_AWAKE,56,PT_HEART,3,true,spMax,puG,true},
    {"I 25 part mixtas",X_AWAKE,25,-1,1,false,0,0,false},
    {"J mareado",X_DIZZY,0,0,0,false,0,0,false},
    {"L enojado",X_ANGRY,0,0,0,false,0,0,false},
    {"K dormido",X_SLEEP,0,0,0,false,0,0,false},
    {"S sorprendido pupila anillo 2t",X_SURPRISE,0,0,0,true,0,puRing,false},
  };
  char fn[100]; snprintf(fn,sizeof fn,"frames_%s.raw",tag); FILE* fo=fopen(fn,"wb");
  for (auto& s : S) {
    rng=12345; T=0; memset(parts,0,sizeof parts);
    coreInit(T,2024); genomeFromSeed(2024); autoOn=false; G.species=s.sp; G.pupil=s.pu; G.toneMode=s.tone?1:0;
    evSet(s.expr,T,0); for(int i=0;i<90;i++){T+=17;coreFrame(T,1.0f/60);}
    for (int k=0;k<s.nclips;k++) clipPlay(k*17%NCLIPS,T,k&1,0.5f);
    std::vector<double> us;
    for (int f=0;f<nfr;f++){
      if (s.np) for (int i=0;i<NPART;i++) if (i<s.np){ Part&p=parts[i]; if(!p.on){ p={(float)(40+(i*37)%160),(float)(30+(i*53)%170),0,0,5.0f,5.0f,10.0f,0.3f*i,1.0f,(uint8_t)(s.ptype<0?i%6:s.ptype),(uint8_t)(i%5),1}; } p.life=5; }
      if (s.nclips) for(int k=0;k<3;k++) if(!slots[k].on) clipPlay((k*17+f)%NCLIPS,T,k&1,0.5f);
      T+=17; updateAnimation(T);
      if (s.big){ cur[P_RX]=74; cur[P_RY]=92; }
      memset(fb,0,sizeof buf); dirtyY0=H; dirtyY1=-1;
      auto a=std::chrono::steady_clock::now(); renderFrame(T);
      us.push_back(std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-a).count());
      if (f<dump) fwrite(buf,2,240*240,fo);
    }
    std::sort(us.begin(),us.end());
    printf("%s|%s|%.1f|%.1f\n",tag,s.name,us[us.size()/2],us[us.size()*9/10]);
  }
  fclose(fo);
}
