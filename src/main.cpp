#include <Arduino.h>
#include <Wire.h>
#include <app/app_config.h>
#include <app/app_controller.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/session_flow.h>
#include <flow/timer_flow.h>
#include <hw/display_config.h>
#include <hw/input.h>
#include <hw/pins.h>
#include <ui.h>

void setup() {
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

  // Load saved timer duration (NVS)
  prefs.begin(appcfg::PREFS_NAMESPACE, false);

  int savedDuration =
      prefs.getInt(appcfg::PREFS_DURATION_KEY, appcfg::DEFAULT_TIMER_DURATION);
  applyTimerPresetSec(savedDuration);
  resetSingleTimerRuntimeState();

  drawMenu();
}

void loop() {
  EncoderStep step = readEncoderStep();
  handleEncoderByScreen(step.plus, step.minus);
  handleBackButton();
  handleSelectButton();
  handleSessionLongPress();
  handleTimerButton();
  if (currentScreen == SCREEN_WIFI) {
    updateWiFiScreen();
  }
  updateSingleTimer();
  updateSessionRun();
}
