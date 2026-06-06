#include <presentation/about_presenter.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <ui/settings/about.h>

void aboutRender() {
  AboutView view;
  view.chip = "ESP32-C3";
  view.flashMb = ESP.getFlashChipSize() / 1024 / 1024;
  view.freeHeap = ESP.getFreeHeap();
  view.firmwareVersion = appcfg::FIRMWARE_VERSION;
  drawAbout(view);
}
