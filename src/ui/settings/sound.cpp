#include <ui/settings/sound.h>

#include <app/app_state.h>
#include <ui/header.h>

namespace {
const char *beepProfileText(BeepProfile profile) {
  switch (profile) {
  case BeepProfile::Soft:
    return "Soft";
  case BeepProfile::Loud:
    return "Loud";
  case BeepProfile::Normal:
  default:
    return "Normal";
  }
}
} // namespace

void drawSound(const AudioStateModel &audioState, SoundRow selectedRow,
               bool editMode) {
  display.clearDisplay();
  drawHeader("SOUND", editMode ? "EDIT" : "");

  auto drawRow = [&](int y, const char *label, const char *value,
                     bool selected) {
    if (selected && !editMode) {
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

    if (selected && editMode) {
      display.drawRect(valueX - 2, y - 1, (int)w + 4, 9, SSD1306_WHITE);
    }

    display.setCursor(valueX, y);
    display.print(value);
  };

  drawRow(20, "Sound", audioState.soundEnabled ? "ON" : "OFF",
          selectedRow == SoundRow::Enabled);
  drawRow(32, "Profile", beepProfileText(audioState.profile),
          selectedRow == SoundRow::Profile);

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print(editMode ? "Rot:Edit Sel:Done" : "Sel:Edit Back:Save");

  display.display();
}
