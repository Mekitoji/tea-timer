#include <ui/session_history.h>

#include <Arduino.h>
#include <cstdio>
#include <ctime>
#include <ui/confirm_overlay.h>
#include <ui/header.h>
#include <ui/layout.h>

namespace {
constexpr int VISIBLE_ROWS = 5;

const SessionLogRecord *selectedRecordOrNull(const SessionJournal &journal,
                                             int selectedIndex) {
  if (selectedIndex < 0 || selectedIndex >= journal.recordCount)
    return nullptr;
  return &journal.records[selectedIndex];
}

void formatEpoch(unsigned long epoch, char *out, size_t outSize) {
  if (!out || outSize == 0)
    return;

  if (epoch == 0) {
    std::snprintf(out, outSize, "--");
    return;
  }

  time_t value = static_cast<time_t>(epoch);
  std::tm tmValue = {};
  localtime_r(&value, &tmValue);
  std::snprintf(out, outSize, "%02d/%02d %02d:%02d", tmValue.tm_mday,
                tmValue.tm_mon + 1, tmValue.tm_hour, tmValue.tm_min);
}

void drawEmpty() {
  drawHeader("HISTORY", "0");
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 24);
  display.print("No sessions");
  display.setCursor(0, 54);
  display.print("Back:Settings");
}

void drawList(const SessionJournal &journal,
              const SessionHistoryStateModel &state) {
  char headerRight[12];
  std::snprintf(headerRight, sizeof(headerRight), "%d/%d",
                state.selectedIndex + 1, journal.recordCount);
  drawHeader("HISTORY", headerRight);

  int first = state.selectedIndex - VISIBLE_ROWS / 2;
  if (first < 0)
    first = 0;
  if (first > journal.recordCount - VISIBLE_ROWS)
    first = journal.recordCount - VISIBLE_ROWS;
  if (first < 0)
    first = 0;

  display.setTextSize(1);
  for (int row = 0; row < VISIBLE_ROWS; row++) {
    int index = first + row;
    if (index >= journal.recordCount)
      break;

    const SessionLogRecord &record = journal.records[index];
    int y = ui::layout::MENU_LIST_START_Y + row * ui::layout::MENU_LIST_STEP_Y;
    bool selected = (index == state.selectedIndex);

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
    display.print(record.presetName[0] ? record.presetName : "Session");
    display.print(" ");
    display.print(record.completedInfusionCount);
    if (record.finishedEarly)
      display.print("E");
  }
}

void drawDetails(const SessionJournal &journal,
                 const SessionHistoryStateModel &state) {
  char headerRight[12];
  std::snprintf(headerRight, sizeof(headerRight), "%d/%d",
                state.selectedIndex + 1, journal.recordCount);
  drawHeader("DETAIL", headerRight);

  const SessionLogRecord *record =
      selectedRecordOrNull(journal, state.selectedIndex);
  if (!record)
    return;

  char startBuf[14];
  char finishBuf[14];
  formatEpoch(record->startedAt, startBuf, sizeof(startBuf));
  formatEpoch(record->finishedAt, finishBuf, sizeof(finishBuf));

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print(record->presetName[0] ? record->presetName : "Session");

  display.setCursor(0, 28);
  display.print("Inf:");
  display.print(record->completedInfusionCount);
  display.print(" Rinse:");
  display.print(record->rinseSec);

  display.setCursor(0, 38);
  display.print("Start:");
  display.print(startBuf);

  display.setCursor(0, 48);
  display.print("End:");
  display.print(finishBuf);

  display.setCursor(0, 58);
  display.print("Hold:Delete");
  if (record->finishedEarly)
    display.print(" Early");
}
} // namespace

void drawSessionHistory(const SessionJournal &journal,
                        const SessionHistoryStateModel &state) {
  display.clearDisplay();

  if (journal.recordCount <= 0) {
    drawEmpty();
  } else if (state.detailOpen) {
    drawDetails(journal, state);
  } else {
    drawList(journal, state);
  }

  if (state.deleteConfirm.active)
    drawConfirmOverlay("Delete record?", state.deleteConfirm);

  display.display();
}
