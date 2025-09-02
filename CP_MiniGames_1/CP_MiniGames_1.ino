/*
  cp_minigames_1.ino
  ---------------------------------------------------------------------------
  CPX/CPB no-screen launcher + 3 games (with two alt-modes)
  Boards      : Adafruit Circuit Playground Express (SAMD21) or Bluefruit (nRF52840)
  Dependencies: Adafruit Circuit Playground library (install via Library Manager)

  OVERVIEW
  - One-time selection at boot (≈1.5 s window), then the chosen game runs forever
    until a hardware reset. In-game A+B (held ≈1.5 s) resets that game back to its
    own idle screen (does NOT return to the launcher).
  - Games included:
      • Hot Potato (default if no boot button is held)
      • Shake Dice  (hold B at boot)  → Alt-mode: Whack-a-Mole (press B in Dice)
      • Simon-4     (hold A at boot)  → Alt-mode: Balancer     (press B in Simon)

  BOOT SELECTION (splashAndPick)
    Hold during the boot window (~1500 ms):
      - A held           → start SIMON (green spinner)
      - B held           → start DICE (blue spinner)
      - none             → start HOT POTATO (amber default)
    A rotating “confirm” spinner shows selection color while held.

  IN-GAME GLOBAL CONTROL
    - A+B held ~1.5 s (AB_RESET_MS) → reset the current game to its idle state.
    - Helper: waitButtonsReleased(ms) prevents stale button presses between phases.

  GAME DESCRIPTIONS
    1) HOT POTATO  (default)
       - Idle: teal “breathe” animation; press A to start a round.
       - Round: ticking speeds up until a randomized deadline; excessive motion
         shortens the fuse slightly. If time expires → “boom” sequence, then back
         to idle. Right button during a round returns to idle.
       - Tuning:
         * FUSE_MIN_S / FUSE_MAX_S : min/max fuse length
         * TICK_START_S / TICK_MIN_S / TICK_DECAY : tick cadence ramp
         * Motion penalty inside potatoRound(): large g-excess hastens the boom.

    2) SHAKE DICE  (boot B)
       - Idle: blue “breathe”; shake above SHAKE_THRESH *or* press A to roll.
       - Roll: short spin animation, then show 1–6 pips on the NeoPixel ring
         with a tone. After ~2 s, return to idle.
       - Press B at any time in Dice to enter ALT-MODE: WHACK-A-MOLE.

       ALT-MODE: WHACK-A-MOLE
       - Idle: teal breathe; press A to start a timed round (default 30 s).
       - A random “mole” LED lights; hit it by shaking (g-excess > HIT_SHAKE_GEX)
         or pressing A within a shrinking time window; score increments on success.
       - End-of-round: blue/amber cues; score display = tens as amber ring flashes,
         ones as green pips (A+B during ones display exits back to Dice).
       - Tuning:
         * START_WINDOW_MS / MIN_WINDOW_MS / DECAY : reaction window curve
         * ROUND_MS : round duration (ms)
         * HIT_SHAKE_GEX : shake hit threshold (reuses SHAKE_THRESH)

    3) SIMON-4  (boot A)
       - Touch pads used as inputs (capacitive): A1(red), A2(green), A5(blue), A6(amber).
         Pad mapping is defined in PADS[] with per-pad pixel index and tone.
       - Idle: teal “breathe”; press A to start a round.
       - Calibrates capacitive baselines (CAP_SAMPLES) on each idle entry and uses
         CAP_DELTA_MIN with CAP_RELEASE_HYST for robust press/release detection.
       - Visual/audio feedback via flashPad() per step; loseFlash() on error.
       - Press B at any time in Simon idle/wait to enter ALT-MODE: BALANCER.

       ALT-MODE: BALANCER
       - Uses accelerometer tilt to steer a “ball” around the ring; stay inside a
         moving amber safe zone. Zone narrows over time; step cadence is STEP_MS.
       - If outside the zone for LOSE_MS → failure flash and zone eases/reset.
       - Win condition: if zone would shrink below 3 LEDs wide, celebratory effect,
         short pause, then restart Balancer fresh.
       - Press A to re-center the safe zone to the current ball position.
       - Tuning:
         * SAFE_WIDTH_START / SAFE_WIDTH_MIN : initial & minimum half-width
         * LOSE_MS : allowed time outside the zone
         * SHRINK_EVERY_MS : difficulty ramp interval
         * STEP_MS : update cadence
         * WIN_PAUSE_MS : pause after a win before restarting

  HARDWARE NOTES
    - NeoPixel ring: 10 LEDs addressed via CircuitPlayground.setPixelColor(i, r,g,b).
    - Accelerometer: motionX/Y/Z() used for shake detection and tilt angle (atan2).
    - Capacitive touch: readCap(pin) for A1/A2/A5/A6; dynamic baselines per idle.
    - BRIGHTNESS: global NeoPixel brightness (0–255). Adjust to taste/power budget.

  TUNING CONSTANTS (top of file)
    - BRIGHTNESS, LAUNCHER_WINDOW_MS, AB_RESET_MS
    - Capacitive: CAP_SAMPLES, CAP_DELTA_MIN, CAP_RELEASE_HYST
    - Dice / Shake thresholds: SHAKE_THRESH (g-excess threshold)
    - Hot Potato timing: FUSE_* and TICK_* constants
    - Whack-a-Mole timing/difficulty: see section above
    - Balancer difficulty: SAFE_WIDTH_*, LOSE_MS, STEP_MS, SHRINK_EVERY_MS

  BUILD/RUN
    1. In Arduino IDE: Board → select your CPX or CPB board.
    2. Tools → Manage Libraries… → install “Adafruit Circuit Playground”.
    3. Compile & upload. On reboot:
       - Hold A for Simon, B for Dice, or hold none for Hot Potato.

  SAFETY & UX
    - Shake thresholds are conservative by default; increase/decrease SHAKE_THRESH
      and HIT_SHAKE_GEX to match your play style and environment.
    - Long A+B hold is intentionally debounced and visually confirmed (blink).

  AUTHOR / LICENSE
    - Prototype and integration: © 2025 Iain Bennett.
    
  ---------------------------------------------------------------------------
*/

#include <Adafruit_CircuitPlayground.h>
#include <Arduino.h>
#include <math.h>

// ==================== Tuning ====================
static const uint8_t  BRIGHTNESS = 40;         // 0..255 overall LED brightness
static const uint16_t LAUNCHER_WINDOW_MS = 1500;
static const uint16_t AB_RESET_MS        = 1500;   // A+B in-game: reset that game

// Simon touch tuning
static const uint8_t  CAP_SAMPLES = 16;     // samples to average for baseline
static const uint16_t CAP_DELTA_MIN = 120;  // minimum rise above baseline to count as touch
static const uint16_t CAP_RELEASE_HYST = 60; // how far below threshold to consider "released"

// Dice (shake) tuning
static const float SHAKE_THRESH = 7.0f;        // g-excess to trigger a roll

// Hot Potato timings
static const float FUSE_MIN_S = 8.0f, FUSE_MAX_S = 16.0f;
static const float TICK_START_S = 0.58f, TICK_MIN_S = 0.12f, TICK_DECAY = 0.94f;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==================== Utilities ====================
enum Game { G_POTATO, G_DICE, G_SIMON };
static Game SELECTED = G_POTATO;

static inline void pixelsOff() {
  for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,0,0);
  CircuitPlayground.strip.show();
}

static inline void solid(uint8_t r,uint8_t g,uint8_t b) {
  for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,r,g,b);
  CircuitPlayground.strip.show();
}

// In-game A+B → reset that game's state (return true when reset is requested)
static inline bool abResetHeld(uint16_t ms=AB_RESET_MS) {
  if (!(CircuitPlayground.leftButton() && CircuitPlayground.rightButton())) return false;
  unsigned long start = millis();
  while (CircuitPlayground.leftButton() && CircuitPlayground.rightButton()) {
    if (millis() - start >= ms) {
      // confirm blink
      for (uint8_t k=0;k<2;k++){
        solid(10,0,10); delay(70);
        pixelsOff(); delay(70);
      }
      return true;
    }
    delay(10);
  }
  return false;
}

// Ensure both buttons are unpressed for a short time
static inline void waitButtonsReleased(uint16_t ms=300){
  unsigned long end = millis() + ms;
  for(;;){
    if (CircuitPlayground.leftButton() || CircuitPlayground.rightButton()){
      end = millis() + ms;
    }
    if (millis() >= end) return;
    delay(10);
  }
}

// Boot splash + one-time selection
// Boot splash + one-time selection with live clockwise confirm spinner
static Game splashAndPick(uint16_t windowMs=LAUNCHER_WINDOW_MS) {
  pixelsOff();

  // Static hints when no button is held
  auto showHints = [](){
    CircuitPlayground.setPixelColor(0,0,15,0);
    CircuitPlayground.setPixelColor(3,0,15,0);
    CircuitPlayground.setPixelColor(6,0,15,0);
    CircuitPlayground.setPixelColor(8,0,15,0); // A (SIMON) hints — green

    CircuitPlayground.setPixelColor(2,0,0,25);
    CircuitPlayground.setPixelColor(7,0,0,25); // B (DICE) hints — blue

    CircuitPlayground.setPixelColor(4,20,12,0);
    CircuitPlayground.setPixelColor(9,20,12,0); // default — amber
    CircuitPlayground.strip.show();
  };

  showHints();

  unsigned long t0 = millis();
  Game pick = G_POTATO;

  // spinner state
  uint8_t idx = 0;
  unsigned long nextStep = 0;
  const uint16_t STEP_MS = 55;          // spin speed
  const uint8_t TRAIL_DIM = 5;          // faint tail brightness (0..255 scale-ish)

  while (millis() - t0 < windowMs) {
    bool a = CircuitPlayground.leftButton();
    bool b = CircuitPlayground.rightButton();

    if (a || b) {
      // Choose color & selection based on which is held
      uint8_t r=0,g=0,bl=0;
      if (a && !b) { g = 30; pick = G_SIMON; }     // green for A
      else if (b && !a) { bl = 35; pick = G_DICE; } // blue for B
      else { r=20; g=0; bl=20; } // both held: magenta-ish (no change to pick)

      // Advance spinner on cadence
      if (millis() >= nextStep) {
        pixelsOff();
        // head pixel
        CircuitPlayground.setPixelColor(idx % 10, r, g, bl);
        // one-pixel tail (dimmed)
        CircuitPlayground.setPixelColor((idx + 9) % 10, r/TRAIL_DIM, g/TRAIL_DIM, bl/TRAIL_DIM);
        CircuitPlayground.strip.show();

        idx = (idx + 1) % 10;
        nextStep = millis() + STEP_MS;
      }
    } else {
      // No buttons: show static hints again
      showHints();
    }

    delay(5);
  }

  pixelsOff();
  return pick;
}

static inline void toneHz(uint16_t f, uint16_t ms){
  CircuitPlayground.playTone(f, ms);
}

// ==================== Game: Hot Potato (runs forever until reset) ====================
static void potatoIdle() {
  pixelsOff();
  waitButtonsReleased(250);   // ensure no stale A press

  // --- Force exactly one full breathe cycle so user SEES idle ---
  for (uint8_t b = 0; b <= 24; b++) {
    uint8_t g = (10 * b) / 12, bl = (30 * b) / 12;
    for (uint8_t i = 0; i < 10; i++) CircuitPlayground.setPixelColor(i, 0, g, bl);
    CircuitPlayground.strip.show();
    delay(14);
  }
  for (int8_t b = 24; b >= 0; b--) {
    uint8_t g = (10 * b) / 12, bl = (30 * b) / 12;
    for (uint8_t i = 0; i < 10; i++) CircuitPlayground.setPixelColor(i, 0, g, bl);
    CircuitPlayground.strip.show();
    delay(14);
  }

  // Now do normal idle breathe that honors a NEW A press
  bool aWas = false;
  for (;;) {
    if (abResetHeld()) { pixelsOff(); waitButtonsReleased(250); } // remain in idle

    // breathe up
    for (uint8_t b = 0; b <= 24; b++) {
      bool aNow = CircuitPlayground.leftButton();
      if (aNow && !aWas) {
        unsigned long t = millis();
        while (CircuitPlayground.leftButton() && (millis() - t) < 80) delay(5);
        if (CircuitPlayground.leftButton()) return;  // start round
      }
      aWas = aNow;
      uint8_t g = (10 * b) / 12, bl = (30 * b) / 12;
      for (uint8_t i = 0; i < 10; i++) CircuitPlayground.setPixelColor(i, 0, g, bl);
      CircuitPlayground.strip.show();
      delay(14);
    }

    // breathe down
    for (int8_t b = 24; b >= 0; b--) {
      bool aNow = CircuitPlayground.leftButton();
      if (aNow && !aWas) {
        unsigned long t = millis();
        while (CircuitPlayground.leftButton() && (millis() - t) < 80) delay(5);
        if (CircuitPlayground.leftButton()) return;  // start round
      }
      aWas = aNow;
      uint8_t g = (10 * b) / 12, bl = (30 * b) / 12;
      for (uint8_t i = 0; i < 10; i++) CircuitPlayground.setPixelColor(i, 0, g, bl);
      CircuitPlayground.strip.show();
      delay(14);
    }
  }
}

static void potatoRound() {
  pixelsOff();
  toneHz(440,100); delay(40); toneHz(660,100);

  float r = random(0,1000)/1000.0f;
  unsigned long deadline = millis() + (unsigned long)((FUSE_MIN_S + (FUSE_MAX_S-FUSE_MIN_S)*r)*1000.0f);
  float interval = TICK_START_S;
  unsigned long nextBeep = 0;

  while (true){
    if (abResetHeld()) return;  // reset to idle
    unsigned long now = millis();
    if (now >= deadline){
      // BOOM
      for (uint8_t k=0;k<2;k++){
        solid(40,0,0); toneHz(180,180); pixelsOff(); delay(50);
      }
      toneHz(120,450);
      solid(40,0,0); delay(600); pixelsOff();
      return; // back to idle (same game)
    }
    if (now >= nextBeep){
      // short beep + chasing pixel
      toneHz(400,50);
      uint8_t k = (uint8_t)((now/83)%10);
      pixelsOff();
      CircuitPlayground.setPixelColor(k,0,25,0);
      CircuitPlayground.strip.show();

      interval = max(TICK_MIN_S, interval * TICK_DECAY);
      nextBeep = now + (unsigned long)(interval*1000.0f);
    }
    // motion tension (optional)
    float ax= CircuitPlayground.motionX();
    float ay= CircuitPlayground.motionY();
    float az= CircuitPlayground.motionZ();
    if (fabs(ax)+fabs(ay)+fabs(az) > 14.0f) {  // Lower for more sensitive
      if (deadline > now + 25) deadline -= 25; // Higher increased penality
    }
    if (CircuitPlayground.rightButton()) { pixelsOff(); return; } // abort round to idle
    delay(4);
  }
}

void runPotatoForever(){
  for(;;){
    potatoIdle();
    potatoRound();
  }
}

// ==================== Alt-Mode: Whack-a-Mole (from Dice via B) ====================
void runWhackAMoleForever(){
  pixelsOff();
  Serial.print("[POTATO] Selected = WHACK");

  // --- Entry cue: green clockwise sweep + quick arpeggio ---
  for (uint8_t k=0; k<10; k++){
    pixelsOff();
    CircuitPlayground.setPixelColor(k, 0, 35, 0);        // green head
    CircuitPlayground.setPixelColor((k+9)%10, 0, 8, 0);  // faint tail
    CircuitPlayground.strip.show();
    CircuitPlayground.playTone(520 + k*14, 12);
    delay(18);
  }
  pixelsOff();
  waitButtonsReleased(200);

  // ---------- helpers ----------
  auto breatheIdleAndWaitA = [&](){
    // teal-ish breathe, wait for a NEW A press
    bool aWas = false;
    for(;;){
      if (abResetHeld()) { pixelsOff(); return false; } // exit to Dice

      // breathe up
      for (uint8_t b=0; b<=24; b++){
        bool aNow = CircuitPlayground.leftButton();
        if (aNow && !aWas){
          unsigned long t = millis();
          while (CircuitPlayground.leftButton() && (millis()-t) < 80) delay(5);
          if (CircuitPlayground.leftButton()) return true; // start game
        }
        aWas = aNow;

        uint8_t g  = (uint8_t)((b *  80) / 24);
        uint8_t bl = (uint8_t)((b * 200) / 24);
        for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i, 0, g, bl);
        CircuitPlayground.strip.show();
        delay(22);
      }
      // breathe down
      for (int8_t b=24; b>=0; b--){
        bool aNow = CircuitPlayground.leftButton();
        if (aNow && !aWas){
          unsigned long t = millis();
          while (CircuitPlayground.leftButton() && (millis()-t) < 80) delay(5);
          if (CircuitPlayground.leftButton()) return true; // start game
        }
        aWas = aNow;

        uint8_t g  = (uint8_t)((b *  80) / 24);
        uint8_t bl = (uint8_t)((b * 200) / 24);
        for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i, 0, g, bl);
        CircuitPlayground.strip.show();
        delay(22);
      }
    }
  };

  auto showOnes = [&](uint8_t ones){
    pixelsOff();
    for (uint8_t i=0;i<ones;i++) CircuitPlayground.setPixelColor(i, 0,35,0);
    CircuitPlayground.strip.show();
    unsigned long t0 = millis();
    while (millis()-t0 < 1800){
      if (abResetHeld()) { pixelsOff(); return false; }
      delay(10);
    }
    pixelsOff();
    return true;
  };

  // ---------- Tuning ----------
  const uint16_t START_WINDOW_MS = 900;    // time to hit the mole (shrinks)
  const uint16_t MIN_WINDOW_MS   = 250;
  const float    DECAY           = 0.93f;  // window *= DECAY after each hit
  const float    HIT_SHAKE_GEX   = SHAKE_THRESH; // reuse your threshold
  const uint32_t ROUND_MS        = 30000;  // play time: 30 seconds

  for(;;){  // alt-mode loop: idle → round → score → idle ...
    // ----- IDLE: breathe until A pressed -----
    if (!breatheIdleAndWaitA()) { return; } // A+B during idle exits to Dice
    waitButtonsReleased(180);
    pixelsOff();

    // ----- Timed round -----
    uint16_t window = START_WINDOW_MS;
    uint16_t score  = 0;
    unsigned long roundStart = millis();

    auto twinkle = [](uint8_t r, uint8_t g, uint8_t b, uint16_t dur=200){
      unsigned long t0=millis();
      while (millis()-t0 < dur){
        uint8_t i = (uint8_t)random(0,10);
        CircuitPlayground.setPixelColor(i, r,g,b);
        CircuitPlayground.strip.show();
        delay(28);
        CircuitPlayground.setPixelColor(i, 0,0,0);
      }
    };

    for(;;){
      if (abResetHeld()) { pixelsOff(); return; } // exit alt-mode entirely
      if (millis() - roundStart >= ROUND_MS) break;

      // Pick and show mole
      uint8_t mole = (uint8_t)random(0,10);
      pixelsOff();
      CircuitPlayground.setPixelColor(mole, 0,35,0);
      CircuitPlayground.strip.show();

      // Wait for hit within window
      unsigned long deadline = millis() + window;
      bool hit = false;
      while (millis() < deadline){
        if (abResetHeld()) { pixelsOff(); return; }
        if (millis() - roundStart >= ROUND_MS) { hit = false; goto END_ROUND_PHASE; }

        float ax= CircuitPlayground.motionX();
        float ay= CircuitPlayground.motionY();
        float az= CircuitPlayground.motionZ();
        float gex = fabs(ax)+fabs(ay)+fabs(az) - 9.81f;

        // Hit by shake OR press A
        if (gex > HIT_SHAKE_GEX || CircuitPlayground.leftButton()){
          hit = true;
          break;
        }
        delay(4);
      }

      if (hit){
        // Success: ding + quick green spin + harder next time
        CircuitPlayground.playTone(700, 90);
        for (uint8_t k=0;k<10;k++){
          pixelsOff();
          CircuitPlayground.setPixelColor((mole+k)%10, 0,35,0);
          CircuitPlayground.strip.show();
          delay(18);
        }
        score++;
        window = (uint16_t)max((int)MIN_WINDOW_MS, (int)((uint16_t)(window * DECAY)));
      } else {
        // Miss: red flash + buzz, and soften difficulty a bit
        solid(40,0,0); CircuitPlayground.playTone(220, 150);
        pixelsOff();   delay(120);
        window = (uint16_t)min((int)START_WINDOW_MS, (int)((uint16_t)(window / DECAY)));
      }

      // Tiny celebration twinkle
      twinkle(0, 20, 0, (uint16_t)min(400, 120 + (int)score*15));
      delay(random(150, 500));
    }

END_ROUND_PHASE:
    // --- End-of-round cue ---
    pixelsOff();
    for (uint8_t i=0;i<3;i++){ solid(0,0,40); toneHz(560 + i*140, 90); pixelsOff(); delay(80); }

    // --- Show score: tens = amber ring flashes; ones = green pips ---
    {
      uint8_t tens = score / 10;
      uint8_t ones = score % 10;

      for (uint8_t t=0; t<tens; t++){
        solid(25,16,0); toneHz(820, 80);  // amber + beep
        unsigned long t0 = millis();
        while (millis()-t0 < 220){
          if (abResetHeld()) { pixelsOff(); return; }
          delay(5);
        }
        pixelsOff();
        delay(500);
      }
      if (tens) delay(150);
      if (!showOnes(ones)) return; // A+B during ones display exits
    }

    // After score: loop back to idle breathe; press A to play again
    pixelsOff();
    waitButtonsReleased(200);
  }
}

// ==================== Game: Shake Dice (runs forever) ====================
void runDiceForever(){
  // Pips layout (ring indices)
  const uint8_t P1[] = {0};
  const uint8_t P2[] = {2,7};
  const uint8_t P3[] = {2,0,7};
  const uint8_t P4[] = {2,4,7,9};
  const uint8_t P5[] = {2,4,0,7,9};
  const uint8_t P6[] = {1,3,5,6,8,9};

  auto showPips = [](const uint8_t* arr, uint8_t n, uint8_t r,uint8_t g,uint8_t b){
    pixelsOff();
    for (uint8_t i=0;i<n;i++) CircuitPlayground.setPixelColor(arr[i], r,g,b);
    CircuitPlayground.strip.show();
  };

  waitButtonsReleased(300);

  for(;;){
    if (abResetHeld()) { pixelsOff(); waitButtonsReleased(250); } // remain in Dice

    // Press B to enter Whack-a-Mole
    if (CircuitPlayground.rightButton()){
      runWhackAMoleForever();
      pixelsOff();
      waitButtonsReleased(200);
      continue; // back to Dice loop after exiting alt-mode
    }

    bool roll = false;  // <-- declare BEFORE using it in the breathe loops

    // breathing idle (checks for shake/A throughout)
    for (uint8_t b = 0; b <= 24; b++) {
      // draw frame
      uint8_t bl = (uint8_t)((b * 70) / 24);   // 0..~70 blue
      for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i, 0, 0, bl);
      CircuitPlayground.strip.show();

      // trigger check
      float ax= CircuitPlayground.motionX();
      float ay= CircuitPlayground.motionY();
      float az= CircuitPlayground.motionZ();
      float gex = fabs(ax)+fabs(ay)+fabs(az) - 9.81f;
      if (gex > SHAKE_THRESH || CircuitPlayground.leftButton()) { roll = true; break; }

      if (abResetHeld()) { pixelsOff(); waitButtonsReleased(250); } // remain in Dice
      delay(18);
    }
    if (!roll) {
      for (int8_t b = 24; b >= 0; b--) {
        uint8_t bl = (uint8_t)((b * 70) / 24);
        for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i, 0, 0, bl);
        CircuitPlayground.strip.show();

        float ax= CircuitPlayground.motionX();
        float ay= CircuitPlayground.motionY();
        float az= CircuitPlayground.motionZ();
        float gex = fabs(ax)+fabs(ay)+fabs(az) - 9.81f;
        if (gex > SHAKE_THRESH || CircuitPlayground.leftButton()) { roll = true; break; }

        if (abResetHeld()) { pixelsOff(); waitButtonsReleased(250); }
        delay(18);
      }
    }

    // If triggered, do the roll sequence
    if (roll){
      // little spin
      unsigned long t0=millis(); uint8_t idx=0;
      while (millis()-t0 < 550){
        if (abResetHeld()) { pixelsOff(); break; }
        pixelsOff();
        CircuitPlayground.setPixelColor((idx++)%10, 0,0,25);
        CircuitPlayground.strip.show();
        delay(30);
      }

      uint8_t n = (uint8_t)random(1,7);  // 1..6
      switch(n){
        case 1: showPips(P1,1, 0,30,0);  break;
        case 2: showPips(P2,2, 0,30,0);  break;
        case 3: showPips(P3,3, 0,30,0);  break;
        case 4: showPips(P4,4,  0,30,0); break;
        case 5: showPips(P5,5,  0,30,0); break;
        case 6: showPips(P6,6,  0,30,0); break;
      }
      CircuitPlayground.playTone(320 + 40*n, 120);
      delay(2000);
    }

    delay(20);
  }
}



// ==================== Game: Simon-4 (runs forever) ====================
// Pads: A1 -> red (pixel 2, 440Hz) | A2 -> green (pixel 6, 523Hz)
//       A5 -> blue (pixel 9, 659Hz) | A6 -> amber (pixel 0, 784Hz)
struct Pad {
  uint8_t  pixelIndex;
  uint16_t freq;
  uint8_t  r,g,b;
  uint8_t  arduinoPin; // A1/A2/A5/A6
};

const Pad PADS[4] = {
  // pixelIndex, freq,  R,  G,  B,  arduinoPin
  {8, 440, 40,  0,  0, A2},   // A1 → Pixel 3  (red)
  {1, 523, 0,  40,  0, A5},   // A2 → Pixel 2  (green)
  {6, 659, 0,   0, 40, A1},   // A5 → Pixel 8  (blue)
  {3, 784, 40, 25,  0, A6}    // A6 → Pixel 7  (amber)
};

// Per-pad dynamic baselines measured at idle
static uint16_t capBase[4];

// Take CAP_SAMPLES readings per pad and average to establish baseline.
static void simonCalibrateCaps() {
  for (uint8_t i=0; i<4; i++) {
    uint32_t acc = 0;
    for (uint8_t k=0; k<CAP_SAMPLES; k++) {
      acc += CircuitPlayground.readCap(PADS[i].arduinoPin);
      delay(2);
    }
    capBase[i] = (uint16_t)(acc / CAP_SAMPLES);
  }
}

// Return true if pad idx is considered pressed now (baseline + margin).
static bool simonPadPressed(uint8_t idx) {
  uint16_t v = CircuitPlayground.readCap(PADS[idx].arduinoPin);
  uint16_t margin = CAP_DELTA_MIN;
  return v > (uint16_t)(capBase[idx] + margin);
}

// Wait for rising edge with hold + release hysteresis.
static int8_t simonWaitPadEdge() {
  for (;;) { // ensure all released
    bool anyHigh = false;
    for (uint8_t i=0;i<4;i++) {
      uint16_t v = CircuitPlayground.readCap(PADS[i].arduinoPin);
      if (v > (uint16_t)(capBase[i] + CAP_DELTA_MIN - CAP_RELEASE_HYST)) { anyHigh = true; break; }
    }
    if (!anyHigh) break;
    delay(5);
  }
  for (;;) { // wait for press + confirm
    for (uint8_t i=0;i<4;i++) {
      if (simonPadPressed(i)) {
        unsigned long t = millis();
        while (simonPadPressed(i) && (millis() - t) < 40) { delay(5); }
        if (simonPadPressed(i)) return (int8_t)i;
      }
    }
    delay(5);
  }
}

static void flashPad(uint8_t idx, uint16_t ms=280){
  const Pad& p = PADS[idx];
  CircuitPlayground.playTone(p.freq, ms);

  // Use explicit pixel index
  CircuitPlayground.setPixelColor(p.pixelIndex, p.r, p.g, p.b);
  CircuitPlayground.strip.show();

  delay(ms);

  CircuitPlayground.setPixelColor(p.pixelIndex, 0,0,0);
  CircuitPlayground.strip.show();
  delay(60);
}

static void loseFlash(){
  for (uint8_t k=0;k<3;k++){
    solid(35,0,0); CircuitPlayground.playTone(180,180);
    pixelsOff(); delay(90);
  }
}

// ==================== Alt-Mode: Balancer (from Simon via B) ====================
void runBalancerForever(){
  pixelsOff();
  Serial.print("[SIMON] Selected = BALANCE");
  // --- Entry cue: amber “safe zone” pulse + two beeps ---
  for (uint8_t rep=0; rep<2; rep++){
    for (uint8_t i=0; i<10; i++) CircuitPlayground.setPixelColor(i, 25, 16, 0); // amber ring
    CircuitPlayground.strip.show();
    CircuitPlayground.playTone(600 + rep*120, 80);
    delay(40);
    pixelsOff();
    delay(40);
  }
  pixelsOff();
  waitButtonsReleased(200);

  // Tuning
  const uint8_t  SAFE_WIDTH_START = 3;   // safe zone half-width in LEDs (total zone = 2*W+1)
  const uint8_t  SAFE_WIDTH_MIN   = 1;
  const uint16_t LOSE_MS          = 700; // time allowed outside safe zone before fail
  const uint16_t STEP_MS          = 35;  // update cadence
  const uint16_t SHRINK_EVERY_MS  = 6000; // shrink difficulty every N ms
  const uint16_t WIN_PAUSE_MS = 3000;

  uint8_t center = 0;              // safe zone center (moves each level)
  uint8_t halfW  = SAFE_WIDTH_START;
  unsigned long outsideSince = 0;  // 0 = currently inside
  unsigned long lastStep = 0, lastShrink = millis();

  auto inZone = [&](uint8_t idx){
    int8_t d = (int8_t)((idx - center + 10) % 10);
    if (d > 5) d -= 10; // shortest circular distance
    return (abs(d) <= (int)halfW);
  };

  auto drawFrame = [&](uint8_t ballIdx){
    // safe zone = amber, ball = blue, rest off
    pixelsOff();
    for (int8_t k=-halfW; k<= (int8_t)halfW; k++){
      uint8_t z = (uint8_t)((center + k + 10) % 10);
      CircuitPlayground.setPixelColor(z, 25,16,0); // amber safe zone
    }
    CircuitPlayground.setPixelColor(ballIdx, 0,0,40); // ball
    CircuitPlayground.strip.show();
  };

  auto loseFlash = [&](){
    for (uint8_t k=0;k<3;k++){ solid(40,0,0); CircuitPlayground.playTone(200,150); pixelsOff(); delay(120); }
  };

  // Randomize initial center
  center = (uint8_t)random(0,10);

  for(;;){
    if (abResetHeld()) { pixelsOff(); return; } // back to Simon

    unsigned long now = millis();
    if (now - lastStep < STEP_MS) { delay(2); continue; }
    lastStep = now;

    // Read tilt and map to ring index
    float ax = CircuitPlayground.motionX();
    float ay = CircuitPlayground.motionY();
    // angle in [-PI, PI], 0 at +X; map around the ring
    float ang = atan2f(ay, ax);                // -pi..pi
    float norm = (ang + (float)M_PI) / (2.0f*(float)M_PI); // 0..1
    uint8_t ball = (uint8_t)roundf(norm * 9.0f) % 10;      // 0..9

    // Difficulty ramp: shrink zone over time OR win if we'd go past 3 total LEDs
    if (now - lastShrink >= SHRINK_EVERY_MS){
      if (halfW > SAFE_WIDTH_MIN){
        halfW--;
        lastShrink = now;
        CircuitPlayground.playTone(660, 80);
      } else {
        // We are at halfW == 1 (3 total LEDs). Next shrink would go past 3 → WIN!
        // Celebration: blue/amber alternate + ascending tones
        for (uint8_t rep = 0; rep < 3; rep++){
          if (abResetHeld()) { pixelsOff(); return; }
          solid(0,0,40); CircuitPlayground.playTone(600 + rep*140, 120); delay(100);
          if (abResetHeld()) { pixelsOff(); return; }
          solid(25,16,0); delay(120);
        }
        pixelsOff();

        // Pause, then restart Balance fresh
        unsigned long tWin = millis();
        while (millis() - tWin < WIN_PAUSE_MS){
          if (abResetHeld()) { pixelsOff(); return; } // allow exit during pause
          delay(10);
        }
        

        // Reset state and start again
        halfW = SAFE_WIDTH_START;
        center = (uint8_t)random(0,10);
        outsideSince = 0;
        lastStep = millis();
        lastShrink = millis();
        pixelsOff();
        delay(200);
      }
    }

    // Draw
    drawFrame(ball);

    // Check zone
    if (inZone(ball)){
      outsideSince = 0; // safe
    } else {
      if (outsideSince == 0) outsideSince = now;
      if (now - outsideSince >= LOSE_MS){
        loseFlash();
        // Reset zone a bit easier again
        halfW = SAFE_WIDTH_START;
        center = (uint8_t)random(0,10);
        outsideSince = 0;
        lastShrink = now;
        pixelsOff();
        delay(250);
      }
    }

    // Optional: press A to “re-center” the safe zone to current ball
    if (CircuitPlayground.leftButton()){
      center = ball;
      CircuitPlayground.playTone(523, 80);
    }
  }
}


void runSimonForever(){
  waitButtonsReleased(300);
  pixelsOff();

  uint8_t seq[64]; uint8_t len=0;

  for(;;){
SIMON_IDLE_TOP:
    // ---------- IDLE (teal breathing) ----------
    simonCalibrateCaps();
    waitButtonsReleased(200);

    // quick teal cue
    for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i, 0, 18, 45);
    CircuitPlayground.strip.show();
    delay(250);
    pixelsOff();

    // one attract cycle
    for (uint8_t b=0;b<=24;b++){
      // ✅ check B continuously during idle
      if (CircuitPlayground.rightButton()){
        runBalancerForever();
        pixelsOff();
        waitButtonsReleased(200);
        goto SIMON_IDLE_TOP;  // re-enter idle after alt-mode
      }

      uint8_t g  = (uint8_t)((b *  80) / 24);
      uint8_t bl = (uint8_t)((b * 200) / 24);
      for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,g,bl);
      CircuitPlayground.strip.show();
      delay(30);
    }
    for (int8_t b=24;b>=0;b--){
      if (CircuitPlayground.rightButton()){
        runBalancerForever();
        pixelsOff();
        waitButtonsReleased(200);
        goto SIMON_IDLE_TOP;
      }

      uint8_t g  = (uint8_t)((b *  80) / 24);
      uint8_t bl = (uint8_t)((b * 200) / 24);
      for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,g,bl);
      CircuitPlayground.strip.show();
      delay(30);
    }

    // wait for NEW A press to start
    bool aWas = false;
    for(;;){
      if (abResetHeld()) { pixelsOff(); waitButtonsReleased(250); simonCalibrateCaps(); }

      for (uint8_t b=0;b<=24;b++){
        // ✅ still allow entering Balancer while waiting for A
        if (CircuitPlayground.rightButton()){
          runBalancerForever();
          pixelsOff();
          waitButtonsReleased(200);
          goto SIMON_IDLE_TOP;
        }

        bool aNow = CircuitPlayground.leftButton();
        if (aNow && !aWas) {
          unsigned long t = millis();
          while (CircuitPlayground.leftButton() && (millis()-t) < 100) delay(5);
          if (CircuitPlayground.leftButton()) goto START_SIMON_ROUND;
        }
        aWas = aNow;

        uint8_t g=(uint8_t)((b *  80) / 24), bl=(uint8_t)((b * 200) / 24);
        for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,g,bl);
        CircuitPlayground.strip.show();
        delay(30);
      }
      for (int8_t b=24;b>=0;b--){
        if (CircuitPlayground.rightButton()){
          runBalancerForever();
          pixelsOff();
          waitButtonsReleased(200);
          goto SIMON_IDLE_TOP;
        }

        bool aNow = CircuitPlayground.leftButton();
        if (aNow && !aWas) {
          unsigned long t = millis();
          while (CircuitPlayground.leftButton() && (millis()-t) < 100) delay(5);
          if (CircuitPlayground.leftButton()) goto START_SIMON_ROUND;
        }
        aWas = aNow;

        uint8_t g=(uint8_t)((b *  80) / 24), bl=(uint8_t)((b * 200) / 24);
        for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,g,bl);
        CircuitPlayground.strip.show();
        delay(30);
      }
    }

START_SIMON_ROUND:
    // … (unchanged)
    ;
  }
}


// ==================== Setup / Loop ====================
void setup(){
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(BRIGHTNESS);
  pixelsOff();

  Serial.begin(115200);
  // Short grace window: wait up to ~600 ms for Serial to appear (but don’t block forever)
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0) < 600) { /* just wait a bit */ }

  Serial.println("Playground Games (single-select) launcher");

  // Brief ready cue
  CircuitPlayground.setPixelColor(4,20,12,0);
  CircuitPlayground.setPixelColor(9,20,12,0);
  delay(250);
  pixelsOff();

  // One-time selection at boot
  SELECTED = splashAndPick();

  // tiny color signature so you know which game is selected
  if (SELECTED == G_SIMON) { solid(0,18,0); }
  else if (SELECTED == G_DICE) { solid(0,0,25); }
  else { solid(20,12,0); }
  delay(200);
  pixelsOff();

  // debug (now likely to be visible)
  Serial.print("[BOOT] Selected = ");
  Serial.println(SELECTED == G_SIMON ? "SIMON" : (SELECTED == G_DICE ? "DICE" : "POTATO"));
}

void loop(){
  // Stay in the chosen game forever until hardware reset.
  switch (SELECTED){
    case G_SIMON:  runSimonForever();  break;
    case G_DICE:   runDiceForever();   break;
    case G_POTATO: runPotatoForever(); break;
  }
}