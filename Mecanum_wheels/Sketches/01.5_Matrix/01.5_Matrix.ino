/**********************************************************************
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Description : Test the expression module
  Auther      : www.freenove.com
  Modification: 2023/04/10
**********************************************************************/
// This library gives us the "Freenove_VK16K33" class, which knows how to
// talk to the LED matrix driver chip (the VK16K33) over the car's I2C bus.
#include "Freenove_VK16K33_Lib.h"

// "matrix" is our one object for controlling BOTH 8x8 LED matrices (the
// car's two "eyes"). We'll call functions on it, like matrix.init(...).
Freenove_VK16K33 matrix = Freenove_VK16K33();

// ---------------------------------------------------------------------
// HOW A PICTURE GETS STORED AS NUMBERS
// ---------------------------------------------------------------------
// Each LED matrix is a grid of 8x8 tiny lights (64 LEDs total). To draw a
// picture, we need to tell the chip exactly which of those 64 lights to
// turn on. Instead of writing 64 separate true/false values, we use a
// clever shortcut: ONE ROW OF 8 LEDS = ONE BYTE (8 bits).
//
// A "byte" is just a number made of 8 binary digits (bits), each one a 0
// or a 1. If we say "1 = LED on" and "0 = LED off", then a single byte can
// describe a whole row of 8 dots at once!
//
// Example: the hex number 0x3C means:
//   0x3C  ->  binary 0 0 1 1 1 1 0 0
//              LED#:  1 2 3 4 5 6 7 8
// So 0x3C lights up the 4 middle LEDs (numbers 3,4,5,6) and leaves the
// 2 LEDs on each end turned off. Reading the picture is just reading
// which bits are 1!
//
// "0x" just means "the number after this is written in hexadecimal
// (base 16) instead of normal base 10" - it's a compact way to write
// binary, because each hex digit = exactly 4 bits.
//
// Each array below holds 8 bytes = 8 rows = one complete 8x8 picture.
// Stacking several of these 8-byte pictures one after another lets us
// store several "video frames" that we can flip through to make an
// animation (like a flipbook)!
// ---------------------------------------------------------------------

// x_array[][8]: a list of pictures (frames) for the LEFT eye matrix.
// Each row of 8 bytes below is one frame; each byte is one row of dots.
byte x_array[][8] = {//Put the data into the left LED matrix
  //////////////////////////////////////////////
  0x00, 0x18, 0x24, 0x42, 0x42, 0x24, 0x18, 0x00,
  0x00, 0x18, 0x24, 0x5A, 0x5A, 0x24, 0x18, 0x00,
  //////////////////////////////////////////////
};

// y_array[][8]: same idea as x_array, but these frames are shown on the
// RIGHT eye matrix at the same time, so both eyes animate together.
byte y_array[][8] = {//Put the data into the right LED matrix
  //////////////////////////////////////////////
  0x00, 0x18, 0x24, 0x42, 0x42, 0x24, 0x18, 0x00,
  0x00, 0x18, 0x24, 0x5A, 0x5A, 0x24, 0x18, 0x00,
  /////////////////////////////////////////////////
};

// setup() runs once, automatically, when the board powers on or resets.
// Here we wake up the LED matrix driver chip and turn off its built-in
// blinking effect so we can control the blinking ourselves in loop().
void setup()
{
  // Start (initialise) the matrix driver chip. 0x71 is its I2C address -
  // like a phone number the Pico uses to talk to this specific chip.
  matrix.init(0x71);
  // Turn off the chip's automatic blink feature (we want steady eyes,
  // and we control any "blinking" ourselves by changing frames).
  matrix.setBlink(VK16K33_BLINK_OFF);
}

// loop() runs over and over, forever, after setup() finishes.
// Each pass, we play through all the eye frames with 500ms between them.
void loop()
{
  showArray(500);
}

// showArray(): displays every frame stored in x_array/y_array, one after
// another, pausing "delay_ms" milliseconds between each one. This is what
// makes the still pictures look like a simple blinking animation.
// Parameter: delay_ms - how many milliseconds to hold each frame on screen.
void showArray(int delay_ms)
{
  // Work out how many frames (pictures) we have, by dividing the total
  // array size by the size of just one frame (8 bytes).
  int count = sizeof(x_array) / sizeof(x_array[0]);
  for (int i = 0; i < count; i++)
  {
    // Send frame "i" to both matrices at once (left eye + right eye).
    matrix.showStaticArray(x_array[i], y_array[i]);
    // Wait here so the frame stays visible before we move to the next one.
    delay(delay_ms);
  }
}
