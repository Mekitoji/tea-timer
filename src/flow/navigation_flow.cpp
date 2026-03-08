#include <flow/navigation_flow.h>

#include <app/app_state.h>

void navigateTo(ScreenState screen) { currentScreen = screen; }

bool goBack() {
  switch (currentScreen) {
  case SCREEN_ABOUT:
  case SCREEN_POWER_SAVE:
  case SCREEN_WIFI:
    navigateTo(SCREEN_SETTINGS);
    return true;
  case SCREEN_SETTINGS:
  case SCREEN_TIMER:
  case SCREEN_SESSION_PRESET:
    navigateTo(SCREEN_MENU);
    return true;
  case SCREEN_SESSION_RUN:
    navigateTo(SCREEN_SESSION_PRESET);
    return true;
  default:
    return false;
  }
}
