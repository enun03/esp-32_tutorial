// RGB Rainbow Pulse v2 for the ELEGOO ESP32 kit (common-cathode RGB LED)
// Glides through a 16-color palette while pulsing quickly and smoothly.
// The pulse never fades to dark, it just "breathes" between bright and brighter.
// Wiring (lesson 2.2): RED -> D4, GREEN -> D5, BLUE -> D2 (each through 220 ohm).

#include <math.h>

// ---- Pins ----
#define RED   4
#define GREEN 5
#define BLUE  2

// ---- Tweak these to change the feel ----
const float SECONDS_PER_COLOR = 0.2;   // time to glide from one palette color to the next
const float PULSE_SECONDS     = 2.0;   // time for one pulse (lower = faster)
const float PULSE_LOW         = 0.45;  // dimmest point of the pulse (never fully off)
const float PULSE_HIGH        = 1.0;   // brightest point (lower to ~0.6 if too bright)
const float GAMMA             = 2.2;   // makes fades look even to the human eye

// ---- Color palette (R, G, B as 0-255). Add, remove, or reorder freely. ----
const uint8_t PALETTE[][3] = {
  {255,   0,   0},  // red
  {255,  60,   0},  // red-orange
  {255, 120,   0},  // orange
  {255, 190,   0},  // amber
  {255, 255,   0},  // yellow
  {150, 255,   0},  // lime
  {  0, 255,   0},  // green
  {  0, 255, 120},  // mint
  {  0, 255, 255},  // cyan
  {  0, 140, 255},  // sky blue
  {  0,   0, 255},  // blue
  { 80,   0, 255},  // indigo
  {150,   0, 255},  // violet
  {255,   0, 255},  // magenta
  {255,   0, 140},  // hot pink
  {255,  40,  70},  // coral
};
const int PALETTE_SIZE = sizeof(PALETTE) / sizeof(PALETTE[0]);

void setup()
{
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
}

// Brightness (0.0-1.0) with gamma correction -> PWM value (0-255)
int toPwm(float value)
{
  if (value < 0) value = 0;
  if (value > 1) value = 1;
  return (int)(pow(value, GAMMA) * 255.0 + 0.5);
}

void loop()
{
  float t = millis() / 1000.0;   // seconds since power-on

  // 1) Color: find where we are between two palette colors and blend them.
  //    Wraps around, so the last color flows back into the first.
  float pos = fmod(t / SECONDS_PER_COLOR, (float)PALETTE_SIZE);
  int   a   = (int)pos;                    // current color
  int   b   = (a + 1) % PALETTE_SIZE;      // next color
  float mix = pos - a;                     // 0.0 = all current, 1.0 = all next

  float r = (PALETTE[a][0] + (PALETTE[b][0] - PALETTE[a][0]) * mix) / 255.0;
  float g = (PALETTE[a][1] + (PALETTE[b][1] - PALETTE[a][1]) * mix) / 255.0;
  float bl = (PALETTE[a][2] + (PALETTE[b][2] - PALETTE[a][2]) * mix) / 255.0;

  // 2) Pulse: a plain sine wave is perfectly smooth and symmetric,
  //    squeezed into the PULSE_LOW..PULSE_HIGH range so it never goes dark.
  float wave  = (sin(t * 2.0 * PI / PULSE_SECONDS) + 1.0) / 2.0;   // 0.0 .. 1.0
  float pulse = PULSE_LOW + (PULSE_HIGH - PULSE_LOW) * wave;

  // 3) Combine and send to the LED
  analogWrite(RED,   toPwm(r  * pulse));
  analogWrite(GREEN, toPwm(g  * pulse));
  analogWrite(BLUE,  toPwm(bl * pulse));

  delay(2);   // ~500 updates per second for very fluid motion
}
