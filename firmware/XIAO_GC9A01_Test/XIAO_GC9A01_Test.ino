// XIAO RP2350 + GC9A01 (240x240 redonda) + 2 botones
// SCL=D8(GPIO2) SDA=D10(GPIO3) DC=D0(GPIO26) CS=D1(GPIO27) RST=D2(GPIO28)
// Azul=D6(GPIO0)  Verde=D7(GPIO1)  (a GND, INPUT_PULLUP)
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

#define PIN_SCK 2
#define PIN_MOSI 3
#define PIN_DC 26
#define PIN_CS 27
#define PIN_RST 28
#define BTN_BLUE 0
#define BTN_GREEN 1

Adafruit_GC9A01A tft(&SPI, PIN_DC, PIN_CS, PIN_RST);

int cntBlue = 0, cntGreen = 0;
bool lastB = false, lastG = false;

void drawUI(bool b, bool g) {
  const uint16_t BG = GC9A01A_BLACK;
  tft.fillScreen(BG);
  tft.drawCircle(120, 120, 119, GC9A01A_WHITE);
  tft.drawCircle(120, 120, 117, 0x7BEF);
  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(2);
  tft.setCursor(52, 38);
  tft.print("XIAO RP2350");
  // Boton azul (izquierda)
  tft.fillCircle(75, 120, 28, b ? GC9A01A_BLUE : 0x0010);
  tft.drawCircle(75, 120, 28, GC9A01A_BLUE);
  // Boton verde (derecha)
  tft.fillCircle(165, 120, 28, g ? GC9A01A_GREEN : 0x0200);
  tft.drawCircle(165, 120, 28, GC9A01A_GREEN);
  tft.setTextSize(3);
  tft.setTextColor(GC9A01A_CYAN);
  tft.setCursor(66, 165);
  tft.print(cntBlue);
  tft.setTextColor(GC9A01A_GREEN);
  tft.setCursor(156, 165);
  tft.print(cntGreen);
  tft.setTextSize(2);
  tft.setTextColor(0xFFE0);
  tft.setCursor(60, 200);
  tft.print(b || g ? "PRESIONADO" : "Pulsa algo");
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_BLUE, INPUT_PULLUP);
  pinMode(BTN_GREEN, INPUT_PULLUP);
  SPI.setSCK(PIN_SCK);
  SPI.setTX(PIN_MOSI);
  tft.begin(40000000);
  tft.setRotation(2);
  uint16_t cols[] = {GC9A01A_RED, GC9A01A_GREEN, GC9A01A_BLUE, GC9A01A_WHITE};
  for (int i = 0; i < 4; i++) { tft.fillScreen(cols[i]); delay(400); }
  drawUI(false, false);
  Serial.println("Pantalla OK. Esperando botones...");
}

void loop() {
  bool b = digitalRead(BTN_BLUE) == LOW;
  bool g = digitalRead(BTN_GREEN) == LOW;
  if (b != lastB || g != lastG) {
    delay(15);  // antirrebote
    b = digitalRead(BTN_BLUE) == LOW;
    g = digitalRead(BTN_GREEN) == LOW;
    if (b && !lastB) { cntBlue++; Serial.println("AZUL"); }
    if (g && !lastG) { cntGreen++; Serial.println("VERDE"); }
    lastB = b; lastG = g;
    drawUI(b, g);
  }
}
