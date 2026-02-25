#pragma once

#include <Adafruit_SSD1306.h>
#include <Preferences.h>

#include <app/session_state.h>
#include <app/timer_state.h>

extern Adafruit_SSD1306 display;
extern Preferences prefs;

// ui/menu
extern const char *menuItems[];
extern const int menuCount;
extern int selected;
extern const char *settingsItems[];
extern const int settingsMenuCount;
extern int settingsSelected;

// general state
extern int editTimeValue;
extern int timerDuration;
extern unsigned long timerStartMillis;
extern bool timerIgnoreReleaseAfterEnter;
extern int timerTotalSec;

// wifi
extern int wifiCount;

// session
extern int sessionTeaIndex;
extern int sessionStepIndex;
extern unsigned long sessionStepStartMs;
extern int sessionStepDurationSec;
extern int sessionStepTotalSec;

extern const char *TEAS[];
extern const int TEA_COUNT;

enum ScreenState {
  SCREEN_MENU,
  SCREEN_SETTINGS,
  SCREEN_ABOUT,
  SCREEN_WIFI,
  SCREEN_TIMER,
  SCREEN_SESSION_MENU,
  SCREEN_SESSION_RUN
};

enum MenuIndex { MENU_START_SESSION, MENU_SESSION, MENU_TIMER, MENU_SETTINGS };

enum SettingsMenuIndex { SETTINGS_WIFI, SETTINGS_ABOUT };

extern ScreenState currentScreen;
