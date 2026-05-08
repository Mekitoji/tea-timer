#include <controllers/history_controller.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <flow/power_flow.h>
#include <flow/session_history_flow.h>
#include <hw/pins.h>

bool handleHistoryEncoderInput(bool stepPlus, bool stepMinus) {
  if (currentScreen != SCREEN_SESSION_HISTORY)
    return false;

  sessionHistoryHandleEncoder(stepPlus, stepMinus);
  return true;
}

bool handleHistorySelectInput() {
  if (currentScreen != SCREEN_SESSION_HISTORY)
    return false;

  // Normal history select is resolved on button release so hold can become
  // delete-confirm without first toggling list/detail state.
  if (app.history.deleteConfirm.active)
    sessionHistoryHandleSelect();

  return true;
}

bool handleHistoryBackInput() {
  if (currentScreen != SCREEN_SESSION_HISTORY)
    return false;

  sessionHistoryHandleBack();
  return true;
}

void handleHistoryLongPressInput() {
  if (isWakeInputGuardActive())
    return;
  if (currentScreen != SCREEN_SESSION_HISTORY) {
    resetSessionHistoryLongPressState();
    return;
  }

  const bool down = (digitalRead(ENC_SW) == LOW);
  sessionHistoryHandleLongPress(down, millis());
}
