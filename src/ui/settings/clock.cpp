#include <ui/settings/clock.h>

#include <app/app_state.h>
#include <cstdio>
#include <ui/header.h>

namespace {
void formatTime(char *buf, size_t bufSize) {
  std::snprintf(buf, bufSize, "%02d:%02d", app.clock.draftHour,
                app.clock.draftMinute);
}

void formatDate(char *buf, size_t bufSize) {
  std::snprintf(buf, bufSize, "%02d-%02d-%04d", app.clock.draftDay,
                app.clock.draftMonth, app.clock.draftYear);
}
} // namespace

void drawClock() {
  display.clearDisplay();
  drawHeader("CLOCK", app.clock.editMode ? "EDIT" : "");

  auto drawRow = [&](int y, const char *label, const char *value,
                     bool selected) {
    if (selected && !app.clock.editMode) {
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

    if (selected && app.clock.editMode) {
      display.drawRect(valueX - 2, y - 1, (int)w + 4, 9, SSD1306_WHITE);
    }

    display.setCursor(valueX, y);
    display.print(value);
  };

  char timeBuf[8];
  char dateBuf[16];
  formatTime(timeBuf, sizeof(timeBuf));
  formatDate(dateBuf, sizeof(dateBuf));

  drawRow(20, "Time", timeBuf, app.clock.selectedRow == ClockRow::Time);
  drawRow(32, "Date", dateBuf, app.clock.selectedRow == ClockRow::Date);
  drawRow(44, "Auto Sync", app.clock.draftAutoSyncEnabled ? "ON" : "OFF",
          app.clock.selectedRow == ClockRow::AutoSync);

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print(app.clock.editMode ? "Rot:Edit Sel:Done"
                                   : "Sel:Edit Back:Exit");
  display.display();
}
