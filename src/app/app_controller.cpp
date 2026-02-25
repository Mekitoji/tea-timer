#include "app/app_controller.h"

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/menu_flow.h>
#include <flow/timer_flow.h>
#include <hw/input.h>
#include <hw/pins.h>
#include <ui.h>

namespace {
unsigned long swHoldStartMs = 0;
bool swWasDown = false;
unsigned long lastStepMs = 0;
bool sessionLongPressFired = false;
unsigned long timerHoldStartMs = 0;
bool timerWasDown = false;
bool timerLongPressFired = false;
unsigned long timerReleaseGuardUntilMs = 0;
} // namespace

void handleEncoderByScreen(bool stepPlus, bool stepMinus) {
  if (stepPlus || stepMinus) {
    if (currentScreen == SCREEN_MENU) {
      selected += stepPlus ? 1 : -1;
      if (selected < 0)
        selected = menuCount - 1;
      if (selected >= menuCount)
        selected = 0;
      drawMenu();
    } else if (currentScreen == SCREEN_TIMER) {
      if (!singleTimerRunning) {
        unsigned long now = millis();
        unsigned long dt = now - lastStepMs;
        lastStepMs = now;

        int step = appcfg::ENC_STEP_NORMAL;
        if (dt < appcfg::ENC_ACCEL_FAST_MS)
          step = appcfg::ENC_STEP_FAST;
        else if (dt < appcfg::ENC_ACCEL_MEDIUM_MS)
          step = appcfg::ENC_STEP_MEDIUM;

        int delta = stepPlus ? step : -step;

        if (!singleTimerStarted) {
          editTimeValue += delta;
          if (editTimeValue < MIN_TIME)
            editTimeValue = MIN_TIME;
          if (editTimeValue > MAX_TIME)
            editTimeValue = MAX_TIME;

          timerTotalSec = editTimeValue;
          drawTimerScreen("Timer", editTimeValue, timerTotalSec);
        } else {
          // Paused state: adjust remaining seconds while keeping elapsed time.
          int elapsed = timerTotalSec - timerDuration;
          if (elapsed < 0)
            elapsed = 0;
          if (elapsed > MAX_TIME)
            elapsed = MAX_TIME;

          int maxRemaining = MAX_TIME - elapsed;
          if (maxRemaining < MIN_TIME)
            maxRemaining = MIN_TIME;

          int newRemaining = timerDuration + delta;
          if (newRemaining < MIN_TIME)
            newRemaining = MIN_TIME;
          if (newRemaining > maxRemaining)
            newRemaining = maxRemaining;

          timerDuration = newRemaining;
          editTimeValue = newRemaining;
          timerTotalSec = elapsed + newRemaining;
          drawTimerScreen("Timer", timerDuration, timerTotalSec);
        }
      }
    } else if (currentScreen == SCREEN_SESSION_MENU) {
      if (TEA_COUNT > 1) {
        sessionTeaIndex += stepPlus ? 1 : -1;
        if (sessionTeaIndex < 0)
          sessionTeaIndex = TEA_COUNT - 1;
        if (sessionTeaIndex >= TEA_COUNT)
          sessionTeaIndex = 0;
        drawSessionMenu();
      }
    } else if (currentScreen == SCREEN_SESSION_RUN) {
      if (!sessionRunning && sessionStepIndex < SESSION_STEP_COUNT) {
        sessionStepDurationSec += stepPlus ? 1 : -1;
        if (sessionStepDurationSec < 1)
          sessionStepDurationSec = 1;
        if (sessionStepDurationSec > MAX_TIME)
          sessionStepDurationSec = MAX_TIME;
        sessionStepTotalSec = sessionStepDurationSec;
        drawSessionRun(sessionStepTotalSec);
      }
    } else if (currentScreen == SCREEN_SETTINGS) {
      settingsSelected += stepPlus ? 1 : -1;
      if (settingsSelected < 0)
        settingsSelected = settingsMenuCount - 1;
      if (settingsSelected >= settingsMenuCount)
        settingsSelected = 0;
      drawSettingsMenu();
    }
  }
}

void handleBackButton() {
  if (!backButtonPressedEvent())
    return;

  if (currentScreen == SCREEN_WIFI || currentScreen == SCREEN_ABOUT) {
    currentScreen = SCREEN_SETTINGS;
    drawSettingsMenu();
    return;
  }

  if (currentScreen == SCREEN_TIMER) {
    applyTimerPresetSec(timerTotalSec);
    prefs.putInt(appcfg::PREFS_DURATION_KEY, timerDuration);
    resetSingleTimerRuntimeState();
  }

  if (currentScreen != SCREEN_MENU) {
    if (currentScreen == SCREEN_SESSION_RUN)
      sessionRunning = false;
    goToMenu();
  }
}

void handleSelectButton() {
  if (buttonPressedEvent()) {
    if (currentScreen == SCREEN_MENU) {
      handleMenuSelect();
    } else if (currentScreen == SCREEN_TIMER) {
      // handled by handleTimerButton
    } else if (currentScreen == SCREEN_SESSION_MENU) {
      goToMenu();
    } else if (currentScreen == SCREEN_SESSION_RUN) {

      if (sessionStepIndex >= SESSION_STEP_COUNT) {
        goToMenu();
      } else if (sessionRunning) {
        int stepSec = sessionStepDurationSec > 0
                          ? sessionStepDurationSec
                          : SESSION_STEPS[sessionStepIndex];
        unsigned long elapsed = (millis() - sessionStepStartMs) / 1000;
        int remaining = stepSec - (int)elapsed;
        if (remaining < 0)
          remaining = 0;

        sessionStepDurationSec = remaining;
        sessionRunning = false;
        drawSessionRun(sessionStepDurationSec);
      } else { // resume or start step
        if (sessionStepDurationSec <= 0)
          sessionStepDurationSec = SESSION_STEPS[sessionStepIndex];
        if (sessionStepTotalSec <= 0)
          sessionStepTotalSec = sessionStepDurationSec;

        sessionRunning = true;
        sessionStepStartMs = millis();
        drawSessionRun(sessionStepDurationSec);
      }
    } else if (currentScreen == SCREEN_SETTINGS) {
      handleSettingsSelect();
    } else if (currentScreen == SCREEN_WIFI || currentScreen == SCREEN_ABOUT) {
      currentScreen = SCREEN_SETTINGS;
      drawSettingsMenu();
    } else {
      goToMenu();
    }
  }
}

void handleSessionLongPress() {
  if (currentScreen != SCREEN_SESSION_RUN) {
    swWasDown = false;
    sessionLongPressFired = false;
    return;
  }

  bool down = (digitalRead(ENC_SW) == LOW);

  if (down && !swWasDown) {
    swWasDown = true;
    swHoldStartMs = millis();
    sessionLongPressFired = false;
  }

  if (!down && swWasDown) {
    swWasDown = false;
    sessionLongPressFired = false;
    return;
  }

  if (!down || !swWasDown || sessionLongPressFired)
    return;

  if (millis() - swHoldStartMs < appcfg::SESSION_HOLD_MS)
    return;

  sessionLongPressFired = true;

  sessionStepIndex++;
  sessionCompleteShown = false;

  if (sessionStepIndex >= SESSION_STEP_COUNT) {
    sessionRunning = false;
    drawSessionComplete();
    sessionCompleteShown = true;
    return;
  }

  sessionStepDurationSec = SESSION_STEPS[sessionStepIndex];
  sessionStepTotalSec = sessionStepDurationSec;
  sessionRunning = false;
  drawSessionRun(sessionStepDurationSec);
}

void handleTimerButton() {
  if (currentScreen != SCREEN_TIMER) {
    timerIgnoreReleaseAfterEnter = false;
    timerWasDown = false;
    timerLongPressFired = false;
    timerReleaseGuardUntilMs = 0;
    return;
  }

  const unsigned long now = millis();
  const bool down = (digitalRead(ENC_SW) == LOW);

  // Ignore the click that opened the Timer screen from menu.
  if (timerIgnoreReleaseAfterEnter) {
    if (!down) {
      timerIgnoreReleaseAfterEnter = false;
      timerWasDown = false;
      timerLongPressFired = false;
    }
    return;
  }

  // After long-press, ignore short bounce transitions around release.
  if (now < timerReleaseGuardUntilMs) {
    if (!down)
      timerWasDown = false;
    return;
  }

  // Button pressed
  if (down && !timerWasDown) {
    timerWasDown = true;
    timerHoldStartMs = now;
    timerLongPressFired = false;
    return;
  }

  // long press reset timer to initial value and stop
  if (down && timerWasDown && !timerLongPressFired &&
      (now - timerHoldStartMs >= appcfg::TIMER_HOLD_MS)) {
    timerLongPressFired = true;
    timerReleaseGuardUntilMs = now + appcfg::TIMER_RELEASE_GUARD_MS;
    applyTimerPresetSec(timerTotalSec);
    prefs.putInt(appcfg::PREFS_DURATION_KEY, timerDuration);
    resetSingleTimerRuntimeState();
    drawTimerScreen("Timer", editTimeValue, timerTotalSec);
    return;
  }

  // button released
  if (!down && timerWasDown) {
    timerWasDown = false;

    // if long press is done, short press would be ignored
    if (timerLongPressFired) {
      timerLongPressFired = false;
      return;
    }

    // Short press
    if (singleTimerRunning) {
      // Pause
      unsigned long elapsed = (now - timerStartMillis) / 1000;
      int remaining = timerDuration - (int)elapsed;
      if (remaining < 0)
        remaining = 0;

      singleTimerRunning = false;
      timerDuration = remaining;
      editTimeValue = remaining;

      drawTimerScreen("Timer", remaining, timerTotalSec);
    } else {
      // Start or Resume
      if (!singleTimerStarted) {
        if (editTimeValue < MIN_TIME)
          editTimeValue = MIN_TIME;
        if (editTimeValue > MAX_TIME)
          editTimeValue = MAX_TIME;

        timerTotalSec = editTimeValue;
        timerDuration = editTimeValue;
        singleTimerStarted = true;
        drawTimerScreen("Timer", timerDuration, timerTotalSec);
      }

      if (timerDuration <= 0) {
        timerDuration = timerTotalSec;
      }

      singleTimerRunning = true;
      timerStartMillis = now;
      resetSingleTimerFlowState();
    }
  }
}
