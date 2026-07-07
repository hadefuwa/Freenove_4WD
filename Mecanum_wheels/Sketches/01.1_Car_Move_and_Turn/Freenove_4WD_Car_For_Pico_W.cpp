#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"

/////////////////////Servo drive area///////////////////////////////////
// This sketch doesn't actually move the servo, but Servo_Setup() is still called
// from setup() because the servo hardware needs to be initialized on every run,
// even if we don't use it here.

// A list of every pin that drives a servo. It's an array (just like a Python list)
// so the code below can loop through them instead of repeating itself for each pin.
uint32_t Servo_Pins[] = { PIN_SERVO1 };
// Work out how many servo pins are in the list above automatically, so if a servo
// is ever added/removed, this number updates itself instead of being typed by hand.
#define NUM_OF_ServoPINS (sizeof(Servo_Pins) / sizeof(uint32_t))
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f };  //Starting duty cycle (how "on" the signal is, 0-100%) for each servo, 0 = not moving yet
float freq1[] = { 50.0f };                      //Servos expect a repeating pulse 50 times per second (50Hz) — this is a standard servo requirement
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];  //An array to hold the PWM controller object for each servo pin

int servo_1_offset = 0;  //Define the offset variable for servo 1

//servo initialization
void Servo_Setup(void) {
  for (uint8_t index = 0; index < NUM_OF_ServoPINS; index++) {
    Servo_Instance[index] = new RP2040_PWM(Servo_Pins[index], freq1[index], dutyCycle1[index]);
    if (Servo_Instance[index]) {
      Servo_Instance[index]->setPWM();
      uint32_t div = Servo_Instance[index]->get_DIV();
      uint32_t top = Servo_Instance[index]->get_TOP();
      PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", Servo_Instance[index]->get_freq_CPU());
    }
  }
}

// Points servo 1 to a specific angle, in degrees.
// angle: the target angle, but it gets clamped (limited) to between 30 and 150 degrees
//        so the servo can't be commanded to spin further than its safe/physical range.
// A servo needs a pulse of a certain length, repeated 50 times a second, to know which
// angle to hold — this function converts the angle into that pulse length (in microseconds).
void Servo_1_Angle(float angle) {
  angle = constrain(angle, 30, 150);                        //Clamp the angle so we never ask the servo to over-rotate
  angle = map(angle, 0.0f, 180.0f, 2500.0f, 12500.0f);       //Convert the angle (0-180) into a pulse length the servo understands (2500-12500 microseconds)
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle / 1000.0f);  //Send that pulse out at 50Hz (divide by 1000 to convert microseconds to milliseconds)
}

// Stores an offset (correction amount) for servo 1. Useful if the servo horn wasn't
// mounted perfectly straight — this offset value can be added elsewhere to nudge
// every angle command so "the middle" really points straight ahead.
void Set_Servo_1_Offset(int offset) {
  servo_1_offset = offset;
}

// Smoothly sweeps a servo from one angle to another, one degree at a time, with a short
// pause between each step so the movement looks gradual instead of snapping instantly.
// servo_id: which servo to move (only 1 is supported here). angle_start/angle_end: the
// beginning and ending angles in degrees (0-180, though servo 1 itself is limited further
// to 30-150 by Servo_1_Angle above).
void Servo_Sweep(int servo_id, int angle_start, int angle_end) {
  if (servo_id == 1) {
    angle_start = constrain(angle_start, 0, 180);
    angle_end = constrain(angle_end, 0, 180);
  }
  if (angle_start > angle_end) {
    for (int i = angle_start; i >= angle_end; i--) {
      if (servo_id == 1)
        Servo_1_Angle(i);
      delay(10);
    }
  }
  if (angle_start < angle_end) {
    for (int i = angle_start; i <= angle_end; i++) {
      if (servo_id == 1)
        Servo_1_Angle(i);
      delay(10);
    }
  }
}

/////////////////////Motor drive area///////////////////////////////////
// The list of all 8 pins used to drive the 4 motors (2 pins per motor: one for each
// spin direction). The order here matches the order the PWM_Instance array is filled
// in below, which is why indices 0-7 are used directly (e.g. PWM_Instance[0]) further down.
uint32_t PWM_Pins[] = { PIN_MOTOR_PWM_RIGHT1, PIN_MOTOR_PWM_RIGHT2, PIN_MOTOR_PWM_RIGHT3, PIN_MOTOR_PWM_RIGHT4, PIN_MOTOR_PWM_LEFT1, PIN_MOTOR_PWM_LEFT2, PIN_MOTOR_PWM_LEFT3, PIN_MOTOR_PWM_LEFT4 };
#define NUM_OF_PINS (sizeof(PWM_Pins) / sizeof(uint32_t))  //Automatically counts how many pins are in the list above (8)
float dutyCycle2[NUM_OF_PINS] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };  //Every motor pin starts at 0% duty cycle (i.e. stopped)
float freq2[] = { 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f };  //Each motor pin gets a 500Hz PWM signal, which the motor driver chip expects
RP2040_PWM* PWM_Instance[NUM_OF_PINS];  //Holds one PWM controller object per motor pin, so we can update each pin's speed later

// Sets up PWM (Pulse Width Modulation) output on all 8 motor pins. PWM lets the Pico
// rapidly switch a pin on and off; the percentage of time it's "on" (the duty cycle)
// controls how much power reaches the motor, which is how we get variable speeds
// instead of just full-on/full-off. This must run once, in setup(), before any
// motor movement functions are called.
void Motor_Setup(void) {
  for (uint8_t index = 0; index < NUM_OF_PINS; index++) {
    PWM_Instance[index] = new RP2040_PWM(PWM_Pins[index], freq2[index], dutyCycle2[index]);
    if (PWM_Instance[index]) {
      PWM_Instance[index]->setPWM();
      uint32_t div = PWM_Instance[index]->get_DIV();
      uint32_t top = PWM_Instance[index]->get_TOP();
      PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", PWM_Instance[index]->get_freq_CPU());
    }
  }
}

// The low-level function that actually talks to the motor driver hardware.
// m1_speed..m4_speed: desired speed for each of the 4 wheels, from -100 (full reverse)
// to 100 (full forward), where negative/positive sign controls direction and the size
// of the number controls how fast. Each motor only has ONE pin active at a time — a
// positive speed drives the "forward" pin with PWM and sets the "backward" pin to 0,
// and vice-versa for negative speeds (that's why the code below flips the sign back to
// positive with -m1_speed etc. before sending it to setPWM, since PWM duty cycle can't
// be negative). You normally won't call this directly — use Motor_Move or Motor_M_Move.
void Motor_Move_Init(int m1_speed, int m2_speed, int m3_speed, int m4_speed) {
  float frequency = 500;
  //Clamp every speed to the safe range (-100 to 100) in case a caller passes something out of range
  m1_speed = constrain(m1_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  m2_speed = constrain(m2_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  m3_speed = constrain(m3_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  m4_speed = constrain(m4_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  //Motor 1 (front-left): drive the "forward" pin if speed is positive, otherwise drive the "backward" pin
  if (m1_speed >= 0) {
    PWM_Instance[0]->setPWM(PIN_MOTOR_PWM_LEFT1, frequency, m1_speed);
    PWM_Instance[1]->setPWM(PIN_MOTOR_PWM_LEFT2, frequency, 0);
  } else {
    m1_speed = -m1_speed;  //Flip back to a positive number, since PWM duty cycle can only be 0-100%, never negative
    PWM_Instance[1]->setPWM(PIN_MOTOR_PWM_LEFT2, frequency, m1_speed);
    PWM_Instance[0]->setPWM(PIN_MOTOR_PWM_LEFT1, frequency, 0);
  }
  //Motor 2 (back-left): same idea as motor 1 above
  if (m2_speed >= 0) {
    PWM_Instance[2]->setPWM(PIN_MOTOR_PWM_LEFT3, frequency, m2_speed);
    PWM_Instance[3]->setPWM(PIN_MOTOR_PWM_LEFT4, frequency, 0);
  } else {
    m2_speed = -m2_speed;
    PWM_Instance[3]->setPWM(PIN_MOTOR_PWM_LEFT4, frequency, m2_speed);
    PWM_Instance[2]->setPWM(PIN_MOTOR_PWM_LEFT3, frequency, 0);
  }
  //Motor 3 (front-right): same idea as motor 1 above
  if (m3_speed >= 0) {
    PWM_Instance[4]->setPWM(PIN_MOTOR_PWM_RIGHT1, frequency, m3_speed);
    PWM_Instance[5]->setPWM(PIN_MOTOR_PWM_RIGHT2, frequency, 0);
  } else {
    m3_speed = -m3_speed;
    PWM_Instance[5]->setPWM(PIN_MOTOR_PWM_RIGHT2, frequency, m3_speed);
    PWM_Instance[4]->setPWM(PIN_MOTOR_PWM_RIGHT1, frequency, 0);
  }
  //Motor 4 (back-right): same idea as motor 1 above
  if (m4_speed >= 0) {
    PWM_Instance[6]->setPWM(PIN_MOTOR_PWM_RIGHT3, frequency, m4_speed);
    PWM_Instance[7]->setPWM(PIN_MOTOR_PWM_RIGHT4, frequency, 0);
  } else {
    m4_speed = -m4_speed;
    PWM_Instance[7]->setPWM(PIN_MOTOR_PWM_RIGHT4, frequency, m4_speed);
    PWM_Instance[6]->setPWM(PIN_MOTOR_PWM_RIGHT3, frequency, 0);
  }
}

// A simpler "tank steering" style movement function for a normal (non-mecanum) style
// drive, where you only pick one speed for the whole left side and one for the whole
// right side (like the two control sticks on a tank). Left_speed/Right_speed range from
// -100 to 100. This sketch doesn't use this function (it uses Motor_M_Move instead,
// which controls all 4 wheels independently), but it's kept here for other sketches
// that don't need full mecanum control.
void Motor_Move(int Left_speed, int Right_speed) {
  int lf, lb, rf, rb;
  lf = lb = Left_speed;    //Both left wheels (front & back) get the same speed
  rf = rb = Right_speed;   //Both right wheels (front & back) get the same speed
//The 4 blocks below only do anything if you've uncommented the matching REVERSE_MOTORx
//line in the header file — they flip an individual wheel's direction to correct wiring.
#ifdef REVERSE_MOTOR1
  lf = -Left_speed;
#endif
#ifdef REVERSE_MOTOR2
  lb = -Left_speed;
#endif
#ifdef REVERSE_MOTOR3
  rf = -Right_speed;
#endif
#ifdef REVERSE_MOTOR4
  rb = -Right_speed;
#endif
  Motor_Move_Init(lf, lb, rf, rb);
}

// The main mecanum-wheel movement function used throughout this sketch.
// M1_speed..M4_speed: independent speed for each wheel (front-left, back-left,
// front-right, back-right), from -100 (full reverse) to 100 (full forward). Unlike a
// normal car, because mecanum wheels can push sideways as well as forward/back, giving
// each wheel a different speed/direction lets the whole car drive forward, spin on the
// spot, strafe sideways, or glide diagonally — see the comments in the .ino file for
// exactly why each combination of speeds produces the movement it does.
void Motor_M_Move(int M1_speed, int M2_speed, int M3_speed,int M4_speed) {
  int lf, lb, rf, rb;
  lf = M1_speed;
  lb = M2_speed;
  rf = M3_speed;
  rb = M4_speed;
//As above, these only take effect if a REVERSE_MOTORx line was uncommented in the header,
//to correct a wheel that's wired to spin the opposite way from what's expected.
#ifdef REVERSE_MOTOR1
  lf = -M1_speed;
#endif
#ifdef REVERSE_MOTOR2
  lb = -M2_speed;
#endif
#ifdef REVERSE_MOTOR3
  rf = -M3_speed;
#endif
#ifdef REVERSE_MOTOR4
  rb = -M4_speed;
#endif
  Motor_Move_Init(lf, lb, rf, rb);
}

