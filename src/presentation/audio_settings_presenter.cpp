#include <presentation/audio_settings_presenter.h>

#include <app/app_state.h>
#include <ui/settings/audio.h>

namespace {
const char *beepProfileText(BeepProfile profile) {
  switch (profile) {
  case BeepProfile::Soft:
    return "Soft";
  case BeepProfile::Loud:
    return "Loud";
  case BeepProfile::Normal:
  default:
    return "Normal";
  }
}

AudioSettingsView buildAudioSettingsView() {
  AudioSettingsView view;
  view.enabled = app.audio.draftAudioEnabled;
  view.profile = beepProfileText(app.audio.draftProfile);
  view.enabledSelected = app.audio.selectedRow == AudioRow::Enabled;
  view.profileSelected = app.audio.selectedRow == AudioRow::Profile;
  view.editMode = app.audio.editMode;
  return view;
}
} // namespace

void audioSettingsRender() { drawAudio(buildAudioSettingsView()); }
