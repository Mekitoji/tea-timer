#include <ui/settings/about.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <ui/header.h>
#include <ui/layout.h>

void drawAbout() {
  display.clearDisplay();
  drawHeader("About Device");

  display.setCursor(0, ui::layout::INFO_ROW1_Y);
  display.print("Chip: ESP32-C3");

  display.setCursor(0, ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y);
  display.print("Flash: ");
  display.print(ESP.getFlashChipSize() / 1024 / 1024);
  display.print("MB");

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 2);
  display.print("Heap: ");
  display.print(ESP.getFreeHeap());

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 3);
  display.print("FW: v1.0.0");

  display.display();
}
