#include <ui/timer.h>

#include <cstdio>
#include <hw/display.h>
#include <ui/header.h>
#include <ui/layout.h>

namespace {
void drawProgressBar(int remaining, int total) {
  if (total < 1)
    total = 1;
  if (remaining < 0)
    remaining = 0;
  if (remaining > total)
    remaining = total;

  display.drawRect(ui::layout::PROGRESS_X, ui::layout::TIMER_PROGRESS_Y,
                   ui::layout::PROGRESS_W, ui::layout::PROGRESS_H,
                   SSD1306_WHITE);

  int elapsed = total - remaining;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > total)
    elapsed = total;

  int fill =
      (total == 0) ? 0 : (elapsed * (ui::layout::PROGRESS_W - 2)) / total;
  if (fill < 0)
    fill = 0;
  if (fill > ui::layout::PROGRESS_W - 2)
    fill = ui::layout::PROGRESS_W - 2;

  display.fillRect(ui::layout::PROGRESS_X + 1, ui::layout::TIMER_PROGRESS_Y + 1,
                   fill, ui::layout::PROGRESS_H - 2, SSD1306_WHITE);
}
} // namespace

void drawTimerScreen(const TimerView &view) {
  display.clearDisplay();
  drawHeader(view.title, view.status);

  display.setTextSize(3);
  display.setCursor(ui::layout::TIMER_VALUE_X, ui::layout::TIMER_VALUE_Y);

  int shown = view.secondsLeft;
  if (shown < 0)
    shown = 0;
  if (shown > 999)
    shown = 999;

  char secBuf[5];
  snprintf(secBuf, sizeof(secBuf), "%3d", shown); // fix w, maybe %03d ?
  display.print(secBuf);

  if (view.showProgress) {
    display.setTextSize(1);
    drawProgressBar(view.secondsLeft, view.totalSeconds);
  }

  display.display();
}
