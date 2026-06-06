#pragma once

#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;

bool displayBegin();
void displaySleep();
void displayWake();
