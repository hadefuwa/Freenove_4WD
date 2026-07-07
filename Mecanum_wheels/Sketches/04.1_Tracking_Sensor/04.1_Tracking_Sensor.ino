/**********************************************************************
  Filename    : Tracking_Sensor.ino
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"
// This file only defines setup() and loop(). All the real sensor-reading work
// happens inside Track_Setup()/Track_Read(), which live in
// Freenove_4WD_Car_For_Pico_W.cpp (declared in the matching .h file above).

// This sketch is the simplest possible demo of the 3 line-tracking sensors
// on the bottom of the car. Each sensor is a tiny infrared "eye" that shines
// invisible light at the floor and checks how much bounces back:
//   - shiny/white floor reflects a lot of light  -> sensor reads 0
//   - black tape/line absorbs the light          -> sensor reads 1
// Later sketches use these same 0/1 readings to make the car steer itself
// along a black line. Here we just print the numbers so you can see them.

// setup() runs once when the Pico W powers on or is reset.
void setup() {
  Serial.begin(115200); //set baud rate
  Track_Setup(); // configure the 3 sensor pins as digital inputs
}

// loop() runs over and over, forever, after setup() finishes.
void loop() {
  Track_Read(); // ask the 3 sensors for fresh readings and fill sensorValue[]
  Serial.print("Sensor Value (L / M / R / ALL) : ");
  // sensorValue[0] = Left sensor   (0 or 1)
  // sensorValue[1] = Middle sensor (0 or 1)
  // sensorValue[2] = Right sensor  (0 or 1)
  // sensorValue[3] = all three combined into ONE number from 0-7.
  // Track_Read() builds that combined number using bit shifting, like this:
  //   sensorValue[3] = (Left << 2) | (Middle << 1) | Right
  // Think of it as writing the 3 readings side by side as one 3-bit binary
  // number "LMR". For example Left=1, Middle=0, Right=1 becomes binary 101,
  // which is 5 in decimal. This is a common trick: instead of tracking 3
  // separate 0/1 values, you pack them into a single number that's easy to
  // print, compare, or use in a switch statement.
  for (int i = 0; i < 4; i++) {
    Serial.print(sensorValue[i]); // print L, M, R, then the combined value
    Serial.print('\t'); // tab character, just to line up the columns nicely
  }
  Serial.print('\n'); // newline, so the next loop's readings start fresh
  delay(500); // wait half a second before reading again, so the output
              // doesn't scroll by too fast to read
}
