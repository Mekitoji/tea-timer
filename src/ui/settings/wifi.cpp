#include <ui/settings/wifi.h>

#include <hw/display.h>
#include <ui/confirm_overlay.h>
#include <ui/header.h>
#include <ui/layout.h>

void drawWiFi(const WifiSettingsView &view) {
  display.clearDisplay();
  drawHeader("Wi-Fi", view.badge);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, ui::layout::INFO_ROW1_Y);
  display.print(view.row1);

  display.setCursor(0, ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y);
  display.print(view.row2);

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 2);
  display.print(view.row3);

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 3);
  display.print(view.row4);

  display.setCursor(0, 56);
  display.print(view.footer);

  if (view.resetConfirmActive)
    drawConfirmOverlay("Reset Wi-Fi?", view.resetConfirmYesSelected);

  display.display();
}
