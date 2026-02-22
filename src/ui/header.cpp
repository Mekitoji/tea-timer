#include <ui/header.h>

#include <app/app_state.h>

void drawHeader(const char *title) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(title);
  display.drawLine(0, UI_HEADER_LINE_Y, 128, UI_HEADER_LINE_Y, SSD1306_WHITE);
}
