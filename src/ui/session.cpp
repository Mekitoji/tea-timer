#include <ui/session.h>

#include <cstdio>
#include <hw/display.h>
#include <ui/confirm_overlay.h>
#include <ui/header.h>
#include <ui/layout.h>

namespace {
void drawSessionProgressBar(int remaining, int totalSec) {
  display.drawRect(ui::layout::PROGRESS_X, ui::layout::SESSION_PROGRESS_Y,
                   ui::layout::PROGRESS_W, ui::layout::PROGRESS_H,
                   SSD1306_WHITE);

  int elapsed = totalSec - remaining;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > totalSec)
    elapsed = totalSec;

  int fill = 0;
  if (totalSec > 0)
    fill = (elapsed * (ui::layout::PROGRESS_W - 2)) / totalSec;
  if (fill < 0)
    fill = 0;
  if (fill > ui::layout::PROGRESS_W - 2)
    fill = ui::layout::PROGRESS_W - 2;

  display.fillRect(ui::layout::PROGRESS_X + 1,
                   ui::layout::SESSION_PROGRESS_Y + 1, fill,
                   ui::layout::PROGRESS_H - 2, SSD1306_WHITE);
}
} // namespace

void drawSessionPresetMenu(const SessionPresetView &view) {
  display.clearDisplay();
  if (!view.hasPreset) {
    drawHeader("SESSION PRESET");
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 24);
    display.print("No presets");
    display.display();
    return;
  }

  char idxBuf[12];
  snprintf(idxBuf, sizeof(idxBuf), "%d/%d", view.presetIndex + 1,
           view.presetCount);
  drawHeader("SESSION PRESET", idxBuf);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print(view.name);

  display.setCursor(0, 26);
  display.print("Dose: ");
  display.print(view.dosePer100ml);

  display.setCursor(0, 34);
  display.print("Temp: ");
  display.print(view.tempC);

  display.setCursor(0, 42);
  display.print("Infusions: ");
  display.print(view.infusionCount);
  display.display();
}

void drawSessionComplete(const SessionCompleteView &view) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  drawHeader("SESSION");

  display.setTextSize(2);
  display.setCursor(ui::layout::SESSION_COMPLETE_TITLE_X,
                    ui::layout::SESSION_COMPLETE_TITLE_Y);
  display.print("COMPLETE");

  display.setTextSize(1);
  display.setCursor(0, ui::layout::SESSION_COMPLETE_TEA_Y);
  display.print("Tea: ");
  display.print(view.teaName);

  display.setCursor(0, ui::layout::SESSION_COMPLETE_HINT_Y);
  display.print("Press to exit");
  display.display();
}

void drawSessionRun(const SessionRunView &view) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  drawHeader("SESSION RUN", view.status);

  display.setTextSize(1);
  display.setCursor(0, ui::layout::SESSION_RUN_TEA_Y);
  display.print(view.teaName);

  display.setCursor(0, ui::layout::SESSION_RUN_STEP_Y);
  if (view.rinseActive) {
    display.print("Rinse 0/");
    display.print(view.infusionCount);
  } else {
    display.print("Infuse ");
    display.print(view.stepIndex + 1);
    display.print("/");
    display.print(view.infusionCount);
  }

  display.setCursor(0, ui::layout::SESSION_RUN_INFUSE_Y);
  display.print("C:");
  display.print(view.totalSec);
  display.print("s");

  if (view.previousSec > 0) {
    char prevBuf[16];
    snprintf(prevBuf, sizeof(prevBuf), "P:%ds", view.previousSec);

    int16_t x1 = 0, y1 = 0;
    uint16_t w = 0, h = 0;
    display.getTextBounds(prevBuf, 0, 0, &x1, &y1, &w, &h);

    int prevX = ui::layout::SESSION_RUN_TIMER_X - 4 - (int)w;
    if (prevX < 36)
      prevX = 36;

    display.setCursor(prevX, ui::layout::SESSION_RUN_INFUSE_Y);
    display.print(prevBuf);
  }

  display.setTextSize(2);
  display.setCursor(ui::layout::SESSION_RUN_TIMER_X,
                    ui::layout::SESSION_RUN_TIMER_Y);
  display.print(view.remaining);

  display.setTextSize(1);
  drawSessionProgressBar(view.remaining, view.totalSec);

  display.setCursor(0, ui::layout::SESSION_RUN_HINT_Y);
  if (view.running)
    display.print("Press:Pause Hold:Skip");
  else
    display.print("Press:Start Hold:Skip");

  if (view.endConfirmActive)
    drawConfirmOverlay("End session?", view.endConfirmYesSelected);

  display.display();
}
