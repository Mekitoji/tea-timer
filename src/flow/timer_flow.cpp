#include "flow/timer_flow.h"

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <hw/audio.h>
#include <hw/pins.h>
#include <ui.h>

namespace {
int lastRemaining = -1;
}

int normalizeTimerPresetSec(int sec) {
  if (sec < MIN_TIME)
    return MIN_TIME;
  if (sec > MAX_TIME)
    return MAX_TIME;
  return sec;
}

void applyTimerPresetSec(int sec) {
  int v = normalizeTimerPresetSec(sec);
  timerDuration = v;
  editTimeValue = v;
  timerTotalSec = v;
}

void resetSingleTimerRuntimeState() {
  setTimerStateStopped();

  timerIgnoreReleaseAfterEnter = false;
  timerStartMillis = 0;
  lastRemaining = -1;
}

void updateSingleTimer() {
  if (currentScreen == SCREEN_TIMER) {
    if (!isTimerRunning())
      return;

    unsigned long elapsed = (millis() - timerStartMillis) / 1000;
    int remaining = timerDuration - (int)elapsed;
    if (remaining < 0)
      remaining = 0;

    if (remaining != lastRemaining) {
      drawTimerScreen("Timer", remaining, timerTotalSec);

      if (remaining <= 3 && remaining > 0) {
        digitalWrite(LED_PIN, HIGH);
        beep(2200, 60);
        digitalWrite(LED_PIN, LOW);
      }

      if (remaining == 0) {
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_PIN, HIGH);
          buzzerOn(2600);
          delay(80);
          buzzerOff();
          digitalWrite(LED_PIN, LOW);
          delay(120);
        }

        applyTimerPresetSec(timerTotalSec);
        prefs.putInt(appcfg::PREFS_DURATION_KEY, timerDuration);
        resetSingleTimerRuntimeState();
        drawTimerScreen("Timer", editTimeValue, timerTotalSec);
      }

      lastRemaining = remaining;
    }
  }
}

void resetSingleTimerFlowState() { lastRemaining = -1; }
