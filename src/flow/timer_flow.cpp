#include "flow/timer_flow.h"

#include <Arduino.h>
#include <app/app_state.h>
#include <hw/audio.h>
#include <hw/pins.h>
#include <ui.h>

namespace {
int lastRemaining = -1;
}

void updateSingleTimer() {
  if (currentScreen == SCREEN_TIMER) {

    unsigned long elapsed = (millis() - timerStartMillis) / 1000;
    int remaining = timerDuration - (int)elapsed;
    if (remaining < 0)
      remaining = 0;

    if (remaining != lastRemaining) {
      drawTimerScreen("Timer", remaining, timerDuration);

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
        goToMenu();
      }

      lastRemaining = remaining;
    }
  }
}

void resetSingleTimerFlowState() { lastRemaining = -1; }
