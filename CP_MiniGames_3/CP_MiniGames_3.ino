/*
  cp_minigames_3.ino
  ---------------------------------------------------------------------------
  CPX/CPB no-screen launcher + 3 arcade games (each with an alt-mode)
  Boards      : Adafruit Circuit Playground Express (SAMD21) or Bluefruit (nRF52840)
  Dependencies: Adafruit Circuit Playground library (install via Library Manager)

  OVERVIEW
  - One-time selection at boot (≈1.5 s window), then the chosen game runs forever
    until a hardware reset. In-game A+B (held ≈1.5 s) resets that game back to its
    own idle screen (does NOT return to the launcher).
  - Games included:
      • Tug-of-War (default if no boot button is held)
        - Press A in idle → SOLO vs CPU
        - Press B in idle → TWO-PLAYER (A vs Right button)
      • Reaction Timer  (hold A at boot) → Alt-mode: Tilt Maze (press B in Reaction)
      • Rapid Fire      (hold B at boot) → Alt-mode: Shaker Challenge (press B in Rapid)

  BOOT SELECTION (splashAndPick)
    Hold during the boot window (~1500 ms):
      - A held           → start REACTION (green spinner)
      - B held           → start RAPID FIRE (blue spinner)
      - none             → start TUG-OF-WAR (amber default)
    A rotating “confirm” spinner shows selection color while held.

  IN-GAME GLOBAL CONTROL
    - A+B held ~1.5 s (AB_RESET_MS) → reset the current game to its idle state.
    - Helper: waitButtonsReleased(ms) prevents stale button presses between phases.

  HARDWARE NOTES
    - NeoPixel ring: 10 LEDs via CircuitPlayground.setPixelColor(i, r,g,b).
    - Accelerometer: motionX/Y/Z() used for shake/tilt.
    - BRIGHTNESS: global NeoPixel brightness (0–255).

  AUTHOR / LICENSE
    - Prototype and integration: © 2025 Iain Bennett.
    
  ---------------------------------------------------------------------------
*/

#include <Adafruit_CircuitPlayground.h>
#include <Arduino.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==================== Tuning ====================
static const uint8_t  BRIGHTNESS         = 55;      // 0..255 overall LED brightness
static const uint16_t LAUNCHER_WINDOW_MS = 1500;
static const uint16_t AB_RESET_MS        = 1500;    // A+B in-game: reset that game

// Reaction Timer
static const uint16_t REACT_ARM_FLASH_MS = 1400;    // "wait..." breathing period (cosmetic)
static const uint16_t REACT_MIN_DELAY_MS = 900;     // random delay after arm
static const uint16_t REACT_MAX_DELAY_MS = 2600;
static const uint16_t REACT_FALSE_PENALTY_MS = 1500;

// Tilt Maze (alt for Reaction)
static const uint16_t TILT_STEP_MS   = 45;
static const uint16_t TILT_LOSE_MS   = 1200; // time allowed outside checkpoint before reset
static const float    TILT_GAIN      = 1.35f; // tilt sensitivity

// Rapid Fire
static const uint16_t RF_ROUND_MS    = 10000;  // a little longer round
static const uint16_t RF_DECAY_MS    = 450;    // slower decay (≈3.8/s)
static const uint8_t  RF_GOAL        = 10;      // slightly lower target

// Shaker Challenge (alt for Rapid)
static const float    SHK_THRESH     = 1.2f;   // ~0.12 g to count as a shake
static const float    SHK_EN_FROM_G  = 0.045f; // energy gain per m/s^2 above thresh
static const float    SHK_EN_DECAY   = 0.985f; // keep as-is (feel)
static const uint16_t SHK_TICK_MS    = 26;
static const uint16_t SHK_ROUND_MS  = 12000;  // total time for a round
static const uint16_t SHK_HOLD_MS   = 1500;   // must keep meter ≥9 for this long to win

// Tug-of-War
static const uint8_t  TUG_TRACK_LEN  = 9;     // distance from center to win
static const uint16_t TUG_STEP_MS    = 22;    // update cadence
// SOLO CPU behavior
static const uint16_t CPU_PULSE_MS   = 260;   // CPU tries every N ms
static const uint8_t  CPU_PUSH_CHANCE= 55;    // % per pulse to push one step
static const uint16_t SOLO_ROUND_MS  = 20000; // solo timeout (ms)

// ==================== Utilities ====================
static inline void pixelsOff() {
  for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,0,0);
  CircuitPlayground.strip.show();
}
static inline void solid(uint8_t r,uint8_t g,uint8_t b) {
  for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,r,g,b);
  CircuitPlayground.strip.show();
}
static inline void setPix(uint8_t i, uint8_t r,uint8_t g,uint8_t b){
  CircuitPlayground.setPixelColor(i%10,r,g,b);
}
static inline bool abResetHeld(uint16_t ms=AB_RESET_MS) {
  if (!(CircuitPlayground.leftButton() && CircuitPlayground.rightButton())) return false;
  unsigned long start = millis();
  while (CircuitPlayground.leftButton() && CircuitPlayground.rightButton()) {
    if (millis() - start >= ms) {
      // confirm blink
      for (uint8_t k=0;k<2;k++){ solid(10,0,10); delay(70); pixelsOff(); delay(70); }
      return true;
    }
    delay(10);
  }
  return false;
}
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
static inline float gex(){
  float ax = CircuitPlayground.motionX();
  float ay = CircuitPlayground.motionY();
  float az = CircuitPlayground.motionZ();

  // Remove gravity by comparing the vector magnitude to 1 g.
  float mag = sqrtf(ax*ax + ay*ay + az*az);   // m/s^2
  float ge  = fabsf(mag - 9.81f);             // "g-excess" in m/s^2

  // Small deadband to ignore sensor noise
  if (ge < 0.05f) ge = 0.0f;
  return ge;
}
static inline void toneHz(uint16_t f, uint16_t ms){ CircuitPlayground.playTone(f, ms); }

// ==================== Boot Selection ====================
enum Game { G_TUG, G_REACT, G_RAPID };
static Game SELECTED = G_TUG;

static Game splashAndPick(uint16_t windowMs=LAUNCHER_WINDOW_MS) {
  pixelsOff();

  auto showHints = [](){
    // A (REACT) green hints
    setPix(0,0,20,0); setPix(3,0,20,0);
    // B (RAPID) blue hints
    setPix(2,0,0,28); setPix(7,0,0,28);
    // default (TUG) amber hints
    setPix(5,24,12,0); setPix(9,24,12,0);
    CircuitPlayground.strip.show();
  };
  showHints();

  unsigned long t0 = millis();
  uint8_t idx=0; unsigned long nextStep=0;
  const uint16_t STEP_MS=55;
  Game pick = G_TUG;

  while (millis() - t0 < windowMs){
    bool a = CircuitPlayground.leftButton();
    bool b = CircuitPlayground.rightButton();

    uint8_t r=0,g=0,bl=0;
    if (a && !b){ g=35; pick=G_REACT; }
    else if (b && !a){ bl=40; pick=G_RAPID; }
    else if (a && b){ r=25; bl=25; }
    else { showHints(); delay(5); continue; }

    if (millis() >= nextStep){
      pixelsOff();
      setPix(idx%10, r,g,bl);
      setPix((idx+9)%10, r/6,g/6,bl/6);
      CircuitPlayground.strip.show();
      idx=(idx+1)%10; nextStep = millis()+STEP_MS;
    }
    delay(5);
  }
  pixelsOff();
  return pick;
}

// ==================== Game: Reaction Timer (with alt Tilt Maze) ====================
static void runTiltMazeForever(){
  pixelsOff(); waitButtonsReleased(180);
  // Waypoints around ring in order
  const uint8_t path[10] = {0,2,4,6,8,1,3,5,7,9};
  uint8_t wp = 0;
  unsigned long lastStep=0, outsideSince=0;
  for(;;){
    if (abResetHeld()) { pixelsOff(); return; } // back to Reaction

    unsigned long now=millis();
    if (now - lastStep < TILT_STEP_MS) { delay(2); continue; }
    lastStep = now;

    // derive ball index from tilt
    float ax= CircuitPlayground.motionX();
    float ay= CircuitPlayground.motionY();
    float ang = atan2f(ay, ax); // -pi..pi
    float norm= (ang + (float)M_PI) / (2.0f*(float)M_PI);
    int ball = (int)roundf(norm * 9.0f) % 10;

    uint8_t target = path[wp];

    // draw: target amber halo, ball blue
    pixelsOff();
    setPix(target, 25,16,0);
    setPix((target+1)%10, 10,6,0); setPix((target+9)%10, 10,6,0);
    setPix(ball, 0,0,40);
    CircuitPlayground.strip.show();

    if (ball == target){
      CircuitPlayground.playTone(600 + 30*wp, 60);
      wp = (uint8_t)((wp+1)%10);
      outsideSince = 0;
      // tiny flash
      for(uint8_t k=0;k<2;k++){ setPix(target,0,25,0); CircuitPlayground.strip.show(); delay(40); setPix(target,25,16,0); CircuitPlayground.strip.show(); delay(40); }
    } else {
      if (outsideSince==0) outsideSince=now;
      if (now - outsideSince >= TILT_LOSE_MS){
        // fail flash then reset to start
        for(uint8_t k=0;k<3;k++){ solid(40,0,0); toneHz(220,120); pixelsOff(); delay(80); }
        wp=0; outsideSince=0;
      }
    }
  }
}

static void runReactionForever(){
  waitButtonsReleased(250);
  pixelsOff();

  for(;;){
REACT_IDLE:
    // Idle breathe teal; TAP A to arm; TAP B to enter Tilt Maze
    bool aWas = false;

    // ---- breathe up ----
    for (uint8_t b=0; b<=24; b++){
      if (abResetHeld()) { pixelsOff(); waitButtonsReleased(200); }
      if (CircuitPlayground.rightButton()){
        runTiltMazeForever(); pixelsOff(); waitButtonsReleased(180); aWas=false; goto REACT_IDLE;
      }

      // draw frame
      for (uint8_t i=0;i<10;i++) setPix(i, 0, (uint8_t)((b*80)/24), (uint8_t)((b*180)/24));
      CircuitPlayground.strip.show();

      // TAP detection (rising edge)
      bool aNow = CircuitPlayground.leftButton();
      if (aNow && !aWas) {  // <-- TAP detected
        waitButtonsReleased(120);
        pixelsOff();
        goto REACT_ARM;     // jump to arm/ready state
      }
      aWas = aNow;

      delay(22);
    }

    // ---- breathe down ----
    for (int8_t b=24; b>=0; b--){
      if (abResetHeld()) { pixelsOff(); waitButtonsReleased(200); }
      if (CircuitPlayground.rightButton()){
        runTiltMazeForever(); pixelsOff(); waitButtonsReleased(180); aWas=false; goto REACT_IDLE;
      }

      for (uint8_t i=0;i<10;i++) setPix(i, 0, (uint8_t)((b*80)/24), (uint8_t)((b*180)/24));
      CircuitPlayground.strip.show();

      bool aNow = CircuitPlayground.leftButton();
      if (aNow && !aWas) {           // <-- TAP detected
        waitButtonsReleased(120);
        pixelsOff();
        goto REACT_ARM;
      }
      aWas = aNow;

      delay(22);
    }

    // loop back to idle breathe if no tap occurred
    continue;

    // -------------------- ARM / READY --------------------
REACT_ARM:
    // ARM: show "ready" blue ring
    for (uint8_t i=0;i<10;i++) setPix(i, 0,0,30);
    CircuitPlayground.strip.show();

    // Small release guard so a long press doesn't trigger false start instantly
    waitButtonsReleased(120);

    // random wait; if early press = penalty
    {
      unsigned long delayMs = REACT_MIN_DELAY_MS
                            + (unsigned long)random(0, REACT_MAX_DELAY_MS-REACT_MIN_DELAY_MS+1);
      unsigned long t0 = millis();
      while (millis()-t0 < delayMs){
        if (abResetHeld()) { pixelsOff(); goto REACT_IDLE; }
        if (CircuitPlayground.leftButton() || CircuitPlayground.rightButton()){
          // false start
          solid(40,0,0); toneHz(200,220); pixelsOff();
          delay(REACT_FALSE_PENALTY_MS);
          goto REACT_IDLE;
        }
        delay(2);
      }
    }

    // GO cue: green flash + tone; measure to first press
    solid(0,35,0); toneHz(880,90);
    {
      unsigned long go = millis();
      for(;;){
        if (abResetHeld()) { pixelsOff(); goto REACT_IDLE; }
        if (CircuitPlayground.leftButton() || CircuitPlayground.rightButton()){
          unsigned long rt = millis()-go;

          // Display reaction time: hundreds as amber flashes, ones-of-10ms as green pips
          pixelsOff();
          uint16_t hundreds = (uint16_t)(rt/100);
          if (hundreds==0) hundreds=1; // always show at least one flash
          for (uint16_t k=0;k<hundreds;k++){ solid(25,16,0); toneHz(760,60); pixelsOff(); delay(220); }
          delay(160);
          uint8_t ones = (uint8_t)((rt/10)%10);
          for (uint8_t i=0;i<ones;i++) setPix(i, 0,35,0);
          CircuitPlayground.strip.show();
          delay(1600);
          pixelsOff();
          waitButtonsReleased(150);
          break; // back to idle
        }

        // small spinner while waiting for press
        uint8_t idx = (uint8_t)(((millis()-go)/40)%10);
        pixelsOff(); setPix(idx,0,35,0); CircuitPlayground.strip.show();
        delay(6);
      }
    }
  }
}

// ==================== Game: Rapid Fire (with alt Shaker Challenge) ====================
static void runShakerForever(){
  pixelsOff(); 
  waitButtonsReleased(180);

  // --- Round state ---
  float energy = 0.0f;
  unsigned long tRoundStart = millis();
  unsigned long tHoldStart  = 0;   // 0 = not holding yet
  bool announcedNearFull = false;

  auto drawMeter = [&](uint8_t n){
    pixelsOff();
    for (uint8_t i=0;i<n;i++) setPix(i, 0, 24, 40);  // teal fill
    CircuitPlayground.strip.show();
  };

  // Optional: show a quick “ready” pulse
  for (uint8_t k=0;k<2;k++){ solid(0,0,30); toneHz(720,70); pixelsOff(); delay(90); }

  for(;;){
    if (abResetHeld()) { pixelsOff(); return; } // back to Rapid idle

    // —— Energy integration ——
    float ge = gex();                       // g-excess
    if (ge > SHK_THRESH) energy += ge * SHK_EN_FROM_G;
    energy *= SHK_EN_DECAY;                 // natural decay
    if (energy < 0) energy = 0;
    if (energy > 2.0f) energy = 2.0f;       // clamp

    // Map to LEDs (no baseline offset now; empty is truly empty)
    //uint8_t n = (uint8_t)constrain((int)(energy * 6.0f + 0.5f), 0, 10);
    uint8_t n = (uint8_t)constrain((int)(energy*6.0f + 0.5f), 0, 10);
    drawMeter(n);

    // Near-full encouragement chirp
    if (n >= 9){
      if (!announcedNearFull && (millis()/280)%2==0) { toneHz(920,22); }
      announcedNearFull = true;
    } else {
      announcedNearFull = false;
    }

    // —— Win condition: hold ≥9 LEDs for SHK_HOLD_MS ——
    if (n >= 9){
      if (tHoldStart == 0) tHoldStart = millis();
      if (millis() - tHoldStart >= SHK_HOLD_MS){
        // WIN!
        for (uint8_t k=0;k<3;k++){ solid(0,35,0); toneHz(980,120); pixelsOff(); delay(120); }
        // Return to Rapid idle
        return;
      }
    } else {
      tHoldStart = 0; // lost the hold, restart the timer
    }

    // —— Lose condition: out of time ——
    if (millis() - tRoundStart >= SHK_ROUND_MS){
      for (uint8_t k=0;k<3;k++){ solid(40,0,0); toneHz(220,140); pixelsOff(); delay(120); }
      return; // back to Rapid idle
    }

    delay(SHK_TICK_MS);
  }
}

static void runRapidForever(){
  waitButtonsReleased(250);
  pixelsOff();

  for(;;){
RAPID_IDLE:
    // Idle: purple breathe; A starts a round; B enters Shaker Challenge
    for (uint8_t b=0;b<=24;b++){
      if (abResetHeld()) { pixelsOff(); waitButtonsReleased(180); }
      if (CircuitPlayground.rightButton()){ runShakerForever(); pixelsOff(); waitButtonsReleased(160); goto RAPID_IDLE; }
      for (uint8_t i=0;i<10;i++) setPix(i, (uint8_t)((b*60)/24),0,(uint8_t)((b*80)/24));
      CircuitPlayground.strip.show(); delay(20);
      if (CircuitPlayground.leftButton()) break;
    }
    if (!CircuitPlayground.leftButton()) continue;
    waitButtonsReleased(120);
    pixelsOff();

    // Round
    unsigned long tEnd = millis()+RF_ROUND_MS;
    unsigned long nextDecay = millis()+RF_DECAY_MS;
    uint8_t filled = 0;

    for(;;){
      if (abResetHeld()) { pixelsOff(); goto RAPID_IDLE; }
      if (CircuitPlayground.leftButton()){
        if (filled < 10){ filled++; toneHz(600 + filled*20, 20); }
        waitButtonsReleased(30);
      }

      // decay over time
      unsigned long now=millis();
      if (now >= nextDecay){
        nextDecay += RF_DECAY_MS;
        if (filled>0) filled--;
      }

      // draw bar
      pixelsOff();
      for (uint8_t i=0;i<filled;i++) setPix(i, 0, 35, 0);
      CircuitPlayground.strip.show();

      // win/lose
      if (filled >= RF_GOAL){
        for (uint8_t k=0;k<3;k++){ solid(0,35,0); toneHz(900,120); pixelsOff(); delay(120); }
        delay(500); goto RAPID_IDLE;
      }
      if (now >= tEnd){
        for (uint8_t k=0;k<3;k++){ solid(40,0,0); toneHz(220,140); pixelsOff(); delay(120); }
        delay(400); goto RAPID_IDLE;
      }
      delay(8);
    }
  }
}

// ==================== Game: Tug-of-War (default) ====================
static void drawTugTrack(int8_t pos){
  // pos: negative = left advantage; positive = right advantage
  pixelsOff();
  // center marker faint
  setPix(0, 8,8,8);
  // draw gradient to both sides
  int8_t leftCount  = (int8_t)max(0, -pos);
  int8_t rightCount = (int8_t)max(0,  pos);
  for (int8_t i=1;i<=leftCount && i<=TUG_TRACK_LEN;i++)  setPix((10 - i)%10, 0, 25, 0);    // left = green
  for (int8_t i=1;i<=rightCount && i<=TUG_TRACK_LEN;i++) setPix((0 + i)%10,   0, 0, 35);  // right = blue
  // head pixel for current position
  if (pos<0) setPix((10 - min((int)TUG_TRACK_LEN, -pos))%10, 0, 40, 0);
  else if (pos>0) setPix(min((int)TUG_TRACK_LEN, pos)%10, 0, 0, 40);
  CircuitPlayground.strip.show();
}

static void tugWinFlash(bool leftWins){
  for (uint8_t k=0;k<3;k++){
    if (leftWins) solid(0,40,0); else solid(0,0,40);
    toneHz(leftWins?820:520, 150);
    pixelsOff(); delay(120);
  }
}

static void runTugSolo(){
  // You (Left, button A) vs CPU (Right)
  int8_t pos = 0;
  unsigned long lastDraw=0, lastCpu=millis();
  unsigned long tEnd = millis()+SOLO_ROUND_MS;

  for(;;){
    if (abResetHeld()) { pixelsOff(); return; }

    unsigned long now = millis();
    if (CircuitPlayground.leftButton()){
      pos--; if (pos < -(int8_t)TUG_TRACK_LEN) pos = -(int8_t)TUG_TRACK_LEN;
      toneHz(660, 15);
      waitButtonsReleased(30);
    }

    if (now - lastCpu >= CPU_PULSE_MS){
      lastCpu = now;
      if ((uint8_t)random(0,100) < CPU_PUSH_CHANCE){ pos++; if (pos > (int8_t)TUG_TRACK_LEN) pos = (int8_t)TUG_TRACK_LEN; toneHz(400,15); }
    }

    if (now - lastDraw >= TUG_STEP_MS){ drawTugTrack(pos); lastDraw = now; }

    if (pos <= -(int8_t)TUG_TRACK_LEN){ tugWinFlash(true); return; }
    if (pos >=  (int8_t)TUG_TRACK_LEN){ tugWinFlash(false); return; }
    if (now >= tEnd){
      // timeout: whoever is ahead wins (or draw → quick neutral flash)
      if (pos<0) tugWinFlash(true);
      else if (pos>0) tugWinFlash(false);
      else { for (uint8_t k=0;k<3;k++){ solid(25,16,0); toneHz(600,120); pixelsOff(); delay(120);} }
      return;
    }
    delay(2);
  }
}

static void runTugTwoPlayer(){
  // Left player taps A, Right player taps Right button
  int8_t pos = 0;
  unsigned long lastDraw=0;
  for(;;){
    if (abResetHeld()) { pixelsOff(); return; }

    if (CircuitPlayground.leftButton()){ pos--; if (pos < -(int8_t)TUG_TRACK_LEN) pos = -(int8_t)TUG_TRACK_LEN; toneHz(760,15); waitButtonsReleased(25); }
    if (CircuitPlayground.rightButton()){ pos++; if (pos > (int8_t)TUG_TRACK_LEN) pos = (int8_t)TUG_TRACK_LEN; toneHz(520,15); waitButtonsReleased(25); }

    unsigned long now=millis();
    if (now - lastDraw >= TUG_STEP_MS){ drawTugTrack(pos); lastDraw = now; }

    if (pos <= -(int8_t)TUG_TRACK_LEN){ tugWinFlash(true); return; }
    if (pos >=  (int8_t)TUG_TRACK_LEN){ tugWinFlash(false); return; }
    delay(2);
  }
}

static void runTugForever(){
  waitButtonsReleased(250);
  pixelsOff();

  for(;;){
TUG_IDLE:
    // Idle: amber breathe, A starts SOLO, B starts TWO-PLAYER
    for (uint8_t b=0;b<=24;b++){
      if (abResetHeld()) { pixelsOff(); waitButtonsReleased(200); }
      for (uint8_t i=0;i<10;i++) setPix(i, (uint8_t)((b*80)/24), (uint8_t)((b*50)/24), 0);
      CircuitPlayground.strip.show(); delay(22);
      if (CircuitPlayground.leftButton() || CircuitPlayground.rightButton()) break;
    }
    if (CircuitPlayground.leftButton()){ waitButtonsReleased(140); runTugSolo(); pixelsOff(); waitButtonsReleased(160); continue; }
    if (CircuitPlayground.rightButton()){ waitButtonsReleased(140); runTugTwoPlayer(); pixelsOff(); waitButtonsReleased(160); continue; }
  }
}

// ==================== Setup / Loop ====================
void setup(){
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(BRIGHTNESS);
  pixelsOff();

  Serial.begin(115200);
  unsigned long t0=millis();
  while (!Serial && (millis()-t0)<600) { /* brief wait for serial */ }

  // Brief ready cue
  setPix(4,24,12,0); setPix(9,24,12,0); CircuitPlayground.strip.show(); delay(220); pixelsOff();

  // One-time selection at boot
  SELECTED = splashAndPick();

  // tiny color signature so you know which game is selected
  if      (SELECTED == G_REACT) { solid(0,18,0); }
  else if (SELECTED == G_RAPID) { solid(0,0,25); }
  else                          { solid(25,16,0); }
  delay(200);
  pixelsOff();

  Serial.print("[BOOT] Selected = ");
  Serial.println(SELECTED == G_REACT ? "REACTION" : (SELECTED == G_RAPID ? "RAPID" : "TUG"));
}

void loop(){
  // Stay in the chosen game forever until hardware reset.
  switch (SELECTED){
    case G_REACT: runReactionForever(); break;
    case G_RAPID: runRapidForever();    break;
    case G_TUG:   runTugForever();      break;
  }
}