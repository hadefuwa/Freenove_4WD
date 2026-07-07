/**********************************************************************
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Description : use servo.
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
// This example shows how to control a servo motor - a small motor that can
// turn to a specific angle (instead of just spinning forever like a normal
// motor). This car uses one servo to pan (turn left/right) a sensor or
// camera mount. We'll sweep it back and forth between 30 and 150 degrees
// to see it move.
#include "Freenove_4WD_Car_For_Pico_W.h"

// setup() runs once, automatically, when the board powers on or resets.
// It's the perfect place to get hardware ready before the main program starts.
void setup()
{
  Servo_Setup();       //Servo initialization: turns on the PWM signal generator for the servo pin
  Servo_1_Angle(90);   //Move servo 1 straight to its middle position (90 degrees) as a starting point
  delay(1000);         //Wait 1000 milliseconds (1 second) so the servo has time to reach 90 degrees
}

// loop() runs over and over again, forever, right after setup() finishes.
// Each call below sweeps the servo smoothly from one angle to another,
// one degree at a time, so the arm's motion looks steady instead of jumpy.
void loop()
{
  // Servo 1 motion path; 90°- 30°- 150°- 90°
   Servo_Sweep(1, 90, 30);    // Sweep from the middle (90°) to the left limit (30°)
   Servo_Sweep(1, 30, 150);   // Sweep all the way across to the right limit (150°)
   Servo_Sweep(1, 150, 90);   // Sweep back to the middle (90°), then loop() repeats forever
}
