#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Freenove_4WD_Car_WS2812.h"

int ws2812_task_mode = 0;         //Which light pattern is currently playing (0-5, see WS2812_Show)
int ws2812_strip_time_now;        //Used together with ws2812_strip_time_next for non-blocking animation timing
int ws2812_strip_time_next;
unsigned char ws2812_strip_color_1[12][3];  //Primary color for each LED, stored as [R,G,B] per LED
unsigned char ws2812_strip_color_2[12][3];  //Secondary color for each LED (used by the blink pattern)

Adafruit_NeoPixel ws2812_strip(LEDS_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800);

// WS2812_Set_Color_1(): sets the "primary" colour used by most light
// patterns. "number" is a bitmask — each bit picks one LED to change (bit0
// = LED 0, bit1 = LED 1, etc.), so you can set several LEDs to the same
// colour in one call, e.g. number=4095 (all bits set) means "all LEDs".
void WS2812_Set_Color_1(int number, unsigned char color_1, unsigned char color_2, unsigned char color_3)  //Set the display color1 for WS2812
{
  for (int i = 0; i < LEDS_COUNT; i++)
  {
    if ((number >> i) & 0x01 == 0x01)
    {
      ws2812_strip_color_1[i][0] = constrain(color_1, 0, 255);
      ws2812_strip_color_1[i][1] = constrain(color_2, 0, 255);
      ws2812_strip_color_1[i][2] = constrain(color_3, 0, 255);
    }
  }
}

// WS2812_Set_Color_2(): sets the "secondary" colour, used by the blink
// pattern to alternate between two colours. Unlike Set_Color_1, "number"
// here is either 0 (meaning "all LEDs") or a specific LED index (1-8).
void WS2812_Set_Color_2(int number, unsigned char color_1, unsigned char color_2, unsigned char color_3)  //Set the display color1 for WS2812
{
  if (number == 0)
  {
    for (int i = 0; i < LEDS_COUNT; i++)
    {
      ws2812_strip_color_2[i][0] = constrain(color_1, 0, 255);
      ws2812_strip_color_2[i][1] = constrain(color_2, 0, 255);
      ws2812_strip_color_2[i][2] = constrain(color_3, 0, 255);
    }
  }
  else if (number > 0 && number <= LEDS_COUNT)
  {
    ws2812_strip_color_2[number - 1][0] = constrain(color_1, 0, 255);
    ws2812_strip_color_2[number - 1][1] = constrain(color_2, 0, 255);
    ws2812_strip_color_2[number - 1][2] = constrain(color_3, 0, 255);
  }
}

// ws2812_close(): turns every LED off (sets them all to black/0,0,0).
void ws2812_close(void)
{
  for (int i = 0; i < LEDS_COUNT; i++)
    ws2812_strip.setPixelColor(i, 0, 0, 0);
  ws2812_strip.show();
}

int ws2812_following_number = 0;
// ws2812_following(): the "chasing lights" pattern. Every 100ms it lights
// up one more LED (from 0 up to ws2812_following_number) in color_1 and
// turns the rest off, so the lit section appears to grow and then reset,
// like a Cylon/KITT scanner effect. Uses the same millis()-based
// non-blocking timing trick as the emotion animations.
void ws2812_following(void)
{
  ws2812_strip_time_next = millis();
  if (ws2812_strip_time_next - ws2812_strip_time_now > 100)
  {
    ws2812_strip_time_now = ws2812_strip_time_next;
    for (int i = 0; i < ws2812_following_number; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0], ws2812_strip_color_1[i][1], ws2812_strip_color_1[i][2]);
    for (int i = ws2812_following_number; i < LEDS_COUNT; i++)
      ws2812_strip.setPixelColor(i, 0, 0, 0);
    ws2812_strip.show();
    ws2812_following_number++;
    if (ws2812_following_number == LEDS_COUNT+1)
      ws2812_following_number = 0;
  }
}

// ws2812_rgb(): the simplest pattern — just keeps every LED lit at its
// set color_1, refreshing every 100ms.
void ws2812_rgb(void)
{
  ws2812_strip_time_next = millis();
  if (ws2812_strip_time_next - ws2812_strip_time_now > 100)
  {
    ws2812_strip_time_now = ws2812_strip_time_next;
    for (int i = 0; i < LEDS_COUNT; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0], ws2812_strip_color_1[i][1], ws2812_strip_color_1[i][2]);
    ws2812_strip.show();
  }
}

int ws2812_blink_flag = 0;
// ws2812_blink(): alternates all LEDs between color_1 and color_2 every
// 500ms, using ws2812_blink_flag to remember which color is currently
// showing.
void ws2812_blink(void)
{
  ws2812_strip_time_next = millis();
  if (ws2812_strip_time_next - ws2812_strip_time_now > 500 && ws2812_blink_flag == 0)
  {
    ws2812_blink_flag = 1;
    ws2812_strip_time_now = ws2812_strip_time_next;
    for (int i = 0; i < LEDS_COUNT; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0], ws2812_strip_color_1[i][1], ws2812_strip_color_1[i][2]);
    ws2812_strip.show();
  }
  else if (ws2812_strip_time_next - ws2812_strip_time_now > 500 && ws2812_blink_flag == 1)
  {
    ws2812_blink_flag = 0;
    ws2812_strip_time_now = ws2812_strip_time_next;
    for (int i = 0; i < LEDS_COUNT; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_2[i][0], ws2812_strip_color_2[i][1], ws2812_strip_color_2[i][2]);
    ws2812_strip.show();
  }
}

int ws2812_breathe_flag = 0;
int breathe_brightness = 0;
// ws2812_breathe(): the "breathing" fade pattern. Slowly ramps
// breathe_brightness up from 0 to 255 (scaling color_1 down as it goes,
// via `* breathe_brightness / 255`), then back down again once it hits the
// top, giving a smooth fade in/out like slow breathing.
void ws2812_breathe(void)
{
  ws2812_strip_time_next = millis();
  if ((ws2812_strip_time_next - ws2812_strip_time_now > 5) && (ws2812_breathe_flag == 0))
  {
    ws2812_strip_time_now = ws2812_strip_time_next;
    breathe_brightness++;
    for (int i = 0; i < LEDS_COUNT; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0]*breathe_brightness / 255, ws2812_strip_color_1[i][1]*breathe_brightness / 255, ws2812_strip_color_1[i][2]*breathe_brightness / 255);
    ws2812_strip.show();
    if (breathe_brightness >= 255)
      ws2812_breathe_flag = 1;
  }
  if ((ws2812_strip_time_next - ws2812_strip_time_now > 5) && (ws2812_breathe_flag == 1))
  {
    ws2812_strip_time_now = ws2812_strip_time_next;
    breathe_brightness--;
    for (int i = 0; i < LEDS_COUNT; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0]*breathe_brightness / 255, ws2812_strip_color_1[i][1]*breathe_brightness / 255, ws2812_strip_color_1[i][2]*breathe_brightness / 255);
    ws2812_strip.show();
    if (breathe_brightness <= 0)
      ws2812_breathe_flag = 0;
  }
}

int rainbow_count = 0;
// ws2812_rainbow(): cycles all the LEDs smoothly through the rainbow, each
// LED offset slightly from its neighbour so a "rainbow band" appears to
// scroll along the strip. Ignores the color_1/color_2 settings entirely —
// the actual colors come from Wheel() below.
void ws2812_rainbow(void)
{
  ws2812_strip.setBrightness(20);
  ws2812_strip_time_next = millis();
  if (ws2812_strip_time_next - ws2812_strip_time_now > 5)
  {
    ws2812_strip_time_now = ws2812_strip_time_next;
    rainbow_count++;
    for (int i = 0; i < LEDS_COUNT; i++)
      ws2812_strip.setPixelColor(LEDS_COUNT-1 - i, Wheel((i * 256 / LEDS_COUNT + rainbow_count) & 255));
    ws2812_strip.show();
    if (rainbow_count > 255)
      rainbow_count = 0;
  }
}

// Wheel(): a standard NeoPixel helper that turns a single number (0-255)
// into a color, sweeping smoothly red -> green -> blue -> back to red as
// pos increases — think of it like walking around a color wheel. Used only
// by the rainbow pattern above.
int Wheel(byte pos)
{
  int WheelPos = pos % 0xff;
  if (WheelPos < 85) {
    return ((255 - WheelPos * 3) << 16) | ((WheelPos * 3) << 8);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return (((255 - WheelPos * 3) << 8) | (WheelPos * 3));
  }
  WheelPos -= 170;
  return ((WheelPos * 3) << 16 | (255 - WheelPos * 3));
}

// WS2812_Setup(): starts the LED strip library, sets an overall brightness
// cap (20/255, quite dim so the strip isn't blindingly bright close up),
// turns everything off, and picks a default starting color. Call once from
// setup().
void WS2812_Setup(void)
{
  ws2812_strip.begin();
  ws2812_strip.setBrightness(20);
  ws2812_close();
  WS2812_Set_Color_1(4095, 0, 0, 100);
  WS2812_Set_Color_2(4095, 0, 0, 0);
  ws2812_strip_time_now = millis();

}

// WS2812_SetMode(): called when the app sends a CMD_LED_MOD command to
// change which light pattern is active. Clamped to 0-5 (see the mode
// numbers in WS2812_Show below) so an invalid number can't be selected.
void WS2812_SetMode(int mode)
{
  ws2812_task_mode = constrain(mode, 0, 5);
}

// WS2812_Show(): called every loop() iteration with the currently active
// mode number, and runs that pattern's function to (maybe) update the
// LEDs this frame. Each pattern function only actually redraws the strip
// when enough time has passed, so calling this constantly is fine and is
// how all the light patterns keep animating over time.
//   0 = off, 1 = solid color, 2 = chasing lights, 3 = blink between two
//   colors, 4 = breathing fade, 5 = rainbow cycle
void WS2812_Show(int mode)
{
  switch (mode)
  {
    case 0://Close the WS2812
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
