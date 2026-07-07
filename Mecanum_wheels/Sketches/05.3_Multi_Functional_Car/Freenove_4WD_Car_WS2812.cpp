#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Freenove_4WD_Car_WS2812.h"

// ============================================================================
// THIS FILE: the WS2812 RGB light strip on the car's body. It supports
// several light "effects" (solid colour, chasing lights, blink, breathing
// fade, rainbow), each drawn a little bit at a time using millis() timing
// (the same non-blocking animation trick used in the Emotion file) so the
// lights can animate while the rest of the program keeps running.
//
// Colours here are stored as three numbers 0-255 for Red, Green and Blue -
// the same RGB system used in image editors, web colours (#RRGGBB), etc.
// Mixing all three at full brightness (255,255,255) gives white; all zero
// gives off/black.
// ============================================================================

int ws2812_task_mode = 0;         // Which light effect WS2812_Show() is currently drawing
int ws2812_strip_time_now;        // millis() reading of when this effect last updated
int ws2812_strip_time_next;       // millis() reading taken "now", each time we check the clock
unsigned char ws2812_strip_color_1[12][3];  // "Colour A" for each of up to 12 LEDs (used by most effects)
unsigned char ws2812_strip_color_2[12][3];  // "Colour B" for each LED (used by the blink effect to alternate with colour A)

byte m_color[5][3] = { { 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 }, { 255, 255, 255 }, { 0, 0, 0 } };  // A small palette: red, green, blue, white, off

Adafruit_NeoPixel ws2812_strip(LEDS_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800);  // The object that actually talks to the LED strip hardware

// WS2812_Set_Color_1 / WS2812_Set_Color_2: store a colour to use later for
// one LED (number = 1..12), or for all of them at once (number = 0, or for
// Color_1 any bit pattern that selects several LEDs). These don't light
// anything up immediately - they just remember the colour for the next time
// one of the ws2812_* effect functions below runs.
void WS2812_Set_Color_1(int number, unsigned char color_1, unsigned char color_2, unsigned char color_3)  //Set the display color1 for WS2812
{
  for (int i = 0; i < 12; i++) {
    if ((number >> i) & 0x01 == 0x01) {
      ws2812_strip_color_1[i][0] = constrain(color_1, 0, 255);
      ws2812_strip_color_1[i][1] = constrain(color_2, 0, 255);
      ws2812_strip_color_1[i][2] = constrain(color_3, 0, 255);
    }
  }
}

//Set the display color2 for WS2812
void WS2812_Set_Color_2(int number, unsigned char color_1, unsigned char color_2, unsigned char color_3)  //Set the display color1 for WS2812
{
  if (number == 0) {
    for (int i = 0; i < 12; i++) {
      ws2812_strip_color_2[i][0] = constrain(color_1, 0, 255);
      ws2812_strip_color_2[i][1] = constrain(color_2, 0, 255);
      ws2812_strip_color_2[i][2] = constrain(color_3, 0, 255);
    }
  } else if (number > 0 && number <= 12) {
    ws2812_strip_color_2[number - 1][0] = constrain(color_1, 0, 255);
    ws2812_strip_color_2[number - 1][1] = constrain(color_2, 0, 255);
    ws2812_strip_color_2[number - 1][2] = constrain(color_3, 0, 255);
  }
}

// ws2812_close: turns every LED off (sets all colours to black/0,0,0).
void ws2812_close(void) {
  for (int i = 0; i < 12; i++)
    ws2812_strip.setPixelColor(i, 0, 0, 0);
  ws2812_strip.show();
}

int ws2812_following_number = 0;
int following_brightness = 100;


// ws2812_following: a "chasing lights" effect - lights up LEDs one at a
// time from the start of the strip, growing longer each frame, then resets
// back to nothing and starts again. Uses the millis()-timing pattern
// (explained in the Emotion file) to advance one step every 100ms.
void ws2812_following(void)
{
  ws2812_strip_time_next = millis();
  if (ws2812_strip_time_next - ws2812_strip_time_now > 100)
  {
    ws2812_strip_time_now = ws2812_strip_time_next;
    for (int i = 0; i < ws2812_following_number; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0], ws2812_strip_color_1[i][1], ws2812_strip_color_1[i][2]);
    for (int i = ws2812_following_number; i < 8; i++)
      ws2812_strip.setPixelColor(i, 0, 0, 0);
    ws2812_strip.show();
    ws2812_following_number++;
    if (ws2812_following_number == 9)
      ws2812_following_number = 0;
  }
}

// ws2812_rgb: shows a plain, unchanging colour on every LED (whatever was
// last set with WS2812_Set_Color_1). "Static" just means it doesn't
// animate, though it still refreshes every 100ms in case the colour changed.
void ws2812_rgb(void) {
  ws2812_strip_time_next = millis();
  if (ws2812_strip_time_next - ws2812_strip_time_now > 100) {
    ws2812_strip_time_now = ws2812_strip_time_next;
    for (int i = 0; i < 12; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0], ws2812_strip_color_1[i][1], ws2812_strip_color_1[i][2]);
    ws2812_strip.show();
  }
}

int ws2812_blink_flag = 0;  // Toggles between 0 and 1 to alternate which colour is showing
// ws2812_blink: alternates every 500ms between "colour 1" and "colour 2"
// on all LEDs - a simple on/off (or colour-swap) flashing effect.
void ws2812_blink(void) {
  ws2812_strip_time_next = millis();
  if (ws2812_strip_time_next - ws2812_strip_time_now > 500 && ws2812_blink_flag == 0) {
    ws2812_blink_flag = 1;
    ws2812_strip_time_now = ws2812_strip_time_next;
    for (int i = 0; i < 12; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0], ws2812_strip_color_1[i][1], ws2812_strip_color_1[i][2]);
    ws2812_strip.show();
  } else if (ws2812_strip_time_next - ws2812_strip_time_now > 500 && ws2812_blink_flag == 1) {
    ws2812_blink_flag = 0;
    ws2812_strip_time_now = ws2812_strip_time_next;
    for (int i = 0; i < 12; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_2[i][0], ws2812_strip_color_2[i][1], ws2812_strip_color_2[i][2]);
    ws2812_strip.show();
  }
}

int ws2812_breathe_flag = 0;  // 0 = currently getting brighter, 1 = currently getting dimmer
int breathe_brightness = 0;   // Current brightness level, 0 (off) to 255 (full colour)
// ws2812_breathe: a "breathing" fade effect - gradually brightens colour 1
// from 0 up to 255, then gradually dims it back down to 0, repeating
// forever, like a slow heartbeat glow.
void ws2812_breathe(void) {
  ws2812_strip_time_next = millis();
  if ((ws2812_strip_time_next - ws2812_strip_time_now > 5) && (ws2812_breathe_flag == 0)) {
    ws2812_strip_time_now = ws2812_strip_time_next;
    breathe_brightness++;
    for (int i = 0; i < 12; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0] * breathe_brightness / 255, ws2812_strip_color_1[i][1] * breathe_brightness / 255, ws2812_strip_color_1[i][2] * breathe_brightness / 255);
    ws2812_strip.show();
    if (breathe_brightness >= 255)
      ws2812_breathe_flag = 1;
  }
  if ((ws2812_strip_time_next - ws2812_strip_time_now > 5) && (ws2812_breathe_flag == 1)) {
    ws2812_strip_time_now = ws2812_strip_time_next;
    breathe_brightness--;
    for (int i = 0; i < 12; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0] * breathe_brightness / 255, ws2812_strip_color_1[i][1] * breathe_brightness / 255, ws2812_strip_color_1[i][2] * breathe_brightness / 255);
    ws2812_strip.show();
    if (breathe_brightness <= 0)
      ws2812_breathe_flag = 0;
  }
}

int rainbow_count = 0;  // Shifts the rainbow pattern along the strip a little more each frame
// ws2812_rainbow: cycles all the LEDs through the colours of the rainbow,
// each LED slightly offset from its neighbour, and the whole pattern slowly
// sliding along the strip over time. Ignores the stored colour_1/colour_2
// and instead calculates colours live using Wheel() below.
void ws2812_rainbow(void) {
  ws2812_strip.setBrightness(255);
  ws2812_strip_time_next = millis();
  if (ws2812_strip_time_next - ws2812_strip_time_now > 5) {
    ws2812_strip_time_now = ws2812_strip_time_next;
    rainbow_count++;
    for (int i = 0; i < LEDS_COUNT; i++)
      ws2812_strip.setPixelColor(LEDS_COUNT - 1 - i, Wheel((i * 256 / LEDS_COUNT + rainbow_count) & 255));
    ws2812_strip.show();
    if (rainbow_count > 255)
      rainbow_count = 0;
  }
}

// Wheel: takes a position 0-255 and returns a packed 24-bit RGB colour that
// smoothly cycles through the rainbow as pos increases (0->255->wraps to 0
// again). This is the classic "colour wheel" helper used in lots of NeoPixel
// example code - it splits the 0-255 range into three bands (red->green,
// green->blue, blue->red) and fades between them.
int Wheel(byte pos) {
  int WheelPos = pos % 0xff;
  if (WheelPos < 85) {
    return ((255 - WheelPos * 3) << 16) | ((WheelPos * 3) << 8);
    Serial.println("ws28121" + String(WheelPos));
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return (((255 - WheelPos * 3) << 8) | (WheelPos * 3));
    Serial.println("ws28122" + String(WheelPos));
  }
  WheelPos -= 170;
  return ((WheelPos * 3) << 16 | (255 - WheelPos * 3));
  Serial.println("ws28123" + String(WheelPos));
}

// WS2812_Setup: starts the LED strip library, sets full brightness, turns
// all LEDs off, and picks a default starting colour. Call once from setup().
void WS2812_Setup(void) {
  ws2812_strip.begin();
  ws2812_strip.setBrightness(255);
  ws2812_close();
  WS2812_Set_Color_1(255, 0, 0, 100);
  WS2812_Set_Color_2(255, 0, 0, 0);
  ws2812_strip_time_now = millis();
}

// WS2812_SetMode: chooses which effect (0-5) WS2812_Show() will play. Values
// outside 0-5 get clamped into range by constrain(), so it can't crash from
// a bad input.
void WS2812_SetMode(int mode) {
  ws2812_task_mode = constrain(mode, 0, 5);
}

// WS2812_Show: draws one frame of whichever light effect `mode` selects.
// "Non-blocking" means it never uses delay() to wait - it's safe to call
// every single time through loop() and it will animate at the right speed
// on its own (thanks to the millis() timing inside each ws2812_* function).
void WS2812_Show(int mode) {
  switch (mode) {
    case 0:  //Close the WS2812
      ws2812_close();
      break;
    case 1:
      ws2812_rgb();
      break;
    case 2:
      ws2812_following();
      break;
    case 3:
      ws2812_blink();
      break;
    case 4:
      ws2812_breathe();
      break;
    case 5:
      ws2812_rainbow();
      break;
    default:
      break;
  }
}

// WS2812_Show1: a simpler alternative helper - immediately paints every LED
// with one of the 5 preset palette colours from m_color[] (index 0-4:
// red/green/blue/white/off). Not used by this sketch's main loop, but kept
// here in case you want a quick one-line way to set a solid colour.
void WS2812_Show1(int ws2812_task_mode1) {
  for (int i = 0; i < 12; i++)
    ws2812_strip.setPixelColor(i, m_color[ws2812_task_mode1][0], m_color[ws2812_task_mode1][1], m_color[ws2812_task_mode1][2]);
  ws2812_strip.show();
}