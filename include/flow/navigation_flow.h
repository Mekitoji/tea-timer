#pragma once

#include <app/app_state.h>

void navigateTo(ScreenState screen);
bool goBack();
bool goBackAndRender();

void showMenuScreen();
void showSettingsScreen();
void showAboutScreen();
void showWiFiScreen();
void showPowerSaveScreen();
