#include <cstdio>
#include <cmath>
#include <cstdlib>
#include "../../firmware/Kiro_Eyes/EyesCore.h"
static uint16_t buf[240*240];
int main(int argc,char**argv){
  fb=buf; birthSkip=true; srand(argc>1?atoi(argv[1]):1); coreInit(0,98765u);
  uint32_t now=0; float dt=1.0f/60; double tot=0; int badO=0,badP=0,badL=0,badA=0,awakeFrames=0; float runO=0,runP=0,maxRunO=0,maxRunP=0; int hist[NEXPR]={0};
  for(int fr=0;fr<60*1800;fr++){            // 30 min simulados
    now+=16; if(rand()%900==0){int r=rand()%4; if(r==0)evEmotion(now); if(r==1)evThumbs(now); if(r==2)evPoke(now); if(r==3)evNextExpr(now);} 
    coreFrame(now,dt); hist[curX]++;
    if(curX==X_AWAKE&&birthVar<0){awakeFrames++;
      float o=eyes[0].o, py=eyes[0].p[1]*(0.5f+0.5f*1);
      if(o<.75f){runO+=dt;} else runO=0; if(runO>maxRunO)maxRunO=runO;
      if(eyes[0].p[1]<36||eyes[1].p[1]<36){runP+=dt;} else runP=0; if(runP>maxRunP)maxRunP=runP;
      if(Vv[K_LID]>.1f)badL++; if(Vv[K_ARC]>.1f)badA++;
    } else {runO=0;runP=0;}
  }
  printf("frames despierto=%d  max seguido ojo<75%%=%.1fs  max seguido ry<36=%.1fs  lid>.1:%d arc>.1:%d\n",awakeFrames,maxRunO,maxRunP,badL,badA);
  for(int i=0;i<NEXPR;i++)printf("%s=%.0fs ",XNAME[i],hist[i]/60.0);
  printf("\n");
}
