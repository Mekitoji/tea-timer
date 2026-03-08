#include <controllers/timer_controller.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <flow/navigation_flow.h>
#include <flow/power_flow.h>
#include <flow/timer_flow.h>
#include <hw/pins.h>
#include <ui/menu.h>

namespace {
int encoderAccelStepForTimestamp(unsigned long nowMs) {
  static unsigned long lastStepMs = 0;

  unsigned long dt = nowMs - lastStepMs;
  lastStepMs = nowMs;

  int step = appcfg::ENC_STEP_NORMAL;
  if (dt < appcfg::ENC_ACCEL_FAST_MS)
    step = appcfg::ENC_STEP_FAST;
  else if (dt < appcfg::ENC_ACCEL_MEDIUM_MS)
    step = appcfg::ENC_STEP_MEDIUM;

  return step;
}
} // namespace

bool handleTimerEncoderInput(bool stepPlus, bool stepMinus) {
  if (currentScreen != SCREEN_TIMER) {
    return false;
  }

  int step = encoderAccelStepForTimestamp(millis());
  int delta = stepPlus ? step : -step;
  timerAdjustByEncoderDelta(delta);

  return true;
}

bool handleTimerBackInput() {
  if (currentScreen != SCREEN_TIMER) {
    return false;
  }

  applyTimerPresetSec(timerTotalSec);
  prefs.putInt(appcfg::PREFS_DURATION_KEY, timerDuration);
  resetSingleTimerRuntimeState();
  timerIgnoreReleaseAfterEnter = false;
  resetTimerLongPressFlowState();
  navigateTo(SCREEN_MENU);
  drawMenu();

  return true;
}

bool handleTimerSelectInput() {
  if (currentScreen != SCREEN_TIMER)
    return false;

  if (isTimerRunning()) {
    timerPauseAt(millis());
  } else {
    timerStartOrResumeAt(millis());
  }

  return true;
}

void handleTimerLongPressInput() {
  if (isWakeInputGuardActive())
    return;

  if (currentScreen != SCREEN_TIMER)
    return;

  if (timerIgnoreReleaseAfterEnter) {
    if (digitalRead(ENC_SW) == HIGH) {
      timerIgnoreReleaseAfterEnter = false;
    }

    return;
  }

  const bool down = (digitalRead(ENC_SW) == LOW);
  processTimerLongPressInput(down, millis());
}
