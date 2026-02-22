#pragma once

#include <Adafruit_SSD1306.h>
#include <Preferences.h>

extern Adafruit_SSD1306 display;
extern Preferences prefs;

// ui/menu
extern const char *menuItems[];
extern const int menuCount;
extern int selected;

// general state
extern int editTimeValue;
extern int timerDuration;
extern unsigned long timerStartMillis;

// wifi
extern int wifiCount;

// session
extern int sessionTeaIndex;
extern int sessionStepIndex;
extern bool sessionRunning;
extern unsigned long sessionStepStartMs;
extern bool sessionCompleteShown;
extern int sessionStepDurationSec;
extern int sessionStepTotalSec;

extern const char *TEAS[];
extern const int TEA_COUNT;

void goToMenu();

enum ScreenState {
  SCREEN_MENU,
  SCREEN_ABOUT,
  SCREEN_WIFI,
  SCREEN_TIMER,
  SCREEN_SET_TIME,
  SCREEN_SESSION_MENU,
  SCREEN_SESSION_RUN
};

enum MenuIndex {
  MENU_START_SESSION,
  MENU_SESSION,
  MENU_START,
  MENU_SET_TIME,
  MENU_WIFI,
  MENU_ABOUT
};

extern ScreenState currentScreen;
