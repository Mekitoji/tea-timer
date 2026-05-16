#include <Arduino.h>
#include <Wire.h>
#include <app/app_controller.h>
#include <app/app_state.h>
#include <app/device_identity.h>
#include <app/tea_config.h>
#include <esp_system.h>
#include <flow/clock_runtime.h>
#include <flow/device_pairing_flow.h>
#include <flow/device_sync_flow.h>
#include <flow/power_flow.h>
#include <flow/session_flow.h>
#include <flow/session_runtime_snapshot_flow.h>
#include <flow/timer_flow.h>
#include <flow/wifi_flow.h>
#include <hw/display_config.h>
#include <hw/input.h>
#include <hw/pins.h>
#include <storage/session_journal_store.h>
#include <storage/device_auth_store.h>
#include <storage/settings_store.h>
#include <ui.h>

void setup() {
  Serial.begin(115200);
  delay(80);
  Serial.printf("[boot] setup reset_reason=%d\n",
                static_cast<int>(esp_reset_reason()));

  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setRotation(0);

  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  pinMode(BACK_SW, INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  settingsStoreBegin();
  deviceAuthStoreBegin();
  sessionJournalStoreBegin();
  deviceIdentityInit();
  devicePairingFlowInit();
  deviceSyncFlowInit();

  app.power.enabled = settingsStoreLoadPowerSaveEnabled();
  app.power.displayOffTimeoutMs = settingsStoreLoadDisplayIdleOffMs();

  setPowerSavingEnabled(app.power.enabled);
  setDisplayIdleOffTimeoutMs(app.power.displayOffTimeoutMs);

  app.audio.profile = settingsStoreLoadBeepProfile();
  app.audio.audioEnabled = settingsStoreLoadAudioEnabled();

  int savedDuration = settingsStoreLoadTimerDurationSec();
  applyTimerPresetSec(savedDuration);
  resetSingleTimerRuntimeState();

  app.clock.autoSyncEnabled = settingsStoreLoadClockAutoSyncEnabled();
  app.clock.source = static_cast<ClockSource>(settingsStoreLoadClockSource());
  app.clock.timeValid = settingsStoreLoadClockValid();
  unsigned long long savedClockEpoch = settingsStoreLoadClockEpoch();
  clockInitializeRuntime(savedClockEpoch);

  wifiInitOnBoot();

  bool sessionRestored = restoreSessionRuntimeSnapshotOnBoot();
  if (!sessionRestored) {
    drawMenu();
  }

  initPowerSaving();
  markUserActivity();
}

void loop() {
  EncoderStep step = readEncoderStep();

  handleEncoderByScreen(step.plus, step.minus);
  handleBackButton();
  handleSelectButton();
  handleLongPress();

  wifiMaintainConnection();

  if (currentScreen == SCREEN_WIFI) {
    updateWiFiScreen();
  }
  if (currentScreen == SCREEN_CLOUD) {
    updateCloudScreen();
  }

  updateMenuClock();
  updateClockRuntime();
  updateClockScreen();
  updateDevicePairingFlow();
  updateDeviceSyncFlow();

  updateSingleTimer();
  updateSessionRun();
  updatePowerSaving();
}
