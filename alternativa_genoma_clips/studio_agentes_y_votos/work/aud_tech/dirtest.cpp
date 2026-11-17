#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstdint>
#include "EyesCore.h"
#undef min
#undef max
static uint16_t B[240*240];
static uint32_t inv32(uint32_t a){ uint32_t x=a; for(int i=0;i<5;i++) x*=2-a*x; return x; }
int main(){
  fb=B; coreInit(0,1);
  uint32_t ia=inv32(1664525u), c=1013904223u;
  int badLast=0, badTag=0, n=0, zeroW=0;
  for (uint32_t s=1;s<=20000;s++){
    genomeFromSeed(s*2654435761u);
    for (int mode=0; mode<2; mode++){
      // rng3 con (rng3>>8)&0xFFFF == 65535 (r=1.0) o 0 (r=0.0)
      uint32_t rng3 = mode==0 ? 0x00FFFF00u : 0x00000000u;
      uint32_t rng2 = (rng3 - c)*ia, rng1=(rng2 - c)*ia;
      rng=rng1; memset(slots,0,sizeof slots);
      directorClip(100000);
      int id=-1; for(int i=0;i<3;i++) if(slots[i].on) id=slots[i].id;
      n++;
      if (id==NCLIPS-1 && mode==0) badLast++;
      if (id>=0 && CLIPS[id].tag>=6){ badTag++; if(zeroW<3){printf("  mode r=%s seed=%u -> clip #%d '%s' tag %d\n", mode?"0.0":"1.0", s, id, CLIPS[id].name, CLIPS[id].tag); zeroW++;} }
    }
  }
  printf("director: %d llamadas con r forzado a 0.0/1.0: eligio el ultimo clip (#%d %s tag %d): %d veces; eligio clip tag>=6: %d veces\n", n, NCLIPS-1, CLIPS[NCLIPS-1].name, CLIPS[NCLIPS-1].tag, badLast, badTag);
}
