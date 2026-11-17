#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include "EyesCore.h"
#undef min
#undef max
static uint16_t buf[240*240];
int main(int argc,char**argv){
  const char* tag=argv[1]; int nfr=200; double SPIHZ=31.25e6; fb=buf; uint32_t T=0;
  double tot=0, totBytes=0; int viol=0; uint64_t hsh=1469598103934665603ULL;
  for (int rep=0; rep<3; rep++) {   // 3 criaturas distintas
   for (int x=1;x<NEXPR;x++){
    rng=12345; T=0; memset(parts,0,sizeof parts); coreInit(T,2024+rep*77); genomeFromSeed(2024+rep*77); autoOn=true; evSet(x,T,0);
    for(int i=0;i<90;i++){T+=17;coreFrame(T,1.0f/60);}
    int py0=H,py1=-1; double rows=0;
    for (int f=0;f<nfr;f++){
      T+=17; coreFrame(T,1.0f/60);
      int r0=H,r1=-1; for(int y=0;y<240;y++) for(int xx=0;xx<240;xx++) if(buf[y*240+xx]){ r0=std::min(r0,y); r1=std::max(r1,y);} 
      if (r1>=0 && (r0<dirtyY0 || r1>dirtyY1)) viol++;
      for(int i=0;i<240*240;i++){ hsh^=buf[i]; hsh*=1099511628211ULL; }
      int y0=std::min(dirtyY0,py0), y1=std::max(dirtyY1,py1); if(dirtyY1<0){y0=py0;y1=py1;}
      rows += y1>=y0 ? y1-y0+1 : 0;
      py0 = dirtyY0<=dirtyY1?dirtyY0:H; py1=dirtyY1;
    }
    tot += rows/nfr; totBytes++;
   }
  }
  double avgRows = tot/totBytes; double ms = avgRows*240*16/SPIHZ*1000+0.25;
  printf("%s: filas enviadas promedio (20 expr x 3 criaturas, auto on) = %.1f -> SPI %.2f ms/cuadro (%.1f fps techo); violaciones de dirty (pixel fuera de filas marcadas)=%d; hash de pixeles=%016llx\n", tag, avgRows, ms, std::min(60.0,1000/ms), viol, (unsigned long long)hsh);
}
