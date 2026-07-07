#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"

/////////////////////Servo drive area///////////////////////////////////
uint32_t Servo_Pins[] = { PIN_SERVO1 };
#define NUM_OF_ServoPINS (sizeof(Servo_Pins) / sizeof(uint32_t))
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f };
float freq1[] = { 50.0f };
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];

int servo_1_offset = 0;  //Define the offset variable for servo 1

// Servo_Setup(): prepares servo 1 to be controlled by PWM (Pulse Width
// Modulation - rapid on/off pulses whose timing tells the servo what angle
// to move to). Must be called once, in setup(), before using Servo_1_Angle().
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

// Servo_1_Angle(angle): turns servo 1 to point at "angle" degrees.
// The angle is clamped (limited) to 30-150 degrees so the servo can't try to
// spin past its safe mechanical range and strain itself. Internally, the
// angle is converted into a pulse width in microseconds (2500-12500), which
// is the "language" servos actually understand - a longer high pulse means
// a bigger angle.
void Servo_1_Angle(float angle) {
  angle = constrain(angle, 30, 150);
  angle = map(angle, 0.0f, 180.0f, 2500.0f, 12500.0f);
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle / 1000.0f);
}

// Set_Servo_1_Offset(offset): stores a small correction angle (degrees) for
// servo 1, useful if the servo horn wasn't attached perfectly straight.
void Set_Servo_1_Offset(int offset) {
  servo_1_offset = offset;
}

// Servo_Sweep(servo_id, angle_start, angle_end): smoothly rotates a servo
// from angle_start to angle_end, one degree at a time with a short delay
// between steps, instead of jumping straight there. This makes the motion
// look slow and steady rather than a sudden snap.
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
// Motor_Setup(): prepares all 8 motor-control pins for PWM output.
// Not used by this sketch (no driving happens here), but shared with sketches
// that do drive the car.
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

// Motor_Move_Init(m1_speed..m4_speed): sets each of the 4 wheel motors to
// its own speed, from -100 (full reverse) to 100 (full forward). Not used
// by this sketch, but shared with driving sketches.
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
// Motor_Move(Left_speed, Right_speed): a simpler way to drive - just set an
// overall left-side and right-side speed. Not used by this sketch.
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
// Buzzer_Setup(): prepares the buzzer pin as an output. Not used by this
// sketch (this sketch never beeps), but shared with sketches that do.
void Buzzer_Setup(void) {
  pinMode(PIN_BUZZER, OUTPUT);
}

// Buzzer_Alert(beat, rebeat): beeps "beat" times (1-9), pauses half a
// second, and repeats that whole pattern "rebeat" times (1-255).
void Buzzer_Alert(int beat, int rebeat) {
  beat = constrain(beat, 1, 9);
  rebeat = constrain(rebeat, 1, 255);
  for (int j = 0; j < rebeat; j++) {
    for (int i = 0; i < beat; i++) {
      freq(PIN_BUZZER, BUZZER_FREQUENCY, 30);
    }
    delay(500);
  }
  freq(PIN_BUZZER, 0, 30);
}

// freq(PIN, freqs, times): makes "PIN" vibrate at "freqs" Hz for "times"
// milliseconds by rapidly switching it HIGH/LOW (that rapid switching is what
// the buzzer hears as a tone). Passing freqs=0 just turns the pin off.
void freq(int PIN, int freqs, int times) {
  if (freqs == 0) {
    digitalWrite(PIN, LOW);
  } else {
    for (int i = 0; i < times * freqs / 500; i++) {
      digitalWrite(PIN, HIGH);
      delayMicroseconds(500000 / freqs);
      digitalWrite(PIN, LOW);
      delayMicroseconds(500000 / freqs);
    }
  }
}

////////////////////Battery drive area/////////////////////////////////////
float batteryVoltage = 0;      //Battery voltage variable
float batteryCoefficient = 4;  //Set the proportional coefficient

// Get_Battery_Voltage_ADC(): reads the raw analog voltage on the battery
// sensing pin 5 times and averages them (to smooth out noise), returning a
// raw ADC number from 0-1023. Not used by this sketch.
int Get_Battery_Voltage_ADC(void) {
  pinMode(PIN_BATTERY, INPUT);
  int batteryADC = 0;
  for (int i = 0; i < 5; i++)
    batteryADC += analogRead(PIN_BATTERY);
  return batteryADC / 5;
}

// Get_Battery_Voltage(): converts the raw ADC reading into a real battery
// voltage, using the board's reference voltage and a calibration
// coefficient. Not used by this sketch.
float Get_Battery_Voltage(void) {
  int batteryADC = Get_Battery_Voltage_ADC();
  batteryVoltage = (batteryADC / 1023.0 * 3.67) * batteryCoefficient;
  return batteryVoltage;
}

// Set_Battery_Coefficient(coefficient): lets you adjust the calibration
// number used above, in case the real voltage divider on the board differs
// slightly from the ideal design value.
void Set_Battery_Coefficient(float coefficient) {
  batteryCoefficient = coefficient;
}

/////////////////////Ultrasonic drive area//////////////////////////////
// Ultrasonic_Setup(): configures the two pins the HC-SR04 sensor uses -
// Trig as an output (so we can send it a "shout now" signal) and Echo as
// an input (so we can listen for how long the echo takes to come back).
// Must be called once, in setup(), before Get_Sonar() is used.
void Ultrasonic_Setup(void) {
  pinMode(PIN_SONIC_TRIG, OUTPUT);  // set trigPin to output mode
  pinMode(PIN_SONIC_ECHO, INPUT);   // set echoPin to input mode
}

// Get_Sonar(): sends one ultrasonic "ping" and returns the distance (in
// centimetres) to whatever it bounces off of, like a bat using echolocation.
// Step by step:
//   1. Pulse the Trig pin HIGH for 10 microseconds - this is the "shout",
//      it tells the HC-SR04 sensor to send out a burst of ultrasonic sound.
//   2. pulseIn() then waits and measures (in microseconds) how long the
//      Echo pin stays HIGH - the sensor holds Echo HIGH for exactly as long
//      as it took the sound to travel out and bounce back ("listening").
//   3. Sound travels at a known speed (SOUND_VELOCITY, 340 m/s), so:
//      distance = time_taken * speed_of_sound, then divide by 2 because the
//      sound had to travel to the object AND back (a round trip), and the
//      units are converted from microseconds/(m/s) into centimetres.
// If no echo comes back within SONIC_TIMEOUT (nothing in range), pulseIn()
// returns 0, and we just report MAX_DISTANCE (300 cm) instead of an error.
float Get_Sonar(void) {
  unsigned long pingTime;
  float distance;
  digitalWrite(PIN_SONIC_TRIG, HIGH);  // make trigPin output high level lasting for 10μs to triger HC_SR04,
  delayMicroseconds(10);
  digitalWrite(PIN_SONIC_TRIG, LOW);
  pingTime = pulseIn(PIN_SONIC_ECHO, HIGH, SONIC_TIMEOUT);  // Wait HC-SR04 returning to the high level and measure out this waitting time
  if (pingTime != 0)
    distance = (float)pingTime * SOUND_VELOCITY / 2 / 10000;  // calculate the distance according to the time
  else
    distance = MAX_DISTANCE;  // no echo detected within the timeout - treat it as "nothing closer than max range"
  return distance;  // return the distance value, in centimetres
}
