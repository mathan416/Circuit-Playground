/*
  cp_minigames_2.ino — Fidget Spinner pack for Circuit Playground (Express / Bluefruit)
  ----------------------------------------------------------------------------
  Boards      : Adafruit Circuit Playground Express (SAMD21) or Bluefruit (nRF52840)
  Libs        : Adafruit Circuit Playground (Library Manager)

  FIDGETS
    • SPINNER (default, no boot buttons)
      - Physicsy spinner on the NeoPixel ring. Flick/tilt to add momentum.
      - A: direction lock (free/positive-only)
      - B: tail length cycle (1 → 2 → 3 → 5)
      - B any time → Alt-mode: COMET PAINTER (tilt to steer head; gentle trail)
    • ORBIT (boot A)
      - Speed follows tilt magnitude.
      - A: sensitivity (Low / Med / High)
      - B: dot size (1 or 3)
    • TWINKLE (boot B)
      - Sparkles fed by motion “energy”.
      - A: density (Low / Med / High)
      - B: hue family (Rainbow / Warm / Cool)

  GLOBAL
    • A+B hold ~1.5s → reset current fidget back to its idle state.

  Author: © 2025 Iain Bennett
*/

#include <Adafruit_CircuitPlayground.h>
#include <Arduino.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==================== Tuning ====================
static const uint8_t  BRIGHTNESS         = 60;     // overall NeoPixel brightness
static const uint16_t LAUNCHER_WINDOW_MS = 1400;   // boot selection window
static const uint16_t AB_RESET_MS        = 1500;   // hold A+B to reset current mode

// Spinner physics
static const float DT_S          = 0.025f;   // physics timestep ~25 ms
static const float DRAG          = 0.985f;   // per-step decay for omega
static const float KICK_GAIN     = 0.55f;    // how strongly measured angVel injects
static const float FLICK_THRESH  = 7.5f;     // rad/s to count as a flick impulse
static const float FLICK_BONUS   = 2.2f;     // extra omega added on flick
static const float OMEGA_MAX     = 22.0f;    // clamp for stability

// Orbit “calm” mode
static const float ORBIT_BASE    = 0.7f;     // base spin (rad/s)
static const float ORBIT_GAIN_LO = 0.9f;
static const float ORBIT_GAIN_MD = 1.4f;
static const float ORBIT_GAIN_HI = 2.0f;

// Twinkle
static const float ENERGY_DECAY  = 0.985f;   // per-step energy sink
static const float ENERGY_FROM_SHAKE = 0.06f; // per g-excess → energy
static const float DENS_LOW = 0.20f, DENS_MED = 0.35f, DENS_HI = 0.55f;

// ==================== Utilities ====================
static inline void pixelsOff() {
  for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,0,0);
  CircuitPlayground.strip.show();
}
static inline void solid(uint8_t r,uint8_t g,uint8_t b) {
  for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,r,g,b);
  CircuitPlayground.strip.show();
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

static inline void waitButtonsReleased(uint16_t ms=250){
  unsigned long end = millis() + ms;
  for(;;){
    if (CircuitPlayground.leftButton() || CircuitPlayground.rightButton()){
      end = millis() + ms;
    }
    if (millis() >= end) return;
    delay(10);
  }
}

// Hue wheel → RGB (0..255). pos: 0..255
static void wheel(uint8_t pos, uint8_t &r, uint8_t &g, uint8_t &b){
  if(pos < 85) { r = pos * 3; g = 255 - pos * 3; b = 0; }
  else if(pos < 170) { pos -= 85; r = 255 - pos * 3; g = 0; b = pos * 3; }
  else { pos -= 170; r = 0; g = pos * 3; b = 255 - pos * 3; }
}

// Quick breathe attract (teal)
static void breatheOnce(uint8_t gMax=80, uint8_t bMax=180, uint16_t stepDelay=18){
  for (uint8_t b=0;b<=24;b++){
    uint8_t g=(uint8_t)((b*gMax)/24), bl=(uint8_t)((b*bMax)/24);
    for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,g,bl);
    CircuitPlayground.strip.show();
    delay(stepDelay);
  }
  for (int8_t b=24;b>=0;b--){
    uint8_t g=(uint8_t)((b*gMax)/24), bl=(uint8_t)((b*bMax)/24);
    for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,g,bl);
    CircuitPlayground.strip.show();
    delay(stepDelay);
  }
}

// Read angle from accelerometer (project onto XY plane)
static inline float ringAngle(){
  float ax= CircuitPlayground.motionX();
  float ay= CircuitPlayground.motionY();
  return atan2f(ay, ax); // -PI..PI
}

// Simple g-excess magnitude
static inline float gex(){
  float ax= CircuitPlayground.motionX();
  float ay= CircuitPlayground.motionY();
  float az= CircuitPlayground.motionZ();
  return fabs(ax)+fabs(ay)+fabs(az) - 9.81f;
}

// ==================== Spinner (and alt: Comet Painter) ====================
static void drawComet(float theta, uint8_t len, uint8_t hueBase){
  // head index
  int head = (int)roundf((theta + (float)M_PI) * (10.0f / (2.0f*(float)M_PI))) % 10;
  for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,0,0);

  for (uint8_t k=0;k<len;k++){
    int idx = (head - (int)k + 100) % 10;
    uint8_t r,g,b;
    uint8_t hue = hueBase - k*12;
    wheel(hue, r,g,b);
    // dim tail
    r = (uint8_t)((uint16_t)r * (uint16_t)(len-k) / (uint16_t)len);
    g = (uint8_t)((uint16_t)g * (uint16_t)(len-k) / (uint16_t)len);
    b = (uint8_t)((uint16_t)b * (uint16_t)(len-k) / (uint16_t)len);
    CircuitPlayground.setPixelColor(idx, r,g,b);
  }
  CircuitPlayground.strip.show();
}

static void runCometPainterForever(){
  waitButtonsReleased(200);
  pixelsOff();
  // Entry cue
  for (uint8_t i=0;i<2;i++){ solid(0,0,40); delay(100); pixelsOff(); delay(100); }

  uint8_t tailLen = 3;
  uint8_t hueBase = 0;
  for(;;){
    if (abResetHeld()) { pixelsOff(); return; } // back to Spinner

    // Controls
    if (CircuitPlayground.leftButton()){ // A: cycle tail length
      tailLen = (tailLen==3)?5:3;
      CircuitPlayground.playTone(600,80);
      waitButtonsReleased(200);
    }
    if (CircuitPlayground.rightButton()){ // B: cycle hue anchors
      hueBase += 40;
      CircuitPlayground.playTone(760,80);
      waitButtonsReleased(200);
    }

    float th = ringAngle();
    drawComet(th, tailLen, hueBase);
    delay(20);
  }
}

static void runSpinnerForever(){
  waitButtonsReleased(200);
  pixelsOff();

  // Spinner state
  float theta = 0.0f;
  float omega = 0.0f;         // rad/s
  float lastAngle = ringAngle();
  unsigned long lastStep = millis();

  // Options
  bool   posOnly   = false;   // A toggles
  uint8_t tailLen  = 3;       // B cycles
  uint8_t hueBase  = 0;

  // idle teaser
  for (uint8_t k=0;k<2;k++) breatheOnce();

  for(;;){
    // Alt-mode: B press jumps to Comet Painter
    if (CircuitPlayground.rightButton()){
      runCometPainterForever();
      pixelsOff();
      waitButtonsReleased(200);
      // re-enter spinner fresh-ish
      lastAngle = ringAngle();
      continue;
    }

    // Options
    if (CircuitPlayground.leftButton()){ // A: direction lock toggle
      posOnly = !posOnly;
      CircuitPlayground.playTone(posOnly ? 700 : 500, 90);
      waitButtonsReleased(220);
    }

    if (abResetHeld()) {
      // Soft reset spinner parameters (don’t exit the fidget)
      theta = 0; omega = 0; tailLen = 3; posOnly=false;
      pixelsOff(); waitButtonsReleased(200);
    }

    // Physics step pacing
    unsigned long now = millis();
    if (now - lastStep < (unsigned long)(DT_S*1000.0f)) { delay(2); continue; }
    float dt = (now - lastStep) / 1000.0f;
    lastStep = now;

    // Measure apparent angular velocity from ringAngle
    float ang = ringAngle();
    float d = ang - lastAngle;
    // unwrap
    if (d >  (float)M_PI)  d -= 2.0f*(float)M_PI;
    if (d < -(float)M_PI)  d += 2.0f*(float)M_PI;
    float angVel = d / dt; // rad/s
    lastAngle = ang;

    // Inject torque from measured motion (flicks/tilt)
    omega += KICK_GAIN * angVel;
    if (fabs(angVel) > FLICK_THRESH) {
      omega += (angVel > 0 ? +FLICK_BONUS : -FLICK_BONUS);
      // tiny “tick” tone
      CircuitPlayground.playTone(420, 20);
    }

    // Direction lock (optional)
    if (posOnly && omega < 0) omega *= -0.6f; // bounce a little

    // Drag
    omega *= powf(DRAG, dt / DT_S);

    // Clamp
    if (omega >  OMEGA_MAX) omega =  OMEGA_MAX;
    if (omega < -OMEGA_MAX) omega = -OMEGA_MAX;

    // Integrate
    theta += omega * dt;
    while (theta >  (float)M_PI) theta -= 2.0f*(float)M_PI;
    while (theta < -(float)M_PI) theta += 2.0f*(float)M_PI;

    // Visualize (comet with speed-based hue)
    float speed = fabs(omega); // 0..~OMEGA_MAX
    uint8_t hue = (uint8_t)constrain((int)(speed * 10.0f), 0, 255);
    hue = (uint8_t)(hue + hueBase);
    drawComet(theta, tailLen, hue);

    // Tail length cycle with quick press of B (but we use B for alt-mode on press at frame start)
    if (!CircuitPlayground.rightButton() && CircuitPlayground.leftButton()==false){
      // nothing; reserved
    }

    // Short tap on B *after* returning from painter: cycle length (quality-of-life)
    if (CircuitPlayground.rightButton()){
      tailLen = (tailLen==1)?2 : (tailLen==2)?3 : (tailLen==3)?5 : 1;
      CircuitPlayground.playTone(760,70);
      waitButtonsReleased(220);
    }
  }
}

// ==================== ORBIT (calm tilt-speed) ====================
static void runOrbitForever(){
  waitButtonsReleased(200);
  pixelsOff();

  float theta = 0.0f;
  unsigned long lastStep = millis();

  // options
  uint8_t sensIdx = 1; // 0=Low,1=Med,2=High
  bool comet3 = true;

  for(;;){
    if (abResetHeld()) { pixelsOff(); waitButtonsReleased(200); } // soft reset idle

    // Controls
    if (CircuitPlayground.leftButton()){ // A: sensitivity cycle
      sensIdx = (uint8_t)((sensIdx+1)%3);
      CircuitPlayground.playTone(560 + sensIdx*120, 80);
      waitButtonsReleased(200);
    }
    if (CircuitPlayground.rightButton()){ // B: toggle dot size
      comet3 = !comet3;
      CircuitPlayground.playTone(comet3?760:520, 80);
      waitButtonsReleased(200);
    }

    // step
    unsigned long now = millis();
    if (now - lastStep < 18) { delay(2); continue; }
    float dt = (now - lastStep)/1000.0f;
    lastStep = now;

    float ax= CircuitPlayground.motionX();
    float ay= CircuitPlayground.motionY();
    float tiltMag = sqrtf(ax*ax + ay*ay); // ~0..9.8
    float gain = (sensIdx==0)?ORBIT_GAIN_LO : (sensIdx==1)?ORBIT_GAIN_MD : ORBIT_GAIN_HI;
    float omega = ORBIT_BASE + gain * (tiltMag * 0.12f); // rad/s

    theta += omega * dt;
    while (theta > (float)M_PI) theta -= 2.0f*(float)M_PI;

    // draw
    int head = (int)roundf((theta + (float)M_PI) * (10.0f / (2.0f*(float)M_PI))) % 10;
    for (uint8_t i=0;i<10;i++) CircuitPlayground.setPixelColor(i,0,0,0);

    uint8_t r,g,b; wheel((uint8_t)(tiltMag*10.0f), r,g,b);
    CircuitPlayground.setPixelColor(head, r,g,b);
    if (comet3){
      CircuitPlayground.setPixelColor((head+9)%10, r/3,g/3,b/3);
      CircuitPlayground.setPixelColor((head+8)%10, r/6,g/6,b/6);
    }
    CircuitPlayground.strip.show();
  }
}

// ==================== TWINKLE (motion-fed sparkles) ====================
static void setHueFamily(uint8_t fam, uint8_t base, uint8_t &r, uint8_t &g, uint8_t &b){
  // fam: 0=rainbow,1=warm,2=cool
  if (fam==0){ wheel(base, r,g,b); return; }
  if (fam==1){ // warm: reds/yellows
    uint8_t p = (uint8_t)(base % 85);
    r = 200 + (p % 55); g = 60 + (p%120); b = 0;
    return;
  }
  // cool: blues/teals
  uint8_t p = (uint8_t)(base % 170);
  r = 0; g = 40 + (p%120); b = 120 + (p%120);
}

static void runTwinkleForever(){
  waitButtonsReleased(200);
  pixelsOff();

  float energy = 0.0f;
  uint8_t fam = 0; // 0 rainbow, 1 warm, 2 cool
  uint8_t densSel = 1; // 0 low,1 med,2 high

  for(;;){
    if (abResetHeld()) { pixelsOff(); energy=0; waitButtonsReleased(200); }

    // Controls
    if (CircuitPlayground.leftButton()){ // A: density
      densSel = (uint8_t)((densSel+1)%3);
      CircuitPlayground.playTone(480 + densSel*120, 80);
      waitButtonsReleased(200);
    }
    if (CircuitPlayground.rightButton()){ // B: hue family
      fam = (uint8_t)((fam+1)%3);
      CircuitPlayground.playTone(720, 80);
      waitButtonsReleased(200);
    }

    // feed energy from motion
    float ge = gex();
    if (ge > 0.4f) energy += ge * ENERGY_FROM_SHAKE;
    energy *= ENERGY_DECAY;
    if (energy < 0) energy = 0; if (energy > 2.0f) energy = 2.0f;

    // density
    float dens = (densSel==0)?DENS_LOW : (densSel==1)?DENS_MED : DENS_HI;
    float pSpawn = dens * (0.25f + energy*0.75f); // 0..~1 per step across the ring

    // decay existing pixels
    for (uint8_t i=0;i<10;i++){
      uint32_t c = CircuitPlayground.strip.getPixelColor(i);
      uint8_t r = (c >> 16) & 0xFF;
      uint8_t g = (c >>  8) & 0xFF;
      uint8_t b =  c        & 0xFF;
      if (r|g|b){
        r = (uint8_t)((r*220)/255);
        g = (uint8_t)((g*220)/255);
        b = (uint8_t)((b*220)/255);
        CircuitPlayground.setPixelColor(i, r,g,b);
      }
    }

    // maybe spawn a few sparkles
    uint8_t tries = 3;
    while (tries--){
      if ((float)random(0,1000)/1000.0f < pSpawn){
        uint8_t idx = (uint8_t)random(0,10);
        uint8_t r,g,b;
        setHueFamily(fam, (uint8_t)random(0,255), r,g,b);
        CircuitPlayground.setPixelColor(idx, r,g,b);
      }
    }

    CircuitPlayground.strip.show();
    delay(24);
  }
}

// ==================== Boot Selection ====================
enum Fidget { F_SPINNER, F_ORBIT, F_TWINKLE };
static Fidget SELECTED = F_SPINNER;

static Fidget splashAndPick(uint16_t windowMs=LAUNCHER_WINDOW_MS) {
  pixelsOff();

  auto showHints = [](){
    // A (ORBIT) green hints
    CircuitPlayground.setPixelColor(0,0,35,0);
    CircuitPlayground.setPixelColor(3,0,35,0);
    // B (TWINKLE) blue hints
    CircuitPlayground.setPixelColor(2,0,0,35);
    CircuitPlayground.setPixelColor(7,0,0,35);
    // default (SPINNER) amber hints
    CircuitPlayground.setPixelColor(5,25,16,0);
    CircuitPlayground.setPixelColor(9,25,16,0);
    CircuitPlayground.strip.show();
  };
  showHints();

  unsigned long t0 = millis();
  uint8_t idx=0; unsigned long nextStep=0;
  const uint16_t STEP_MS=55;

  Fidget pick = F_SPINNER;

  while (millis() - t0 < windowMs){
    bool a = CircuitPlayground.leftButton();
    bool b = CircuitPlayground.rightButton();

    uint8_t r=0,g=0,bl=0;
    if (a && !b){ g=35; pick=F_ORBIT; }
    else if (b && !a){ bl=40; pick=F_TWINKLE; }
    else if (a && b){ r=25; bl=25; } // magenta holding both
    else { showHints(); delay(5); continue; }

    if (millis() >= nextStep){
      pixelsOff();
      CircuitPlayground.setPixelColor(idx%10, r,g,bl);
      CircuitPlayground.setPixelColor((idx+9)%10, r/6,g/6,bl/6);
      CircuitPlayground.strip.show();
      idx=(idx+1)%10; nextStep = millis()+STEP_MS;
    }
    delay(5);
  }

  pixelsOff();
  return pick;
}

// ==================== Setup / Loop ====================
void setup(){
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(BRIGHTNESS);
  pixelsOff();

  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && (millis()-t0)<600) { /* wait a bit */ }

  // tiny ready cue
  CircuitPlayground.setPixelColor(4,25,16,0);
  CircuitPlayground.setPixelColor(9,25,16,0);
  delay(220);
  pixelsOff();

  SELECTED = splashAndPick();

  // signature flash
  if (SELECTED==F_ORBIT) solid(0,18,0);
  else if (SELECTED==F_TWINKLE) solid(0,0,25);
  else solid(25,16,0);
  delay(200);
  pixelsOff();
}

void loop(){
  switch (SELECTED){
    case F_SPINNER:  runSpinnerForever();  break;
    case F_ORBIT:    runOrbitForever();    break;
    case F_TWINKLE:  runTwinkleForever();  break;
  }
}