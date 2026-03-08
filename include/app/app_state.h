#pragma once

#include <Adafruit_SSD1306.h>
#include <Preferences.h>

#include <app/confirm_state.h>
#include <app/session_presets.h>
#include <app/session_state.h>
#include <app/timer_state.h>

extern Adafruit_SSD1306 display;
extern Preferences prefs;

// ui/menu
extern const char *menuItems[];
extern const int menuCount;
extern const char *settingsItems[];
extern const int settingsMenuCount;

enum ScreenState {
  SCREEN_MENU,
  SCREEN_SETTINGS,
  SCREEN_ABOUT,
  SCREEN_WIFI,
  SCREEN_TIMER,
  SCREEN_SESSION_PRESET,
  SCREEN_SESSION_RUN,
  SCREEN_POWER_SAVE
};

enum MenuIndex { MENU_SESSION, MENU_TIMER, MENU_SETTINGS };

enum SettingsMenuIndex { SETTINGS_WIFI, SETTINGS_POWER_SAVE, SETTINGS_ABOUT };

extern ScreenState currentScreen;

struct TimerStateModel {
  int editTimeValue = 0;
  int timerDuration = 0;
  unsigned long timerStartMillis = 0;
  bool timerIgnoreReleaseAfterEnter = false;
  int timerTotalSec = 0;
};

struct SessionStateModel {
  ConfirmState endConfirm;
  int presetIndex = 0;
  int stepIndex = 0;
  unsigned long stepStartMs = 0;
  int stepDurationSec = 0;
  int stepTotalSec = 0;
  int stepCount = 0;
  int steps[SESSION_MAX_STEPS] = {0};
  int rinseSec = 0;
  bool rinseActive = false;
};

struct WiFiStateModel {
  ConfirmState resetConfirm;
};

struct UiStateModel {
  int menuSelected = 0;
  int settingsSelected = 0;
};

struct PowerStateModel {
  bool enabled = false;
  bool editEnabled = false;
};

struct AppState {
  TimerStateModel timer;
  SessionStateModel session;
  WiFiStateModel wifi;
  UiStateModel ui;
  PowerStateModel power;
};

extern AppState app;
