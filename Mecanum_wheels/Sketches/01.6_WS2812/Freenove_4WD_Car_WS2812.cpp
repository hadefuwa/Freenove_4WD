#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Freenove_4WD_Car_WS2812.h"

int ws2812_task_mode = 0;              // Which lighting mode is active right now (0=off ... 5=rainbow)
int ws2812_strip_time_now;             // Timestamp (ms) of the last time we updated the LEDs
int ws2812_strip_time_next;            // Timestamp (ms) right now, used to measure elapsed time
// Each row here is one LED's color, stored as [Red, Green, Blue], each 0-255.
// Two full sets are kept so patterns like "blink" can swap between two colors.
unsigned char ws2812_strip_color_1[12][3];
unsigned char ws2812_strip_color_2[12][3];

// The Adafruit_NeoPixel object is our "remote control" for the physical LED strip:
// it knows how many LEDs there are, which pin they're wired to, and what electrical
// protocol (GRB color order, 800kHz timing) this specific type of LED needs.
Adafruit_NeoPixel ws2812_strip(LEDS_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800);

// Stores an RGB color (color_1=Red, color_2=Green, color_3=Blue, each 0-255) into
// "color slot 1" for one or more LEDs, chosen using 'number' as a bitmask: bit i
// (i.e. 1<<i) set to 1 means "include LED i". For example, passing 4095 (all bits
// set) applies the color to every LED. Note: this only stores the color in memory —
// it doesn't light up the LEDs. That happens later in ws2812_strip.show().
void WS2812_Set_Color_1(int number, unsigned char color_1, unsigned char color_2, unsigned char color_3)
{
  for (int i = 0; i < LEDS_COUNT; i++)
  {
    if ((number >> i) & 0x01 == 0x01)   // Shift 'number' right by i bits and check if bit 0 is set (is LED i selected?)
    {
      ws2812_strip_color_1[i][0] = constrain(color_1, 0, 255);  // Red   (constrain just clamps the value into 0-255, in case it wasn't already)
      ws2812_strip_color_1[i][1] = constrain(color_2, 0, 255);  // Green
      ws2812_strip_color_1[i][2] = constrain(color_3, 0, 255);  // Blue
    }
  }
}

// Same idea as WS2812_Set_Color_1, but fills "color slot 2" instead, and uses a
// different (simpler) way to pick which LED(s): number==0 means "all LEDs", while
// number from 1..LEDS_COUNT means "just that one LED" (LED numbers start at 1 here,
// not 0, so we subtract 1 when indexing the array).
void WS2812_Set_Color_2(int number, unsigned char color_1, unsigned char color_2, unsigned char color_3)
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

// Turns every LED on the strip off, by setting each one's color to (0,0,0) - "no
// red, no green, no blue" means no light at all. ws2812_strip.show() is what
// actually sends the new colors down the wire to the physical LEDs.
void ws2812_close(void)
{
  for (int i = 0; i < LEDS_COUNT; i++)
    ws2812_strip.setPixelColor(i, 0, 0, 0);
  ws2812_strip.show();
}

int ws2812_following_number = 0;   // How many LEDs (counting from the start) are currently lit in the chase pattern
// "Chasing" pattern: lights up LEDs one by one from the start of the strip, then
// resets back to zero and starts again, over and over - like a progress bar that
// keeps filling up and restarting.
void ws2812_following(void)
{
  ws2812_strip_time_next = millis();   // millis() = milliseconds since the board started running
  if (ws2812_strip_time_next - ws2812_strip_time_now > 100)   // Only update every 100ms, so it animates at a steady speed instead of instantly
  {
    ws2812_strip_time_now = ws2812_strip_time_next;
    for (int i = 0; i < ws2812_following_number; i++)   // Light up the LEDs we've reached so far, using color slot 1
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0], ws2812_strip_color_1[i][1], ws2812_strip_color_1[i][2]);
    for (int i = ws2812_following_number; i < LEDS_COUNT; i++)   // Keep the rest of the LEDs off
      ws2812_strip.setPixelColor(i, 0, 0, 0);
    ws2812_strip.show();       // Actually send these colors out to the physical strip
    ws2812_following_number++; // Next time, light up one more LED than before
    if (ws2812_following_number == LEDS_COUNT+1)
      ws2812_following_number = 0;   // Reached the end - start the chase over from the beginning
  }
}

// Plain "static color" display: shows whatever color is stored in color slot 1 on
// every LED, without changing anything. It re-applies the color every 100ms, which
// looks the same as doing nothing but keeps the strip refreshed.
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

int ws2812_blink_flag = 0;   // Tracks which of the two colors is currently showing (0 = color slot 1, 1 = color slot 2)
// "Blink" pattern: alternates the whole strip between color slot 1 and color slot
// 2 every 500 milliseconds, like a warning light flashing between two colors.
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

int ws2812_breathe_flag = 0;    // 0 = currently getting brighter, 1 = currently getting dimmer
int breathe_brightness = 0;     // Current brightness scale, counts 0 up to 255 then back down to 0
// "Breathing" pattern: smoothly fades color slot 1's color from off to full
// brightness and back again, like a sleeping laptop's pulsing light.
void ws2812_breathe(void)
{
  ws2812_strip_time_next = millis();
  if ((ws2812_strip_time_next - ws2812_strip_time_now > 5) && (ws2812_breathe_flag == 0))
  {
    ws2812_strip_time_now = ws2812_strip_time_next;
    breathe_brightness++;
    // Scale each color channel down by brightness/255 (a fraction from 0.0 to 1.0),
    // so the color dims evenly while keeping the same red/green/blue mix.
    for (int i = 0; i < LEDS_COUNT; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0]*breathe_brightness / 255, ws2812_strip_color_1[i][1]*breathe_brightness / 255, ws2812_strip_color_1[i][2]*breathe_brightness / 255);
    ws2812_strip.show();
    if (breathe_brightness >= 255)
      ws2812_breathe_flag = 1;   // Reached full brightness - switch to fading out
  }
  if ((ws2812_strip_time_next - ws2812_strip_time_now > 5) && (ws2812_breathe_flag == 1))
  {
    ws2812_strip_time_now = ws2812_strip_time_next;
    breathe_brightness--;
    for (int i = 0; i < LEDS_COUNT; i++)
      ws2812_strip.setPixelColor(i, ws2812_strip_color_1[i][0]*breathe_brightness / 255, ws2812_strip_color_1[i][1]*breathe_brightness / 255, ws2812_strip_color_1[i][2]*breathe_brightness / 255);
    ws2812_strip.show();
    if (breathe_brightness <= 0)
      ws2812_breathe_flag = 0;   // Reached fully off - switch back to fading in
  }
}

int rainbow_count = 0;   // Shifts the rainbow pattern along the strip a little more each update, making it scroll
// "Rainbow" pattern: makes the whole strip display a smoothly scrolling rainbow of
// colors, using the Wheel() helper below to convert a position into an RGB color.
void ws2812_rainbow(void)
{
  ws2812_strip.setBrightness(20);   // Keep brightness modest (out of 255) so the LEDs aren't blindingly bright / power-hungry
  ws2812_strip_time_next = millis();
  if (ws2812_strip_time_next - ws2812_strip_time_now > 5)
  {
    ws2812_strip_time_now = ws2812_strip_time_next;
    rainbow_count++;   // Advances the rainbow's "starting point" each update, which is what makes it scroll over time
    for (int i = 0; i < LEDS_COUNT; i++)
      // Spread the 256 possible wheel colors evenly across the LEDs, then add
      // rainbow_count so the whole pattern slides along as time passes.
      // "& 255" keeps the result wrapped into the 0-255 range Wheel() expects.
      ws2812_strip.setPixelColor(LEDS_COUNT-1 - i, Wheel((i * 256 / LEDS_COUNT + rainbow_count) & 255));
    ws2812_strip.show();
    if (rainbow_count > 255)
      rainbow_count = 0;   // Wrap back around once we've gone all the way through the color wheel
  }
}

// Converts a position on an imaginary color wheel (0-255) into a packed RGB color.
// As pos increases from 0 to 255, the returned color smoothly cycles
// red -> green -> blue -> back to red, giving a full rainbow. The color is packed
// into a single int as 0x00RRGGBB (red in bits 16-23, green in bits 8-15, blue in
// bits 0-7) via bit shifting (<<), which is how Adafruit_NeoPixel likes colors passed
// as one number instead of three separate values.
int Wheel(byte pos)
{
  int WheelPos = pos % 0xff;   // 0xff is 255 in hexadecimal; this just keeps pos within 0-255
  if (WheelPos < 85) {
    // First third of the wheel: fade from red down while green rises up (red -> yellow -> green)
    return ((255 - WheelPos * 3) << 16) | ((WheelPos * 3) << 8);
  }
  if (WheelPos < 170) {
    // Second third: fade from green down while blue rises up (green -> cyan -> blue)
    WheelPos -= 85;
    return (((255 - WheelPos * 3) << 8) | (WheelPos * 3));
  }
  // Final third: fade from blue down while red rises up (blue -> magenta -> red)
  WheelPos -= 170;
  return ((WheelPos * 3) << 16 | (255 - WheelPos * 3));
}

// Prepares the LED strip for use: connects to it, sets a default (dim) brightness,
// turns all LEDs off, and picks starting colors for color slot 1 (dim blue) and
// color slot 2 (off). Call this once from setup().
void WS2812_Setup(void)
{
  ws2812_strip.begin();          // Initialize communication with the physical LED strip
  ws2812_strip.setBrightness(20); // Overall brightness limiter (0-255) applied on top of each LED's own color
  ws2812_close();
  WS2812_Set_Color_1(4095, 0, 0, 100);   // 4095 in binary selects all LEDs (bits 0-11); color = R:0, G:0, B:100 -> dim blue
  WS2812_Set_Color_2(4095, 0, 0, 0);     // All LEDs, color = off (black)
  ws2812_strip_time_now = millis();

}

// Changes which lighting mode is remembered in ws2812_task_mode (clamped to the
// valid range 0-5). Note: WS2812_Show() below is what actually needs to be called
// with a mode number to run one of these patterns.
void WS2812_SetMode(int mode)
{
  ws2812_task_mode = constrain(mode, 0, 5);
}

// The "traffic director" for the LED strip: call this repeatedly from loop() with
// a mode number and it runs the matching pattern function below. It's called
// "non-blocking" because each pattern function returns quickly (it only updates
// the LEDs occasionally, based on elapsed time) instead of using delay() and
// freezing the rest of the program.
// Modes: 0=off, 1=static color, 2=chasing lights, 3=blink, 4=breathe, 5=rainbow
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
