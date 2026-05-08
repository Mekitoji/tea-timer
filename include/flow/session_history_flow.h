#pragma once

void sessionHistoryEnter();
void sessionHistoryRender();
void sessionHistoryHandleEncoder(bool stepPlus, bool stepMinus);
void sessionHistoryHandleSelect();
void sessionHistoryHandleBack();
void sessionHistoryHandleLongPress(bool down, unsigned long nowMs);
void resetSessionHistoryLongPressState();
