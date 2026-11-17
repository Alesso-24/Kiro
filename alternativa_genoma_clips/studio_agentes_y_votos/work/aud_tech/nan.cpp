#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstdint>
#include "EyesCore.h"
#undef min
#undef max
static uint16_t B[240*240];
int main(){
  fb=B; coreInit(0,9); uint32_t T=0;
  for(int i=0;i<100;i++){T+=16;coreFrame(T,1/60.f);}
  coreFrame(T+=16, NAN);
  printf("tras 1 cuadro con dt=NaN: DT=%g cur[P_RX]=%g\n", DT, cur[P_RX]);
  for(int i=0;i<600;i++){T+=16;coreFrame(T,1/60.f);}
  int nz=0; for(int i=0;i<240*240;i++) nz+=B[i]!=0;
  printf("600 cuadros normales despues: DT=%g cur[P_RX]=%g pixeles no negros=%d oob=%llu plot=%llu dirty=%d..%d\n", DT, cur[P_RX], nz, g_oob, g_plot, dirtyY0, dirtyY1);
}
