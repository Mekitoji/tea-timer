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

extern const char *TEAS[];
extern const int TEA_COUNT;
