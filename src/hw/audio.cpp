#include <hw/audio.h>

#include <Arduino.h>
#include <hw/pins.h>

void buzzerOn(int freq) { tone(BUZZER_PIN, freq); }
void buzzerOff() { noTone(BUZZER_PIN); }
