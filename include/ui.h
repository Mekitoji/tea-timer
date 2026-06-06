#pragma once

#include <ui/models/about_view.h>
#include <ui/models/audio_settings_view.h>
#include <ui/models/clock_settings_view.h>
#include <ui/models/power_settings_view.h>
#include <ui/models/session_history_view.h>
#include <ui/models/wifi_settings_view.h>

void drawMenu();
void drawSettingsMenu();
void drawAbout(const AboutView &view);

void drawTimerScreen(const char *title, int secondsLeft, int totalSeconds);

void drawSessionPresetMenu();
void drawSessionComplete();
void drawSessionRun(int remaining);
void drawSessionHistory(const SessionHistoryView &view);

void drawAudio(const AudioSettingsView &view);
void drawClock(const ClockSettingsView &view);
void drawPowerSave(const PowerSettingsView &view);

void drawWiFi(const WifiSettingsView &view);

void updateMenuClock();
