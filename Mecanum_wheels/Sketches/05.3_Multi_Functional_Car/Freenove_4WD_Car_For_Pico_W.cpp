#include <Arduino.h>
#include <Wire.h>
#include "Freenove_4WD_Car_Emotion.h"
#include "Freenove_4WD_Car_For_Pico_W.h"

// ============================================================================
// THIS FILE: all the low-level hardware control for the car itself. It's
// organised into labelled sections (search for the //// banners): servo,
// motors, buzzer, battery, light sensors, ultrasonic sensor, line-tracking
// sensors, and finally the "which mode is the car in" selector logic that
// ties several of those together. Comments below focus on the functions the
// main .ino sketch actually calls.
// ============================================================================

/////////////////////Servo drive area///////////////////////////////////
uint32_t Servo_Pins[] = { PIN_SERVO1 };
#define NUM_OF_ServoPINS (sizeof(Servo_Pins) / sizeof(uint32_t))
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f };
float freq1[] = { 50.0f };
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];  // Objects that generate the PWM signal steering each servo

int servo_1_offset = 0;  //Define the offset variable for servo 1

// Servo_Setup: creates a PWM (Pulse Width Modulation) generator for the head
// servo. PWM is how you tell a servo motor what angle to turn to - it's a
// signal that switches on/off very fast, and the fraction of time it's "on"
// tells the servo where to point.
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

// Servo_1_Angle: points the head servo to a given angle in degrees. First
// it clamps the angle to a safe range (30-150, protecting the servo from
// being asked to over-rotate), then converts degrees into the PWM pulse
// width (in microseconds) that the servo actually expects, using map() to
// rescale from the 0-180 degree range to a 2500-12500 microsecond range.
void Servo_1_Angle(float angle) {
  angle = constrain(angle, 30, 150);
  angle = map(angle, 0.0f, 180.0f, 2500.0f, 12500.0f);
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle / 1000.0f);
}

// Set_Servo_1_Offset: stores a small correction angle for servo 1, useful if
// the servo horn isn't mounted perfectly straight.
void Set_Servo_1_Offset(int offset) {
  servo_1_offset = offset;
}

// Servo_Sweep: smoothly moves a servo from angle_start to angle_end, one
// degree at a time with a short delay between steps, instead of snapping
// straight there. Only servo_id 1 exists on this car.
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
// Motor_Setup: creates a PWM generator for each of the 8 motor-driver pins
// (2 pins per wheel: one for "spin this way", one for "spin the other way").
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

// Motor_Move_Init: the lowest-level motor function - takes a speed for
// each of the 4 wheels (-100 to 100, negative = reverse, positive =
// forward) and turns each one into the correct pair of PWM signals. Each
// wheel is driven by 2 pins: to go forward, one pin gets the PWM signal and
// the other gets 0; to go backward, it's the other way around. This is how
// simple DC motor "H-bridge" style drivers work.
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
// Motor_Move: a simpler "tank steering" helper - just give it a left-side
// speed and a right-side speed, and it copies that to the front+back wheels
// on each side (so all left wheels match, all right wheels match). The
// #ifdef blocks flip a wheel's sign if you've enabled a REVERSE_MOTORx
// #define up in the header, without needing to rewire anything.
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
// Motor_M_Move: the "full control" helper - lets you set all 4 wheels to
// independent speeds directly (this is what makes the mecanum-wheel sideways
// and rotating moves in handleControl() in the main .ino possible, since
// mecanum wheels can move diagonally/sideways when wheels spin in different
// combinations, not just forward/backward like normal wheels).
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
// Buzzer_Setup: prepares the buzzer pin for output so we can drive it.
void Buzzer_Setup(void) {
  pinMode(PIN_BUZZER, OUTPUT);
}

// Buzzer_Variable: beeps the buzzer `times` times, at the given `frequency`
// (pitch, in Hz) for `time` milliseconds each beep, with an equal pause of
// silence between beeps. tone() is an Arduino function that generates a
// square-wave sound at a given pitch on a pin.
void Buzzer_Variable(int frequency, int time, int times) {
  for (int i = 0; i < times; i++) {
    tone(2, frequency);
    delay(time);
    tone(2, 0);
    delay(time);
  }
}

// Buzzer_Alarm: a simple on/off alarm tone using freq() below instead of
// tone() - turning `enable` on plays a beep-pause-beep-pause pattern once;
// turning it off makes sure the buzzer is silent.
void Buzzer_Alarm(bool enable) {
  if (enable == 1) {
    freq(PIN_BUZZER, 2000, 30);
    delay(500);
    freq(PIN_BUZZER, 0, 30);
    delay(500);
  } else
    freq(PIN_BUZZER, 0, 30);
}

//Buzzer alarm function
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

// freq: a hand-written alternative to tone() that manually toggles the pin
// HIGH/LOW at the right rate to produce a sound wave at `freqs` Hz, for
// `times` milliseconds worth of on/off toggling. digitalWrite HIGH then LOW
// with a calculated delay between them is literally how you build a square
// wave "from scratch" if you don't want to use the built-in tone() helper.
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
// The car measures its own battery voltage, and when the battery is
// getting low (voltage droops below a standard value) it gives the motors
// a small speed boost to compensate, so the car doesn't visibly slow down
// as the battery drains.
float batteryVoltage = 0;      //Battery voltage variable
float batteryCoefficient = 4;  //Set the proportional coefficient
int oa_VoltageCompensationToSpeed;  // Extra speed to add to counter a low battery

// Get_Battery_Voltage_ADC: reads the raw analog-to-digital (ADC) value from
// the battery-sensing pin 5 times and averages them, to smooth out
// electrical noise in the reading.
int Get_Battery_Voltage_ADC(void) {
  pinMode(PIN_BATTERY, INPUT);
  int batteryADC = 0;
  for (int i = 0; i < 5; i++)
    batteryADC += analogRead(PIN_BATTERY);
  return batteryADC / 5;
}

// Get_Battery_Voltage: converts the raw ADC reading into an actual voltage
// number, by scaling it against the ADC's reference range (0-1023 steps =
// 0-3.3 volts) and then correcting for the voltage-divider circuit using
// batteryCoefficient.
float Get_Battery_Voltage(void) {
  int batteryADC = Get_Battery_Voltage_ADC();
  batteryVoltage = (batteryADC / 1023.0 * 3.3) * batteryCoefficient;
  return batteryVoltage;
}

// oa_CalculateVoltageCompensation: compares the current battery voltage
// against BAT_VOL_STANDARD (a fully-charged reference voltage) and works
// out how much extra motor speed to add to make up for a drooping battery.
void oa_CalculateVoltageCompensation() {
  Get_Battery_Voltage();
  float voltageOffset = BAT_VOL_STANDARD - batteryVoltage;
  oa_VoltageCompensationToSpeed = voltageOffset * OA_SPEED_OFFSET_PER_V;
  // Serial.print(batteryVoltage);
  // Serial.print('\t');
  // Serial.print(voltageOffset);
  // Serial.print('\t');
  // Serial.println(oa_VoltageCompensationToSpeed);
}

// Set_Battery_Coefficient: lets you override the voltage-divider correction
// factor used when converting raw ADC readings into real voltage.
void Set_Battery_Coefficient(float coefficient) {
  batteryCoefficient = coefficient;
}

/////////////////////Photosensitive drive area//////////////////////////
int light_init_value = 0;  //Set the car's initial environment ADC value
// Photosensitive_Setup: prepares the two light-sensor pins for reading.
void Photosensitive_Setup(void) {
  pinMode(Left_PHOTOSENSITIVE_PIN, INPUT);
  pinMode(Right_PHOTOSENSITIVE_PIN, INPUT);
}

// getLeftPhotosensitiveADCValue / getRightPhotosensitiveADCValue: read the
// brightness level seen by the left/right light sensor. Higher ADC values
// generally mean less light reaching that sensor (depends on the sensor's
// wiring) - Light_Car() below compares the two to know which way to steer.
int getLeftPhotosensitiveADCValue(void) {
  int photosensitiveADCValue = analogRead(Left_PHOTOSENSITIVE_PIN);
  return photosensitiveADCValue;
}
int getRightPhotosensitiveADCValue(void) {
  int photosensitiveADCValue = analogRead(Right_PHOTOSENSITIVE_PIN);
  return photosensitiveADCValue;
}

#define LIGHT_MIN_MOVED (50 + oa_VoltageCompensationToSpeed)
#define LIGHT_MODE_CRUISE_SPEED (25 + oa_VoltageCompensationToSpeed)  //0-100
bool isLightModeFirstStarting = true;                                 //is_light_mode_first_starting
// Light_Car: "light tracing" driving mode. Compares the two light sensor
// readings - if the right side sees more light than the left, it speeds up
// the left wheels slightly (and vice versa) so the car curves towards the
// brighter side, like a moth heading for a lamp. If it's too dark on both
// sides it just stops. isLightModeFirstStarting gives it one small forward
// nudge the very first time this mode is switched on.
void Light_Car(int mode) {
  if (mode == 1) {
    if (isLightModeFirstStarting) {
      isLightModeFirstStarting = false;
      Motor_Move(40, 40);
      delay(200);
    }
    int leftLightValue = getLeftPhotosensitiveADCValue();
    int rightLightValue = getRightPhotosensitiveADCValue();
    int diffLightValue = (rightLightValue - leftLightValue) / 8;
    if (leftLightValue > LIGHT_MIN_MOVED && rightLightValue > LIGHT_MIN_MOVED) {
      int lsp = constrain(LIGHT_MODE_CRUISE_SPEED + diffLightValue, -100, 100);
      int rsp = constrain(LIGHT_MODE_CRUISE_SPEED - diffLightValue, -100, 100);
      Serial.println("sp: " + String(lsp) + " " + String(rsp) + " " + String(diffLightValue));
      Motor_Move(lsp, rsp);
    } else {
      Motor_Move(0, 0);
    }
  }
}

/////////////////////Ultrasonic drive area//////////////////////////////
// Ultrasonic_Setup: prepares the trigger pin (we send a pulse out on this)
// and echo pin (the sensor sends a pulse back on this) for the HC-SR04
// distance sensor.
void Ultrasonic_Setup(void) {
  pinMode(PIN_SONIC_TRIG, OUTPUT);  // set trigPin to output mode
  pinMode(PIN_SONIC_ECHO, INPUT);   // set echoPin to input mode
}

// Get_Sonar: measures distance using sound, like a bat! It sends a short
// ultrasonic "ping" out of the TRIG pin, then times how long it takes for
// the echo to bounce back and arrive on the ECHO pin. Since sound travels
// at a known speed (SOUND_VELOCITY), time-taken tells us distance: the
// pulse has to travel there AND back, so we divide by 2.
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
  Serial.print("Distance: " + String(distance) + "\n");  //Print ultrasonic distance
  return distance;                                       // return the distance value
}

/////////////////////Track drive area//////////////////////////////
#define SPEED_LV4 (80 + oa_VoltageCompensationToSpeed)
#define SPEED_LV3 (70 + oa_VoltageCompensationToSpeed)
#define SPEED_LV2 (60 + oa_VoltageCompensationToSpeed)
#define SPEED_LV1 (45 + oa_VoltageCompensationToSpeed)
unsigned char sensorValue[4];  //define an array: [0]=left,[1]=center,[2]=right,[3]=all three packed together

// Track_Setup: prepares the three line-tracking IR sensor pins for reading.
void Track_Setup(void) {
  pinMode(PIN_TRACKING_LEFT, INPUT);    //
  pinMode(PIN_TRACKING_RIGHT, INPUT);   //
  pinMode(PIN_TRACKING_CENTER, INPUT);  //
}

// Track_Read: reads each of the 3 line sensors (each gives 0 or 1 depending
// on whether it sees the dark line or the light floor), then packs all
// three readings into one number (sensorValue[3]) using bit-shifting, so
// Track_Car() can look them up with a single switch statement instead of
// checking 3 separate values. For example left=1,center=0,right=0 becomes
// binary 100 = 4.
void Track_Read(void) {
  sensorValue[0] = digitalRead(PIN_TRACKING_LEFT);
  sensorValue[1] = digitalRead(PIN_TRACKING_CENTER);
  sensorValue[2] = digitalRead(PIN_TRACKING_RIGHT);
  sensorValue[3] = sensorValue[0] << 2 | sensorValue[1] << 1 | sensorValue[2];
}

// Track_Car: "line tracking" driving mode. Looks at the packed 3-bit sensor
// reading to decide what to do: line under the middle sensor only -> drive
// straight; line under left sensor(s) -> curve left; line under right
// sensor(s) -> curve right; all sensors see line -> stop (probably at a
// junction/end); no sensors see line -> keep going straight and hope to
// find it again. It also updates the LED matrix face to match (happy going
// straight, sad/crying when the line is lost, wheel icons while turning).
void Track_Car(int mode) {
  if (mode == 1) {
    Track_Read();
    switch (sensorValue[3]) {  //white : 0  light , black: 1 ,
      case 2:                  //010
      case 5:
        Emotion_SetMode(3);
        Motor_Move(SPEED_LV1, SPEED_LV1);  //Move Forward
        break;
      case 7:              //111
        Motor_Move(0, 0);  //Stop
        break;
      case 0:  //000
        Emotion_SetMode(4);
        Motor_Move(SPEED_LV1, SPEED_LV1);  //Move Forward
        break;
      case 1:                               //001
      case 3:                               //011
        Emotion_SetMode(5);                 //Left
        Motor_Move(SPEED_LV3, -SPEED_LV4);  //Turn
        break;
      case 4:                               //100
      case 6:                               //110
        Emotion_SetMode(6);                 //Right
        Motor_Move(-SPEED_LV4, SPEED_LV3);  //Turn Right
        break;
      default:
        break;
    }
  }
}

// Ultrasonic_Car: "obstacle avoidance" driving mode. Swings the head servo
// to look left, centre and right (using the ultrasonic sensor as its
// "eyes"), measuring the distance to anything in front at each angle. Then:
//   - if something is close straight ahead, back up and turn towards
//     whichever side (left/right) has more open space,
//   - if something is close only on one side, back up then curve away from it,
//   - otherwise, cruise forward.
// Each call only does ONE left-center-right scan and then decides on one
// action, alternating which direction (left-to-right vs right-to-left) it
// scans each time it's called, so consecutive calls build up a full picture
// without ever pausing the whole program for long.
#define SONAR_MODE_CRUISE_SPEED (40 + oa_VoltageCompensationToSpeed)
typedef uint8_t u8;
#define COUNT_GET_SONAR 1
void Ultrasonic_Car() {
  int distance[3], tempDistance[3][5], sumDisntance;
  static u8 leftToRight = 0, servoAngle = 0, lastServoAngle = 0;  //
  const u8 scanAngle[2][3] = { { 150, 90, 30 }, { 30, 90, 150 } };
  int speedOffset = oa_VoltageCompensationToSpeed;
  for (int i = 0; i < 3; i++) {
    servoAngle = scanAngle[leftToRight][i];
    Servo_1_Angle(servoAngle);
    if (lastServoAngle != servoAngle) {
      delay(100);
    }
    lastServoAngle = servoAngle;
    for (int j = 0; j < COUNT_GET_SONAR; j++) {
      tempDistance[i][j] = Get_Sonar();
      delayMicroseconds(2 * SONIC_TIMEOUT);
      sumDisntance += tempDistance[i][j];
    }
    if (leftToRight == 0) {
      distance[i] = sumDisntance / COUNT_GET_SONAR;
    } else {
      distance[2 - i] = sumDisntance / COUNT_GET_SONAR;
    }
    sumDisntance = 0;
  }
  leftToRight = (leftToRight + 1) % 2;
  Serial.println("Sonar : " + String(distance[0]) + " " + String(distance[1]) + " " + String(distance[2]));
  if (distance[1] < OBSTACLE_DISTANCE) {                                                               //Too little distance ahead
    if (distance[0] > distance[2] && distance[0] > OBSTACLE_DISTANCE) {                                //Left distance is greater than right distance
      Motor_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Move back
      delay(100);
      Motor_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset), (SONAR_MODE_CRUISE_SPEED + speedOffset));
    } else if (distance[0] < distance[2] && distance[2] > OBSTACLE_DISTANCE) {                         //Right distance is greater than left distance
      Motor_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Move back
      delay(100);
      Motor_Move((SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset));
    } else {  //Get into the dead corner, move back, then turn.
      Motor_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset));
      delay(100);
      Motor_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset), (SONAR_MODE_CRUISE_SPEED + speedOffset));
    }
  } else {                                                                                             //No obstacles ahead
    if (distance[0] < OBSTACLE_DISTANCE_LOW) {                                                         //Obstacles on the left front.
      Motor_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Move back
      delay(100);
      Motor_Move((70 + speedOffset), (20 + speedOffset));
    } else if (distance[2] < OBSTACLE_DISTANCE_LOW) {                                                  //Obstacles on the right front.
      Motor_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Move back
      delay(100);
      Motor_Move((20 + speedOffset), (70 + speedOffset));
    } else {  //Cruising
      Motor_Move((30 + speedOffset), (30 + speedOffset));
    }
  }
}

//////////////////////Car drive area////////////////////////////////////////
int carFlag = 0;  // Which driving mode is currently active (see CAR_MODE_* constants)
// Car_SetMode: switches the active driving mode. The main .ino calls this
// whenever a mode-changing button is pressed on the remote.
void Car_SetMode(int mode) {
  carFlag = mode;
}

// Car_Select: called every loop() - looks at the current mode and hands
// control to the matching self-driving function. In CAR_MODE_MANUAL
// (mode 0) nothing happens here, because the remote control (handleControl
// in the .ino) is already driving the motors directly.
void Car_Select(int mode) {
  // Emotion_and_Ultrasonic_Setup();
  switch (mode) {
    case CAR_MODE_LIGHT_TRACING:
      Light_Car(1);
      break;
    case CAR_MODE_LINE_TRACKING:
      Track_Car(1);
      break;
    case CAR_MODE_SONAR:
      Ultrasonic_Car();
      break;
    case CAR_MODE_MANUAL:
    default:
      break;
  }
}
int Check_Module_value = 0;  // Which accessory is currently detected on the car's "head": MATRIX_IS_EXIST or SONAR_IS_ESIST

// i2CAddrTest: checks whether any chip is listening at I2C address `addr`
// by trying to start a conversation with it. I2C is a simple 2-wire
// protocol where each connected chip has its own address; if nothing
// answers, Wire.endTransmission() returns a non-zero error code instead
// of 0. This is how the car automatically figures out whether the LED
// matrix (which uses I2C) is plugged in, without you having to tell it.
bool i2CAddrTest(uint8_t addr) {
  Wire.begin();
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    return true;
  }
  return false;
}

int headModuleValue = 0;
int lastHeadModuleValue = 0;
// Emotion_and_Ultrasonic_Setup: called every loop() to detect which "head"
// accessory is currently attached - the LED matrix (found via I2C) or the
// ultrasonic sensor (assumed present if the matrix isn't found). If the
// detected accessory has changed since last time, it (re)initialises
// whichever one is now present. This lets you physically swap the matrix
// and ultrasonic sensor modules without reflashing the code.
void Emotion_and_Ultrasonic_Setup() {

  if (!i2CAddrTest(0x71)) {
    Check_Module_value = SONAR_IS_ESIST;
    headModuleValue = 1;
  } else {
    headModuleValue = 2;
    Check_Module_value = MATRIX_IS_EXIST;
  }
  if (headModuleValue != lastHeadModuleValue) {
    if (headModuleValue == 1) {  //
      Serial.print("\n Ultrasonic reInit.\n");
      Ultrasonic_Setup();
    } else if (headModuleValue == 2) {  //
      Serial.print("\n Emotion reInit.\n");
      delay(100);
      Emotion_Setup();
    }
    lastHeadModuleValue = headModuleValue;
  }
}