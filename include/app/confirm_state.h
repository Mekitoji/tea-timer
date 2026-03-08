#pragma once

struct ConfirmState {
  bool active = false;
  bool yesSelected = false;
};

void openConfirm(ConfirmState &state);
void closeConfirm(ConfirmState &state);
void setConfirmChoice(ConfirmState &state, bool yes);
