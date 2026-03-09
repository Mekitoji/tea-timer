#include <flow/audio_profile_flow.h>

#include <app/app_state.h>

int audioProfileCountdownFreq() {
  switch (app.audio.profile) {
  case BeepProfile::Soft:
    return 1600;
  case BeepProfile::Loud:
    return 2800;
  case BeepProfile::Normal:
  default:
    return 2200;
  }
}

int audioProfileTimerDoneFreq() {
  switch (app.audio.profile) {
  case BeepProfile::Soft:
    return 1900;
  case BeepProfile::Loud:
    return 3200;
  case BeepProfile::Normal:
  default:
    return 2600;
  }
}

int audioProfileSessionStepDoneFreq() {
  switch (app.audio.profile) {
  case BeepProfile::Soft:
    return 1800;
  case BeepProfile::Loud:
    return 3000;
  case BeepProfile::Normal:
  default:
    return 2500;
  }
}

int audioProfileSessionDoneFreq() {
  switch (app.audio.profile) {
  case BeepProfile::Soft:
    return 2000;
  case BeepProfile::Loud:
    return 3400;
  case BeepProfile::Normal:
  default:
    return 3000;
  }
}

int audioProfileBeepDurationMs() {
  switch (app.audio.profile) {
  case BeepProfile::Soft:
    return 40;
  case BeepProfile::Loud:
    return 110;
  case BeepProfile::Normal:
  default:
    return 70;
  }
}
