#ifndef _FREENOVE_4WD_CAR_H
#define _FREENOVE_4WD_CAR_H
// This header is the "menu" of everything the car's driver library (the
// .cpp file next to this one) can do: servos, motors, buzzer, battery
// check, light sensor, ultrasonic distance sensor, line-tracking sensors,
// and the LED face/emotion display. This sketch (04.1) only actually uses
// the "Track drive area" section below - the rest is here because the
// library file is shared by every example sketch in this kit.

#include <Arduino.h>
#include "RP2040_PWM.h"

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
void Motor_Move(int Left_speed, int Right_speed);//A function to control the car motor

//////////////////////Buzzer drive area///////////////////////////////////
//Buzzer pin definition             
#define PIN_BUZZER 2                    //Define the pins for the Pico control buzzer
#define BUZZER_FREQUENCY 2000           //Define the resonant frequency of the buzzer 
void Buzzer_Setup(void);                //Buzzer initialization
void Buzzer_Alert(int beat, int rebeat);//Buzzer alarm function
void freq(int PIN, int freqs, int times); 

////////////////////Battery drive area/////////////////////////////////////
#define PIN_BATTERY        26        //Set the battery detection voltage pin
#define LOW_VOLTAGE_VALUE  525      //Set the minimum battery voltage
extern float batteryCoefficient;    //Set the proportional coefficient

int Get_Battery_Voltage_ADC(void);   //Gets the battery ADC value
float Get_Battery_Voltage(void);     //Get the battery voltage value
void Set_Battery_Coefficient(float coefficient);//Set the partial pressure coefficient

////////////////////Photosensitive drive area//////////////////////////////
#define PHOTOSENSITIVE_PIN   27
void Photosensitive_Setup(void);           //Photosensitive initialization
int Get_Photosensitive(void);              //Gets the photosensitive resistance value

/////////////////////Ultrasonic drive area/////////////////////////////////
#define PIN_SONIC_TRIG    4            //define Trig pin
#define PIN_SONIC_ECHO    5            //define Echo pin
#define MAX_DISTANCE      300           //cm
#define SONIC_TIMEOUT (MAX_DISTANCE*60) // calculate timeout 
#define SOUND_VELOCITY    340           //soundVelocity: 340m/s
void Ultrasonic_Setup(void);//Ultrasonic initialization
float Get_Sonar(void);//Obtain ultrasonic distance data

/////////////////////Track drive area//////////////////////////////
// This is the part this sketch actually uses: the 3 infrared line-tracking
// ("IR reflectance") sensors on the underside of the car. Each one is wired
// to its own digital input pin, and reads either LOW/0 (white floor) or
// HIGH/1 (black line).
#define PIN_TRACKING_LEFT   12
#define PIN_TRACKING_CENTER 11
#define PIN_TRACKING_RIGHT  10
// sensorValue is a global array shared between Track_Read() (which fills it)
// and any sketch (which reads it). "extern" means "this array really lives
// in the .cpp file, but I'm telling you its type here so you can use it."
// sensorValue[0..2] hold the raw Left/Middle/Right 0-or-1 readings.
// sensorValue[3] holds all three combined into one number, 0-7.
extern unsigned char sensorValue[4];
void Track_Setup(void);//Sets the 3 tracking sensor pins as digital inputs
void Track_Read(void);//Reads all 3 sensors and updates sensorValue[]

//////////////////////Emotion drive area////////////////////////////////
#define EMOTION_ADDRESS 0x71
#define EMOTION_SDA     4
#define EMOTION_SCL     5
void Emotion_Setup();                            //Initialize
void eyesRotate(int delay_ms);                   //Turn the eyes-1
void eyesBlink(int delay_ms);                    //Wink the eyes-2
void eyesSmile(int delay_ms);                    //Smile-3
void eyesCry(int delay_ms);                      //Cry-4
void eyesBlink1(int delay_ms);                   //Wink the eyes-5
void showArrow(int arrow_direction,int delay_ms);//Arrow-6
void wheel(int mode,int delay_ms);               //wheel-7
void carMove(int mode,int delay_ms);             //car-8
void expressingLove(void);                       //expressing love-9
void saveWater(int delay_ms);                    //save water-10



#endif
