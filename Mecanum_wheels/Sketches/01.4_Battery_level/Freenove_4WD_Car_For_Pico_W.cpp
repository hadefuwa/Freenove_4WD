#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"

/////////////////////Servo drive area///////////////////////////////////
uint32_t Servo_Pins[] = { PIN_SERVO1 };
#define NUM_OF_ServoPINS (sizeof(Servo_Pins) / sizeof(uint32_t))
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f };
float freq1[] = { 50.0f };
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];

int servo_1_offset = 0;  //Define the offset variable for servo 1

// Sets up the hardware PWM (Pulse Width Modulation) that drives servo 1.
// PWM is how a microcontroller creates an "analog-like" signal using only
// on/off pulses - it's how we tell a servo motor what angle to move to.
// Takes no parameters and returns nothing; it just prepares the hardware
// so Servo_1_Angle() can be used later.
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

// Moves servo 1 to a chosen angle, in degrees.
// angle: the target angle, but it gets clamped (limited) to 30-150 degrees
// so the servo horn never tries to over-rotate and strain itself.
// Internally the angle is converted into a pulse-width number the PWM
// hardware understands - you don't need to know that number, just the
// degrees you pass in.
void Servo_1_Angle(float angle) {
  angle = constrain(angle, 30, 150);
  angle = map(angle, 0.0f, 180.0f, 2500.0f, 12500.0f);
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle / 1000.0f);
}

// Stores a correction ("offset") in degrees for servo 1, in case it isn't
// perfectly centred when built into the car. offset: how many degrees to
// nudge future angle commands by. Returns nothing.
void Set_Servo_1_Offset(int offset) {
  servo_1_offset = offset;
}

// Smoothly sweeps a servo from one angle to another, one degree at a time,
// with a short delay between each step so the motion looks gradual rather
// than an instant jump. servo_id: which servo to move (only 1 is wired up
// here). angle_start/angle_end: the sweep's starting and ending angles, in
// degrees.
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
// Sets up the hardware PWM channels for all 4 wheel motors (8 pins total -
// 2 per motor, one for each direction). Takes no parameters and returns
// nothing; must be called once before the motors can be driven.
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

// Drives all 4 motors directly at the given speeds. m1_speed..m4_speed:
// one number per wheel, from -100 (full speed backwards) to 100 (full
// speed forwards) - these get clamped to that range automatically. There
// are no units like volts here; it's just a percentage-style speed value
// that gets turned into a PWM signal for the motor driver.
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
// A simpler way to drive the car: just give a left-side and right-side
// speed and this works out each individual wheel's speed for you.
// Left_speed/Right_speed: -100 to 100, same meaning as in
// Motor_Move_Init() above. The #ifdef blocks below let a builder flip an
// individual wheel's direction (see REVERSE_MOTOR1..4 in the header) if it
// was wired in backwards.
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
// Prepares the buzzer pin so the program can switch it on and off. Takes
// no parameters and returns nothing.
void Buzzer_Setup(void) {
  pinMode(PIN_BUZZER, OUTPUT);
}

// Beeps the buzzer in a repeating pattern - like a simple alarm.
// beat: how many short beeps make up one "phrase" (clamped to 1-9).
// rebeat: how many times to repeat that phrase (clamped to 1-255).
void Buzzer_Alert(int beat, int rebeat) {
  beat = constrain(beat, 1, 9);
  rebeat = constrain(rebeat, 1, 255);
  for (int j = 0; j < rebeat; j++) {
    for (int i = 0; i < beat; i++) {
      freq(PIN_BUZZER, BUZZER_FREQUENCY, 10);
    }
    delay(300);
  }
  freq(PIN_BUZZER, 0, 10);
  delay(300);
}

// Toggles a pin HIGH/LOW rapidly to make a simple square-wave tone on a
// buzzer - the faster it toggles, the higher-pitched the sound. PIN: which
// pin to pulse. freqs: the tone's frequency in Hertz (cycles per second) -
// pass 0 to just turn the sound off. times: roughly how long to buzz for,
// in an internal unit tied to the timing maths below (not plain
// milliseconds).
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
// Quick electronics primer for this section:
//   - ADC (Analog-to-Digital Converter): a piece of hardware inside the
//     Pico W that measures a real-world voltage (something that can be
//     ANY value, like 1.7V or 2.35V) and turns it into a whole number a
//     program can use. analogRead() here returns roughly 0-1023, where
//     0 means "0 volts" and 1023 means "the ADC's maximum, 3.3 volts".
//   - Voltage divider: the car's battery can be much higher than 3.3V,
//     which would damage the ADC pin if connected directly. So two
//     resistors are wired between the battery and PIN_BATTERY to scale
//     the voltage down proportionally (e.g. to a fraction of the real
//     battery voltage) before it reaches the Pico W. To get the real
//     battery voltage back, we have to "undo" that scaling with maths -
//     that's what batteryCoefficient and the formula below are for.

float batteryVoltage = 0;         //Battery voltage variable
float batteryCoefficient = 3.95;  //Set the proportional coefficient

// Reads the battery's raw ADC value - a whole number roughly in the range
// 0-1023 (NOT volts yet, just a sensor count). Takes several readings in a
// row and averages them, which smooths out small, noisy fluctuations you'd
// otherwise see from reading only once. Returns that averaged raw value.
int Get_Battery_Voltage_ADC(void) {
  pinMode(PIN_BATTERY, INPUT);
  int batteryADC = 0;
  for (int i = 0; i < 5; i++)         //Take 5 separate ADC readings...
    batteryADC += analogRead(PIN_BATTERY);
  return batteryADC / 5;              //...and average them by dividing the total by 5.
}

// Converts the raw ADC count into an actual battery voltage, in volts (V).
// Returns a decimal number like 7.4, not a whole ADC count.
float Get_Battery_Voltage(void) {
  int batteryADC = Get_Battery_Voltage_ADC();
  // Step 1: (batteryADC / 1023.0 * 3.3) turns the raw 0-1023 ADC count
  //         into the voltage actually measured AT THE PIN (0.0-3.3V) -
  //         batteryADC/1023.0 gives "what fraction of full-scale" the
  //         reading is, and multiplying by 3.3 (the ADC's reference
  //         voltage) converts that fraction into volts.
  // Step 2: multiplying by batteryCoefficient "undoes" the voltage
  //         divider's scaling-down, giving back the true battery voltage,
  //         which is higher than what the pin itself saw.
  batteryVoltage = (batteryADC / 1023.0 * 3.3) * batteryCoefficient;
  return batteryVoltage;
}

// Changes the scaling number (batteryCoefficient) used above to convert
// the pin's voltage back into the real battery voltage. coefficient: the
// new multiplier to use - you'd tweak this if you measured the battery
// with a multimeter and found the calculated voltage was slightly off, to
// calibrate the sensor. Returns nothing.
void Set_Battery_Coefficient(float coefficient) {
  batteryCoefficient = coefficient;
}
