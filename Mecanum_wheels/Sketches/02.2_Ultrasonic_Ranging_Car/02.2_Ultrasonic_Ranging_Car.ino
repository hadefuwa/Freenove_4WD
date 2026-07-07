/**********************************************************************
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Description : Ultrasonic Car.
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
#include <Arduino.h>              // Core Arduino functions (pinMode, digitalWrite, delay, etc.)
#include "Freenove_4WD_Car_For_Pico_W.h"  // Declares all the car's helper functions (motors, servo, ultrasonic sensor...)

// setup() runs ONCE when the board powers on or resets.
// Think of it like a "getting ready" checklist before the robot starts moving.
void setup() {
  Ultrasonic_Setup();           // Get the ultrasonic distance sensor's pins ready to use
  Motor_Setup();                // Get the 4 wheel motors ready to use
  Servo_Setup();                // Get the sensor's steering servo ready to use (it swivels the sensor left/right)
}

// loop() runs over and over again, forever, after setup() finishes.
// Every trip through loop() the robot: looks around with the sensor, decides what to do,
// and moves the wheels accordingly. This is what makes the robot "think" continuously.
void loop()
{
  Ultrasonic_Car();  // Do one round of "scan for obstacles, then drive or dodge"
}
