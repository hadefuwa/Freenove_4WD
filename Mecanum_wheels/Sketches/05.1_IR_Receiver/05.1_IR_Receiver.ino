/**********************************************************************
  Filename    : Infrared Remote Control
  Description : Decode the infrared remote control and print it out through the serial port.
  Auther      : www.freenove.com
  Modification: 2023/04/13
**********************************************************************/
// This library does the hard work of listening to the IR receiver sensor
// and turning the flickering infrared light pulses into numbers we can use.
#include <IRremote.hpp>
#define IR_Pin 3  // Infrared receiving pin
#define ENABLE_LED_FEEDBACK true
#define DISABLE_LED_FEEDBACK false

// setup() runs once when the board powers on or is reset.
// It gets the Serial Monitor and the IR receiver ready to use, then
// prints a message so we know the program is alive and listening.
void setup() {
  Serial.begin(115200);                           // Initialize the serial port and set the baud rate to 115200
  IrReceiver.begin(IR_Pin, DISABLE_LED_FEEDBACK);  // Start the receiver
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(IR_Pin);  //print the infrared receiving pin
}

// loop() runs over and over, forever, after setup() finishes.
// Every remote button sends its own unique numeric code as a burst of
// infrared light. This function checks whether a new code has just been
// received, and if so, prints that number to the Serial Monitor. Since
// each button always sends the same number, comparing this printed value
// against a known list of codes (e.g. with if/switch statements) is how
// you could figure out exactly which button was pressed.
void loop() {
  if (IrReceiver.decode()) {  // true only when a fresh IR signal has just arrived
    // decodedRawData holds the unique number sent by whichever button was pressed
    unsigned long value = IrReceiver.decodedIRData.decodedRawData;
    Serial.println(value, HEX);  // Print "old" raw data
    IrReceiver.resume();    // Enable receiving of the next value
  }
}
