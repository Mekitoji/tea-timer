#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>
#include <display_config.h>
#include <input.h>
#include <pins.h>
#include <tea_config.h>
#include <ui.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Preferences prefs;

// ---- single timer ----
unsigned long timerStartMillis = 0;
int timerDuration = 10;
int editTimeValue = 10;

// ---- WiFi scan ----
int wifiCount = 0;

unsigned long swHoldStartMs = 0;
bool swWasDown = false;

int sessionTeaIndex = 0;
int sessionStepIndex = 0;
bool sessionRunning = false;
unsigned long sessionStepStartMs = 0;
bool sessionCompleteShown = false;

// ---- menu ----
const char *menuItems[] = {"Start Session", "Session", "Start",
                           "Set time",      "WiFi",    "About"};
extern const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

// ---- session UI ----
const char *TEAS[] = {"Default Tea"};
const int TEA_COUNT = sizeof(TEAS) / sizeof(TEAS[0]);

// ---- screens ----
enum ScreenState {
  SCREEN_MENU,
  SCREEN_ABOUT,
  SCREEN_WIFI,
  SCREEN_TIMER,
  SCREEN_SET_TIME,
  SCREEN_SESSION_MENU,
  SCREEN_SESSION_RUN
};

enum MenuIndex {
  MENU_START_SESSION,
  MENU_SESSION,
  MENU_START,
  MENU_SET_TIME,
  MENU_WIFI,
  MENU_ABOUT
};

ScreenState currentScreen = SCREEN_MENU;

int selected = 0;

// ---- encoder accel for Set Time ----
unsigned long lastStepMs = 0;

// ---------- buzzer ----------
void buzzerOn(int freq) { tone(BUZZER_PIN, freq); }
void buzzerOff() { noTone(BUZZER_PIN); }

void beep(int freq, int ms) {
  buzzerOn(freq);
  delay(ms);
  buzzerOff();
}

// ---------- UI primitives ----------

void goToMenu() {
  currentScreen = SCREEN_MENU;
  drawMenu();
}

// ---------- setup ----------
void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setRotation(0);

  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  pinMode(BACK_SW, INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Load saved timer duration (NVS)
  prefs.begin("tea_timer", false);
  timerDuration = prefs.getInt("dur_sec", 10);
  if (timerDuration < MIN_TIME)
    timerDuration = MIN_TIME;
  if (timerDuration > MAX_TIME)
    timerDuration = MAX_TIME;
  editTimeValue = timerDuration;

  drawMenu();
}

// ---------- loop ----------
void loop() {

  EncoderStep step = readEncoderStep();
  bool stepPlus = step.plus;
  bool stepMinus = step.minus;

  // ---- apply encoder steps by screen ----
  if (stepPlus || stepMinus) {
    if (currentScreen == SCREEN_MENU) {
      selected += stepPlus ? 1 : -1;
      if (selected < 0)
        selected = menuCount - 1;
      if (selected >= menuCount)
        selected = 0;
      drawMenu();
    } else if (currentScreen == SCREEN_SET_TIME) {
      unsigned long now = millis();
      unsigned long dt = now - lastStepMs;
      lastStepMs = now;

      int step = 1;
      if (dt < 80)
        step = 10;
      else if (dt < 140)
        step = 5;

      editTimeValue += (stepPlus ? step : -step);
      if (editTimeValue < MIN_TIME)
        editTimeValue = MIN_TIME;
      if (editTimeValue > MAX_TIME)
        editTimeValue = MAX_TIME;

      drawSetTime();
    } else if (currentScreen == SCREEN_SESSION_MENU) {
      if (TEA_COUNT > 1) {
        sessionTeaIndex += stepPlus ? 1 : -1;
        if (sessionTeaIndex < 0)
          sessionTeaIndex = TEA_COUNT - 1;
        if (sessionTeaIndex >= TEA_COUNT)
          sessionTeaIndex = 0;
        drawSessionMenu();
      }
    }
  }

  // ---- dedicated back button ----
  if (backButtonPressedEvent()) {
    if (currentScreen != SCREEN_MENU) {
      if (currentScreen == SCREEN_SESSION_RUN)
        sessionRunning = false;
      goToMenu();
    }
  }

  // ---- button press event ----
  if (buttonPressedEvent()) {
    if (currentScreen == SCREEN_MENU) {
      if (selected == MENU_START_SESSION) {
        sessionStepIndex = 0;
        sessionRunning = false;
        sessionCompleteShown = false;
        currentScreen = SCREEN_SESSION_RUN;
        drawSessionRun(SESSION_STEPS[0]);
      } else if (selected == MENU_SESSION) {
        currentScreen = SCREEN_SESSION_MENU;
        drawSessionMenu();
      } else if (selected == MENU_START) {
        currentScreen = SCREEN_TIMER;
        timerStartMillis = millis();
      } else if (selected == MENU_SET_TIME) {
        currentScreen = SCREEN_SET_TIME;
        editTimeValue = timerDuration;
        drawSetTime();
      } else if (selected == MENU_WIFI) {
        currentScreen = SCREEN_WIFI;
        drawWiFi();
      } else if (selected == MENU_ABOUT) {
        currentScreen = SCREEN_ABOUT;
        drawAbout();
      }
    } else if (currentScreen == SCREEN_SET_TIME) {
      timerDuration = editTimeValue;
      prefs.putInt("dur_sec", timerDuration); // save
      goToMenu();
    } else if (currentScreen == SCREEN_SESSION_MENU) {
      goToMenu();
    } else if (currentScreen == SCREEN_SESSION_RUN) {

      if (sessionStepIndex >= SESSION_STEP_COUNT) {
        goToMenu();
      } else if (!sessionRunning) {
        sessionRunning = true;
        sessionStepStartMs = millis();
      }
    } else {
      goToMenu();
    }
  }

  // ---- non-blocking long press exit (only in session run) ----
  if (currentScreen == SCREEN_SESSION_RUN) {
    bool down = (digitalRead(ENC_SW) == LOW);

    if (down && !swWasDown) {
      swWasDown = true;
      swHoldStartMs = millis();
    }

    if (!down && swWasDown) {
      swWasDown = false;
    }

    if (down && swWasDown && (millis() - swHoldStartMs >= 1200)) {
      while (digitalRead(ENC_SW) == LOW)
        delay(5);
      delay(10);

      sessionRunning = false;
      goToMenu();
    }
  }

  // ---- single timer update ----
  if (currentScreen == SCREEN_TIMER) {
    static int lastRemaining = -1;

    unsigned long elapsed = (millis() - timerStartMillis) / 1000;
    int remaining = timerDuration - (int)elapsed;
    if (remaining < 0)
      remaining = 0;

    if (remaining != lastRemaining) {
      drawTimerScreen("Timer", remaining, timerDuration);

      if (remaining <= 3 && remaining > 0) {
        digitalWrite(LED_PIN, HIGH);
        beep(2200, 60);
        digitalWrite(LED_PIN, LOW);
      }

      if (remaining == 0) {
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_PIN, HIGH);
          buzzerOn(2600);
          delay(80);
          buzzerOff();
          digitalWrite(LED_PIN, LOW);
          delay(120);
        }
        goToMenu();
      }

      lastRemaining = remaining;
    }
  }

  // ---- session run update ----
  if (currentScreen == SCREEN_SESSION_RUN) {
    static int lastRemaining = -1;

    if (sessionStepIndex >= SESSION_STEP_COUNT) {
      if (!sessionCompleteShown) {
        drawSessionComplete();
        sessionCompleteShown = true;
      }
      return;
    }

    int stepSec = SESSION_STEPS[sessionStepIndex];
    int remaining = stepSec;

    if (sessionRunning) {
      unsigned long elapsed = (millis() - sessionStepStartMs) / 1000;
      remaining = stepSec - (int)elapsed;
      if (remaining < 0)
        remaining = 0;
    }

    if (remaining != lastRemaining) {
      drawSessionRun(remaining);

      if (sessionRunning && remaining <= 3 && remaining > 0) {
        digitalWrite(LED_PIN, HIGH);
        beep(2200, 60);
        digitalWrite(LED_PIN, LOW);
      }

      if (sessionRunning && remaining == 0) {
        // finish step
        for (int i = 0; i < 2; i++) {
          digitalWrite(LED_PIN, HIGH);
          buzzerOn(2500);
          delay(70);
          buzzerOff();
          digitalWrite(LED_PIN, LOW);
          delay(120);
        }

        sessionRunning = false;
        sessionStepIndex++;

        if (sessionStepIndex >= SESSION_STEP_COUNT) {
          for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            buzzerOn(3000);
            delay(120);
            buzzerOff();
            digitalWrite(LED_PIN, LOW);
            delay(160);
          }

          drawSessionComplete();
          sessionCompleteShown = true;
          lastRemaining = -999;
          return;
        }

        // show next step ready
        drawSessionRun(SESSION_STEPS[sessionStepIndex]);
      }

      lastRemaining = remaining;
    }
  }
}
