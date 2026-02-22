#include "input.h"
#include <Arduino.h>
#include <pins.h>

bool buttonPressedEvent() {
  static bool lastStable = HIGH;
  static bool lastRead = HIGH;
  static unsigned long lastChange = 0;
  const unsigned long DEBOUNCE = 30;

  bool r = digitalRead(ENC_SW);
  unsigned long t = millis();

  if (r != lastRead) {
    lastRead = r;
    lastChange = t;
  }

  if ((t - lastChange) > DEBOUNCE && lastStable != lastRead) {
    lastStable = lastRead;
    if (lastStable == LOW) {
      while (digitalRead(ENC_SW) == LOW)
        delay(5);
      delay(10);
      return true;
    }
  }
  return false;
}

bool backButtonPressedEvent() {
  static bool lastStable = HIGH;
  static bool lastRead = HIGH;
  static unsigned long lastChange = 0;
  const unsigned long DEBOUNCE = 30;

  bool r = digitalRead(BACK_SW);
  unsigned long t = millis();

  if (r != lastRead) {
    lastRead = r;
    lastChange = t;
  }

  if ((t - lastChange) > DEBOUNCE && lastStable != lastRead) {
    lastStable = lastRead;
    if (lastStable == LOW) {
      while (digitalRead(BACK_SW) == LOW)
        delay(5);
      delay(10);
      return true;
    }
  }
  return false;
}

EncoderStep readEncoderStep() {
  static int lastEncoded = 0;
  static long encoderAccum = 0;

  int a = digitalRead(ENC_A);
  int b = digitalRead(ENC_B);

  int encoded = (a << 1) | b;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderAccum++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderAccum--;

  lastEncoded = encoded;

  EncoderStep step = {false, false};

  if (encoderAccum >= 4) {
    step.plus = true;
    encoderAccum = 0;
  }
  if (encoderAccum <= -4) {
    step.minus = true;
    encoderAccum = 0;
  }

  return step;
}
