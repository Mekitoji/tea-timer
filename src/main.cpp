#include <Arduino.h>
#include <Wire.h>
#include <app/app_config.h>
#include <app/app_controller.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <esp_system.h>
#include <flow/power_flow.h>
#include <flow/session_flow.h>
#include <flow/timer_flow.h>
#include <flow/wifi_flow.h>
#include <hw/display_config.h>
#include <hw/input.h>
#include <hw/pins.h>
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
  app.power.enabled = settingsStoreLoadPowerSaveEnabled();
  app.power.editEnabled = app.power.enabled;
  setPowerSavingEnabled(app.power.enabled);

  int savedDuration = settingsStoreLoadTimerDurationSec();
  applyTimerPresetSec(savedDuration);
  resetSingleTimerRuntimeState();

  wifiInitOnBoot();

  drawMenu();

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
  updateSingleTimer();
  updateSessionRun();
  updatePowerSaving();
}
