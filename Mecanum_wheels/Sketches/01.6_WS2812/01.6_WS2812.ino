/**********************************************************************
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Description : Basic usage of LEDPixel, 
                Make the strip light up in different colors gradually.
  Auther      : www.freenove.com
  Modification: 2023/04/10
**********************************************************************/
#include <Arduino.h>             // Core Arduino functions (pinMode, millis, etc.)
#include <Adafruit_NeoPixel.h>   // Library that knows how to talk to WS2812 RGB LEDs
#include "Freenove_4WD_Car_WS2812.h" // Our own helper functions for the LED strip, declared in this header

// setup() runs ONCE when the board powers on or resets.
void setup() {
 WS2812_Setup();      // Get the LED strip ready: connect to it and pick starting colors
}

// loop() runs OVER AND OVER FOREVER after setup() finishes.
void loop() {
  WS2812_Show(5);   // Mode 5 = "rainbow" mode. Try changing this number (0-5) to see other light patterns!
}
