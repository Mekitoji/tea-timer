#include <controllers/menu_controller.h>

#include <app/app_state.h>
#include <flow/menu_flow.h>
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

  handleMenuSelect();

  return true;
}
