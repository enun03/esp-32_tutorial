// RGB Rainbow Pulse + Digital Inputs for the ELEGOO ESP32 kit
// Press the ON button  -> RGB LED fades in and runs the rainbow pulse.
// Press the OFF button -> RGB LED fades out smoothly.
//
// RGB LED wiring (lesson 2.2): RED -> D4, GREEN -> D5, BLUE -> D2 (each through 220 ohm)
// Button wiring: one leg of each button to its GPIO pin, the other leg to GND.
//                No resistor needed: INPUT_PULLUP uses the ESP32's internal pull-up.
// Button A (GPIO 19) = ON, Button B (GPIO 21) = OFF, same as the Digital Inputs lesson.

#include <math.h>

// ---- RGB LED pins ----
#define RED   4
#define GREEN 5
#define BLUE  2

// ---- Button pins (from the Digital Inputs lesson) ----
#define BUTTON_ON  19   // button A
#define BUTTON_OFF 21   // button B

// ---- Tweak these to change the feel ----
const float SECONDS_PER_COLOR = 0.2;   // time to glide from one palette color to the next
const float PULSE_SECONDS     = 2.0;   // time for one pulse (lower = faster)
const float PULSE_LOW         = 0.45;  // dimmest point of the pulse (never fully off)
const float PULSE_HIGH        = 1.0;   // brightest point (lower to ~0.6 if too bright)
const float FADE_SECONDS      = 0.4;   // how long the LED takes to fade in/out when switched
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

// ---- State ----
bool  lightOn       = false;   // what the buttons last asked for
float master        = 0.0;     // current overall brightness, 0.0 (off) .. 1.0 (on)
unsigned long lastMs = 0;      // used to time the fade

void setup()
{
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  // Pressed button connects the pin to GND, so it reads LOW when pressed
  pinMode(BUTTON_ON,  INPUT_PULLUP);
  pinMode(BUTTON_OFF, INPUT_PULLUP);

  lastMs = millis();
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
  // 1) Read the buttons (same idea as the Digital Inputs lesson)
  if (digitalRead(BUTTON_ON) == LOW)  lightOn = true;
  if (digitalRead(BUTTON_OFF) == LOW) lightOn = false;

  // 2) Fade the master brightness toward on (1.0) or off (0.0)
  unsigned long now = millis();
  float dt = (now - lastMs) / 1000.0;
  lastMs = now;

  float step = dt / FADE_SECONDS;
  if (lightOn) master = min(1.0f, master + step);
  else         master = max(0.0f, master - step);

  // Fully off: make sure all channels are dark and skip the color math
  if (master <= 0.0) {
    analogWrite(RED, 0);
    analogWrite(GREEN, 0);
    analogWrite(BLUE, 0);
    delay(2);
    return;
  }

  float t = now / 1000.0;   // seconds since power-on

  // 3) Color: blend between two neighboring palette colors
  float pos = fmod(t / SECONDS_PER_COLOR, (float)PALETTE_SIZE);
  int   a   = (int)pos;
  int   b   = (a + 1) % PALETTE_SIZE;
  float mix = pos - a;

  float r  = (PALETTE[a][0] + (PALETTE[b][0] - PALETTE[a][0]) * mix) / 255.0;
  float g  = (PALETTE[a][1] + (PALETTE[b][1] - PALETTE[a][1]) * mix) / 255.0;
  float bl = (PALETTE[a][2] + (PALETTE[b][2] - PALETTE[a][2]) * mix) / 255.0;

  // 4) Pulse: smooth sine wave that never goes dark
  float wave  = (sin(t * 2.0 * PI / PULSE_SECONDS) + 1.0) / 2.0;
  float pulse = PULSE_LOW + (PULSE_HIGH - PULSE_LOW) * wave;

  // 5) Combine color, pulse, and on/off fade, then send to the LED
  float level = pulse * master;
  analogWrite(RED,   toPwm(r  * level));
  analogWrite(GREEN, toPwm(g  * level));
  analogWrite(BLUE,  toPwm(bl * level));

  delay(2);
}
