#define LILYGO_LGFX_USE_T_DISPLAY

#include <LilyGo_LovyanGFX.h>

LilyGo_T_Display display;

void setup() {
  display.begin(1);
}

void loop() {

  // Screen 1
  display.fillScreen(TFT_WHITE);
  display.setTextColor(TFT_BLACK, TFT_WHITE);
  display.setTextSize(3);
  display.drawString("ROBOT", 70, 20);

  display.fillCircle(120, 80, 35, TFT_YELLOW);
  display.fillCircle(108, 70, 5, TFT_BLACK);
  display.fillCircle(132, 70, 5, TFT_BLACK);

  delay(2000);

  // Screen 2
  display.fillScreen(TFT_WHITE);
  display.setTextColor(TFT_BLACK, TFT_WHITE);
  display.setTextSize(3);
  display.drawString("WARNING", 50, 20);

  // Simple warning symbol
  display.fillTriangle(120, 45, 85, 105, 155, 105, TFT_RED);
  display.setTextColor(TFT_WHITE, TFT_RED);
  display.setTextSize(3);
  display.drawString("!", 115, 65);

  delay(2000);

  // Screen 3
  display.fillScreen(TFT_WHITE);
  display.setTextColor(TFT_BLACK, TFT_WHITE);
  display.setTextSize(3);
  display.drawString("MOVING", 65, 20);

  display.fillCircle(120, 75, 30, TFT_BLUE);

  delay(2000);
}