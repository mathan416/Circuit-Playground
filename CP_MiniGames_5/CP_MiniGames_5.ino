/*
  CP_MiniGames_5 - Puzzle and Logic pack for Circuit Playground Express/Bluefruit

  Boot picker:
    A held      -> Code Breaker
    B held      -> Safe Cracker
    No buttons  -> Lights Out

  From Lights Out idle, press B for Reflex Maze.
  In any mode, hold A+B for about 1.5 seconds to reset/exit that mode.
*/

#include <Adafruit_CircuitPlayground.h>
#include <Arduino.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const uint8_t BRIGHTNESS = 45;
static const uint16_t LAUNCHER_WINDOW_MS = 1500;
static const uint16_t AB_RESET_MS = 1500;

enum Game { G_LIGHTS, G_CODE, G_SAFE };
static Game SELECTED = G_LIGHTS;

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

static int tiltIndex() {
  float ax = CircuitPlayground.motionX();
  float ay = CircuitPlayground.motionY();
  float ang = atan2f(ay, ax);
  return ((int)roundf((ang + (float)M_PI) * 10.0f / (2.0f * (float)M_PI)) + 10) % 10;
}

static Game splashAndPick(uint16_t windowMs = LAUNCHER_WINDOW_MS) {
  unsigned long start = millis();
  uint8_t idx = 0;
  unsigned long nextStep = 0;
  Game pick = G_LIGHTS;

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
    if (a && !b) { g = 35; pick = G_CODE; }
    else if (b && !a) { bl = 40; pick = G_SAFE; }
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

static void drawLights(uint16_t mask, uint8_t cursor) {
  pixelsOff();
  for (uint8_t i = 0; i < 10; i++) {
    if (mask & (1 << i)) setPix(i, i == cursor ? 35 : 18, i == cursor ? 28 : 12, 0);
    else if (i == cursor) setPix(i, 0, 0, 28);
  }
  CircuitPlayground.strip.show();
}

static void toggleAt(uint16_t &mask, uint8_t pos) {
  mask ^= (1 << pos);
  mask ^= (1 << ((pos + 9) % 10));
  mask ^= (1 << ((pos + 1) % 10));
}

static void runReflexMazeForever() {
  const uint8_t path[10] = {0, 3, 6, 9, 2, 5, 8, 1, 4, 7};
  uint8_t step = 0;
  uint16_t window = 1300;
  unsigned long deadline = millis() + window;
  bool prevA = false;

  waitButtonsReleased();
  for (;;) {
    if (abResetHeld()) { pixelsOff(); return; }
    uint8_t target = path[step];
    uint8_t cursor = tiltIndex();
    pixelsOff();
    setPix(target, 28, 16, 0);
    setPix(cursor, 0, 0, 35);
    CircuitPlayground.strip.show();

    bool a = CircuitPlayground.leftButton();
    if (a && !prevA) {
      if (cursor == target) {
        step = (step + 1) % 10;
        if (window > 500) window -= 55;
        deadline = millis() + window;
        toneHz(660 + step * 20, 55);
      } else {
        loseFlash();
        step = 0;
        window = 1300;
        deadline = millis() + window;
      }
      waitButtonsReleased(80);
    }
    prevA = a;

    if (millis() > deadline) {
      loseFlash();
      step = 0;
      window = 1300;
      deadline = millis() + window;
    }
    if (step == 0 && window < 650) {
      winFlash();
      window = 1300;
    }
    delay(18);
  }
}

static void runLightsOutForever() {
  randomSeed(millis());
  waitButtonsReleased();

  for (;;) {
    uint16_t mask = 0;
    for (uint8_t i = 0; i < 10; i++) if (random(2)) mask |= (1 << i);
    for (uint8_t i = 0; i < 8; i++) toggleAt(mask, random(10));
    if (mask == 0) mask = 0x02B5;
    uint8_t cursor = 0;
    uint8_t moves = 0;

    while (mask != 0) {
      if (abResetHeld()) break;
      if (CircuitPlayground.rightButton()) { runReflexMazeForever(); waitButtonsReleased(); break; }
      if (CircuitPlayground.leftButton()) {
        toggleAt(mask, cursor);
        moves++;
        toneHz(420 + cursor * 25, 45);
        waitButtonsReleased(100);
      }
      cursor = tiltIndex();
      drawLights(mask, cursor);
      delay(25);
    }

    if (mask == 0) {
      winFlash();
      pixelsOff();
      for (uint8_t i = 0; i < moves && i < 10; i++) setPix(i, 0, 28, 0);
      CircuitPlayground.strip.show();
      delay(900);
    }
  }
}

static void codeColor(uint8_t c, uint8_t &r, uint8_t &g, uint8_t &b) {
  const uint8_t colors[4][3] = {{35, 0, 0}, {0, 30, 0}, {0, 0, 35}, {28, 18, 0}};
  r = colors[c][0]; g = colors[c][1]; b = colors[c][2];
}

static void drawCodeGuess(uint8_t guess[4], uint8_t cursor) {
  pixelsOff();
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t r, g, b;
    codeColor(guess[i], r, g, b);
    setPix(i * 2, r, g, b);
    if (i == cursor) setPix(i * 2 + 1, 18, 18, 18);
  }
  CircuitPlayground.strip.show();
}

static void runCodeBreakerForever() {
  uint8_t secret[4];
  uint8_t guess[4] = {0, 0, 0, 0};
  uint8_t cursor = 0;
  uint8_t tries = 0;
  randomSeed(millis());
  for (uint8_t i = 0; i < 4; i++) secret[i] = random(4);
  waitButtonsReleased();

  for (;;) {
    if (abResetHeld()) { pixelsOff(); return; }
    drawCodeGuess(guess, cursor);

    if (CircuitPlayground.leftButton()) {
      guess[cursor] = (guess[cursor] + 1) % 4;
      toneHz(420 + guess[cursor] * 100, 55);
      waitButtonsReleased(100);
    }
    if (CircuitPlayground.rightButton()) {
      cursor++;
      if (cursor >= 4) {
        tries++;
        uint8_t exact = 0;
        for (uint8_t i = 0; i < 4; i++) if (guess[i] == secret[i]) exact++;
        pixelsOff();
        for (uint8_t i = 0; i < exact; i++) setPix(i, 0, 30, 0);
        for (uint8_t i = exact; i < 4; i++) setPix(i, 28, 14, 0);
        CircuitPlayground.strip.show();
        toneHz(exact == 4 ? 880 : 360 + exact * 80, 250);
        delay(900);
        if (exact == 4) {
          winFlash();
          for (uint8_t i = 0; i < 4; i++) secret[i] = random(4);
          for (uint8_t i = 0; i < 4; i++) guess[i] = 0;
          tries = 0;
        } else if (tries >= 8) {
          loseFlash();
          pixelsOff();
          for (uint8_t i = 0; i < 4; i++) {
            uint8_t r, g, b;
            codeColor(secret[i], r, g, b);
            setPix(i * 2, r, g, b);
          }
          CircuitPlayground.strip.show();
          delay(1600);
          for (uint8_t i = 0; i < 4; i++) secret[i] = random(4);
          for (uint8_t i = 0; i < 4; i++) guess[i] = 0;
          tries = 0;
        }
        cursor = 0;
      }
      toneHz(660, 45);
      waitButtonsReleased(100);
    }
    delay(15);
  }
}

static void runSafeCrackerForever() {
  randomSeed(millis());
  int combo[3];
  combo[0] = random(10);
  combo[1] = random(10);
  combo[2] = random(10);
  uint8_t stage = 0;
  uint8_t cursor = 0;
  bool prevA = false;
  waitButtonsReleased();

  for (;;) {
    if (abResetHeld()) { pixelsOff(); return; }
    cursor = tiltIndex();
    int d = abs((int)cursor - combo[stage]);
    d = min(d, 10 - d);

    pixelsOff();
    if (d == 0) setPix(cursor, 0, 32, 0);
    else if (d == 1) setPix(cursor, 28, 16, 0);
    else setPix(cursor, 0, 0, 34);
    for (uint8_t i = 0; i < stage; i++) setPix(i, 0, 28, 0);
    CircuitPlayground.strip.show();

    if (d == 0) toneHz(760, 20);
    else if (d == 1) toneHz(520, 12);

    bool a = CircuitPlayground.leftButton();
    if (a && !prevA) {
      if (cursor == combo[stage]) {
        stage++;
        solid(0, 25, 0);
        toneHz(700 + stage * 80, 90);
        delay(120);
        if (stage >= 3) {
          winFlash();
          combo[0] = random(10); combo[1] = random(10); combo[2] = random(10);
          stage = 0;
        }
      } else {
        loseFlash();
        stage = 0;
      }
      waitButtonsReleased(100);
    }
    prevA = a;
    delay(80);
  }
}

void setup() {
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(BRIGHTNESS);
  pixelsOff();
  Serial.begin(115200);
  SELECTED = splashAndPick();
  if (SELECTED == G_CODE) solid(0, 22, 0);
  else if (SELECTED == G_SAFE) solid(0, 0, 28);
  else solid(25, 14, 0);
  delay(180);
  pixelsOff();
}

void loop() {
  if (SELECTED == G_CODE) runCodeBreakerForever();
  else if (SELECTED == G_SAFE) runSafeCrackerForever();
  else runLightsOutForever();
}
