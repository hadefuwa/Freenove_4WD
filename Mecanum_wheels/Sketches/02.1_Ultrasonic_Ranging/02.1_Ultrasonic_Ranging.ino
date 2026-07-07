/**********************************************************************
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Description : Ultrasonic ranging and servo.
  Auther      : www.freenove.com
  Modification: 2023/04/13
**********************************************************************/
#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"

// setup() runs once when the board powers on or resets.
// Here we get the ultrasonic sensor and the pan servo ready to use,
// and point the servo straight ahead (90 degrees) before the main loop starts.
void setup() {
  Serial.begin(115200);//Open the serial port and set the baud rate to 115200 bits/second, so Serial Monitor can show our printed text
  Ultrasonic_Setup();  //Ultrasonic module initialization
  Servo_Setup();       //Servo motor initialization
  Servo_1_Angle(90);   //Set the initial value of Servo 1 to 90 degrees (straight ahead)
  delay(500);          //Wait for the servo to arrive at the specified location - motors need a moment to physically move!
}

// loop() runs over and over forever after setup() finishes.
// Each time through, the servo turns the ultrasonic sensor to a new angle,
// waits for it to get there, then "pings" for a distance reading and prints it.
// The pattern is: right -> center -> left -> center, like a head slowly looking around.
void loop() {
  Servo_1_Angle(150);  //Turn servo 1 to 150 degrees (look right)
  Serial.print("Distance: " + String(Get_Sonar()) + "\n");//Ping and print the distance to whatever is in front of the sensor now
  delay(500);          //Give the servo time to finish turning and the reading time to settle before moving on

  Servo_1_Angle(90);   //Turn servo 1 to 90 degrees (back to center)
  Serial.print("Distance: " + String(Get_Sonar()) + "\n");//Print ultrasonic distance
  delay(500);

  Servo_1_Angle(30);   //Turn servo 1 to 30 degrees (look left)
  Serial.print("Distance: " + String(Get_Sonar()) + "\n");//Print ultrasonic distance
  delay(500);

  Servo_1_Angle(90);   //Turn servo 1 to 90 degrees (back to center, ready to repeat the sweep)
  Serial.print("Distance: " + String(Get_Sonar()) + "\n");//Print ultrasonic distance
  delay(500);
}
