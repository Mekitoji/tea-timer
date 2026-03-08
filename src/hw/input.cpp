#include <hw/input.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <hw/pins.h>

namespace {
struct ButtonDebounceState {
  bool lastStable = HIGH;
  bool lastRead = HIGH;
  unsigned long lastChange = 0;
};

bool pressedEventForPin(int pin, ButtonDebounceState &state) {
  const unsigned long DEBOUNCE = appcfg::INPUT_DEBOUNCE_MS;

  bool r = digitalRead(pin);
  unsigned long t = millis();

  if (r != state.lastRead) {
    state.lastRead = r;
    state.lastChange = t;
  }

  if ((t - state.lastChange) > DEBOUNCE && state.lastStable != state.lastRead) {
    state.lastStable = state.lastRead;
    if (state.lastStable == LOW)
      return true;
  }

  return false;
}

} // namespace

bool buttonPressedEvent() {
  static ButtonDebounceState encState;
  return pressedEventForPin(ENC_SW, encState);
}

bool backButtonPressedEvent() {
  static ButtonDebounceState backState;
  return pressedEventForPin(BACK_SW, backState);
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
