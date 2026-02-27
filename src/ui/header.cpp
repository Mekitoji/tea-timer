#include <ui/header.h>

#include <app/app_state.h>
#include <ui/layout.h>

namespace {
void drawHeaderBase(const char *title) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(title);
  display.drawLine(0, ui::layout::UI_HEADER_LINE_Y, 128,
                   ui::layout::UI_HEADER_LINE_Y, SSD1306_WHITE);
}
} // namespace

void drawHeader(const char *title) {
  drawHeaderBase(title);
}

void drawHeader(const char *title, const char *rightText) {
  drawHeaderBase(title);
  if (!rightText || !rightText[0])
    return;

  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t w = 0;
  uint16_t h = 0;
  display.getTextBounds(rightText, 0, 0, &x1, &y1, &w, &h);

  int rightX = 128 - (int)w;
  if (rightX < 0)
    rightX = 0;

  display.setCursor(rightX, 0);
  display.print(rightText);
}
