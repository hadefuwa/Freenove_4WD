/**********************************************************************
  Filename    : Photosensitive_Car.ino
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"
// This sketch steers the car based on which side sees more light.
// It reads two light sensors (one on the left, one on the right), then speeds
// up the wheels on the brighter side and slows the wheels on the darker side.
// All the sensor-reading and motor logic lives in
// Freenove_4WD_Car_For_Pico_W.cpp, inside a function called Light_Car().


// setup() runs ONCE, right when the Pico boots up.
// Think of it as "get everything ready before the game starts."
void setup() {
  Buzzer_Setup();
  Photosensitive_Setup();  //Photosensitive initialization
  Motor_Setup();           //Motor initialization
  Buzzer_Alert(1, 1);      //The buzzer beeps to prompt the user to start playing
  Serial.begin(115200);    //Initialize the serial port and set the baud rate to 115200
}

// loop() runs over and over and over, forever, as fast as the Pico can manage.
// Every time through the loop we call Light_Car(), which checks the light
// sensors and decides how to drive the wheels - that's what makes the car
// keep "chasing" the light instead of just reacting to it once.
void loop() {
  Light_Car();
}
