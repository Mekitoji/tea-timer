#include "app/app_state.h"

#include <Wire.h>
#include <app/app_config.h>
#include <hw/display_config.h>
#include <ui.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Preferences prefs;

unsigned long timerStartMillis = 0;
int timerDuration = appcfg::DEFAULT_TIMER_DURATION;
int editTimeValue = appcfg::DEFAULT_TIMER_DURATION;
bool timerIgnoreReleaseAfterEnter = false;
int timerTotalSec = appcfg::DEFAULT_TIMER_DURATION;

int wifiCount = 0;

// session state
int sessionTeaIndex = 0;
int sessionStepIndex = 0;
unsigned long sessionStepStartMs = 0;
int sessionStepDurationSec = 0;
int sessionStepTotalSec = 0;

// menu state
const char *menuItems[] = {"Start Session", "Session", "Timer", "Settings"};
const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

const char *settingsItems[] = {"WiFi", "About"};
const int settingsMenuCount = sizeof(settingsItems) / sizeof(settingsItems[0]);

int settingsSelected = 0;

// tea options
const char *TEAS[] = {"Default Tea"};
const int TEA_COUNT = sizeof(TEAS) / sizeof(TEAS[0]);

ScreenState currentScreen = SCREEN_MENU;
int selected = 0;
