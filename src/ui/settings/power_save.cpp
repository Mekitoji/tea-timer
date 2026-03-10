#include <ui/settings/power_save.h>

#include <app/app_state.h>
#include <cstdio>
#include <ui/header.h>

namespace {
void formatTimeoutLabel(unsigned long timeoutMs, char *buf, size_t bufSize) {
  unsigned long sec = timeoutMs / 1000UL;
  if (sec % 60UL == 0) {
    std::snprintf(buf, bufSize, "%lum", sec / 60UL);
  } else {
    std::snprintf(buf, bufSize, "%lus", sec);
  }
}
} // namespace

void drawPowerSave(const PowerStateModel &powerState) {
  display.clearDisplay();
  drawHeader("Power Save", powerState.editMode ? "EDIT" : "");

  auto drawRow = [&](int y, const char *label, const char *value,
                     bool selected) {
    if (selected && !powerState.editMode) {
      display.fillRect(0, y - 1, 128, 9, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(2, y);
    display.print(label);

    int16_t x1 = 0, y1 = 0;
    uint16_t w = 0, h = 0;
    display.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);
    int valueX = 126 - (int)w;
    if (valueX < 2)
      valueX = 2;

    if (selected && powerState.editMode) {
      display.drawRect(valueX - 2, y - 1, (int)w + 4, 9, SSD1306_WHITE);
    }

    display.setCursor(valueX, y);
    display.print(value);
  };

  char timeoutBuf[12];
  formatTimeoutLabel(powerState.draftDisplayOffTimeoutMs, timeoutBuf,
                     sizeof(timeoutBuf));

  drawRow(20, "Mode", powerState.draftEnabled ? "ON" : "OFF",
          powerState.selectedRow == PowerRow::Enabled);
  drawRow(32, "Timeout", timeoutBuf,
          powerState.selectedRow == PowerRow::Timeout);

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print(powerState.editMode ? "Rot:Edit Sel:Done"
                                    : "Sel:Edit Back:Save");
  display.display();
}
