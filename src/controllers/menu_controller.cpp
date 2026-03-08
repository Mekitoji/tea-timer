#include <flow/navigation_flow.h>
#include <flow/timer_flow.h>
#include <ui/session.h>
#include <ui/timer.h>
#include <controllers/menu_controller.h>

#include <app/app_state.h>
#include <ui/menu.h>

bool handleMenuEncoderInput(bool stepPlus, bool stepMinus) {
  if (currentScreen != SCREEN_MENU)
    return false;

  selected += stepPlus ? 1 : -1;
  if (selected < 0)
    selected = menuCount - 1;
  if (selected >= menuCount)
    selected = 0;
  drawMenu();

  return true;
}

bool handleMenuSelectInput() {
  if (currentScreen != SCREEN_MENU)
    return false;

  switch (selected) {
  case MENU_TIMER:
    navigateTo(SCREEN_TIMER);
    applyTimerPresetSec(timerDuration);
    resetSingleTimerRuntimeState();
    timerIgnoreReleaseAfterEnter = true;
    drawTimerScreen("Timer", editTimeValue, timerTotalSec);
    break;

  case MENU_SESSION:
    navigateTo(SCREEN_SESSION_PRESET);
    drawSessionPresetMenu();
    break;

  case MENU_SETTINGS:
    navigateTo(SCREEN_SETTINGS);
    settingsSelected = 0;
    drawSettingsMenu();
    break;

  default:
    break;
  }

  return true;
}
