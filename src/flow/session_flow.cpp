#include "flow/session_flow.h"

#include <Arduino.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <hw/audio.h>
#include <hw/pins.h>
#include <ui.h>

namespace {
int lastRemaining = -1;
}

void resetSessionFlowState() { lastRemaining = -1; }

void updateSessionRun() {
  if (currentScreen == SCREEN_SESSION_RUN) {
    if (sessionStepIndex >= SESSION_STEP_COUNT) {
      if (!sessionCompleteShown) {
        drawSessionComplete();
        sessionCompleteShown = true;
      }
      return;
    }

    int stepSec = sessionStepDurationSec > 0 ? sessionStepDurationSec
                                             : SESSION_STEPS[sessionStepIndex];
    int remaining = stepSec;

    if (sessionRunning) {
      unsigned long elapsed = (millis() - sessionStepStartMs) / 1000;
      remaining = stepSec - (int)elapsed;
      if (remaining < 0)
        remaining = 0;
    }

    if (remaining != lastRemaining) {
      drawSessionRun(remaining);

      if (sessionRunning && remaining <= 3 && remaining > 0) {
        digitalWrite(LED_PIN, HIGH);
        beep(2200, 60);
        digitalWrite(LED_PIN, LOW);
      }

      if (sessionRunning && remaining == 0) {
        for (int i = 0; i < 2; i++) {
          digitalWrite(LED_PIN, HIGH);
          buzzerOn(2500);
          delay(70);
          buzzerOff();
          digitalWrite(LED_PIN, LOW);
          delay(120);
        }

        sessionRunning = false;
        sessionStepIndex++;

        if (sessionStepIndex >= SESSION_STEP_COUNT) {
          for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            buzzerOn(3000);
            delay(120);
            buzzerOff();
            digitalWrite(LED_PIN, LOW);
            delay(160);
          }

          drawSessionComplete();
          sessionCompleteShown = true;
          lastRemaining = -999;
          return;
        }

        sessionStepDurationSec = SESSION_STEPS[sessionStepIndex];
        sessionStepTotalSec = sessionStepDurationSec;
        drawSessionRun(sessionStepDurationSec);
      }

      lastRemaining = remaining;
    }
  }
}
