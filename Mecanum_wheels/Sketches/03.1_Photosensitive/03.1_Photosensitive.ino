/**********************************************************************
  Filename    : Photosensitive_Sensor.ino
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Auther      : www.freenove.com
  Modification: 2023/04/13
**********************************************************************/
// ---------------------------------------------------------------------------
// A photosensitive sensor (a.k.a. photoresistor / LDR - "Light Dependent
// Resistor") is a little component whose resistance changes depending on
// how much light hits it: lots of light = low resistance, darkness = high
// resistance. The Pico can't measure resistance directly, but it CAN measure
// voltage using its ADC (Analog-to-Digital Converter). The sensor is wired
// up so that the voltage on the pin changes along with the light level, and
// analogRead() converts that voltage into a plain number (a "raw ADC value")
// that our code can use. On this board, more light generally gives a
// DIFFERENT (not necessarily bigger) number than darkness - the important
// thing for this sketch is comparing the left and right readings to each
// other, so you can tell which side has more/less light shining on it.
// ---------------------------------------------------------------------------

// GPIO pin numbers on the Pico that the left/right photosensitive sensors are wired to.
#define Left_PHOTOSENSITIVE_PIN 28  //Define the pins that Raspberry Pi Pico reads photosensitive
#define Right_PHOTOSENSITIVE_PIN 27 //Define the pins that Raspberry Pi Pico reads photosensitive
// These variables remember the most recent raw ADC reading (a whole number,
// typically 0-4095 on the Pico's 12-bit ADC) from each sensor, so we can
// print them later. Bigger/smaller doesn't map simply to "brighter", so just
// compare the two numbers to see which side is getting more/less light.
int getLeftPhotosensitiveADCValue;  //Define a variable to store the left photosensitive adc value
int getRightPhotosensitiveADCValue; //Define a variable to store the right photosensitive adc value

// setup() runs once when the board powers on or resets. It gets the two
// sensor pins ready to read from, and opens the Serial connection so we can
// send text back to the Serial Monitor on your computer.
void setup()
{
  pinMode(Left_PHOTOSENSITIVE_PIN, INPUT);//Configure the pins for input mode
  pinMode(Right_PHOTOSENSITIVE_PIN, INPUT);//Configure the pins for input mode
  Serial.begin(115200);          //Initialize the serial port and set the baud rate to 115200
}

// loop() runs over and over, forever, after setup() finishes. Each time
// through, it reads both light sensors, prints their raw ADC values to the
// Serial Monitor so you can compare the left and right readings, then waits
// half a second before doing it all again.
void loop()
{
  getLeftPhotosensitiveADCValue = analogRead(Left_PHOTOSENSITIVE_PIN);//Read ADC1 value value
  getRightPhotosensitiveADCValue = analogRead(Right_PHOTOSENSITIVE_PIN);//Read ADC2 value value
  Serial.print("The photosensitive ADC on the left: ");
  Serial.println(getLeftPhotosensitiveADCValue);       //Print the photosensitive ADC value on the left
  Serial.print("The photosensitive ADC on the right: ");
  Serial.println(getRightPhotosensitiveADCValue);      //Print the photosensitive ADC value on the right
  delay(500); //Pause for 500 milliseconds so the readings don't scroll by too fast to read
}

