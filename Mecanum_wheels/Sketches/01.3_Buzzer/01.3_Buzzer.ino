/**********************************************************************
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Description : Use buzzer
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
#include "Freenove_4WD_Car_For_Pico_W.h"  // Brings in all the pin numbers and functions for the car (motors, servo, buzzer)

// setup() runs exactly ONCE, right when the board powers on or is reset.
// It's the perfect place to get things ready before the main action starts.
void setup() {
  Buzzer_Setup();      // Tell the Pico W that the buzzer pin will be used to SEND a signal out (an "output")
  Buzzer_Alert(4, 3);  // Beep the buzzer: 4 quick beeps per burst, repeated 3 times in a row
}

// loop() runs over and over again, forever, right after setup() finishes.
// Here it just waits - all the beeping already happened once in setup().
void loop() {
  delay(1000);  // Pause for 1000 milliseconds (1 second) before checking loop() again
}
