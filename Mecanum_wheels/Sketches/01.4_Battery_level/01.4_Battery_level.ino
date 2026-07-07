/**********************************************************************
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Description : Read battery voltage.
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
#include "Freenove_4WD_Car_For_Pico_W.h"
// This header brings in all the car's helper functions, including the two
// battery-reading functions we use below: Get_Battery_Voltage_ADC() and
// Get_Battery_Voltage(). They are actually defined in
// Freenove_4WD_Car_For_Pico_W.cpp - have a look there for the full story on
// how a raw ADC number gets turned into a real voltage.

// setup() runs ONCE, right when the Pico W powers on or is reset.
// It's the place to get everything ready before the main program starts.
void setup() {
  Serial.begin(115200);                     //Start the Serial connection so we can send text to the computer. 115200 is the "baud rate" - how fast the bits travel; both sides must agree on this number.
  Servo_Setup();    //Get the servo motor ready to move (not actually used by this battery-reading demo, but the shared setup code always initializes it).
}

// loop() runs over and over again, forever, after setup() finishes.
// Each time through, it reads the battery and prints the result, then
// waits a bit before doing it all again - like a repeat block in Scratch.
void loop() {
  Serial.print("Battery ADC : ");
  Serial.println(Get_Battery_Voltage_ADC());//Ask for the raw ADC reading (a whole number, roughly 0-1023) and print it as-is, with no conversion to volts yet.
  Serial.print("Battery Voltage : ");
  Serial.print(Get_Battery_Voltage());      //Ask for the reading again, but this time already converted into real volts (a decimal number like 7.4).
  Serial.println("V");
  delay(300);                               //Pause for 300 milliseconds (0.3 seconds) so the readings don't scroll by too fast to read.
}
