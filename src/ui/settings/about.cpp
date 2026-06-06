#include <ui/settings/about.h>

#include <stdio.h>

#include <hw/display.h>
#include <ui/header.h>
#include <ui/layout.h>

namespace {
void formatUptime(unsigned long seconds, char *out, size_t size) {
  unsigned long days = seconds / 86400UL;
  unsigned long hours = (seconds % 86400UL) / 3600UL;
  unsigned long minutes = (seconds % 3600UL) / 60UL;
  unsigned long secs = seconds % 60UL;

  if (days > 0) {
    snprintf(out, size, "%lud %luh", days, hours);
  } else if (hours > 0) {
    snprintf(out, size, "%luh %lum", hours, minutes);
  } else if (minutes > 0) {
    snprintf(out, size, "%lum %lus", minutes, secs);
  } else {
    snprintf(out, size, "%lus", secs);
  }
}
} // namespace

void drawAbout(const AboutView &view) {
  display.clearDisplay();
  drawHeader("About Device");

  display.setCursor(0, ui::layout::INFO_ROW1_Y);
  display.print("FW: ");
  display.print(view.firmwareVersion);

  display.setCursor(0, ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y);
  display.print("FS free: ");
  if (view.storageAvailable) {
    display.print(view.storageFreeKb);
    display.print("KB");
  } else {
    display.print("N/A");
  }

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 2);
  display.print("Heap: ");
  display.print(view.freeHeap);

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 3);

  char uptimeBuf[16];
  formatUptime(view.uptimeSeconds, uptimeBuf, sizeof(uptimeBuf));
  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 4);
  display.print("Up: ");
  display.print(uptimeBuf);

  display.display();
}
