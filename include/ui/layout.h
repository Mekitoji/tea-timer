#pragma once

namespace ui {
namespace layout {

// Common menu rows
constexpr int MENU_ITEM_X = 4;
constexpr int MENU_LIST_START_Y = 17;
constexpr int MENU_LIST_STEP_Y = 8;
constexpr int MENU_ITEM_H = 8;
constexpr int MENU_ITEM_BG_X = 0;
constexpr int MENU_ITEM_BG_W = 128;

// Progress bar
constexpr int PROGRESS_X = 6;
constexpr int PROGRESS_W = 116;
constexpr int PROGRESS_H = 8;
constexpr int TIMER_PROGRESS_Y = 54;
constexpr int SESSION_PROGRESS_Y = 46;

// Timer screen
constexpr int TIMER_VALUE_X = 40;
constexpr int TIMER_VALUE_Y = 22;

// Timer status badge
constexpr int TIMER_STATUS_X = 82;
constexpr int TIMER_STATUS_Y = 1;
constexpr int TIMER_STATUS_W = 44;
constexpr int TIMER_STATUS_H = 11;
constexpr int TIMER_STATUS_TEXT_X = 85;
constexpr int TIMER_STATUS_TEXT_Y = 3;

// Header
constexpr int UI_HEADER_LINE_Y = 15;

// Info / Wi-Fi
constexpr int INFO_ROW1_Y = UI_HEADER_LINE_Y + 2;
constexpr int INFO_ROW_STEP_Y = 10;
constexpr int WIFI_LIST_START_Y = 18;
constexpr int WIFI_LIST_STEP_Y = 10;
constexpr int WIFI_EMPTY_Y = 22;
constexpr int WIFI_SCAN_MSG_Y = 23;

// Session menu
constexpr int SESSION_MENU_TEA_Y = 18;
constexpr int SESSION_MENU_TEA_VALUE_X = 30;
constexpr int SESSION_MENU_STEPS_Y = 34;
constexpr int SESSION_MENU_STEPS_VALUE_X = 45;
constexpr int SESSION_MENU_HINT_Y = 54;

// Session complete
constexpr int SESSION_COMPLETE_TITLE_X = 8;
constexpr int SESSION_COMPLETE_TITLE_Y = 24;
constexpr int SESSION_COMPLETE_TEA_Y = 42;
constexpr int SESSION_COMPLETE_HINT_Y = 54;

// Session run
constexpr int SESSION_RUN_TEA_Y = 18;
constexpr int SESSION_RUN_TEA_VALUE_X = 28;
constexpr int SESSION_RUN_STEP_Y = 28;
constexpr int SESSION_RUN_INFUSE_Y = 38;
constexpr int SESSION_RUN_TIMER_X = 78;
constexpr int SESSION_RUN_TIMER_Y = 28;
constexpr int SESSION_RUN_HINT_Y = 56;

// Status box
constexpr int SESSION_STATUS_X = 74;
constexpr int SESSION_STATUS_Y = 1;
constexpr int SESSION_STATUS_W = 52;
constexpr int SESSION_STATUS_H = 11;
constexpr int SESSION_STATUS_TEXT_X = 77;
constexpr int SESSION_STATUS_TEXT_Y = 3;

} // namespace layout
} // namespace ui
