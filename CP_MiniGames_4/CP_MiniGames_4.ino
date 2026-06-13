/*
  CP_MiniGames_4 - Rhythm and Sound pack for Circuit Playground Express/Bluefruit

  Boot picker:
    A held      -> Tone Memory
    B held      -> Sound Meter
    No buttons  -> Beat Match

  From Beat Match idle, press B for Metronome Toy.
  In any mode, hold A+B for about 1.5 seconds to reset/exit that mode.
*/

#include <Adafruit_CircuitPlayground.h>
#include <Arduino.h>

static const uint8_t BRIGHTNESS = 45;
static const uint16_t LAUNCHER_WINDOW_MS = 1500;
static const uint16_t AB_RESET_MS = 1500;

enum Game { G_BEAT, G_MEMORY, G_METER };
static Game SELECTED = G_BEAT;

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

static void wheel(uint8_t pos, uint8_t &r, uint8_t &g, uint8_t &b) {
  if (pos < 85) { r = pos * 3; g = 255 - pos * 3; b = 0; }
  else if (pos < 170) { pos -= 85; r = 255 - pos * 3; g = 0; b = pos * 3; }
  else { pos -= 170; r = 0; g = pos * 3; b = 255 - pos * 3; }
}

static void breathe(uint8_t rMax, uint8_t gMax, uint8_t bMax, uint8_t loops = 1) {
  for (uint8_t loop = 0; loop < loops; loop++) {
    for (uint8_t v = 0; v <= 24; v++) {
      solid((rMax * v) / 24, (gMax * v) / 24, (bMax * v) / 24);
      delay(18);
      if (abResetHeld()) return;
    }
    for (int8_t v = 24; v >= 0; v--) {
      solid((rMax * v) / 24, (gMax * v) / 24, (bMax * v) / 24);
      delay(18);
      if (abResetHeld()) return;
    }
  }
}

static int soundLevel() {
  return CircuitPlayground.soundSensor();
}

static Game splashAndPick(uint16_t windowMs = LAUNCHER_WINDOW_MS) {
  pixelsOff();
  unsigned long start = millis();
  uint8_t idx = 0;
  unsigned long nextStep = 0;
  Game pick = G_BEAT;

  while (millis() - start < windowMs) {
    bool a = CircuitPlayground.leftButton();
    bool b = CircuitPlayground.rightButton();

    if (!a && !b) {
      pixelsOff();
      setPix(4, 24, 14, 0);
      setPix(9, 24, 14, 0);
      setPix(0, 0, 25, 0);
      setPix(2, 0, 0, 28);
      CircuitPlayground.strip.show();
      delay(10);
      continue;
    }

    uint8_t r = 0, g = 0, bl = 0;
    if (a && !b) { g = 35; pick = G_MEMORY; }
    else if (b && !a) { bl = 40; pick = G_METER; }
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

static void runMetronomeForever() {
  waitButtonsReleased();
  uint16_t bpm = 90;
  uint8_t pattern = 0;
  uint8_t beat = 0;
  unsigned long nextBeat = 0;

  for (;;) {
    if (abResetHeld()) { pixelsOff(); return; }
    if (CircuitPlayground.leftButton()) {
      bpm += 15;
      if (bpm > 180) bpm = 60;
      toneHz(420 + bpm, 70);
      waitButtonsReleased(120);
    }
    if (CircuitPlayground.rightButton()) {
      pattern = (pattern + 1) % 3;
      toneHz(760, 70);
      waitButtonsReleased(120);
    }

    unsigned long now = millis();
    if (now >= nextBeat) {
      bool accent = (pattern == 0) ? true : ((pattern == 1) ? (beat == 0) : (beat == 0 || beat == 2));
      pixelsOff();
      for (uint8_t i = 0; i <= beat; i++) setPix(i, accent ? 30 : 0, accent ? 20 : 22, accent ? 0 : 22);
      CircuitPlayground.strip.show();
      toneHz(accent ? 880 : 520, accent ? 55 : 35);
      beat = (beat + 1) % 4;
      nextBeat = now + (60000UL / bpm);
    }
    delay(5);
  }
}

static void runBeatForever() {
  const uint16_t BPM = 92;
  const uint16_t BEAT_MS = 60000UL / BPM;
  waitButtonsReleased();

  for (;;) {
    unsigned long nextBeat = millis() + 500;
    uint8_t score = 0;
    uint8_t round = 0;
    bool prevA = false;

    while (round < 16) {
      if (abResetHeld()) break;
      if (CircuitPlayground.rightButton()) { runMetronomeForever(); waitButtonsReleased(); break; }

      unsigned long now = millis();
      int16_t diff = (int16_t)(now - nextBeat);
      uint8_t pos = round % 10;
      uint8_t chase = (uint8_t)(((long)(BEAT_MS - abs(diff)) * 9L) / BEAT_MS);
      if (abs(diff) > (int16_t)BEAT_MS) chase = 0;

      pixelsOff();
      setPix(pos, 28, 16, 0);
      setPix(chase, 0, 0, 24);
      CircuitPlayground.strip.show();

      bool a = CircuitPlayground.leftButton();
      if (a && !prevA) {
        uint16_t adiff = abs(diff);
        if (adiff <= 110) { score++; solid(0, 28, 0); toneHz(880, 45); }
        else { solid(35, 0, 0); toneHz(180, 70); }
        delay(55);
      }
      prevA = a;

      if (now >= nextBeat) {
        toneHz(660, 35);
        nextBeat += BEAT_MS;
        round++;
      }
      delay(5);
    }

    pixelsOff();
    for (uint8_t i = 0; i < score && i < 10; i++) setPix(i, 0, 30, 0);
    CircuitPlayground.strip.show();
    toneHz(420 + score * 25, 200);
    delay(900);

    while (!CircuitPlayground.leftButton() && !CircuitPlayground.rightButton()) {
      if (abResetHeld()) break;
      breathe(20, 12, 0, 1);
    }
    waitButtonsReleased();
  }
}

static void showMemoryPad(uint8_t p, uint16_t ms) {
  static const uint8_t pix[4] = {0, 2, 5, 7};
  static const uint8_t col[4][3] = {{35,0,0}, {0,30,0}, {0,0,35}, {28,18,0}};
  static const uint16_t freq[4] = {330, 440, 554, 659};
  pixelsOff();
  setPix(pix[p], col[p][0], col[p][1], col[p][2]);
  CircuitPlayground.strip.show();
  toneHz(freq[p], ms);
  delay(ms);
  pixelsOff();
  delay(110);
}

static uint8_t readMemoryInput() {
  for (;;) {
    if (abResetHeld()) return 255;
    if (CircuitPlayground.leftButton()) { waitButtonsReleased(80); return 0; }
    if (CircuitPlayground.rightButton()) { waitButtonsReleased(80); return 1; }
    if (CircuitPlayground.readCap(A1) > 250) { delay(120); return 2; }
    if (CircuitPlayground.readCap(A6) > 250) { delay(120); return 3; }
    delay(8);
  }
}

static void runToneMemoryForever() {
  uint8_t seq[24];
  uint8_t level = 1;
  randomSeed(millis() ^ soundLevel());

  for (;;) {
    for (uint8_t i = 0; i < 24; i++) seq[i] = random(4);
    bool lost = false;

    while (!lost && level <= 24) {
      for (uint8_t i = 0; i < level; i++) showMemoryPad(seq[i], 230);
      for (uint8_t i = 0; i < level; i++) {
        uint8_t got = readMemoryInput();
        if (got == 255) return;
        showMemoryPad(got, 90);
        if (got != seq[i]) { lost = true; break; }
      }
      if (!lost) {
        for (uint8_t i = 0; i < level && i < 10; i++) setPix(i, 0, 28, 0);
        CircuitPlayground.strip.show();
        toneHz(760, 120);
        delay(350);
        pixelsOff();
        level++;
      }
    }

    for (uint8_t k = 0; k < 3; k++) { solid(35, 0, 0); toneHz(160, 120); pixelsOff(); delay(90); }
    level = 1;
    waitButtonsReleased(400);
  }
}

static void runSoundMeterForever() {
  uint16_t base = 0;
  for (uint8_t i = 0; i < 20; i++) { base += soundLevel(); delay(12); }
  base /= 20;
  uint8_t palette = 0;

  for (;;) {
    if (abResetHeld()) { pixelsOff(); return; }
    if (CircuitPlayground.leftButton()) {
      base = soundLevel();
      toneHz(500, 80);
      waitButtonsReleased(120);
    }
    if (CircuitPlayground.rightButton()) {
      palette = (palette + 1) % 3;
      toneHz(720, 80);
      waitButtonsReleased(120);
    }

    int level = soundLevel() - (int)base;
    if (level < 0) level = 0;
    uint8_t leds = constrain(level / 12, 0, 10);
    pixelsOff();
    for (uint8_t i = 0; i < leds; i++) {
      uint8_t r = 0, g = 0, b = 0;
      if (palette == 0) { r = i > 7 ? 35 : 0; g = i > 4 ? 25 : 0; b = i <= 4 ? 30 : 0; }
      else if (palette == 1) { r = 28; g = i * 2; b = 0; }
      else wheel(i * 22, r, g, b);
      setPix(i, r, g, b);
    }
    CircuitPlayground.strip.show();
    delay(35);
  }
}

void setup() {
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(BRIGHTNESS);
  pixelsOff();
  Serial.begin(115200);
  SELECTED = splashAndPick();
  if (SELECTED == G_MEMORY) solid(0, 22, 0);
  else if (SELECTED == G_METER) solid(0, 0, 28);
  else solid(25, 14, 0);
  delay(180);
  pixelsOff();
}

void loop() {
  if (SELECTED == G_MEMORY) runToneMemoryForever();
  else if (SELECTED == G_METER) runSoundMeterForever();
  else runBeatForever();
}
