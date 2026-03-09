#include <app/app_state.h>

#include <Wire.h>
#include <hw/display_config.h>
#include <ui.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// menu state
const char *menuItems[] = {"Sessions", "Timer", "Settings"};
const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

const char *settingsItems[] = {"WiFi", "Power Save", "Sound", "About"};
const int settingsMenuCount = sizeof(settingsItems) / sizeof(settingsItems[0]);

ScreenState currentScreen = SCREEN_MENU;

AppState app;
