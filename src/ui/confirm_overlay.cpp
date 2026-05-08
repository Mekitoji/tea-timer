#include <ui/confirm_overlay.h>

#include <app/app_state.h>

void drawConfirmOverlay(const char *title, const ConfirmState &state) {
  const int x = 10;
  const int y = 19;
  const int w = 108;
  const int h = 24;

  display.fillRect(x, y, w, h, SSD1306_BLACK);
  display.drawRect(x, y, w, h, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x + 6, y + 4);
  display.print(title);
  display.setCursor(x + 6, y + 14);
  if (state.yesSelected)
    display.print("No [YES]");
  else
    display.print("[NO] Yes");
}
