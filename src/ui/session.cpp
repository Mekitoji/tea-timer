#include <ui/session.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <ui/header.h>
#include <ui/layout.h>

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
  display.setCursor(ui::layout::SESSION_COMPLETE_TITLE_X,
                    ui::layout::SESSION_COMPLETE_TITLE_Y);
  display.print("COMPLETE");

  display.setTextSize(1);
  display.setCursor(0, ui::layout::SESSION_COMPLETE_TEA_Y);
  display.print("Tea: ");
  display.print(TEAS[sessionTeaIndex]);

  display.setCursor(0, ui::layout::SESSION_COMPLETE_HINT_Y);
  display.print("Press to exit");

  display.display();
}

void drawSessionMenu() {
  display.clearDisplay();
  drawHeader("Session");

  display.setCursor(0, ui::layout::SESSION_MENU_TEA_Y);
  display.print("Tea:");
  display.setCursor(ui::layout::SESSION_MENU_TEA_VALUE_X,
                    ui::layout::SESSION_MENU_TEA_Y);
  display.print(TEAS[sessionTeaIndex]);

  display.setCursor(0, ui::layout::SESSION_MENU_STEPS_Y);
  display.print("Steps:");
  display.setCursor(ui::layout::SESSION_MENU_STEPS_VALUE_X,
                    ui::layout::SESSION_MENU_STEPS_Y);
  display.print(SESSION_STEP_COUNT);
  display.print(" infusions");

  display.setCursor(0, ui::layout::SESSION_MENU_HINT_Y);
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

  display.setCursor(0, ui::layout::SESSION_RUN_TEA_Y);
  display.print("Tea:");
  display.setCursor(ui::layout::SESSION_RUN_TEA_VALUE_X,
                    ui::layout::SESSION_RUN_TEA_Y);
  display.print(TEAS[sessionTeaIndex]);

  display.setCursor(0, ui::layout::SESSION_RUN_STEP_Y);
  display.print("Step ");
  display.print(sessionStepIndex + 1);
  display.print("/");
  display.print(SESSION_STEP_COUNT);

  display.setCursor(0, ui::layout::SESSION_RUN_INFUSE_Y);
  if (sessionStepIndex == 0)
    display.print("Rinse ");
  else
    display.print("Infuse ");

  display.print(totalSec);
  display.print("s");

  // ---- TIMER BIG ----
  display.setTextSize(2);
  display.setCursor(ui::layout::SESSION_RUN_TIMER_X,
                    ui::layout::SESSION_RUN_TIMER_Y);
  display.print(remaining);

  // ---- STATUS ----
  display.setTextSize(1);

  SessionRunStatus status =
      resolveSessionRunStatus(isSessionRunning(), remaining, totalSec);
  const char *statusLabel = statusText(status);

  display.drawRect(ui::layout::SESSION_STATUS_X, ui::layout::SESSION_STATUS_Y,
                   ui::layout::SESSION_STATUS_W, ui::layout::SESSION_STATUS_H,
                   SSD1306_WHITE);
  display.setCursor(ui::layout::SESSION_STATUS_X + 3,
                    ui::layout::SESSION_STATUS_Y + 2);
  display.print(statusLabel);

  // ---- PROGRESS BAR ----
  display.drawRect(ui::layout::PROGRESS_X, ui::layout::SESSION_PROGRESS_Y,
                   ui::layout::PROGRESS_W, ui::layout::PROGRESS_H,
                   SSD1306_WHITE);

  int total = totalSec;
  int elapsed = total - remaining;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > total)
    elapsed = total;

  int fill =
      (total == 0) ? 0 : (elapsed * (ui::layout::PROGRESS_W - 2)) / total;
  display.fillRect(ui::layout::PROGRESS_X + 1,
                   ui::layout::SESSION_PROGRESS_Y + 1, fill,
                   ui::layout::PROGRESS_H - 2, SSD1306_WHITE);

  // ---- helper text ----
  display.setTextSize(1);
  display.setCursor(0, ui::layout::SESSION_RUN_HINT_Y);
  if (isSessionRunning())
    display.print("Press:Pause Hold:Skip");
  else
    display.print("Press:Start Hold:Skip");

  display.display();
}
