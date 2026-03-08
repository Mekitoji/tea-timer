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

// session state
int sessionStepIndex = 0;
unsigned long sessionStepStartMs = 0;
int sessionStepDurationSec = 0;
int sessionStepTotalSec = 0;

int sessionPresetIndex = 0;
int sessionStepCount = 0;
int sessionSteps[SESSION_MAX_STEPS] = {0};
int sessionRinseSec = 0;
bool sessionRinseActive = false;
bool sessionEndConfirmActive = false;
bool sessionEndConfirmYes = false;
bool wifiResetConfirmActive = false;
bool wifiResetConfirmYes = false;

// power save settings
bool powerSaveEnabled = appcfg::DEFAULT_POWER_SAVE_ENABLED;
bool powerSaveEditEnabled = appcfg::DEFAULT_POWER_SAVE_ENABLED;

// menu state
const char *menuItems[] = {"Sessions", "Timer", "Settings"};
const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

const char *settingsItems[] = {"WiFi", "Power Save", "About"};
const int settingsMenuCount = sizeof(settingsItems) / sizeof(settingsItems[0]);

int settingsSelected = 0;

ScreenState currentScreen = SCREEN_MENU;
int selected = 0;
