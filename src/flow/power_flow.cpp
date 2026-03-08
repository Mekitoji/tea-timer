#include <flow/power_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <driver/gpio.h>
#include <esp_sleep.h>
#include <flow/wifi_flow.h>
#include <hw/pins.h>

namespace {
unsigned long lastActivityMs = 0;
bool displaySleeping = false;
bool powerSavingEnabled = appcfg::DEFAULT_POWER_SAVE_ENABLED;
unsigned long wakeInputGuardUntilMs = 0;
} // namespace

bool canEnterLightSleep() {
  if (isTimerRunning() || isSessionRunning())
    return false;
  if (currentScreen == SCREEN_WIFI)
    return false;

  // Avoid entering sleep while an input line is already active.
  if (digitalRead(ENC_SW) == LOW || digitalRead(BACK_SW) == LOW ||
      digitalRead(ENC_A) == LOW || digitalRead(ENC_B) == LOW)
    return false;

  return true;
}

void initPowerSaving() {
  gpio_wakeup_enable((gpio_num_t)ENC_SW, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable((gpio_num_t)BACK_SW, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable((gpio_num_t)ENC_A, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable((gpio_num_t)ENC_B, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
}

void markUserActivity() {
  lastActivityMs = millis();

  if (displaySleeping) {
    display.ssd1306_command(SSD1306_DISPLAYON);
    displaySleeping = false;
  }
}

void updatePowerSaving() {
  if (!powerSavingEnabled)
    return;

  if (currentScreen == SCREEN_WIFI && wifiProvisionIsActive()) {
    lastActivityMs = millis();
    if (displaySleeping) {
      display.ssd1306_command(SSD1306_DISPLAYON);
      displaySleeping = false;
    }
    return;
  }

  if (lastActivityMs == 0)
    lastActivityMs = millis();

  if (isTimerRunning() || isSessionRunning()) {
    lastActivityMs = millis();
    if (displaySleeping) {
      display.ssd1306_command(SSD1306_DISPLAYON);
      displaySleeping = false;
    }
    return;
  }

  unsigned long idleMs = millis() - lastActivityMs;

  if (!displaySleeping && idleMs >= appcfg::DISPLAY_IDLE_OFF_MS) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    displaySleeping = true;
  }

  if (!appcfg::ENABLE_LIGHT_SLEEP)
    return;

  if (idleMs < appcfg::LIGHT_SLEEP_IDLE_MS)
    return;

  if (!canEnterLightSleep())
    return;

  esp_light_sleep_start();

  wakeInputGuardUntilMs = millis() + appcfg::INPUT_DEBOUNCE_MS * 2;
  lastActivityMs = millis();

  if (displaySleeping) {
    display.ssd1306_command(SSD1306_DISPLAYON);
    displaySleeping = false;
  }
}

bool markUserActivityAndConsumeIfWoke() {
  bool woke = displaySleeping;
  markUserActivity();
  if (woke) {
    wakeInputGuardUntilMs = millis() + appcfg::INPUT_DEBOUNCE_MS * 2;
    return true;
  }
  return false;
}

bool isWakeInputGuardActive() { return millis() < wakeInputGuardUntilMs; }

void setPowerSavingEnabled(bool enabled) {
  powerSavingEnabled = enabled;
  lastActivityMs = millis();
  wakeInputGuardUntilMs = 0;

  if (!powerSavingEnabled) {
    if (displaySleeping) {
      display.ssd1306_command(SSD1306_DISPLAYON);
      displaySleeping = false;
    }
  }
}

bool isPowerSavingEnabled() { return powerSavingEnabled; }
