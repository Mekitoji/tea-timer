#include <ui/session.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <ui/header.h>

namespace {
enum class SessionRunStatus { Ready, Running, Paused };

SessionRunStatus resolveSessionRunStatus(bool isRunning, int remaining,
                                         int totalSec) {
  if (isRunning)
    return SessionRunStatus::Running;
  if (remaining < totalSec)
    return SessionRunStatus::Paused;
  return SessionRunStatus::Ready;
}

const char *statusText(SessionRunStatus status) {
  switch (status) {
  case SessionRunStatus::Running:
    return "RUNNING";
  case SessionRunStatus::Paused:
    return "PAUSED";
  case SessionRunStatus::Ready:
  default:
    return "READY";
  }
}
} // namespace

void drawSessionComplete() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  drawHeader("SESSION");

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
  drawHeader("Session");

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

  int totalSec =
      sessionStepTotalSec > 0
          ? sessionStepTotalSec
          : (sessionStepDurationSec > 0 ? sessionStepDurationSec
                                        : SESSION_STEPS[sessionStepIndex]);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ---- HEADER ----
  drawHeader("SESSION RUN");

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

  display.print(totalSec);
  display.print("s");

  // ---- TIMER BIG ----
  display.setTextSize(2);
  display.setCursor(78, 28);
  display.print(remaining);

  // ---- STATUS ----
  display.setTextSize(1);

  SessionRunStatus status =
      resolveSessionRunStatus(sessionRunning, remaining, totalSec);
  const char *statusLabel = statusText(status);

  const int statusX = 74;
  const int statusY = 1;
  const int statusW = 52;
  const int statusH = 11;

  display.drawRect(statusX, statusY, statusW, statusH, SSD1306_WHITE);
  display.setCursor(statusX + 3, statusY + 2);
  display.print(statusLabel);

  // ---- PROGRESS BAR ----
  int x = 6;
  int y = 46;
  int w = 116;
  int h = 8;

  display.drawRect(x, y, w, h, SSD1306_WHITE);

  int total = totalSec;
  int elapsed = total - remaining;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > total)
    elapsed = total;

  int fill = (total == 0) ? 0 : (elapsed * (w - 2)) / total;
  display.fillRect(x + 1, y + 1, fill, h - 2, SSD1306_WHITE);

  // ---- helper text ----
  display.setTextSize(1);
  display.setCursor(0, 56);
  if (sessionRunning)
    display.print("Press:Pause Hold:Skip");
  else
    display.print("Press:Start Hold:Skip");

  display.display();
}
