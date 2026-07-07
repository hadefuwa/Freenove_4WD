#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"

/////////////////////Servo drive area///////////////////////////////////
// These arrays hold the settings for every servo we control (here, just one:
// servo 1 on PIN_SERVO1). Using arrays like this means the same code could
// support more servos later just by adding more entries.
uint32_t Servo_Pins[] = { PIN_SERVO1 };                          // Which physical pins the servos are wired to
#define NUM_OF_ServoPINS (sizeof(Servo_Pins) / sizeof(uint32_t)) // How many servos we have (calculated from the array size)
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f };                   // Starting duty cycle for each servo (0 = signal starts off)
float freq1[] = { 50.0f };                                       // PWM frequency in Hz: standard servos expect 50 pulses per second
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];                    // Will hold a PWM "controller" object for each servo pin

int servo_1_offset = 0;  //Define the offset variable for servo 1

// Sets up the PWM hardware for every servo pin listed above. Call this once
// from setup() before using any other servo function - it "wakes up" the
// pins so they can send the pulse signal a servo needs to know its target angle.
void Servo_Setup(void) {
  for (uint8_t index = 0; index < NUM_OF_ServoPINS; index++) {
    Servo_Instance[index] = new RP2040_PWM(Servo_Pins[index], freq1[index], dutyCycle1[index]); // Create a PWM controller for this pin
    if (Servo_Instance[index]) {           // Only continue if creating it worked (it returns a real object, not nothing/null)
      Servo_Instance[index]->setPWM();     // Actually start generating the PWM signal on this pin
      uint32_t div = Servo_Instance[index]->get_DIV();  // Internal timer setting the Pico used to make the 50Hz signal
      uint32_t top = Servo_Instance[index]->get_TOP();  // Internal timer "counting limit" for the same signal
      PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", Servo_Instance[index]->get_freq_CPU()); // Prints debug info (only shows up if debug logging is turned on)
    }
  }
}

// Moves servo 1 to a specific angle, in degrees. Even though the parameter
// is called "angle" and thought of as 0-180 degrees, this particular servo
// is physically limited to turning between 30 and 150 degrees, so anything
// outside that range gets clamped (limited) to the nearest allowed value.
void Servo_1_Angle(float angle) {
  angle = constrain(angle, 30, 150);                        // Clamp the requested angle into the safe 30-150 degree range
  angle = map(angle, 0.0f, 180.0f, 2500.0f, 12500.0f);       // Convert degrees into a pulse length in microseconds (this is what the servo actually reads)
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle / 1000.0f); // Send a 50Hz signal whose "on" time (in milliseconds) tells the servo which angle to hold
}

// Stores a small correction value ("offset") for servo 1, in degrees.
// This exists so that if the servo horn (the arm attached to it) wasn't
// installed perfectly centered, you can nudge all future angles to compensate.
// Note: this function only saves the number - nothing in this file currently
// applies servo_1_offset to the servo's movement.
void Set_Servo_1_Offset(int offset) {
  servo_1_offset = offset;
}

// Smoothly turns a servo from angle_start to angle_end, one degree at a
// time, waiting 10 milliseconds between each step. This is what makes the
// servo glide instead of snapping instantly to the new angle.
// servo_id: which servo to move (only 1 is supported here).
// angle_start / angle_end: the starting and ending angles in degrees (0-180).
void Servo_Sweep(int servo_id, int angle_start, int angle_end) {
  if (servo_id == 1) {
    angle_start = constrain(angle_start, 0, 180);  // Keep the requested start angle within a valid 0-180 degree range
    angle_end = constrain(angle_end, 0, 180);       // Same for the end angle
  }
  if (angle_start > angle_end) {           // Case 1: sweeping "downwards" (e.g. 150 -> 90)
    for (int i = angle_start; i >= angle_end; i--) {
      if (servo_id == 1)
        Servo_1_Angle(i);   // Move to the next angle, one degree closer to angle_end
      delay(10);            // Small pause (10 ms) so the sweep is smooth and gives the servo time to actually move
    }
  }
  if (angle_start < angle_end) {           // Case 2: sweeping "upwards" (e.g. 30 -> 150)
    for (int i = angle_start; i <= angle_end; i++) {
      if (servo_id == 1)
        Servo_1_Angle(i);   // Move to the next angle, one degree closer to angle_end
      delay(10);            // Small pause (10 ms), same reason as above
    }
  }
}

/////////////////////Motor drive area///////////////////////////////////
// The functions below control the car's four wheel motors. This particular
// sketch never calls them (it only uses the servo), but they live in this
// shared file alongside the servo code, so they're commented too.
uint32_t PWM_Pins[] = { PIN_MOTOR_PWM_RIGHT1, PIN_MOTOR_PWM_RIGHT2, PIN_MOTOR_PWM_RIGHT3, PIN_MOTOR_PWM_RIGHT4, PIN_MOTOR_PWM_LEFT1, PIN_MOTOR_PWM_LEFT2, PIN_MOTOR_PWM_LEFT3, PIN_MOTOR_PWM_LEFT4 };
#define NUM_OF_PINS (sizeof(PWM_Pins) / sizeof(uint32_t))
float dutyCycle2[NUM_OF_PINS] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
float freq2[] = { 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f };  // Motors use a much faster PWM frequency (500Hz) than the servo (50Hz)
RP2040_PWM* PWM_Instance[NUM_OF_PINS];

// Sets up the PWM hardware for all 8 motor pins (2 pins per wheel x 4
// wheels). Call once from setup() before trying to move any motor.
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

// Low-level function that directly drives all 4 wheel motors.
// Each *_speed parameter ranges from -100 (full speed backwards) to
// 100 (full speed forwards); 0 means stopped. Each wheel motor has two
// wires/pins, so forwards vs backwards is achieved by sending the PWM
// "on" signal to one pin or the other, never both at once.
void Motor_Move_Init(int m1_speed, int m2_speed, int m3_speed, int m4_speed) {
  float frequency = 500;
  m1_speed = constrain(m1_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  m2_speed = constrain(m2_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  m3_speed = constrain(m3_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  m4_speed = constrain(m4_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  if (m1_speed >= 0) {
    PWM_Instance[0]->setPWM(PIN_MOTOR_PWM_LEFT1, frequency, m1_speed);
    PWM_Instance[1]->setPWM(PIN_MOTOR_PWM_LEFT2, frequency, 0);
  } else {
    m1_speed = -m1_speed;
    PWM_Instance[1]->setPWM(PIN_MOTOR_PWM_LEFT2, frequency, m1_speed);
    PWM_Instance[0]->setPWM(PIN_MOTOR_PWM_LEFT1, frequency, 0);
  }
  if (m2_speed >= 0) {
    PWM_Instance[2]->setPWM(PIN_MOTOR_PWM_LEFT3, frequency, m2_speed);
    PWM_Instance[3]->setPWM(PIN_MOTOR_PWM_LEFT4, frequency, 0);
  } else {
    m2_speed = -m2_speed;
    PWM_Instance[3]->setPWM(PIN_MOTOR_PWM_LEFT4, frequency, m2_speed);
    PWM_Instance[2]->setPWM(PIN_MOTOR_PWM_LEFT3, frequency, 0);
  }
  if (m3_speed >= 0) {
    PWM_Instance[4]->setPWM(PIN_MOTOR_PWM_RIGHT1, frequency, m3_speed);
    PWM_Instance[5]->setPWM(PIN_MOTOR_PWM_RIGHT2, frequency, 0);
  } else {
    m3_speed = -m3_speed;
    PWM_Instance[5]->setPWM(PIN_MOTOR_PWM_RIGHT2, frequency, m3_speed);
    PWM_Instance[4]->setPWM(PIN_MOTOR_PWM_RIGHT1, frequency, 0);
  }
  if (m4_speed >= 0) {
    PWM_Instance[6]->setPWM(PIN_MOTOR_PWM_RIGHT3, frequency, m4_speed);
    PWM_Instance[7]->setPWM(PIN_MOTOR_PWM_RIGHT4, frequency, 0);
  } else {
    m4_speed = -m4_speed;
    PWM_Instance[7]->setPWM(PIN_MOTOR_PWM_RIGHT4, frequency, m4_speed);
    PWM_Instance[6]->setPWM(PIN_MOTOR_PWM_RIGHT3, frequency, 0);
  }
}
// Drives the car like a simple tank: all left-side wheels move at
// Left_speed and all right-side wheels move at Right_speed (each from
// -100 to 100). If the REVERSE_MOTOR# options near the top of the header
// file are turned on, this flips the direction for that specific wheel
// (useful if a motor was wired in backwards).
void Motor_Move(int Left_speed, int Right_speed) {
  int lf, lb, rf, rb;
  lf = lb = Left_speed;
  rf = rb = Right_speed;
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

// Drives each of the 4 mecanum wheels independently at its own speed
// (-100 to 100). Mecanum wheels have angled rollers around their edge,
// so by choosing different speeds/directions for each wheel the car can
// drive sideways or diagonally, not just forwards/backwards - unlike Motor_Move().
void Motor_M_Move(int M1_speed, int M2_speed, int M3_speed,int M4_speed) {
  int lf, lb, rf, rb;
  lf = M1_speed;
  lb = M2_speed;
  rf = M3_speed;
  rb = M4_speed;
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

