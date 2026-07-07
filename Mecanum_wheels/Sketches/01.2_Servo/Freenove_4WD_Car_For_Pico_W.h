// This is a "header file". It doesn't do any work itself - it just lists
// (declares) what pins, variables, and functions exist in
// Freenove_4WD_Car_For_Pico_W.cpp, so other files (like the .ino sketch)
// know they're available to use.
#ifndef _FREENOVE_4WD_CAR_H
#define _FREENOVE_4WD_CAR_H
// The three lines above/below are an "include guard": they stop this file's
// contents from being copy-pasted in twice if something #includes it twice.

#include <Arduino.h>
#include "RP2040_PWM.h"  // Library that lets the Pico W generate PWM signals (see note below)

// #define REVERSE_MOTOR1
// #define REVERSE_MOTOR2
// #define REVERSE_MOTOR3
// #define REVERSE_MOTOR4

/////////////////////Servo drive area///////////////////////////////////
// A servo is a small motor with built-in electronics that turns to (and
// holds) a specific angle, instead of spinning continuously like a normal
// motor. It's controlled using PWM (Pulse Width Modulation) - a signal that
// rapidly switches a pin on and off; the length of time it stays "on" during
// each pulse tells the servo which angle to turn to.
#define PIN_SERVO1  13    //define servo pin: GPIO pin 13 on the Pico W sends the PWM signal to servo 1

extern int servo_1_offset;                      //Define the offset variable for servo 1 (declared here, actually stored in the .cpp file)

void Servo_Setup(void);                         //servo initialization
void Servo_1_Angle(float angle);                //Set the rotation parameters of servo 1, and the parameters are 30-150 degrees
void Set_Servo_1_Offset(int offset);            //Set servo 1 offset
void Servo_Sweep(int servo_id, int angle_start, int angle_end);//Servo sweep function;

/////////////////////Motor drive area///////////////////////////////////
// This sketch doesn't drive the wheel motors, but the shared library file
// still declares them here. Each wheel motor needs two pins (like a "+" and
// "-" terminal) so its direction can be flipped by swapping which one is
// receiving the PWM signal.
#define PIN_MOTOR_PWM_RIGHT1  7          //Define the positive pole of M3
#define PIN_MOTOR_PWM_RIGHT2  6          //Define the positive pole of M3
#define PIN_MOTOR_PWM_RIGHT3  9          //Define the negative pole of M4
#define PIN_MOTOR_PWM_RIGHT4  8          //Define the negative pole of M4
#define PIN_MOTOR_PWM_LEFT1   18         //Define the positive pole of M1
#define PIN_MOTOR_PWM_LEFT2   19         //Define the negative pole of M1
#define PIN_MOTOR_PWM_LEFT3   21         //Define the positive pole of M2
#define PIN_MOTOR_PWM_LEFT4   20         //Define the negative pole of M2
#define MOTOR_SPEED_MIN       -100       //Define a minimum speed limit for wheels (-100 = full speed backwards)
#define MOTOR_SPEED_MAX       100        //Define a maximum speed limit for wheels (100 = full speed forwards)

void Motor_Setup(void);                //motor initialization
void Motor_Move_Init(int m1_speed, int m2_speed, int m3_speed, int m4_speed);//A function to control the car motor
void Motor_M_Move(int M1_speed, int M2_speed, int M3_speed,int M4_speed);//A function to control the maicanum wheel car motor
void Motor_Move(int Left_speed, int Right_speed);//A function to control the car motor

#endif
