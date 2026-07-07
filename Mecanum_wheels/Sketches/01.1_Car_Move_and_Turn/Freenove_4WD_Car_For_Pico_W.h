// Header file: this only DECLARES what functions, constants and pins exist.
// The actual code that does the work lives in Freenove_4WD_Car_For_Pico_W.cpp.
// Splitting "what exists" (.h) from "how it works" (.cpp) is a common C++ pattern
// that lets other files (like our .ino sketch) use these functions just by
// #include-ing this header, without needing to see their full implementation.

// These next 3 lines stop this file from being read/inserted twice if several
// other files #include it — without this "include guard", the compiler would
// complain about the same functions/constants being defined more than once.
#ifndef _FREENOVE_4WD_CAR_H
#define _FREENOVE_4WD_CAR_H

#include <Arduino.h>
#include "RP2040_PWM.h"

// If a motor spins the "wrong" way compared to how it's wired/mounted, uncomment
// the matching line below to flip that one wheel's direction in software instead
// of re-wiring it. All 4 are commented out here because this kit's wheels already
// spin the correct way by default.
// #define REVERSE_MOTOR1
// #define REVERSE_MOTOR2
// #define REVERSE_MOTOR3
// #define REVERSE_MOTOR4

/////////////////////Servo drive area///////////////////////////////////
#define PIN_SERVO1  13    //define servo pin

extern int servo_1_offset;                      //Define the offset variable for servo 1

void Servo_Setup(void);                         //servo initialization
void Servo_1_Angle(float angle);                //Set the rotation parameters of servo 1, and the parameters are 30-150 degrees
void Set_Servo_1_Offset(int offset);            //Set servo 1 offset
void Servo_Sweep(int servo_id, int angle_start, int angle_end);//Servo sweep function;

/////////////////////Motor drive area///////////////////////////////////
// Each mecanum wheel motor needs 2 pins to control it: one pin turns the motor one way,
// the other pin turns it the opposite way. Only one of the pair is ever "on" at a time.
// M1 = front-left wheel, M2 = back-left wheel, M3 = front-right wheel, M4 = back-right wheel.
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

void Motor_Setup(void);                //motor initialization
void Motor_Move_Init(int m1_speed, int m2_speed, int m3_speed, int m4_speed);//A function to control the car motor
void Motor_M_Move(int M1_speed, int M2_speed, int M3_speed,int M4_speed);//A function to control the maicanum wheel car motor
void Motor_Move(int Left_speed, int Right_speed);//A function to control the car motor

#endif
