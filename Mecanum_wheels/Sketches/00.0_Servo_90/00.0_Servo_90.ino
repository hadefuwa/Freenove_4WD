/**********************************************************************
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Description : Set the Angle value of servo to 90°
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
// This brings in all the servo functions (Servo_Setup, Servo_1_Angle, etc.)
// that are written in Freenove_4WD_Car_For_Pico_W.cpp. Think of the .h file
// as a "menu" of what's available, and the .cpp file as where it's cooked up.
#include "Freenove_4WD_Car_For_Pico_W.h"

// setup() runs exactly ONCE, right when the Pico powers on or is reset.
// It's the place to get hardware ready before the main program starts.
void setup() {
  Servo_Setup();  // Get the servo's PWM signal ready to use (see the .cpp file for details)
}

// loop() runs over and over again, forever, after setup() finishes.
// This sketch just keeps telling the steering servo to point at 90 degrees,
// which is "straight ahead" for a servo that can swing between 0-180 degrees.
void loop() {
  Servo_1_Angle(90);  // Command servo 1 to move to (and hold) the 90 degree position
  delay(1000);         // Wait 1000 milliseconds (1 second) before repeating the command
}
