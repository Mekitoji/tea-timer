#pragma once

constexpr int MIN_TIME = 1;
constexpr int MAX_TIME = 600;

inline int clampTeaDurationSec(int sec) {
  if (sec < MIN_TIME)
    return MIN_TIME;
  if (sec > MAX_TIME)
    return MAX_TIME;
  return sec;
}

inline int clampOptionalTeaDurationSec(int sec) {
  if (sec <= 0)
    return 0;
  return clampTeaDurationSec(sec);
}
