#include <app/confirm_state.h>

void openConfirm(ConfirmState &state) {
  state.active = true;
  state.yesSelected = false;
}

void closeConfirm(ConfirmState &state) {
  state.active = false;
  state.yesSelected = false;
}

void setConfirmChoice(ConfirmState &state, bool yes) {
  state.yesSelected = yes;
}
