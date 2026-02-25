#pragma once

struct EncoderStep {
  bool plus;
  bool minus;
};

EncoderStep readEncoderStep();
int encoderAccelStepForTimestamp(unsigned long nowMs);

bool buttonPressedEvent();
bool backButtonPressedEvent();
