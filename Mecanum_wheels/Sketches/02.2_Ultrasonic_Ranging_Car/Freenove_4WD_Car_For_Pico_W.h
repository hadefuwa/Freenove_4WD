#ifndef _FREENOVE_4WD_CAR_H
#define _FREENOVE_4WD_CAR_H

// This "include guard" (#ifndef/#define/#endif) stops this file from being
// pasted in twice if more than one file includes it. You don't need to
// worry about it — it's just C++ housekeeping.

#include <Arduino.h>
#include "RP2040_PWM.h"

// These are all commented out (turned off). If one of your wheels ever spins
// the wrong way, you'd uncomment the matching line below to flip its direction.
// #define REVERSE_MOTOR1
// #define REVERSE_MOTOR2
// #define REVERSE_MOTOR3
// #define REVERSE_MOTOR4

/////////////////////Servo drive area///////////////////////////////////
// The servo is the little motor that swivels the ultrasonic sensor left and right,
// like a head turning to look around, so the sensor can "look" in different directions.
#define PIN_SERVO1  13    //Pico W pin the steering servo is plugged into

extern int servo_1_offset;                      //A small correction angle if the servo isn't perfectly centered

void Servo_Setup(void);                         //Get the servo ready to use
void Servo_1_Angle(float angle);                //Turn servo 1 to face a given angle (allowed range: 30-150 degrees)
void Set_Servo_1_Offset(int offset);            //Adjust servo 1's "zero point" if it points slightly off-center
void Servo_Sweep(int servo_id, int angle_start, int angle_end);//Smoothly sweep a servo from one angle to another (not used in this sketch)

/////////////////////Motor drive area///////////////////////////////////
// Each wheel motor is controlled through a PAIR of pins: one pin pushes the
// wheel forward, the other pushes it backward. Only one of the pair is
// switched on at a time, depending on which direction we want that wheel to spin.
#define PIN_MOTOR_PWM_RIGHT1  7          //Front-right wheel motor (M3), "forward" pin
#define PIN_MOTOR_PWM_RIGHT2  6          //Front-right wheel motor (M3), "backward" pin
#define PIN_MOTOR_PWM_RIGHT3  9          //Rear-right wheel motor (M4), "forward" pin
#define PIN_MOTOR_PWM_RIGHT4  8          //Rear-right wheel motor (M4), "backward" pin
#define PIN_MOTOR_PWM_LEFT1   18         //Front-left wheel motor (M1), "forward" pin
#define PIN_MOTOR_PWM_LEFT2   19         //Front-left wheel motor (M1), "backward" pin
#define PIN_MOTOR_PWM_LEFT3   21         //Rear-left wheel motor (M2), "forward" pin
#define PIN_MOTOR_PWM_LEFT4   20         //Rear-left wheel motor (M2), "backward" pin
#define MOTOR_SPEED_MIN       -100       //Slowest allowed speed value (negative = full speed backward)
#define MOTOR_SPEED_MAX       100        //Fastest allowed speed value (positive = full speed forward)

void Motor_Setup(void);                //Get all 4 wheel motors ready to use
void Motor_Move_Init(int m1_speed, int m2_speed, int m3_speed, int m4_speed);//Low-level function that actually spins each of the 4 wheels at its own speed (-100 to 100)
void Motor_Move(int Left_speed, int Right_speed);//Simple driving: set one speed for both left wheels and one speed for both right wheels
void Motor_M_Move(int M1_speed, int M2_speed, int M3_speed,int M4_speed);//Mecanum driving: set all 4 wheels independently (needed for the sideways/turning tricks mecanum wheels can do)
//////////////////////Buzzer drive area///////////////////////////////////
//Buzzer pin definition
#define PIN_BUZZER 2                    //Pico W pin the beeper (buzzer) is plugged into
#define BUZZER_FREQUENCY 2000           //Pitch (in Hz) the buzzer beeps at
void Buzzer_Setup(void);                //Get the buzzer pin ready to use
void Buzzer_Alert(int beat, int rebeat);//Beep a pattern: "beat" beeps per burst, repeated "rebeat" times
void freq(int PIN, int freqs, int times); //Low-level helper: buzz a pin at a chosen frequency for a chosen time

////////////////////Battery drive area/////////////////////////////////////
#define PIN_BATTERY        26        //Pico W pin used to measure the battery voltage
#define LOW_VOLTAGE_VALUE  525      //Sensor reading below which the battery is considered too low
#define BAT_VOL_STANDARD	8.4        //The "full battery" voltage we compare against
#define OA_SPEED_OFFSET_PER_V	3    //How much extra motor speed to add per volt the battery has dropped, so the car still drives at roughly the same speed as the battery drains
extern int oa_VoltageCompensationToSpeed;  //The extra speed (calculated using the two settings above) added to every motor command

extern float batteryCoefficient;    //Scaling number used to convert the raw sensor reading into real volts

int Get_Battery_Voltage_ADC(void);   //Read the raw battery sensor value (an "ADC" reading, not yet real volts)
float Get_Battery_Voltage(void);     //Read the battery voltage, converted into real volts
void Set_Battery_Coefficient(float coefficient);//Change the scaling number used when converting sensor readings to volts
void oa_CalculateVoltageCompensation(void); //Work out how much extra motor speed is needed to make up for a low battery

/////////////////////Ultrasonic drive area/////////////////////////////////
// The ultrasonic sensor works like bat sonar: it sends out a sound "ping" you
// can't hear, times how long the echo takes to bounce back, and uses that time
// to calculate how far away the nearest object is.
#define PIN_SONIC_TRIG    4            //Pin that sends out the sound pulse ("trigger")
#define PIN_SONIC_ECHO    5            //Pin that listens for the sound bouncing back ("echo")
#define MAX_DISTANCE      300           //Furthest distance (in cm) we bother measuring
#define SONIC_TIMEOUT (MAX_DISTANCE*60) //How long (in microseconds) to wait for an echo before giving up
#define SOUND_VELOCITY    340           //Speed of sound in air, in metres per second, used to turn "echo time" into "distance"

#define SONAR_MODE_CRUISE_SPEED (40+oa_VoltageCompensationToSpeed)  //Normal driving speed while cruising, auto-adjusted for battery level
typedef uint8_t u8;  //Just a short nickname for "unsigned 8-bit number" (0-255), used to save typing
#define COUNT_GET_SONAR 1  //How many sonar readings to average together for each direction

void Ultrasonic_Setup(void);//Get the ultrasonic sensor's pins ready to use
float Get_Sonar(void);//Take one distance measurement (in cm) with the ultrasonic sensor
extern int distance[4];

// These two numbers are the THRESHOLDS the robot's decisions are built on:
// "how close is too close?" See Ultrasonic_Car() in the .cpp file for how they're used.
#define OBSTACLE_DISTANCE      45  //If something in front is closer than this (cm), it counts as "an obstacle ahead"
#define OBSTACLE_DISTANCE_LOW  25  //A closer/stricter threshold used for objects just off to the side

void get_distance(int car_mode);
void Ultrasonic_Car(void);//The main "look, decide, drive" behaviour: scan with the sensor, then steer around obstacles

#endif
