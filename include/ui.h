#pragma once

struct AudioStateModel;
struct PowerStateModel;
struct SessionHistoryStateModel;
struct SessionJournal;

void drawMenu();
void drawSettingsMenu();
void drawAbout();

void drawTimerScreen(const char *title, int secondsLeft, int totalSeconds);

void drawSessionPresetMenu();
void drawSessionComplete();
void drawSessionRun(int remaining);
void drawSessionHistory(const SessionJournal &journal,
                        const SessionHistoryStateModel &state);

void drawAudio(const AudioStateModel &audioState);
void drawClock();
void drawPowerSave(const PowerStateModel &powerState);

void drawWiFi();

void updateMenuClock();
