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

  app.ui.menuSelected += stepPlus ? 1 : -1;
  if (app.ui.menuSelected < 0)
    app.ui.menuSelected = menuCount - 1;
  if (app.ui.menuSelected >= menuCount)
    app.ui.menuSelected = 0;
  drawMenu();

  return true;
}

bool handleMenuSelectInput() {
  if (currentScreen != SCREEN_MENU)
    return false;

  switch (app.ui.menuSelected) {
  case MENU_TIMER:
    navigateTo(SCREEN_TIMER);
    applyTimerPresetSec(app.timer.timerDuration);
    resetSingleTimerRuntimeState();
    app.timer.timerIgnoreReleaseAfterEnter = true;
    drawTimerScreen("Timer", app.timer.editTimeValue, app.timer.timerTotalSec);
    break;

  case MENU_SESSION:
    navigateTo(SCREEN_SESSION_PRESET);
    drawSessionPresetMenu();
    break;

  case MENU_SETTINGS:
    navigateTo(SCREEN_SETTINGS);
    app.ui.settingsSelected = 0;
    drawSettingsMenu();
    break;

  default:
    break;
  }

  return true;
}
