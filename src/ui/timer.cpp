#include <ui/timer.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <ui/header.h>
#include <ui/layout.h>

void drawProgressBar(int remaining, int total) {
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

void drawTimerScreen(const char *title, int secondsLeft, int totalSeconds) {
  display.clearDisplay();
  drawHeader(title);

  display.setTextSize(3);
  display.setCursor(ui::layout::TIMER_VALUE_X, ui::layout::TIMER_VALUE_Y);
  display.print(secondsLeft);

  display.setTextSize(1);
  drawProgressBar(secondsLeft, totalSeconds);

  display.display();
}

void drawSetTime() {
  display.clearDisplay();
  drawHeader("Set Time");

  int mm = editTimeValue / 60;
  int ss = editTimeValue % 60;

  display.setTextSize(2);
  display.setCursor(ui::layout::SET_TIME_VALUE_X, ui::layout::SET_TIME_VALUE_Y);
  if (mm < 10)
    display.print('0');
  display.print(mm);
  display.print(':');
  if (ss < 10)
    display.print('0');
  display.print(ss);

  display.setTextSize(1);
  display.setCursor(ui::layout::SET_TIME_HINT_X, ui::layout::SET_TIME_HINT_Y);
  display.print("Press to save");

  display.display();
}
