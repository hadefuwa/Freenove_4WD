// This pair of lines is called an "include guard". If some other file
// includes this header twice (directly or indirectly), the guard makes sure
// the contents below are only seen by the compiler once, avoiding errors
// about things being defined twice.
#ifndef _FREENOVE_4WD_CAR_H
#define _FREENOVE_4WD_CAR_H

#include <Arduino.h>     // Gives us core Arduino types/functions (delay, etc.)
#include "RP2040_PWM.h"  // A library that lets us generate PWM signals on the Pico's pins.
                         // PWM (Pulse Width Modulation) is a way of rapidly switching a pin
                         // on and off to fake an "in-between" signal - it's how we tell a
                         // servo motor what angle to move to.

/////////////////////Servo drive area///////////////////////////////////
#define PIN_SERVO1  13    // The steering/pan servo's signal wire is connected to GPIO pin 13

extern int servo_1_offset;                      // A small correction (in degrees) added to servo 1's angle,
                                                 // in case it isn't mounted perfectly straight.
                                                 // "extern" means: this variable really lives in the .cpp file,
                                                 // this is just announcing that it exists so other files can use it.

void Servo_Setup(void);                         // Prepares servo 1's PWM output so it's ready to receive angle commands
void Servo_1_Angle(float angle);                // Moves servo 1 to "angle" degrees (0-180 range, but clamped to 30-150 for safety)
void Set_Servo_1_Offset(int offset);            // Stores a mounting-correction offset (in degrees) for servo 1
void Servo_Sweep(int servo_id, int angle_start, int angle_end);// Slowly rotates a servo from angle_start to angle_end, one degree
                                                                // at a time, with a short pause between steps (a smooth sweep)

#endif
