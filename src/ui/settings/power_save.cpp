#include <ui/settings/power_save.h>

#include <app/app_state.h>
#include <ui/header.h>

void drawPowerSave(bool enabled) {
  display.clearDisplay();
  drawHeader("Power Save");

  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print("Mode:");

  display.drawRect(44, 16, 36, 12, SSD1306_WHITE);
  display.setCursor(52, 18);
  display.print(enabled ? "ON" : "OFF");

  display.display();
}
