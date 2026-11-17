#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstdint>
#include "EyesCore.h"
#undef min
#undef max
static uint16_t B[240*240];
int main(){
  for (uint32_t trial=0; trial<3; trial++){
    fb=B; rng=12345; uint32_t T = 4294967296u - 14000u + trial*1700u;
    coreInit(T,55); genomeFromSeed(55); autoOn=true;
    nextBlink=T+500; nextIdleAct=T+3000; nextForm=T+1500; nextClip=T+2000; lastActivity=T;
    printf("trial %u: wrap en t=+%.1f s.  por segundo: [parpadeos chooseForm cambiosExpr clipsDirector]\n", trial, (4294967296u-T)/1000.0);
    uint32_t nf=nextForm, nc=nextClip; uint8_t pe=expr; bool pb=false;
    int bl=0,fm=0,ex=0,cl=0; int sec=0;
    for (int f=0; f<60*30; f++){
      T += 17; lastActivity=T; if (!isAsleep()) {} 
      coreFrame(T, 1.0f/60);
      bool b=eye[0].blinkT>0; if (b&&!pb) bl++; pb=b;
      if (nextForm!=nf){fm++; nf=nextForm;}
      if (nextClip!=nc){cl++; nc=nextClip;}
      if (expr!=pe){ex++; pe=expr;}
      if ((f+1)%59==0){ printf("  s%02d %s now=%10u [%2d %2d %2d %2d]\n", sec, (T<100000u && sec>0 && T < 60000u)?"post":"    ", T, bl,fm,ex,cl); bl=fm=ex=cl=0; sec++; }
    }
  }
}
