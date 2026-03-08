#include <hw/audio.h>

#include <Arduino.h>
#include <hw/pins.h>

void buzzerOn(int freq) { tone(BUZZER_PIN, freq); }
void buzzerOff() { noTone(BUZZER_PIN); }

void beep(int freq, int ms) {
  buzzerOn(freq);
  delay(ms);
  buzzerOff();
}
