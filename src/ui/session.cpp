#include <ui/session.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <app/tea_config.h>

void drawSessionComplete() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Header (yellow)
  display.setTextSize(1);
  display.setCursor(0, 2);
  display.print("SESSION");
  display.drawLine(0, 14, 128, 14, SSD1306_WHITE);

  // Body
  display.setTextSize(2);
  display.setCursor(8, 24);
  display.print("COMPLETE");

  display.setTextSize(1);
  display.setCursor(0, 42);
  display.print("Tea: ");
  display.print(TEAS[sessionTeaIndex]);

  display.setCursor(0, 54);
  display.print("Press to exit");

  display.display();
}

void drawSessionMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Session");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print("Tea:");
  display.setCursor(30, 18);
  display.print(TEAS[sessionTeaIndex]);

  display.setCursor(0, 34);
  display.print("Steps:");
  display.setCursor(45, 34);
  display.print(SESSION_STEP_COUNT);
  display.print(" infusions");

  display.setCursor(0, 54);
  display.print("Press: back");

  display.display();
}

void drawSessionRun(int remaining) {
  if (sessionStepIndex >= SESSION_STEP_COUNT) {
    drawSessionComplete();
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ---- HEADER ----
  display.setTextSize(1);
  display.setCursor(0, 2);
  display.print("SESSION RUN");

  display.drawLine(0, 14, 128, 14, SSD1306_WHITE);

  // ---- INFO BLOCK ----
  display.setTextSize(1);

  display.setCursor(0, 18);
  display.print("Tea:");
  display.setCursor(28, 18);
  display.print(TEAS[sessionTeaIndex]);

  display.setCursor(0, 28);
  display.print("Step ");
  display.print(sessionStepIndex + 1);
  display.print("/");
  display.print(SESSION_STEP_COUNT);

  display.setCursor(0, 38);
  if (sessionStepIndex == 0)
    display.print("Rinse ");
  else
    display.print("Infuse ");

  display.print(SESSION_STEPS[sessionStepIndex]);
  display.print("s");

  // ---- TIMER BIG ----
  display.setTextSize(2);
  display.setCursor(78, 28);
  display.print(remaining);

  // ---- PROGRESS BAR ----
  int x = 6;
  int y = 50;
  int w = 116;
  int h = 8;

  display.drawRect(x, y, w, h, SSD1306_WHITE);

  int total = SESSION_STEPS[sessionStepIndex];
  int elapsed = total - remaining;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > total)
    elapsed = total;

  int fill = (total == 0) ? 0 : (elapsed * (w - 2)) / total;
  display.fillRect(x + 1, y + 1, fill, h - 2, SSD1306_WHITE);

  display.display();
}
