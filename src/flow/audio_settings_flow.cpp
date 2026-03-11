#include <flow/audio_settings_flow.h>

#include <app/app_state.h>
#include <flow/navigation_flow.h>
#include <storage/settings_store.h>
#include <ui.h>

namespace {
AudioRow nextRow(AudioRow row, bool plus) {
  if (plus) {
    return (row == AudioRow::Enabled) ? AudioRow::Profile : AudioRow::Enabled;
  }
  return (row == AudioRow::Profile) ? AudioRow::Enabled : AudioRow::Profile;
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

void audioSettingsRender() { drawAudio(app.audio); }

void audioSettingsEnter() {
  app.audio.selectedRow = AudioRow::Enabled;
  app.audio.editMode = false;
  app.audio.draftAudioEnabled = app.audio.audioEnabled;
  app.audio.draftProfile = app.audio.profile;
}

void audioSettingsHandleEncoder(bool stepPlus, bool stepMinus) {
  if (!stepPlus && !stepMinus)
    return;
  const bool plus = stepPlus;

  if (!app.audio.editMode) {
    app.audio.selectedRow = nextRow(app.audio.selectedRow, plus);
    audioSettingsRender();
    return;
  }

  if (app.audio.selectedRow == AudioRow::Enabled) {
    app.audio.draftAudioEnabled = !app.audio.draftAudioEnabled;
  } else {
    app.audio.draftProfile = nextProfile(app.audio.draftProfile, plus);
  }

  audioSettingsRender();
}

void audioSettingsHandleSelect() {
  app.audio.editMode = !app.audio.editMode;
  audioSettingsRender();
}

void audioSettingsHandleBack() {
  app.audio.audioEnabled = app.audio.draftAudioEnabled;
  app.audio.profile = app.audio.draftProfile;
  settingsStoreSaveAudioEnabled(app.audio.audioEnabled);
  settingsStoreSaveBeepProfile(app.audio.profile);
  showSettingsScreen();
}
