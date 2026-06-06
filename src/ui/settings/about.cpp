#include <ui/settings/about.h>

#include <hw/display.h>
#include <ui/header.h>
#include <ui/layout.h>

void drawAbout(const AboutView &view) {
  display.clearDisplay();
  drawHeader("About Device");

  display.setCursor(0, ui::layout::INFO_ROW1_Y);
  display.print("Chip: ");
  display.print(view.chip);

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
  display.print("FW: ");
  display.print(view.firmwareVersion);

  display.display();
}
