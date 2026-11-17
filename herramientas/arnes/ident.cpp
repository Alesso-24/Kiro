#include <cstdio>
#include <cmath>
#include "../../firmware/Kiro_Eyes/EyesCore.h"
static uint16_t buf[240*240];
static void show(const char* n){ printf("%s: c1=%.0f,%.0f,%.0f c2=%.0f two=%d mir=%d scale=%.4f gap=%.2f pd=%.3f pst=%d eye0.gene=%.3f eye0.k=%.1f\n",n,cr.c1.r,cr.c1.g,cr.c1.b,cr.c2.r,(int)cr.two,(int)cr.mir,cr.scale,cr.gap,cr.pd,(int)cr.pstyle,eyes[0].gene,eyes[0].k);}
int main(){
  fb=buf; birthSkip=true; uint32_t ns=123456789u;
  coreInit(0,ns); show("arranque (reinicio)");
  rng=777; evNewCreature(0, ns*2246822519u+374761393u); show("renacer (evNewCreature)");
}
