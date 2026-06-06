#pragma once

#include <ui/models/about_view.h>
#include <ui/models/audio_settings_view.h>
#include <ui/models/clock_settings_view.h>
#include <ui/models/menu_view.h>
#include <ui/models/power_settings_view.h>
#include <ui/models/session_history_view.h>
#include <ui/models/session_view.h>
#include <ui/models/timer_view.h>
#include <ui/models/wifi_settings_view.h>

void drawMenu(const MenuView &view);
void drawSettingsMenu(const MenuView &view);
void drawAbout(const AboutView &view);

void drawTimerScreen(const TimerView &view);

void drawSessionPresetMenu(const SessionPresetView &view);
void drawSessionComplete(const SessionCompleteView &view);
void drawSessionRun(const SessionRunView &view);
void drawSessionHistory(const SessionHistoryView &view);

void drawAudio(const AudioSettingsView &view);
void drawClock(const ClockSettingsView &view);
void drawPowerSave(const PowerSettingsView &view);

void drawWiFi(const WifiSettingsView &view);
