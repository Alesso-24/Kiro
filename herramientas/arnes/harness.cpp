#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "../../firmware/Kiro_Eyes/EyesCore.h"
static uint16_t buf[240*240];
static void dump(const char* name){ FILE* f=fopen(name,"wb"); fwrite(buf,2,240*240,f); fclose(f); }
int main(int argc,char**argv){
  // uso: harness seed modo...   modo: "e:<id>:<segundos>" salida out_<id>.raw ; "b:<seg1,seg2..>" nacimiento
  fb=buf; uint32_t seed=argc>1?atoi(argv[1]):7; birthSkip=true; coreInit(0,seed);
  uint32_t now=0; float dt=1.0f/60;
  for(int a=2;a<argc;a++){
    int id; float sec;
    if(sscanf(argv[a],"e:%d:%f",&id,&sec)==2){
      evSet(1,now,0); setExpr(id,false);
      for(float t=0;t<sec;t+=dt){ now+=16; coreFrame(now,dt);} char n[64]; snprintf(n,64,"out_e%d.raw",id); dump(n);
    } else if(argv[a][0]=='b'){
      evBirth(now,0); float marks[]={5,9.8,10.3,11.5,13.5,16,19,24,31,32.2,33.4,34.4,35.4,36.4,38}; int mi=0; float t=0;
      while(mi<15){ coreFrame(now+=16,dt); t+=dt; if(t>=marks[mi]){char n[64];snprintf(n,64,"out_b%d.raw",mi);dump(n);mi++;} }
    }
  }
  printf("ok nan=%d\n",(int)!std::isfinite(St));
}
