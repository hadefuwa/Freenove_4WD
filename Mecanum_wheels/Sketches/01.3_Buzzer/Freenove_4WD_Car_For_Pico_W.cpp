#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"

// NOTE: This file contains code for the servo AND the four motors, in
// addition to the buzzer. This sketch only ever calls the buzzer functions
// near the bottom of the file - the servo/motor code below is unused here,
// but is documented too since it's shared by every sketch in the kit.

/////////////////////Servo drive area///////////////////////////////////
uint32_t Servo_Pins[] = { PIN_SERVO1 };
#define NUM_OF_ServoPINS (sizeof(Servo_Pins) / sizeof(uint32_t))
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f };
float freq1[] = { 50.0f };
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];

int servo_1_offset = 0;  //Define the offset variable for servo 1

// Sets up the PWM hardware for the steering servo so it's ready to receive
// angle commands. Not used by the buzzer sketch.
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

// Points servo 1 at a given angle (0-180 degrees). The angle is first
// clamped into range, then converted into a pulse-width number the servo
// hardware understands. Not used by the buzzer sketch.
void Servo_1_Angle(float angle) {
  angle = constrain(angle, 0, 180);
  angle = map(angle, 0.0f, 180.0f, 2500.0f, 12500.0f);
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle / 1000.0f);
}

// Stores a correction value (offset) for servo 1, in case it isn't
// perfectly centered when built. Not used by the buzzer sketch.
void Set_Servo_1_Offset(int offset) {
  servo_1_offset = offset;
}

// Smoothly moves a servo from one angle to another, one degree at a time,
// with a tiny delay between each step so the motion looks gradual instead
// of snapping instantly. Not used by the buzzer sketch.
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
uint32_t PWM_Pins[] = { PIN_MOTOR_PWM_RIGHT1, PIN_MOTOR_PWM_RIGHT2, PIN_MOTOR_PWM_RIGHT3, PIN_MOTOR_PWM_RIGHT4, PIN_MOTOR_PWM_LEFT1, PIN_MOTOR_PWM_LEFT2, PIN_MOTOR_PWM_LEFT3, PIN_MOTOR_PWM_LEFT4 };
#define NUM_OF_PINS (sizeof(PWM_Pins) / sizeof(uint32_t))
float dutyCycle2[NUM_OF_PINS] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
float freq2[] = { 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f };
RP2040_PWM* PWM_Instance[NUM_OF_PINS];
// Sets up the PWM hardware for all four drive motors so they're ready to
// receive speed commands. Not used by the buzzer sketch.
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

// Drives all four motors at the given speeds (-100 to 100 each). Each motor
// is controlled by two pins (like a positive and negative terminal); a
// positive speed sends PWM to one pin, a negative speed sends it to the
// other, which is how the motor is made to spin in reverse. Not used by
// the buzzer sketch.
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
// Simplified motor control: just give an overall left-side and right-side
// speed, and this figures out each individual wheel (handling any motors
// that need to be flipped via the REVERSE_MOTOR defines above). Not used
// by the buzzer sketch.
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

//////////////////////Buzzer drive area///////////////////////////////////
// Gets the buzzer pin ready to be controlled by our code. OUTPUT mode means
// "this pin will be used to SEND voltage out", rather than reading a signal
// in (which would be INPUT mode). This must be called before the buzzer
// will make any sound.
void Buzzer_Setup(void) {
  pinMode(PIN_BUZZER, OUTPUT);
}

// Beeps the buzzer in a pattern: "beat" is how many short beeps happen in a
// row (a "burst"), and "rebeat" is how many times that whole burst repeats.
// For example, Buzzer_Alert(4, 3) beeps 4 times, pauses, then does that
// 2 more times (3 bursts total). The constrain() calls just make sure
// nobody accidentally asks for a silly number of beeps (like 0 or 1000).
void Buzzer_Alert(int beat, int rebeat) {
  beat = constrain(beat, 1, 9);
  rebeat = constrain(rebeat, 1, 255);
  for (int j = 0; j < rebeat; j++) {       // Repeat the whole burst "rebeat" times
    for (int i = 0; i < beat; i++) {       // Play "beat" short beeps in this burst
      freq(PIN_BUZZER, BUZZER_FREQUENCY, 30);  // One short beep at the buzzer's resonant pitch, lasting 30ms
    }
    delay(300);  // Short pause between bursts so you can tell them apart
  }
  freq(PIN_BUZZER, 0, 30);  // Make sure the buzzer ends up silent (frequency 0 = off)
  delay(300);
}

// This is the function that actually makes the buzzer produce a sound.
// A buzzer doesn't have a "volume knob" - the only way to make noise with a
// simple digital pin is to flip it HIGH (on/full voltage) and LOW (off/no
// voltage) very quickly, over and over. Each HIGH-then-LOW flip is one sound
// wave. Doing that "freqs" times per second is what creates a musical pitch
// - e.g. 2000 flips/second sounds like a higher note than 500 flips/second.
//   PIN   = which pin to pulse (the buzzer's pin)
//   freqs = how many times per second to pulse it (the pitch); 0 means "stay off"
//   times = roughly how long (in milliseconds) to keep beeping for
void freq(int PIN, int freqs, int times) {
  if (freqs == 0) {
    digitalWrite(PIN, LOW);  // No pulsing requested - just make sure the buzzer is off
  } else {
    // Work out how many HIGH/LOW cycles are needed to fill "times" milliseconds at this pitch
    for (int i = 0; i < times * freqs / 500; i++) {
      digitalWrite(PIN, HIGH);              // Turn the buzzer pin ON (full voltage) - buzzer clicks
      delayMicroseconds(500000 / freqs);     // Wait out half of one wave's period
      digitalWrite(PIN, LOW);               // Turn the buzzer pin OFF (no voltage) - buzzer clicks back
      delayMicroseconds(500000 / freqs);     // Wait out the other half of the wave's period
    }
  }
}
