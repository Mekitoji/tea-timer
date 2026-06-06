#include <ui/session_history.h>

#include <cstdio>
#include <hw/display.h>
#include <ui/confirm_overlay.h>
#include <ui/header.h>
#include <ui/layout.h>

namespace {
constexpr int VISIBLE_ROWS = 5;

void drawEmpty() {
  drawHeader("HISTORY", "0");
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 24);
  display.print("No sessions");
  display.setCursor(0, 54);
  display.print("Back:Settings");
}

void drawList(const SessionHistoryView &view) {
  char headerRight[12];
  std::snprintf(headerRight, sizeof(headerRight), "%d/%d",
                view.selectedIndex + 1, view.itemCount);
  drawHeader("HISTORY", headerRight);

  int first = view.selectedIndex - VISIBLE_ROWS / 2;
  if (first < 0)
    first = 0;
  if (first > view.itemCount - VISIBLE_ROWS)
    first = view.itemCount - VISIBLE_ROWS;
  if (first < 0)
    first = 0;

  display.setTextSize(1);
  for (int row = 0; row < VISIBLE_ROWS; row++) {
    int index = first + row;
    if (index >= view.itemCount)
      break;

    const SessionHistoryItemView &item = view.items[index];
    int y = ui::layout::MENU_LIST_START_Y + row * ui::layout::MENU_LIST_STEP_Y;
    bool selected = (index == view.selectedIndex);

    if (selected) {
      display.fillRect(ui::layout::MENU_ITEM_BG_X, y - 1,
                       ui::layout::MENU_ITEM_BG_W, ui::layout::MENU_ITEM_H,
                       SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(ui::layout::MENU_ITEM_X, y);
    display.print(index + 1);
    display.print(" ");
    display.print(item.title);
    display.print(" ");
    display.print(item.completedInfusions);
    if (item.finishedEarly)
      display.print("E");
  }
}

void drawDetails(const SessionHistoryView &view) {
  char headerRight[12];
  std::snprintf(headerRight, sizeof(headerRight), "%d/%d",
                view.selectedIndex + 1, view.itemCount);
  drawHeader("DETAIL", headerRight);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print(view.detail.title);

  display.setCursor(0, 28);
  display.print("Inf:");
  display.print(view.detail.completedInfusions);
  display.print(" Rinse:");
  display.print(view.detail.rinseSec);

  display.setCursor(0, 38);
  display.print("Start:");
  display.print(view.detail.startedAt);

  display.setCursor(0, 48);
  display.print("End:");
  display.print(view.detail.finishedAt);

  display.setCursor(0, 58);
  if (view.detail.finishedEarly)
    display.print("Early  ");

  display.print("Hold:Delete");
}
} // namespace

void drawSessionHistory(const SessionHistoryView &view) {
  display.clearDisplay();

  if (view.itemCount <= 0) {
    drawEmpty();
  } else if (view.detailOpen) {
    drawDetails(view);
  } else {
    drawList(view);
  }

  if (view.deleteConfirmActive)
    drawConfirmOverlay("Delete record?", view.deleteConfirmYesSelected);

  display.display();
}
