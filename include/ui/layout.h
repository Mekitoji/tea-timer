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

// Header
constexpr int UI_HEADER_LINE_Y = 15;

// Info / Wi-Fi
constexpr int INFO_ROW1_Y = UI_HEADER_LINE_Y + 2;
constexpr int INFO_ROW_STEP_Y = 10;

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

} // namespace layout
} // namespace ui
