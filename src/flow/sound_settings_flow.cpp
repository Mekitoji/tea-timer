#include <flow/sound_settings_flow.h>

#include <app/app_state.h>
#include <flow/navigation_flow.h>
#include <storage/settings_store.h>
#include <ui.h>

namespace {
SoundRow nextRow(SoundRow row, bool plus) {
  if (plus) {
    return (row == SoundRow::Enabled) ? SoundRow::Profile
                                      : SoundRow::Enabled;
  }
  return (row == SoundRow::Profile) ? SoundRow::Enabled : SoundRow::Profile;
}

BeepProfile nextProfile(BeepProfile p, bool plus) {
  int v = static_cast<int>(p) + (plus ? 1 : -1);
  if (v > static_cast<int>(BeepProfile::Loud))
    v = static_cast<int>(BeepProfile::Soft);
  if (v < static_cast<int>(BeepProfile::Soft))
    v = static_cast<int>(BeepProfile::Loud);
  return static_cast<BeepProfile>(v);
}
} // namespace

void soundSettingsRender() {
  drawSound(app.ui.soundDraft, app.ui.soundSelected, app.ui.soundEditMode);
}

void soundSettingsEnter() {
  app.ui.soundSelected = SoundRow::Enabled;
  app.ui.soundEditMode = false;
  app.ui.soundDraft = app.audio;
}

void soundSettingsHandleEncoder(bool stepPlus, bool stepMinus) {
  if (!stepPlus && !stepMinus)
    return;
  const bool plus = stepPlus; // в проекте за тик обычно только одно направление

  if (!app.ui.soundEditMode) {
    app.ui.soundSelected = nextRow(app.ui.soundSelected, plus);
    soundSettingsRender();
    return;
  }

  if (app.ui.soundSelected == SoundRow::Enabled) {
    app.ui.soundDraft.soundEnabled = !app.ui.soundDraft.soundEnabled;
  } else {
    app.ui.soundDraft.profile = nextProfile(app.ui.soundDraft.profile, plus);
  }

  soundSettingsRender();
}

void soundSettingsHandleSelect() {
  app.ui.soundEditMode = !app.ui.soundEditMode;
  soundSettingsRender();
}

void soundSettingsHandleBack() {
  app.audio = app.ui.soundDraft;
  settingsStoreSaveSoundEnabled(app.audio.soundEnabled);
  settingsStoreSaveBeepProfile(app.audio.profile);
  showSettingsScreen();
}
