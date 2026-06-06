#include <app/app_state.h>

// menu state
const char *menuItems[] = {"Sessions", "Timer", "History", "Settings"};
const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

const char *settingsItems[] = {"WiFi", "Clock", "Power Save", "Audio", "About"};
const int settingsMenuCount = sizeof(settingsItems) / sizeof(settingsItems[0]);

ScreenState currentScreen = SCREEN_MENU;

AppState app;
