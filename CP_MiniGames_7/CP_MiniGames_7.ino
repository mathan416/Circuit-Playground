/*
  CP_MiniGames_7 - Two-player pack for Circuit Playground Express/Bluefruit

  Boot picker:
    A held      -> Territory
    B held      -> Tug Deluxe
    No buttons  -> Duel

  From Duel idle, press B for Memory Duel.
  In any mode, hold A+B for about 1.5 seconds to reset/exit that mode.
*/

#include <Adafruit_CircuitPlayground.h>
#include <Arduino.h>

static const uint8_t BRIGHTNESS = 50;
static const uint16_t LAUNCHER_WINDOW_MS = 1500;
static const uint16_t AB_RESET_MS = 1500;

enum Game { G_DUEL, G_TERRITORY, G_TUG };
static Game SELECTED = G_DUEL;

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
  Game pick = G_DUEL;

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
    if (a && !b) { g = 35; pick = G_TERRITORY; }
    else if (b && !a) { bl = 40; pick = G_TUG; }
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

static void leftWin() {
  for (uint8_t k = 0; k < 4; k++) { solid(0, 30, 0); toneHz(720 + k * 60, 70); pixelsOff(); delay(70); }
}

static void rightWin() {
  for (uint8_t k = 0; k < 4; k++) { solid(0, 0, 35); toneHz(520 + k * 70, 70); pixelsOff(); delay(70); }
}

static void falseStart() {
  for (uint8_t k = 0; k < 3; k++) { solid(35, 0, 0); toneHz(180, 100); pixelsOff(); delay(80); }
}

static void drawScores(uint8_t left, uint8_t right) {
  pixelsOff();
  for (uint8_t i = 0; i < left && i < 5; i++) setPix(i, 0, 28, 0);
  for (uint8_t i = 0; i < right && i < 5; i++) setPix(9 - i, 0, 0, 32);
  CircuitPlayground.strip.show();
}

static void runMemoryDuelForever() {
  uint8_t seq[20];
  uint8_t len = 1;
  bool leftTurn = true;
  randomSeed(millis());
  for (uint8_t i = 0; i < 20; i++) seq[i] = random(2);
  waitButtonsReleased();

  for (;;) {
    if (abResetHeld()) { pixelsOff(); return; }
    bool missed = false;
    for (uint8_t i = 0; i < len; i++) {
      pixelsOff();
      if (seq[i] == 0) { setPix(1, 0, 30, 0); toneHz(520, 120); }
      else { setPix(8, 0, 0, 35); toneHz(720, 120); }
      CircuitPlayground.strip.show();
      delay(180);
      pixelsOff();
      delay(110);
    }

    for (uint8_t i = 0; i < len; i++) {
      bool got = false;
      uint8_t val = 0;
      unsigned long deadline = millis() + 2500;
      while (!got && millis() < deadline) {
        if (abResetHeld()) return;
        if (CircuitPlayground.leftButton()) { val = 0; got = true; waitButtonsReleased(80); }
        if (CircuitPlayground.rightButton()) { val = 1; got = true; waitButtonsReleased(80); }
        delay(5);
      }
      if (!got || val != seq[i]) {
        if (leftTurn) rightWin(); else leftWin();
        len = 1;
        leftTurn = !leftTurn;
        for (uint8_t j = 0; j < 20; j++) seq[j] = random(2);
        missed = true;
        break;
      }
    }

    if (!missed) {
      len++;
      if (len > 20) {
        if (leftTurn) leftWin(); else rightWin();
        len = 1;
        for (uint8_t j = 0; j < 20; j++) seq[j] = random(2);
      }
      leftTurn = !leftTurn;
    }

    pixelsOff();
    setPix(leftTurn ? 1 : 8, 0, leftTurn ? 30 : 0, leftTurn ? 0 : 35);
    CircuitPlayground.strip.show();
    delay(700);
  }
}

static void runDuelForever() {
  randomSeed(millis());
  waitButtonsReleased();

  for (;;) {
    while (!CircuitPlayground.leftButton() && !CircuitPlayground.rightButton()) {
      if (abResetHeld()) break;
      solid(18, 10, 0);
      delay(120);
      pixelsOff();
      delay(120);
    }
    if (CircuitPlayground.rightButton() && !CircuitPlayground.leftButton()) {
      runMemoryDuelForever();
      waitButtonsReleased();
      continue;
    }
    waitButtonsReleased(250);

    solid(0, 0, 26);
    unsigned long goTime = millis() + random(900, 3200);
    bool failed = false;
    while (millis() < goTime) {
      if (abResetHeld()) { failed = true; break; }
      if (CircuitPlayground.leftButton()) { rightWin(); failed = true; break; }
      if (CircuitPlayground.rightButton()) { leftWin(); failed = true; break; }
      delay(5);
    }
    if (failed) continue;

    solid(0, 30, 0);
    toneHz(900, 80);
    for (;;) {
      if (abResetHeld()) break;
      bool a = CircuitPlayground.leftButton();
      bool b = CircuitPlayground.rightButton();
      if (a && !b) { leftWin(); waitButtonsReleased(); break; }
      if (b && !a) { rightWin(); waitButtonsReleased(); break; }
      if (a && b) { falseStart(); waitButtonsReleased(); break; }
      delay(3);
    }
  }
}

static void runTerritoryForever() {
  int8_t owner[10];
  for (uint8_t i = 0; i < 10; i++) owner[i] = 0;
  uint8_t leftPos = 0;
  uint8_t rightPos = 5;
  unsigned long endTime = millis() + 30000UL;
  waitButtonsReleased();

  for (;;) {
    if (abResetHeld()) { pixelsOff(); return; }
    if (millis() > endTime) {
      uint8_t left = 0, right = 0;
      for (uint8_t i = 0; i < 10; i++) {
        if (owner[i] < 0) left++;
        if (owner[i] > 0) right++;
      }
      if (left >= right) leftWin(); else rightWin();
      for (uint8_t i = 0; i < 10; i++) owner[i] = 0;
      endTime = millis() + 30000UL;
    }

    if (CircuitPlayground.leftButton()) {
      owner[leftPos] = -1;
      leftPos = (leftPos + 1) % 10;
      toneHz(520, 25);
      waitButtonsReleased(30);
    }
    if (CircuitPlayground.rightButton()) {
      owner[rightPos] = 1;
      rightPos = (rightPos + 9) % 10;
      toneHz(680, 25);
      waitButtonsReleased(30);
    }

    pixelsOff();
    for (uint8_t i = 0; i < 10; i++) {
      if (owner[i] < 0) setPix(i, 0, 25, 0);
      else if (owner[i] > 0) setPix(i, 0, 0, 30);
      else setPix(i, 4, 4, 4);
    }
    setPix(leftPos, 20, 20, 20);
    setPix(rightPos, 20, 20, 20);
    CircuitPlayground.strip.show();
    delay(20);
  }
}

static void drawTug(int8_t pos, uint8_t round) {
  pixelsOff();
  for (uint8_t i = 0; i < 5; i++) setPix(i, 0, 8, 0);
  for (uint8_t i = 5; i < 10; i++) setPix(i, 0, 0, 10);
  uint8_t idx = constrain((int)pos + 5, 0, 9);
  setPix(idx, 30, 24, 0);
  setPix(round % 10, 8, 0, 8);
  CircuitPlayground.strip.show();
}

static void runTugDeluxeForever() {
  waitButtonsReleased();
  uint8_t round = 0;

  for (;;) {
    int8_t pos = 0;
    unsigned long endTime = millis() + 12000UL;
    uint16_t decayMs = 700;
    unsigned long nextDecay = millis() + decayMs;

    while (millis() < endTime) {
      if (abResetHeld()) { pixelsOff(); return; }
      if (CircuitPlayground.leftButton()) {
        pos--;
        if (pos < -5) pos = -5;
        toneHz(560, 20);
        waitButtonsReleased(30);
      }
      if (CircuitPlayground.rightButton()) {
        pos++;
        if (pos > 4) pos = 4;
        toneHz(700, 20);
        waitButtonsReleased(30);
      }
      if (millis() > nextDecay) {
        if (pos < 0) pos++;
        else if (pos > 0) pos--;
        nextDecay = millis() + decayMs;
        if (decayMs > 260) decayMs -= 20;
      }
      drawTug(pos, round);
      if (pos <= -5) { leftWin(); break; }
      if (pos >= 4) { rightWin(); break; }
      delay(18);
    }

    if (millis() >= endTime) {
      if (pos <= 0) leftWin(); else rightWin();
    }
    round++;
    delay(500);
  }
}

void setup() {
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(BRIGHTNESS);
  pixelsOff();
  Serial.begin(115200);
  SELECTED = splashAndPick();
  if (SELECTED == G_TERRITORY) solid(0, 22, 0);
  else if (SELECTED == G_TUG) solid(0, 0, 28);
  else solid(25, 14, 0);
  delay(180);
  pixelsOff();
}

void loop() {
  if (SELECTED == G_TERRITORY) runTerritoryForever();
  else if (SELECTED == G_TUG) runTugDeluxeForever();
  else runDuelForever();
}
