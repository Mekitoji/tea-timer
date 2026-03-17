#pragma once

void clockInitializeRuntime(unsigned long long savedEpoch);
void clockRequestNtpSync();
void updateClockRuntime();
void updateClockScreen();
void clockEnter();
void clockRender();
void clockHandleEncoder(bool stepPlus, bool stepMinus);
void clockHandleSelect();
void clockHandleBack();
