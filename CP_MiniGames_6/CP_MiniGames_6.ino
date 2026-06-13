/*
  CP_MiniGames_6 - Motion and Fitness pack for Circuit Playground Express/Bluefruit

  Boot picker:
    A held      -> Balance Hold
    B held      -> Jump Counter
    No buttons  -> Step Sprint

  From Step Sprint idle, press B for Freeze Dance.
  In any mode, hold A+B for about 1.5 seconds to reset/exit that mode.
*/

#include <Adafruit_CircuitPlayground.h>
#include <Arduino.h>
#include <math.h>

static const uint8_t BRIGHTNESS = 50;
static const uint16_t LAUNCHER_WINDOW_MS = 1500;
static const uint16_t AB_RESET_MS = 1500;
static const uint16_t SPRINT_MS = 15000;
static const uint16_t FREEZE_ROUND_MS = 22000;

enum Game { G_SPRINT, G_BALANCE, G_JUMP };
static Game SELECTED = G_SPRINT;

static inline void pixelsOff() {
  for (uint8_t i = 0; i < 10; i++) CircuitPlayground.setPixelColor(i, 0, 0, 0);
  CircuitPlayground.strip.show();
}

static inline void setPix(uint8_t i, uint8_t r, uint8_t g, uint8_t b) {
  CircuitPlayground.setPixelColor(i % 10, r, g, b);
}

static inline void solid(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t i = 0; i < 10; i++) setPix(i, r, g, b);
  CircuitPlayground.strip.show();
}

static inline void toneHz(uint16_t f, uint16_t ms) {
  CircuitPlayground.playTone(f, ms);
}

static float gex() {
  float ax = CircuitPlayground.motionX();
  float ay = CircuitPlayground.motionY();
  float az = CircuitPlayground.motionZ();
  float mag = sqrtf(ax * ax + ay * ay + az * az);
  return fabsf(mag - 9.81f);
}

static float tiltAmount() {
  float ax = CircuitPlayground.motionX();
  float ay = CircuitPlayground.motionY();
  return sqrtf(ax * ax + ay * ay);
}

static bool abResetHeld(uint16_t ms = AB_RESET_MS) {
  if (!(CircuitPlayground.leftButton() && CircuitPlayground.rightButton())) return false;
  unsigned long start = millis();
  while (CircuitPlayground.leftButton() && CircuitPlayground.rightButton()) {
    if (millis() - start >= ms) {
      for (uint8_t k = 0; k < 2; k++) { solid(18, 0, 18); delay(70); pixelsOff(); delay(70); }
      return true;
    }
    delay(10);
  }
  return false;
}

static void waitButtonsReleased(uint16_t ms = 250) {
  unsigned long end = millis() + ms;
  for (;;) {
    if (CircuitPlayground.leftButton() || CircuitPlayground.rightButton()) end = millis() + ms;
    if (millis() >= end) return;
    delay(10);
  }
}

static Game splashAndPick(uint16_t windowMs = LAUNCHER_WINDOW_MS) {
  unsigned long start = millis();
  uint8_t idx = 0;
  unsigned long nextStep = 0;
  Game pick = G_SPRINT;

  while (millis() - start < windowMs) {
    bool a = CircuitPlayground.leftButton();
    bool b = CircuitPlayground.rightButton();
    if (!a && !b) {
      pixelsOff();
      setPix(4, 24, 14, 0); setPix(9, 24, 14, 0);
      setPix(0, 0, 25, 0); setPix(2, 0, 0, 28);
      CircuitPlayground.strip.show();
      delay(10);
      continue;
    }

    uint8_t r = 0, g = 0, bl = 0;
    if (a && !b) { g = 35; pick = G_BALANCE; }
    else if (b && !a) { bl = 40; pick = G_JUMP; }
    else { r = 25; bl = 25; }

    if (millis() >= nextStep) {
      pixelsOff();
      setPix(idx, r, g, bl);
      setPix(idx + 9, r / 5, g / 5, bl / 5);
      CircuitPlayground.strip.show();
      idx = (idx + 1) % 10;
      nextStep = millis() + 55;
    }
    delay(5);
  }
  pixelsOff();
  return pick;
}

static void winFlash() {
  for (uint8_t k = 0; k < 4; k++) { solid(0, 30, 0); toneHz(680 + k * 80, 70); pixelsOff(); delay(70); }
}

static void loseFlash() {
  for (uint8_t k = 0; k < 3; k++) { solid(35, 0, 0); toneHz(180, 110); pixelsOff(); delay(80); }
}

static void drawMeter(uint8_t leds, uint8_t r, uint8_t g, uint8_t b) {
  pixelsOff();
  for (uint8_t i = 0; i < leds && i < 10; i++) setPix(i, r, g, b);
  CircuitPlayground.strip.show();
}

static void runFreezeDanceForever() {
  randomSeed(millis());
  waitButtonsReleased();
  unsigned long roundStart = millis();
  unsigned long phaseEnd = millis();
  bool movePhase = true;
  uint8_t score = 0;

  for (;;) {
    if (abResetHeld()) { pixelsOff(); return; }
    unsigned long now = millis();
    if (now - roundStart >= FREEZE_ROUND_MS) {
      pixelsOff();
      for (uint8_t i = 0; i < score && i < 10; i++) setPix(i, 0, 30, 0);
      CircuitPlayground.strip.show();
      toneHz(600 + score * 30, 250);
      delay(1200);
      roundStart = millis();
      phaseEnd = millis();
      score = 0;
    }

    if (now >= phaseEnd) {
      movePhase = !movePhase;
      phaseEnd = now + random(movePhase ? 900 : 700, movePhase ? 1800 : 1500);
      toneHz(movePhase ? 700 : 300, 90);
    }

    float motion = gex();
    if (movePhase) {
      solid(0, 24, 0);
      if (motion > 1.2f) {
        if (score < 10) score++;
        delay(120);
      }
    } else {
      solid(28, 0, 0);
      if (motion > 1.5f) {
        loseFlash();
        roundStart = millis();
        phaseEnd = millis();
        score = 0;
      }
    }
    delay(45);
  }
}

static void runStepSprintForever() {
  waitButtonsReleased();

  for (;;) {
    while (!CircuitPlayground.leftButton()) {
      if (abResetHeld()) break;
      if (CircuitPlayground.rightButton()) { runFreezeDanceForever(); waitButtonsReleased(); }
      solid(18, 12, 0);
      delay(120);
      pixelsOff();
      delay(120);
    }
    waitButtonsReleased(120);

    unsigned long start = millis();
    uint8_t steps = 0;
    bool armed = true;
    while (millis() - start < SPRINT_MS) {
      if (abResetHeld()) break;
      float m = gex();
      if (armed && m > 2.3f) {
        if (steps < 99) steps++;
        armed = false;
        toneHz(520 + (steps % 10) * 20, 18);
      }
      if (!armed && m < 0.9f) armed = true;

      uint8_t leds = min((uint8_t)10, (uint8_t)((steps + 2) / 3));
      drawMeter(leds, 0, 28, 0);
      delay(25);
    }

    pixelsOff();
    uint8_t ones = steps % 10;
    for (uint8_t i = 0; i < ones; i++) setPix(i, 0, 30, 0);
    CircuitPlayground.strip.show();
    toneHz(440 + ones * 40, 300);
    delay(1300);
  }
}

static void runBalanceHoldForever() {
  uint8_t best = 0;
  waitButtonsReleased();

  for (;;) {
    unsigned long holdStart = 0;
    unsigned long lastTick = 0;
    uint8_t seconds = 0;

    while (seconds < 10) {
      if (abResetHeld()) { pixelsOff(); return; }
      float tilt = tiltAmount();
      bool level = tilt < 2.2f;

      if (level) {
        if (holdStart == 0) holdStart = millis();
        seconds = min((uint8_t)10, (uint8_t)((millis() - holdStart) / 1000));
        drawMeter(seconds, 0, 28, 0);
        if (millis() - lastTick > 1000) { toneHz(600 + seconds * 30, 35); lastTick = millis(); }
      } else {
        holdStart = 0;
        pixelsOff();
        uint8_t warn = min((uint8_t)9, (uint8_t)(tilt));
        setPix(warn, 30, 0, 0);
        CircuitPlayground.strip.show();
      }
      delay(40);
    }

    best = max(best, seconds);
    winFlash();
    drawMeter(best, 0, 0, 30);
    delay(1000);
  }
}

static void runJumpCounterForever() {
  uint8_t jumps = 0;
  bool armed = true;
  unsigned long lastJump = 0;
  waitButtonsReleased();

  for (;;) {
    if (abResetHeld()) { pixelsOff(); return; }
    if (CircuitPlayground.leftButton()) {
      jumps = 0;
      toneHz(440, 80);
      waitButtonsReleased(100);
    }

    float m = gex();
    if (armed && m > 5.0f && millis() - lastJump > 280) {
      jumps++;
      lastJump = millis();
      armed = false;
      toneHz(740, 45);
    }
    if (!armed && m < 1.2f) armed = true;

    pixelsOff();
    uint8_t tens = jumps / 10;
    uint8_t ones = jumps % 10;
    for (uint8_t i = 0; i < ones; i++) setPix(i, 0, 30, 0);
    if (tens > 0) setPix(9, 28, 16, 0);
    CircuitPlayground.strip.show();
    delay(30);
  }
}

void setup() {
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(BRIGHTNESS);
  pixelsOff();
  Serial.begin(115200);
  SELECTED = splashAndPick();
  if (SELECTED == G_BALANCE) solid(0, 22, 0);
  else if (SELECTED == G_JUMP) solid(0, 0, 28);
  else solid(25, 14, 0);
  delay(180);
  pixelsOff();
}

void loop() {
  if (SELECTED == G_BALANCE) runBalanceHoldForever();
  else if (SELECTED == G_JUMP) runJumpCounterForever();
  else runStepSprintForever();
}
