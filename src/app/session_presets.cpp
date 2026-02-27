#include <app/session_presets.h>

namespace {
const int STEPS_GREEN[] = {15, 20, 25, 30, 40, 55, 75, 100, 140};
const int STEPS_WHITE_FRESH[] = {20,  25,  30,  40,  55,  75,
                                 100, 130, 170, 220, 300};
const int STEPS_WHITE_AGED[] = {10,  15,  20,  30,  45,  60, 80,
                                110, 150, 210, 300, 420, 600};
const int STEPS_YELLOW[] = {15, 20, 25, 30, 40, 55, 75, 100, 140};
const int STEPS_OOLONG[] = {8,  10, 12, 15, 18, 22, 28,
                            35, 45, 60, 75, 90, 120, 150};
const int STEPS_TGY[] = {12, 15, 18, 20,  25,  30, 35,
                         45, 55, 70, 90, 120, 150, 180};
const int STEPS_YANCHA[] = {5,  8,  10, 12, 15, 18,  22, 28,
                            35, 45, 60, 75, 90, 120, 150};
const int STEPS_DANCONG[] = {5,  6,  8,  10, 12, 15, 18,
                             22, 28, 35, 45, 60, 75, 90};
const int STEPS_GABA[] = {15, 18, 20, 25, 30, 35, 45, 55, 70, 90, 120, 150};
const int STEPS_RED[] = {10, 12, 15, 18, 22, 28, 35, 45, 60, 90};
const int STEPS_DIANHONG[] = {12, 15, 18, 22, 28, 35, 45, 60, 90, 130};
const int STEPS_SHENG_Y[] = {5,  6,  8,  10, 12, 15, 18, 22,
                             28, 35, 45, 60, 75, 90, 120};
const int STEPS_SHENG_A[] = {6,  8,  10, 12, 15, 18,  22, 28,
                             35, 45, 60, 75, 90, 120, 150};
const int STEPS_SHU[] = {8,  10, 12, 15, 18,  22,  28, 35,
                         45, 60, 75, 90, 120, 150, 180};
const int STEPS_HEICHA[] = {10, 12, 15, 18,  22,  28, 35,
                            45, 60, 75, 90, 120, 150, 180};
} // namespace

const SessionPreset SESSION_PRESETS[] = {
    {"Green", "4-5g/100ml", "75-80C", 0, STEPS_GREEN,
     (int)(sizeof(STEPS_GREEN) / sizeof(STEPS_GREEN[0]))},
    {"White Fresh", "5g/100ml", "85-90C", 0, STEPS_WHITE_FRESH,
     (int)(sizeof(STEPS_WHITE_FRESH) / sizeof(STEPS_WHITE_FRESH[0]))},
    {"White Aged", "5-7g/100ml", "95-100C", 10, STEPS_WHITE_AGED,
     (int)(sizeof(STEPS_WHITE_AGED) / sizeof(STEPS_WHITE_AGED[0]))},
    {"Yellow", "5g/100ml", "80-85C", 0, STEPS_YELLOW,
     (int)(sizeof(STEPS_YELLOW) / sizeof(STEPS_YELLOW[0]))},
    {"Oolong", "6-7g/100ml", "95C", 5, STEPS_OOLONG,
     (int)(sizeof(STEPS_OOLONG) / sizeof(STEPS_OOLONG[0]))},
    {"TieGuanYin", "6-8g/100ml", "90-95C", 10, STEPS_TGY,
     (int)(sizeof(STEPS_TGY) / sizeof(STEPS_TGY[0]))},
    {"Yancha", "6-8g/100ml", "95-100C", 5, STEPS_YANCHA,
     (int)(sizeof(STEPS_YANCHA) / sizeof(STEPS_YANCHA[0]))},
    {"Dancong", "6-8g/100ml", "95-100C", 5, STEPS_DANCONG,
     (int)(sizeof(STEPS_DANCONG) / sizeof(STEPS_DANCONG[0]))},
    {"GABA Oolong", "5-7g/100ml", "90-95C", 5, STEPS_GABA,
     (int)(sizeof(STEPS_GABA) / sizeof(STEPS_GABA[0]))},
    {"Red", "4-6g/100ml", "85-92C", 0, STEPS_RED,
     (int)(sizeof(STEPS_RED) / sizeof(STEPS_RED[0]))},
    {"Dianhong", "4-5.5g/100ml", "90C", 0, STEPS_DIANHONG,
     (int)(sizeof(STEPS_DIANHONG) / sizeof(STEPS_DIANHONG[0]))},
    {"Sheng Young", "6-8g/100ml", "90-95C", 10, STEPS_SHENG_Y,
     (int)(sizeof(STEPS_SHENG_Y) / sizeof(STEPS_SHENG_Y[0]))},
    {"Sheng Aged", "6-8g/100ml", "96-100C", 10, STEPS_SHENG_A,
     (int)(sizeof(STEPS_SHENG_A) / sizeof(STEPS_SHENG_A[0]))},
    {"Shu", "5-7g/100ml", "100C", 5, STEPS_SHU,
     (int)(sizeof(STEPS_SHU) / sizeof(STEPS_SHU[0]))},
    {"Heicha", "6-8g/100ml", "100C", 10, STEPS_HEICHA,
     (int)(sizeof(STEPS_HEICHA) / sizeof(STEPS_HEICHA[0]))},
};

const int SESSION_PRESET_COUNT =
    (int)(sizeof(SESSION_PRESETS) / sizeof(SESSION_PRESETS[0]));
