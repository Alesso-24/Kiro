p='dirfix/EyesCore.h'
s=open(p,encoding='utf-8',newline='').read().replace(chr(13)+chr(10),chr(10))
a=s.index('void directorClip(uint32_t now) {')
b=s.index('// --- secuencias ---')
new='''static float clipWeight(int i) {
  switch (CLIPS[i].tag) {
    case 0: return 1.0f;
    case 1: return 0.5f + G.trait[5] + G.trait[0];
    case 2: return 0.1f + 0.8f * G.trait[2];
    case 3: return 0.1f + 0.8f * G.trait[1];
    case 4: return 0.1f + 0.8f * G.trait[3];
    case 5: return 0.5f + G.trait[4];
    default: return 0.0f;      // 6 = energia/sistema, 7 = reaccion de movimiento: solo por evento
  }
}
void directorClip(uint32_t now) {
  nextClip = now + (uint32_t)((2200 + 4200 * frand01()) / (0.55f + G.trait[0]));
  float tot = 0;
  for (int i = 0; i < NCLIPS; i++) tot += clipWeight(i);
  if (tot <= 0) return;
  float r = frand01() * tot;
  int pick = -1;                                   // sin arreglo en la pila; nunca elige peso 0
  for (int i = 0; i < NCLIPS; i++) {
    float w = clipWeight(i);
    if (w <= 0) continue;
    pick = i;
    if (r < w) break;
    r -= w;
  }
  if (pick >= 0) clipPlay(pick, now, frand() > 0, 0.9f + 0.25f * frand01());
}

'''
s=s[:a]+new+s[b:]
open(p,'w',encoding='utf-8',newline='').write(s)
print('ok')
