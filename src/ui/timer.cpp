#include <ui/timer.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <ui/header.h>

void drawProgressBar(int remaining, int total) {
  int x = 6, y = 54, w = 116, h = 8;
  display.drawRect(x, y, w, h, SSD1306_WHITE);

  int elapsed = total - remaining;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > total)
    elapsed = total;

  int fill = (total == 0) ? 0 : (elapsed * (w - 2)) / total;
  if (fill < 0)
    fill = 0;
  if (fill > w - 2)
    fill = w - 2;

  display.fillRect(x + 1, y + 1, fill, h - 2, SSD1306_WHITE);
}

void drawTimerScreen(const char *title, int secondsLeft, int totalSeconds) {
  display.clearDisplay();
  drawHeader(title);

  display.setTextSize(3);
  display.setCursor(40, 22);
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
  display.setCursor(10, 28);
  if (mm < 10)
    display.print('0');
  display.print(mm);
  display.print(':');
  if (ss < 10)
    display.print('0');
  display.print(ss);

  display.setTextSize(1);
  display.setCursor(0, 54);
  display.print("Press to save");

  display.display();
}
