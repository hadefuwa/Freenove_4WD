#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"

/////////////////////Servo drive area///////////////////////////////////
// These arrays are set up so this code could control more than one servo
// just by adding more pin numbers - right now there's only PIN_SERVO1, so
// every array below has exactly 1 element.
uint32_t Servo_Pins[] = { PIN_SERVO1 };
// NUM_OF_ServoPINS works out (at compile time) how many entries are in
// Servo_Pins, by dividing the total size in bytes by the size of one entry.
// This means if you add more pins to Servo_Pins above, this number updates
// itself automatically - you never have to count them by hand.
#define NUM_OF_ServoPINS (sizeof(Servo_Pins) / sizeof(uint32_t))
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f };  // Starting duty cycle (0%) for each servo - gets set properly later
float freq1[] = { 50.0f };                      // PWM frequency in Hz. Hobby servos expect a pulse every 50 times
                                                 // per second (i.e. one pulse every 20 milliseconds).
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];   // One PWM "controller object" per servo pin, created in Servo_Setup()

int servo_1_offset = 0;  // Mounting-correction offset (degrees) for servo 1; 0 means "no correction needed"

// Gets each servo's PWM hardware ready to use. Must be called once from
// setup() before you can command any servo to move. If you had more than
// one servo pin in Servo_Pins, this loop would set all of them up.
void Servo_Setup(void) {
  for (uint8_t index = 0; index < NUM_OF_ServoPINS; index++) {
    // "new" creates a fresh RP2040_PWM object on this pin, at 50Hz, starting at 0% duty cycle
    Servo_Instance[index] = new RP2040_PWM(Servo_Pins[index], freq1[index], dutyCycle1[index]);
    if (Servo_Instance[index]) {  // Only continue if the object was created successfully
      Servo_Instance[index]->setPWM();  // Actually start generating the PWM signal on the pin
      // DIV and TOP are internal numbers the Pico's PWM hardware uses to turn the 50Hz
      // frequency into clock ticks. You don't need to understand these to use servos -
      // they're only printed out here to help with debugging.
      uint32_t div = Servo_Instance[index]->get_DIV();
      uint32_t top = Servo_Instance[index]->get_TOP();
      PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", Servo_Instance[index]->get_freq_CPU());
    }
  }
}

// Moves servo 1 to the given angle. "angle" is in degrees, where a normal
// hobby servo can physically move from 0 to 180 degrees, but here it's
// clamped (limited) to 30-150 degrees to stop it straining against its
// mechanical end-stops and possibly damaging itself or the steering linkage.
void Servo_1_Angle(float angle) {
  angle = constrain(angle, 30, 150);  // Clamp angle so it never goes below 30 or above 150 degrees
  // Servos are controlled by the WIDTH of a pulse, measured in microseconds (millionths of a
  // second), not directly by degrees. Here we convert 0-180 degrees into the matching pulse
  // width of 2500-12500 (which setPWM below divides by 1000 to get microseconds, i.e. 2.5-12.5).
  // These numbers come from this specific servo's datasheet/behaviour.
  angle = map(angle, 0.0f, 180.0f, 2500.0f, 12500.0f);
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle / 1000.0f);  // Send the pulse: pin, 50Hz, pulse width in microseconds
}

// Stores a correction offset (in degrees) for servo 1, in case the physical
// servo horn isn't perfectly aligned when mounted. (Note: this function only
// saves the number here - using it to actually shift Servo_1_Angle's output
// would need extra code, which isn't included in this simple example.)
void Set_Servo_1_Offset(int offset) {
  servo_1_offset = offset;
}

// Smoothly rotates a servo from angle_start to angle_end, moving one degree
// at a time with a short pause, instead of jumping straight there. This
// makes the motion look gentle rather than a sudden snap.
// servo_id: which servo to move (only 1 is supported here)
// angle_start / angle_end: the degree values (0-180) to sweep between
void Servo_Sweep(int servo_id, int angle_start, int angle_end) {
  if (servo_id == 1) {
    angle_start = constrain(angle_start, 0, 180);  // Keep the requested range within what a servo can physically do
    angle_end = constrain(angle_end, 0, 180);
  }
  if (angle_start > angle_end) {  // Sweeping "backwards" (e.g. 150 down to 30)
    for (int i = angle_start; i >= angle_end; i--) {
      if (servo_id == 1)
        Servo_1_Angle(i);
      delay(10);  // Wait 10 milliseconds between each 1-degree step, so the sweep takes some time to complete
    }
  }
  if (angle_start < angle_end) {  // Sweeping "forwards" (e.g. 30 up to 150)
    for (int i = angle_start; i <= angle_end; i++) {
      if (servo_id == 1)
        Servo_1_Angle(i);
      delay(10);
    }
  }
}
