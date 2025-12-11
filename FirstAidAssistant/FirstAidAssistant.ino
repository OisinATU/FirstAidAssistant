#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// --- Pin Definitions ---
#define TFT_CS   5
#define TFT_DC   2   // A0 = DC
#define TFT_RST  4

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing TFT...");

  tft.initR(INITR_BLACKTAB);  // We'll try others if needed
  tft.setRotation(1);

  delay(500);

  tft.fillScreen(ST77XX_RED);
  delay(1000);
  tft.fillScreen(ST77XX_GREEN);
  delay(1000);
  tft.fillScreen(ST77XX_BLUE);
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 30);
  tft.println("First Aid");
  tft.setCursor(10, 60);
  tft.println("Assistant");
}




void loop() {}
