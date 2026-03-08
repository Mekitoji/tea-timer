#include <ui/session.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <cstdio>
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

int normalizedPresetIndex() {
  if (SESSION_PRESET_COUNT <= 0)
    return -1;
  if (app.session.presetIndex < 0)
    return 0;
  if (app.session.presetIndex >= SESSION_PRESET_COUNT)
    return SESSION_PRESET_COUNT - 1;
  return app.session.presetIndex;
}

const SessionPreset *currentPresetOrNull() {
  int index = normalizedPresetIndex();
  if (index < 0)
    return nullptr;
  return &SESSION_PRESETS[index];
}

bool hasVisibleRinse() { return app.session.rinseSec > 0; }

int currentTotalSec() {
  if (app.session.stepTotalSec > 0)
    return app.session.stepTotalSec;
  if (app.session.stepDurationSec > 0)
    return app.session.stepDurationSec;
  if (app.session.rinseActive && app.session.rinseSec > 0)
    return app.session.rinseSec;
  if (app.session.stepIndex >= 0 && app.session.stepIndex < app.session.stepCount)
    return app.session.steps[app.session.stepIndex];
  return MIN_TIME;
}
} // namespace

void drawSessionPresetMenu() {
  display.clearDisplay();
  const SessionPreset *preset = currentPresetOrNull();
  if (!preset) {
    drawHeader("SESSION PRESET");
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 24);
    display.print("No presets");
    display.display();
    return;
  }

  int index = normalizedPresetIndex();
  char idxBuf[12];
  snprintf(idxBuf, sizeof(idxBuf), "%d/%d", index + 1, SESSION_PRESET_COUNT);
  drawHeader("SESSION PRESET", idxBuf);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print(preset->name);

  display.setCursor(0, 26);
  display.print("Dose: ");
  display.print(preset->dosePer100ml);

  display.setCursor(0, 34);
  display.print("Temp: ");
  display.print(preset->tempC);

  int infusions = preset->stepCount;
  if (infusions < 0)
    infusions = 0;

  display.setCursor(0, 42);
  display.print("Infusions: ");
  display.print(infusions);
  display.display();
}

void drawSessionComplete() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  drawHeader("SESSION");

  display.setTextSize(2);
  display.setCursor(ui::layout::SESSION_COMPLETE_TITLE_X,
                    ui::layout::SESSION_COMPLETE_TITLE_Y);
  display.print("COMPLETE");

  display.setTextSize(1);
  const SessionPreset *preset = currentPresetOrNull();
  display.setCursor(0, ui::layout::SESSION_COMPLETE_TEA_Y);
  display.print("Tea: ");
  if (preset) {
    display.print(preset->name);
  } else {
    display.print("N/A");
  }

  display.setCursor(0, ui::layout::SESSION_COMPLETE_HINT_Y);
  display.print("Press to exit");
  display.display();
}

void drawSessionRun(int remaining) {
  if (!app.session.rinseActive && app.session.stepIndex >= app.session.stepCount) {
    drawSessionComplete();
    return;
  }

  int totalSec = currentTotalSec();
  if (totalSec < MIN_TIME)
    totalSec = MIN_TIME;

  if (remaining < 0)
    remaining = 0;
  if (remaining > totalSec)
    remaining = totalSec;

  SessionRunStatus status =
      resolveSessionRunStatus(isSessionRunning(), remaining, totalSec);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  drawHeader("SESSION RUN", statusText(status));

  const SessionPreset *preset = currentPresetOrNull();
  display.setTextSize(1);
  display.setCursor(0, ui::layout::SESSION_RUN_TEA_Y);
  if (preset) {
    display.print(preset->name);
  } else {
    display.print("Preset N/A");
  }

  int infusions = app.session.stepCount;
  if (infusions < 0)
    infusions = 0;

  display.setCursor(0, ui::layout::SESSION_RUN_STEP_Y);
  if (app.session.rinseActive && hasVisibleRinse()) {
    display.print("Rinse 0/");
    display.print(infusions);
  } else {
    display.print("Infuse ");
    display.print(app.session.stepIndex + 1);
    display.print("/");
    display.print(infusions);
  }

  display.setCursor(0, ui::layout::SESSION_RUN_INFUSE_Y);
  display.print("Step ");
  display.print(totalSec);
  display.print("s");

  display.setTextSize(2);
  display.setCursor(ui::layout::SESSION_RUN_TIMER_X,
                    ui::layout::SESSION_RUN_TIMER_Y);
  display.print(remaining);

  display.setTextSize(1);

  display.drawRect(ui::layout::PROGRESS_X, ui::layout::SESSION_PROGRESS_Y,
                   ui::layout::PROGRESS_W, ui::layout::PROGRESS_H,
                   SSD1306_WHITE);

  int elapsed = totalSec - remaining;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > totalSec)
    elapsed = totalSec;

  int fill =
      (elapsed * (ui::layout::PROGRESS_W - 2)) / (totalSec > 0 ? totalSec : 1);
  if (fill < 0)
    fill = 0;
  if (fill > ui::layout::PROGRESS_W - 2)
    fill = ui::layout::PROGRESS_W - 2;

  display.fillRect(ui::layout::PROGRESS_X + 1,
                   ui::layout::SESSION_PROGRESS_Y + 1, fill,
                   ui::layout::PROGRESS_H - 2, SSD1306_WHITE);

  display.setCursor(0, ui::layout::SESSION_RUN_HINT_Y);
  if (isSessionRunning())
    display.print("Press:Pause Hold:Skip");
  else
    display.print("Press:Start Hold:Skip");

  if (app.session.endConfirm.active) {
    const int x = 10;
    const int y = 19;
    const int w = 108;
    const int h = 24;

    display.fillRect(x, y, w, h, SSD1306_BLACK);
    display.drawRect(x, y, w, h, SSD1306_WHITE);

    display.setCursor(x + 6, y + 4);
    display.print("End session?");
    display.setCursor(x + 6, y + 14);
    if (app.session.endConfirm.yesSelected)
      display.print("No [YES]");
    else
      display.print("[NO] Yes");
  }

  display.display();
}
