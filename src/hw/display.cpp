#include <hw/display.h>

#include <Wire.h>

#include <hw/display_config.h>
#include <hw/pins.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void displayBegin() {
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setRotation(0);
}

void displaySleep() { display.ssd1306_command(SSD1306_DISPLAYOFF); }

void displayWake() { display.ssd1306_command(SSD1306_DISPLAYON); }
