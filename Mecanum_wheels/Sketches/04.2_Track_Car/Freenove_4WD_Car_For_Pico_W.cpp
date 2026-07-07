// This file is the "hardware driver" for the whole car: it contains the
// actual code that talks to the motors, sensors, buzzer, and LED face.
// The .ino sketch only calls these functions by name (like Track_Read()
// or Motor_M_Move()) - it never needs to know the pin numbers or PWM
// details, because that complexity is all handled in here. This keeps
// the "brain" logic in the .ino short and readable.
#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"
#include "Freenove_VK16K33_Lib.h"
#include "Array.h"
#include <Wire.h>

/////////////////////Servo drive area///////////////////////////////////
uint32_t Servo_Pins[] = { PIN_SERVO1 };
#define NUM_OF_ServoPINS (sizeof(Servo_Pins) / sizeof(uint32_t))
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f };
float freq1[] = { 50.0f };
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];

int servo_1_offset = 0;  //Define the offset variable for servo 1

// Sets up the PWM (Pulse Width Modulation) hardware for the servo pin.
// PWM is how the Pico tells a servo/motor "how much power" or "what
// angle" to use, by rapidly switching the pin on and off. This lesson's
// sketch doesn't use the servo, but Motor_Setup() below uses the same
// PWM technique for the drive motors.
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

// Moves servo 1 to a target angle (30-150 degrees; anything outside that
// range gets clamped by constrain()). The angle is converted into a PWM
// pulse-width the servo motor understands, using map() to rescale it.
void Servo_1_Angle(float angle) {
  angle = constrain(angle, 30, 150);
  angle = map(angle, 0.0f, 180.0f, 2500.0f, 12500.0f);
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle / 1000.0f);
}

// Stores a small correction angle (offset) for servo 1, useful if the
// physical servo horn isn't mounted perfectly straight.
void Set_Servo_1_Offset(int offset) {
  servo_1_offset = offset;
}

// Smoothly sweeps a servo from angle_start to angle_end, one degree at a
// time with a 10ms pause between steps, instead of jumping there instantly.
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
// Sets up PWM on all 8 motor-driver pins (2 pins per wheel: one for
// "spin forward", one for "spin backward"). Called once from setup().
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

// The lowest-level motor function - every other motor function eventually
// calls this one. Each wheel gets its own speed from -100 (full reverse)
// to 100 (full forward); constrain() clamps out-of-range values so the
// motors are never told to do something impossible.
// Each wheel has TWO pins (one "forward" pin, one "backward" pin). Since
// a positive speed means "spin forward", we send that speed to the
// forward pin and 0 to the backward pin - and flip that around for a
// negative speed. This is how a single number like -60 becomes "spin
// backward at 60% power" on real motor hardware.
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
// Simple "tank style" driving: give one speed for both left wheels and
// one speed for both right wheels (like the two joysticks on an old RC
// tank). The #ifdef blocks below let a builder flip an individual
// motor's direction (see REVERSE_MOTOR1-4 near the top of the header)
// without touching this logic - useful if a motor was wired backwards.
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
// "Mecanum style" driving: each of the 4 wheels gets its OWN independent
// speed (M1 = left-front, M2 = left-back, M3 = right-front, M4 =
// right-back). This is the function the line-following switch/case
// calls, because giving the left and right sides opposite-signed speeds
// (one positive, one negative) makes the car pivot/rotate in place
// instead of driving straight - exactly what's needed to steer back onto
// the line.
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

//////////////////////Buzzer drive area///////////////////////////////////
// Prepares the buzzer pin so we can send it beep signals. Not used by
// this line-following sketch, but shared here with other lessons.
void Buzzer_Setup(void) {
  pinMode(PIN_BUZZER, OUTPUT);
}

// Beeps the buzzer "beat" times in a row, then repeats that whole burst
// "rebeat" times with a short pause in between - like a repeating alarm.
void Buzzer_Alert(int beat, int rebeat) {
  beat = constrain(beat, 1, 9);
  rebeat = constrain(rebeat, 1, 255);
  for (int j = 0; j < rebeat; j++) {
    for (int i = 0; i < beat; i++) {
      freq(PIN_BUZZER, BUZZER_FREQUENCY, 10);
    }
    delay(500);
  }
  freq(PIN_BUZZER, 0, 10);
}

// Toggles a pin HIGH/LOW at a given frequency (freqs, in Hz) for a
// duration ("times"), which is what actually makes the buzzer produce a
// tone - a speaker/buzzer makes sound by vibrating, and vibrating is just
// rapid on/off switching.
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

// Reads the raw battery voltage sensor 5 times and averages the results,
// which smooths out electrical noise so the reading is more reliable.
// ADC = "Analog to Digital Converter" - it turns a real-world voltage
// into a plain number the microcontroller can do math with.
int Get_Battery_Voltage_ADC(void) {
  pinMode(PIN_BATTERY, INPUT);
  int batteryADC = 0;
  for (int i = 0; i < 5; i++)
    batteryADC += analogRead(PIN_BATTERY);
  return batteryADC / 5;
}

// Converts the raw ADC number into an actual voltage (in volts), using a
// scaling formula tuned for this board's voltage-divider circuit.
float Get_Battery_Voltage(void) {
  int batteryADC = Get_Battery_Voltage_ADC();
  batteryVoltage = (batteryADC / 1023.0 * 3.67) * batteryCoefficient;
  return batteryVoltage;
}

// Lets other code fine-tune the battery voltage scaling factor.
void Set_Battery_Coefficient(float coefficient) {
  batteryCoefficient = coefficient;
}

/////////////////////Photosensitive drive area//////////////////////////
// Prepares the light-sensor pin for reading (not used in this sketch).
void Photosensitive_Setup(void) {
  pinMode(PHOTOSENSITIVE_PIN, INPUT);
}

// Reads how much light is hitting the photoresistor - higher/lower
// numbers mean more/less light, depending on the sensor's wiring.
int Get_Photosensitive(void) {
  int photosensitiveADC = analogRead(PHOTOSENSITIVE_PIN);
  return photosensitiveADC;
}

/////////////////////Ultrasonic drive area//////////////////////////////
// Prepares the ultrasonic distance sensor's two pins: one that sends a
// sound pulse (Trig) and one that listens for its echo (Echo).
void Ultrasonic_Setup(void) {
  pinMode(PIN_SONIC_TRIG, OUTPUT);  // set trigPin to output mode
  pinMode(PIN_SONIC_ECHO, INPUT);   // set echoPin to input mode
}

// Measures distance to the nearest object by "shouting" an ultrasonic
// pulse and timing how long it takes the echo to bounce back - the same
// idea bats and submarines use (sonar). Not used in this line-following
// sketch, but shared with other lessons that need obstacle detection.
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
    distance = MAX_DISTANCE;
  return distance;  // return the distance value
}

/////////////////////Track drive area//////////////////////////////
// sensorValue[0..2] = raw left/center/right readings (each 0 or 1).
// sensorValue[3]    = all three combined into one number from 0-7 -
//                     this is what the .ino's switch/case reads.
unsigned char sensorValue[4];  //define an array

// Prepares the 3 line-tracking sensor pins as INPUTs, so we can read
// digital HIGH/LOW signals from them. Called once from setup().
void Track_Setup(void) {
  pinMode(PIN_TRACKING_LEFT, INPUT);    //
  pinMode(PIN_TRACKING_RIGHT, INPUT);   //
  pinMode(PIN_TRACKING_CENTER, INPUT);  //
}

// Reads the 3 line sensors (LOOK step of the line-follower) and packs
// them into a single 3-bit number for easy decision-making.
//
// Each sensor gives 0 or 1. We place them into specific "bit slots" of a
// binary number using << (shift left) and | (bitwise OR):
//   sensorValue[0] << 2   -> left sensor becomes the BIGGEST bit  (worth 4)
//   sensorValue[1] << 1   -> center sensor becomes the MIDDLE bit (worth 2)
//   sensorValue[2]        -> right sensor becomes the SMALLEST bit (worth 1)
// OR-ing them together combines the three bits into one number, e.g. if
// left=1, center=0, right=0, the result is 4 (binary 100). If only the
// center sensor is 1, the result is 2 (binary 010). This is exactly the
// number the switch/case in the .ino sketch reads as sensorValue[3].
void Track_Read(void) {
  sensorValue[0] = digitalRead(PIN_TRACKING_LEFT);
  sensorValue[1] = digitalRead(PIN_TRACKING_CENTER);
  sensorValue[2] = digitalRead(PIN_TRACKING_RIGHT);
  sensorValue[3] = sensorValue[0] << 2 | sensorValue[1] << 1 | sensorValue[2];
}

//////////////////Emotion drive area////////////////////////////////
// This section drives the small 8x8 LED "face" on top of the car. All the
// picture data it displays (eyes, arrows, wheels, etc.) lives in Array.h.
//
// These animation functions are all "non-blocking": instead of using
// delay() (which would freeze the whole program and stop the car from
// reacting to the line), they check "has enough time passed since the
// last frame?" using millis() (the number of milliseconds since the
// Pico turned on). That way loop() can call these every single pass
// without ever pausing the robot.

Freenove_VK16K33 matrix = Freenove_VK16K33();
int time_before = 0;  //Record each non-blocking time
int time_count = 0;   //Record the number of non-blocking times
int time_flag = 0;    //Record the blink time

// Starts up the LED matrix display over I2C and resets the animation timer.
// Called once from setup().
void Emotion_Setup() {
  matrix.init(EMOTION_ADDRESS);
  time_before = millis();
}

// Plays the "rotating eyes" animation, cycling one frame every delay_ms
// milliseconds. Not used by this line-following sketch.
void eyesRotate(int delay_ms) {
  int count = sizeof(eyeRotate1) / sizeof(eyeRotate1[0]);
  if (millis() - time_before >= delay_ms) {
    matrix.showStaticArray(eyeRotate1[time_count], eyeRotate2[time_count]);
    time_before = millis();
    time_count++;
    if (time_count >= count)
      time_count = 0;
  }
}

// Plays a slow blinking-eyes animation. Not used by this line-following
// sketch (it uses the faster eyesBlink1 instead).
void eyesBlink(int delay_ms) {
  int count = sizeof(eyeBlink) / sizeof(eyeBlink[0]);
  if (millis() - time_before >= delay_ms) {
    time_before = millis();
    time_count++;
    if (time_count >= 25) {
      time_count = 0;
      time_flag = 1;
    }
    if (time_flag == 0)
      matrix.showStaticArray(eyeBlink[0], eyeBlink[0]);
    else if (time_flag == 1) {
      matrix.showStaticArray(eyeBlink[time_count], eyeBlink[time_count]);
      if (time_count >= (count - 1)) {
        time_flag = 0;
        time_count = 0;
      }
    }
  }
}

// Plays a smiling-eyes animation. Not used by this line-following sketch.
void eyesSmile(int delay_ms) {
  int count = sizeof(eyeSmile) / sizeof(eyeSmile[0]);
  if (millis() - time_before >= delay_ms) {
    matrix.showStaticArray(eyeSmile[time_count], eyeSmile[time_count]);
    time_before = millis();
    time_count++;
    if (time_count >= count)
      time_count = 0;
  }
}

// Plays a crying-eyes animation. Not used by this line-following sketch.
void eyesCry(int delay_ms) {
  int count = sizeof(eyeCry1) / sizeof(eyeCry1[0]);
  if (millis() - time_before >= delay_ms) {
    matrix.showStaticArray(eyeCry1[time_count], eyeCry2[time_count]);
    time_before = millis();
    time_count++;
    if (time_count >= count)
      time_count = 0;
  }
}

// Plays a quick double-blink animation - this is the ACT step used when
// the switch/case in the .ino detects the "all sensors see the line"
// (case 7, the stop/finish marker), giving the car a happy "we made it!"
// expression while it stops.
void eyesBlink1(int delay_ms) {
  int count = sizeof(eyeBlink1) / sizeof(eyeBlink1[0]);
  if (millis() - time_before >= delay_ms) {
    time_before = millis();
    time_count++;
    if (time_count >= 15) {
      time_count = 0;
      time_flag = 1;
    }
    if (time_flag == 0)
      matrix.showStaticArray(eyeBlink1[0], eyeBlink1[0]);
    else if (time_flag == 1) {
      matrix.showStaticArray(eyeBlink1[time_count], eyeBlink1[time_count]);
      if (time_count >= (count - 1)) {
        time_flag = 0;
        time_count = 0;
      }
    }
  }
}

// Shows an animated arrow pointing in one of 4 directions (1=up, 2=down,
// 3=left, 4=right), sliding it across the LED matrix a bit further each
// call. This sketch calls showArrow(1, ...) whenever it's driving
// straight, so the arrow "flies" upward to mean "going forward".
void showArrow(int arrow_direction, int delay_ms) {
  if (arrow_direction == 1) {
    if (millis() - time_before >= delay_ms) {
      matrix.showLedMatrix(arrow_up, 4, time_count - 8);
      time_before = millis();
      time_count++;
      if (time_count > 16)
        time_count = 0;
    }
  } else if (arrow_direction == 2) {
    if (millis() - time_before >= delay_ms) {
      matrix.showLedMatrix(arrow_dowm, 4, 8 - time_count);
      time_before = millis();
      time_count++;
      if (time_count > 16)
        time_count = 0;
    }
  } else if (arrow_direction == 3) {
    if (millis() - time_before >= delay_ms) {
      matrix.showLedMatrix(arrow_left, 8 - time_count, 0);
      time_before = millis();
      time_count++;
      if (time_count > 8)
        time_count = 0;
    }
  } else if (arrow_direction == 4) {
    if (millis() - time_before >= delay_ms) {
      matrix.showLedMatrix(arrow_right, time_count, 0);
      time_before = millis();
      time_count++;
      if (time_count > 8)
        time_count = 0;
    }
  } else
    matrix.clear();
}

// Shows a spinning "wheel" icon on the LED matrix: mode 1 spins it as if
// turning right, mode 2 as if turning left. The .ino calls wheel(2, ...)
// when steering left and wheel(1, ...) when steering right, so the face
// gives a visual hint of which way the car is currently correcting.
void wheel(int mode, int delay_ms) {
  if (mode == 1) {
    int count = sizeof(wheel_left) / sizeof(wheel_left[0]);
    if (millis() - time_before >= delay_ms) {
      matrix.showStaticArray(wheel_left[time_count], wheel_left[time_count]);
      time_before = millis();
      time_count++;
      if (time_count >= count)
        time_count = 0;
    }
  } else if (mode == 2) {
    int count = sizeof(wheel_right) / sizeof(wheel_right[0]);
    if (millis() - time_before >= delay_ms) {
      matrix.showStaticArray(wheel_right[time_count], wheel_right[time_count]);
      time_before = millis();
      time_count++;
      if (time_count >= count)
        time_count = 0;
    }
  } else
    matrix.clear();
}

// Shows a little car icon sliding left (mode 1) or right (mode 2) across
// the LED matrix. Not used by this line-following sketch.
void carMove(int mode, int delay_ms) {
  if (mode == 1) {
    if (millis() - time_before >= delay_ms) {
      matrix.showLedMatrix(car_left, 8 - time_count, 0);
      time_before = millis();
      time_count++;
      if (time_count >= 8)
        time_count = 0;
    }
  } else if (mode == 2) {
    if (millis() - time_before >= delay_ms) {
      matrix.showLedMatrix(car_right, time_count, 0);
      time_before = millis();
      time_count++;
      if (time_count >= 8)
        time_count = 0;
    }
  } else
    matrix.clear();
}

// Shows a heart-shaped "I love you" icon on the LED matrix. Not used by
// this line-following sketch.
void expressingLove(int delay_ms) {
  int count = sizeof(I_love_you) / sizeof(I_love_you[0]);
  if (millis() - time_before >= delay_ms) {
    matrix.showStaticArray(I_love_you[0], I_love_you[1]);
    time_before = millis();
    time_count++;
    if (time_count >= count)
      time_count = 0;
  }
}

// Shows a "save water" reminder icon on the LED matrix. Not used by
// this line-following sketch.
void saveWater(int delay_ms) {
  int count = sizeof(save_water_left) / sizeof(save_water_left[0]);
  if (millis() - time_before >= delay_ms) {
    matrix.showStaticArray(save_water_left[time_count], save_water_right[time_count]);
    time_before = millis();
    time_count++;
    if (time_count >= count)
      time_count = 0;
  }
}
