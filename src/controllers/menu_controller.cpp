#include <controllers/menu_controller.h>

#include <app/app_state.h>
#include <flow/menu_flow.h>
#include <flow/navigation_flow.h>
#include <flow/session_flow.h>
#include <flow/session_history_flow.h>
#include <flow/timer_flow.h>
#include <presentation/session_presenter.h>
#include <presentation/timer_presenter.h>

bool handleMenuEncoderInput(bool stepPlus, bool stepMinus) {
  if (currentScreen != SCREEN_MENU)
    return false;

  app.ui.menuSelected += stepPlus ? 1 : -1;
  if (app.ui.menuSelected < 0)
    app.ui.menuSelected = menuCount - 1;
  if (app.ui.menuSelected >= menuCount)
    app.ui.menuSelected = 0;
  menuRender();

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
    timerRender(app.timer.editTimeValue);
    break;

  case MENU_SESSION:
    navigateTo(SCREEN_SESSION_PRESET);
    sessionPresetRender();
    break;

  case MENU_SETTINGS:
    navigateTo(SCREEN_SETTINGS);
    app.ui.settingsSelected = 0;
    settingsMenuRender();
    break;

  case MENU_HISTORY:
    sessionHistoryEnter();
    break;

  default:
    break;
  }

  return true;
}
