// Alloy_Eyes - ojos de gelatina "Lil Guy" a color para GC9A01 (240x240 redonda) con XIAO RP2350
//
// Fluidez / sin parpadeo:
//   * Se dibuja TODO en un framebuffer RGB565 en RAM (doble buffer) y nunca se toca la pantalla
//     mientras se dibuja (nada de fillScreen directo -> no hay parpadeo).
//   * El cuadro terminado se manda por DMA (SPI0 a 16 bits) mientras la CPU ya dibuja el siguiente.
//   * Solo se envian las filas que cambiaron (la zona de los ojos), no los 240 renglones.
//   * Bordes con anti-aliasing (cobertura por pixel), fisica con paso de tiempo real (dt).
//
// Pines: SCL=D8(GPIO2) SDA=D10(GPIO3) DC=D0(GPIO26) CS=D1(GPIO27) RST=D2(GPIO28)
//        Boton azul=D6(GPIO0)  Boton verde=D7(GPIO1)  (a GND, INPUT_PULLUP)
//
// Botones:  azul corto = tocar (3 seguidos = amor) | azul largo = piquete
//           verde corto = jugar (hype)             | verde largo = siguiente expresion (demo de todas)
//           azul + verde juntos = enamorado
//           Tras 60 s sin tocar nada se duerme (Zzz); cualquier boton lo despierta.
// Serial (115200) para pruebas: 1..9 expresion, t color, p piquete, w guino, d info de rendimiento.
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"
#include <math.h>

#define PIN_SCK 2
#define PIN_MOSI 3
#define PIN_DC 26
#define PIN_CS 27
#define PIN_RST 28
#define BTN_BLUE 0
#define BTN_GREEN 1
uint32_t spiHz = 31300000UL;   // con CPU a 250 MHz: 25 / 31 / 42 / 62 MHz posibles. Serial "f" lo cambia en vivo.
#define FRAME_US 16667UL  // objetivo 60 fps

#include "EyesCore.h"

Adafruit_GC9A01A tft(&SPI, PIN_DC, PIN_CS, PIN_RST);
static uint16_t bufA[W * H] __attribute__((aligned(4)));
static uint16_t bufB[W * H] __attribute__((aligned(4)));
uint16_t* shown = bufB;   // el que se esta mandando / ya mandado
int dmaCh = -1;
bool dmaBusy = false;

// ---------- Pantalla: envio por DMA ----------
void pushWait() {
  if (!dmaBusy) return;
  uint32_t tw = micros();   // con limite de tiempo: si el DMA/SPI se atora, se aborta en vez de congelar el firmware
  while (dma_channel_is_busy(dmaCh) && (micros() - tw) < 150000UL) {}
  if (dma_channel_is_busy(dmaCh)) dma_channel_abort(dmaCh);
  tw = micros();
  while (spi_is_busy(spi0) && (micros() - tw) < 50000UL) {}
  // IMPORTANTE: el core NO restaura el formato si los ajustes no cambian; sin esto los comandos del
  // siguiente cuadro salen de 16 bits, la ventana se corrompe y la pantalla se congela / queda negra.
  spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  tft.endWrite();
  dmaBusy = false;
}

void pushStart(const uint16_t* buf, int y0, int y1) {
  int rows = y1 - y0 + 1;
  tft.startWrite();
  spi_set_baudrate(spi0, spiHz);
  tft.setAddrWindow(0, y0, W, rows);
  gpio_put(PIN_DC, 1);
  spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);  // 16 bits = RGB565 big-endian directo
  dma_channel_config c = dma_channel_get_default_config(dmaCh);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
  channel_config_set_dreq(&c, spi_get_dreq(spi0, true));
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, false);
  dma_channel_configure(dmaCh, &c, &spi_get_hw(spi0)->dr, buf + y0 * W, rows * W, true);
  dmaBusy = true;
}


struct Btn { uint8_t pin; bool down; bool longDone; uint32_t t0; uint32_t lastChg; };
Btn bBlue = {BTN_BLUE, false, false, 0, 0}, bGreen = {BTN_GREEN, false, false, 0, 0};
bool comboDone = false;

void pollButtons(uint32_t now) {
  Btn* bs[2] = {&bBlue, &bGreen};
  bool prevDown[2] = {bBlue.down, bGreen.down};
  bool rel[2] = {false, false};
  for (int i = 0; i < 2; i++) {
    Btn& b = *bs[i];
    bool p = digitalRead(b.pin) == LOW;
    if (p != b.down && now - b.lastChg > 20) {
      b.lastChg = now;
      b.down = p;
      if (p) { b.t0 = now; b.longDone = false; } else rel[i] = true;
    }
  }
  // combo: los dos abajo a la vez
  if (bBlue.down && bGreen.down && !comboDone) {
    comboDone = true;
    bBlue.longDone = bGreen.longDone = true;
    evSet(X_LOVE, now, 4000);
  }
  if (!bBlue.down && !bGreen.down) comboDone = false;
  // largos
  for (int i = 0; i < 2; i++) {
    Btn& b = *bs[i];
    if (b.down && !b.longDone && now - b.t0 > (i == 0 ? 800 : 700)) {
      b.longDone = true;
      wake(now);
      if (i == 0) evPoke(now); else evNextExpr(now);   // verde largo = recorrer TODAS las expresiones
    }
  }
  // cortos (al soltar)
  for (int i = 0; i < 2; i++) {
    if (!rel[i] || bs[i]->longDone) continue;
    if (i == 0) evTap(now); else evPlay(now);   // azul = tocar (3 = amor), verde = jugar
  }
  (void)prevDown;
}

// ---------- Rendimiento ----------
uint32_t perfT0 = 0, perfFrames = 0, perfRenderSum = 0, perfRenderMax = 0, perfWaitSum = 0, perfRowsSum = 0;
bool perfOn = true;

void pollSerial(uint32_t now) {
  static bool usbOn = false;
  while (Serial.available()) {
    int c = Serial.read();
    if (c >= '1' && c <= '9') { evSet(c - '0', now, 4000); Serial.printf("expr %d\n", c - '0'); }
    else if (c == 't') evTheme(now);
    else if (c == 'p') evPoke(now);
    else if (c == 'w') evWink();
    else if (c == 'g') evPlay(now);
    else if (c == 'u') { usbOn = !usbOn; evUsb(now, usbOn); }
    else if (c == ' ') evNextExpr(now);
    else if (c == 'f') { static int fi = 1; const uint32_t F[4] = {25000000UL, 31300000UL, 41700000UL, 62500000UL}; fi = (fi + 1) % 4; spiHz = F[fi]; Serial.printf("SPI %lu Hz\n", (unsigned long)spiHz); }
    else if (c == 'n') evNewCreature(now, micros() ^ now);
    else if (c == 'c') { static int ci = 0; clipPlay(ci, now, (ci & 1) != 0, 1.0f); ci = (ci + 1) % NCLIPS; }
    else if (c == 'j') evLoud(now);
    else if (c == 'k') evShake(now, false);
    else if (c == 'K') evShake(now, true);
    else if (c == 'v') evThumbs(now);
    else if (c == 'x') evFlick(now);
    else if (c == 'i') { static bool cl = false; cl = !cl; evCold(now, cl); }
    else if (c == 'd') perfOn = !perfOn;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_BLUE, INPUT_PULLUP);
  pinMode(BTN_GREEN, INPUT_PULLUP);
  // El core deja clk_peri a 48 MHz (SPI maximo 24 MHz). Lo pasamos a la frecuencia de la CPU.
  uint32_t fsys = clock_get_hz(clk_sys);
  clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, fsys, fsys);
  SPI.setSCK(PIN_SCK);
  SPI.setTX(PIN_MOSI);
  tft.begin(spiHz);
  tft.setRotation(2);
  tft.fillScreen(0);
  dmaCh = dma_claim_unused_channel(true);
  memset(bufA, 0, sizeof(bufA));
  memset(bufB, 0, sizeof(bufB));
  fb = bufA;
  coreInit(millis(), micros());
  perfT0 = millis();
}

void loop() {
  static uint32_t lastStart = micros();
  static int prevY0 = 0, prevY1 = -1;  // filas dibujadas en el cuadro anterior

  while ((uint32_t)(micros() - lastStart) < FRAME_US) {}   // limite a 60 fps
  uint32_t ts = micros();
  float dtm = (ts - lastStart) * 1e-6f;
  lastStart = ts;
  uint32_t now = millis();
  pollSerial(now);
  pollButtons(now);

  coreFrame(now, dtm);
  uint32_t tr = micros() - ts;

  // enviar: las filas de este cuadro + las del anterior (para borrar lo que ya no esta)
  int y0 = min(dirtyY0, prevY0), y1 = max(dirtyY1, prevY1);
  if (dirtyY1 < 0) { y0 = prevY0; y1 = prevY1; }
  prevY0 = dirtyY0 <= dirtyY1 ? dirtyY0 : H;
  prevY1 = dirtyY1;
  uint32_t tw0 = micros();
  pushWait();   // el envio anterior debe haber terminado
  uint32_t tw = micros() - tw0;
  uint16_t* t = fb; fb = shown; shown = t;   // 'shown' = recien dibujado
  if (y1 >= y0) pushStart(shown, y0, y1);

  perfFrames++; perfRenderSum += tr; perfWaitSum += tw; perfRowsSum += (y1 >= y0 ? y1 - y0 + 1 : 0);
  if (tr > perfRenderMax) perfRenderMax = tr;
  if (now - perfT0 >= 1000) {
    if (perfOn)
      Serial.printf("[PERF] %lu fps | render avg %lu us max %lu us | espera DMA avg %lu us | filas %lu\n",
                    perfFrames, perfRenderSum / perfFrames, perfRenderMax, perfWaitSum / perfFrames,
                    perfRowsSum / perfFrames);
    perfT0 = now; perfFrames = perfRenderSum = perfRenderMax = perfWaitSum = perfRowsSum = 0;
  }
}
