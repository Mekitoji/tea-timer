#pragma once

enum class LongPressEvent {
  None,
  ShortReleased,
  LongPressed,
};

struct LongPressTracker {
  unsigned long holdStartMs = 0;
  bool wasDown = false;
  bool longPressFired = false;

  void reset() {
    wasDown = false;
    longPressFired = false;
  }

  LongPressEvent update(bool down, unsigned long nowMs,
                        unsigned long holdMs) {
    if (down && !wasDown) {
      wasDown = true;
      holdStartMs = nowMs;
      longPressFired = false;
      return LongPressEvent::None;
    }

    if (!down && wasDown) {
      bool shortReleased = !longPressFired;
      reset();
      return shortReleased ? LongPressEvent::ShortReleased
                           : LongPressEvent::None;
    }

    if (!down || !wasDown || longPressFired)
      return LongPressEvent::None;

    if (nowMs - holdStartMs < holdMs)
      return LongPressEvent::None;

    longPressFired = true;
    return LongPressEvent::LongPressed;
  }
};
