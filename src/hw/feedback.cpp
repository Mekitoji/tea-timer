#include <hw/feedback.h>

#include <Arduino.h>
#include <hw/audio.h>
#include <hw/pins.h>

void pulseLedAndAudio(int freq, int onMs, bool audioEnabled) {
  digitalWrite(LED_PIN, HIGH);
  if (audioEnabled) {
    buzzerOn(freq);
  }
  delay(onMs);
  buzzerOff();
  digitalWrite(LED_PIN, LOW);
}
