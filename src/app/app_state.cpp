#include <Wire.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <hw/display_config.h>
#include <ui.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Preferences prefs;

unsigned long timerStartMillis = 0;
int timerDuration = appcfg::DEFAULT_TIMER_DURATION;
int editTimeValue = appcfg::DEFAULT_TIMER_DURATION;

int wifiCount = 0;

int sessionTeaIndex = 0;
int sessionStepIndex = 0;
bool sessionRunning = false;
unsigned long sessionStepStartMs = 0;
bool sessionCompleteShown = false;

const char *menuItems[] = {"Start Session", "Session", "Start",
                           "Set time",      "WiFi",    "About"};
const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

const char *TEAS[] = {"Default Tea"};
const int TEA_COUNT = sizeof(TEAS) / sizeof(TEAS[0]);

ScreenState currentScreen = SCREEN_MENU;
int selected = 0;

void goToMenu() {
  currentScreen = SCREEN_MENU;
  drawMenu();
}
