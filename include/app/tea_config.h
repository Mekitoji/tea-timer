#pragma once

constexpr int MIN_TIME = 5;
constexpr int MAX_TIME = 600;

inline constexpr int SESSION_STEPS[] = {5, // rinse
                                        10, 15, 20, 25,  35, 45,
                                        60, 75, 90, 105, 120};

constexpr int SESSION_STEP_COUNT =
    sizeof(SESSION_STEPS) / sizeof(SESSION_STEPS[0]);
