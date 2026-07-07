// This header file is like a "table of contents" for the car's functions.
// It declares (promises) what functions and constants exist in
// Freenove_4WD_Car_For_Pico_W.cpp, so other files (like our .ino sketch)
// can use them just by #include-ing this header, without needing to see
// how each function is actually written inside.
#ifndef _FREENOVE_4WD_CAR_H
#define _FREENOVE_4WD_CAR_H

#include <Arduino.h>
#include "RP2040_PWM.h"

// These are commented out on purpose. If a motor spins the "wrong" way when
// wired up, uncommenting the matching line here flips its direction in code
// instead of you having to re-wire the motor by hand.
// #define REVERSE_MOTOR1
// #define REVERSE_MOTOR2
// #define REVERSE_MOTOR3
// #define REVERSE_MOTOR4

/////////////////////Servo drive area///////////////////////////////////
#define PIN_SERVO1  13    //The Pico W pin wire connected to the pan servo's signal wire

extern int servo_1_offset;                      //A small correction angle (in degrees) added to servo 1, in case it's not perfectly centered when built

void Servo_Setup(void);                         //Gets servo 1 ready to receive angle commands (must be called once, in setup())
void Servo_1_Angle(float angle);                //Turns servo 1 to point at "angle" degrees; only 30-150 degrees is allowed (see .cpp for why)
void Set_Servo_1_Offset(int offset);            //Stores a correction offset (degrees) to apply to servo 1's angle
void Servo_Sweep(int servo_id, int angle_start, int angle_end);//Slowly turns a servo from angle_start to angle_end, one degree at a time

/////////////////////Motor drive area///////////////////////////////////
// (Not used by this Ultrasonic Ranging sketch, but declared here because
// this header/cpp pair is shared by every example sketch in the kit.)
#define PIN_MOTOR_PWM_RIGHT1  7          //Define the positive pole of M3
#define PIN_MOTOR_PWM_RIGHT2  6          //Define the positive pole of M3
#define PIN_MOTOR_PWM_RIGHT3  9          //Define the negative pole of M4
#define PIN_MOTOR_PWM_RIGHT4  8          //Define the negative pole of M4
#define PIN_MOTOR_PWM_LEFT1   18         //Define the positive pole of M1
#define PIN_MOTOR_PWM_LEFT2   19         //Define the negative pole of M1
#define PIN_MOTOR_PWM_LEFT3   21         //Define the positive pole of M2
#define PIN_MOTOR_PWM_LEFT4   20         //Define the negative pole of M2
#define MOTOR_SPEED_MIN       -100       //Define a minimum speed limit for wheels
#define MOTOR_SPEED_MAX       100        //Define a maximum speed limit for wheels

void Motor_Setup(void);                  //Gets all four drive motors ready to be controlled
void Motor_Move_Init(int m1_speed, int m2_speed, int m3_speed, int m4_speed);//Sets each of the 4 wheel motors to its own speed (-100 to 100)
void Motor_Move(int Left_speed, int Right_speed);//Sets the left-side and right-side wheels to a speed each, like simple tank-style driving

//////////////////////Buzzer drive area///////////////////////////////////
// (Also not used by this sketch - the buzzer lets other sketches beep.)
//Buzzer pin definition
#define PIN_BUZZER 2                    //Define the pins for the Pico W control buzzer
#define BUZZER_FREQUENCY 2000           //Define the resonant frequency of the buzzer
void Buzzer_Setup(void);                //Buzzer initialization
void Buzzer_Alert(int beat, int rebeat);//Beeps the buzzer "beat" times, and repeats that pattern "rebeat" times
void freq(int PIN, int freqs, int times);//Makes a pin buzz at a given frequency (Hz) for a given duration (ms) by rapidly toggling it on/off

////////////////////Battery drive area/////////////////////////////////////
// (Also not used by this sketch - lets other sketches check the battery level.)
#define PIN_BATTERY        26        //Set the battery detection voltage pin
#define LOW_VOLTAGE_VALUE  525       //Set the minimum battery voltage
extern float batteryCoefficient;     //Set the proportional coefficient

int Get_Battery_Voltage_ADC(void);   //Reads the raw analog-to-digital value (0-1023) from the battery sensing pin
float Get_Battery_Voltage(void);     //Converts the raw ADC reading into an actual battery voltage (volts)
void Set_Battery_Coefficient(float coefficient);//Set the partial pressure coefficient

/////////////////////Ultrasonic drive area/////////////////////////////////
// The HC-SR04 ultrasonic sensor works like a bat's sonar: it sends out a
// short "shout" (an ultrasonic sound pulse humans can't hear) on the Trig
// pin, then listens on the Echo pin for the sound to bounce off something
// and come back. The longer the echo takes, the farther away the object is.
#define PIN_SONIC_TRIG    4             //Pin that tells the sensor "shout now" (the trigger/output pin)
#define PIN_SONIC_ECHO    5             //Pin the sensor uses to tell us "I heard the echo" (goes HIGH while waiting/listening)
#define MAX_DISTANCE      300           //The farthest distance (cm) we'll trust a reading for; anything farther counts as "no echo"
#define SONIC_TIMEOUT (MAX_DISTANCE*60) // How long (microseconds) to wait for an echo before giving up - based on sound needing ~60us to travel 1cm there and back
#define SOUND_VELOCITY    340           //Speed of sound in air, in metres per second - used to turn echo time into a distance
void Ultrasonic_Setup(void);//Sets up the Trig pin as an output and the Echo pin as an input (must be called once, in setup())
float Get_Sonar(void);//Sends one ping and returns the measured distance to the nearest object, in centimetres

#endif
