#include <hw/feedback.h>

#include <Arduino.h>
#include <hw/audio.h>
#include <hw/pins.h>

void pulseLedAndSound(int freq, int onMs, bool soundEnabled) {
  digitalWrite(LED_PIN, HIGH);
  if (soundEnabled) {
    buzzerOn(freq);
  }
  delay(onMs);
  buzzerOff();
  digitalWrite(LED_PIN, LOW);
}
