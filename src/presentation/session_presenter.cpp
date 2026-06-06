#include <presentation/session_presenter.h>

#include <app/app_state.h>
#include <app/tea_config.h>
#include <ui/session.h>

namespace {
int normalizedPresetIndex() {
  if (SESSION_PRESET_TOTAL_COUNT <= 0)
    return -1;
  if (app.session.presetIndex < 0)
    return 0;
  if (app.session.presetIndex >= SESSION_PRESET_TOTAL_COUNT)
    return SESSION_PRESET_TOTAL_COUNT - 1;
  return app.session.presetIndex;
}

const SessionPreset *currentPresetOrNull() {
  int index = normalizedPresetIndex();
  if (index < 0)
    return nullptr;
  return &SESSION_PRESETS[index];
}

int currentTotalSec() {
  if (app.session.stepTotalSec > 0)
    return app.session.stepTotalSec;
  if (app.session.stepDurationSec > 0)
    return app.session.stepDurationSec;
  if (app.session.rinseActive)
    return app.session.rinseSec;
  if (app.session.stepIndex >= 0 &&
      app.session.stepIndex < app.session.stepCount)
    return app.session.steps[app.session.stepIndex];
  return MIN_TIME;
}

int previousStepSec() {
  if (app.session.rinseActive)
    return 0;

  if (app.session.stepIndex > 0) {
    int previous = app.session.steps[app.session.stepIndex - 1];
    return previous > 0 ? previous : 0;
  }

  if (app.session.stepIndex == 0 && app.session.rinseSec > 0)
    return app.session.rinseSec;

  return 0;
}

const char *sessionRunStatusText(int remaining, int totalSec) {
  if (isSessionRunning())
    return "RUNNING";
  if (remaining < totalSec)
    return "PAUSED";
  return "READY";
}

SessionPresetView buildSessionPresetView() {
  SessionPresetView view;
  const SessionPreset *preset = currentPresetOrNull();
  if (preset) {
    view.hasPreset = true;
    view.presetIndex = normalizedPresetIndex();
    view.presetCount = SESSION_PRESET_TOTAL_COUNT;
    view.name = preset->name;
    view.dosePer100ml = preset->dosePer100ml;
    view.tempC = preset->tempC;
    view.infusionCount = preset->stepCount > 0 ? preset->stepCount : 0;
  }
  return view;
}

SessionCompleteView buildSessionCompleteView() {
  SessionCompleteView view;
  const SessionPreset *preset = currentPresetOrNull();
  view.teaName = preset ? preset->name : "N/A";
  return view;
}

SessionRunView buildSessionRunView(int remaining) {
  int totalSec = currentTotalSec();
  if (totalSec < 0)
    totalSec = 0;
  if (!app.session.rinseActive && totalSec < MIN_TIME)
    totalSec = MIN_TIME;

  if (remaining < 0)
    remaining = 0;
  if (remaining > totalSec)
    remaining = totalSec;

  const SessionPreset *preset = currentPresetOrNull();
  SessionRunView view;
  view.status = sessionRunStatusText(remaining, totalSec);
  view.teaName = preset ? preset->name : "Preset N/A";
  view.rinseActive = app.session.rinseActive;
  view.stepIndex = app.session.stepIndex;
  view.infusionCount = app.session.stepCount > 0 ? app.session.stepCount : 0;
  view.totalSec = totalSec;
  view.previousSec = previousStepSec();
  view.remaining = remaining;
  view.running = isSessionRunning();
  view.endConfirmActive = app.session.endConfirm.active;
  view.endConfirmYesSelected = app.session.endConfirm.yesSelected;
  return view;
}
} // namespace

void sessionPresetRender() { drawSessionPresetMenu(buildSessionPresetView()); }

void sessionCompleteRender() { drawSessionComplete(buildSessionCompleteView()); }

void sessionRunRender(int remaining) {
  if (!app.session.rinseActive &&
      app.session.stepIndex >= app.session.stepCount) {
    sessionCompleteRender();
    return;
  }
  drawSessionRun(buildSessionRunView(remaining));
}
