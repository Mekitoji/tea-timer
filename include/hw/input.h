#pragma once

struct EncoderStep {
  bool plus;
  bool minus;
};

EncoderStep readEncoderStep();

bool buttonPressedEvent();
bool backButtonPressedEvent();
