#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"

/////////////////////Servo drive area///////////////////////////////////
uint32_t Servo_Pins[] = { PIN_SERVO1 };
#define NUM_OF_ServoPINS (sizeof(Servo_Pins) / sizeof(uint32_t))  //How many servos we're controlling (just 1 here)
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f };  //Starting duty cycle (signal "on" percentage) for each servo
float freq1[] = { 50.0f };                      //PWM signal frequency servos expect (50 Hz is standard for hobby servos)
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];   //Holds the PWM controller object for each servo

int servo_1_offset = 0;  //Small correction angle if servo 1 isn't perfectly centered

// Sets up the PWM (Pulse Width Modulation) hardware that talks to the servo.
// PWM is how the Pico sends "go to this angle" signals to a servo motor.
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

// Turns servo 1 to face a given angle.
// angle: desired direction in degrees. Even though a full circle is 0-180,
// this servo is limited (constrained) to 30-150 degrees so the sensor arm
// physically doesn't crash into the car's body.
void Servo_1_Angle(float angle) {
  angle = constrain(angle, 30, 150);                          // Clamp the angle into the safe 30-150 range
  angle = map(angle, 0.0f, 180.0f, 2500.0f, 12500.0f);         // Convert the angle into the pulse-width number the servo hardware expects
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle / 1000.0f);  // Send that signal to the servo
}

// Stores a small correction angle (offset) for servo 1, in case it doesn't
// sit perfectly straight when it's supposed to be centered.
void Set_Servo_1_Offset(int offset) {
  servo_1_offset = offset;
}

// Smoothly rotates a servo one degree at a time from angle_start to angle_end,
// like a slow-motion head turn. (Not used by the obstacle-avoidance logic below,
// but handy for testing the servo.)
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
#define NUM_OF_PINS (sizeof(PWM_Pins) / sizeof(uint32_t))  //Total number of motor pins (8: forward+backward pin for each of the 4 wheels)
float dutyCycle2[NUM_OF_PINS] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };  //All motors start switched off (0%)
float freq2[] = { 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f };  //PWM frequency used to control motor speed
RP2040_PWM* PWM_Instance[NUM_OF_PINS];  //Holds the PWM controller object for each motor pin

// Sets up the PWM hardware for all 8 motor pins so we can control wheel speed later.
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

// The lowest-level motor function: directly sets the speed of each of the 4
// wheels (m1 = front-left, m2 = rear-left, m3 = front-right, m4 = rear-right).
// Speed range is -100 (full reverse) to 100 (full forward), 0 = stopped.
// This is where the "forward/backward pin pair" trick happens: for each wheel,
// whichever direction we want, we turn ON the matching pin at the given speed
// and make sure the OTHER pin (the opposite direction) is OFF (0).
void Motor_Move_Init(int m1_speed, int m2_speed, int m3_speed, int m4_speed) {
  float frequency = 500;
  // Clamp all speeds so nobody can accidentally ask for more than full speed
  m1_speed = constrain(m1_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  m2_speed = constrain(m2_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  m3_speed = constrain(m3_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  m4_speed = constrain(m4_speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
  if (m1_speed >= 0) {                                              // Positive (or zero) speed means "spin forward"
    PWM_Instance[0]->setPWM(PIN_MOTOR_PWM_LEFT1, frequency, m1_speed);  // Drive the "forward" pin at this speed
    PWM_Instance[1]->setPWM(PIN_MOTOR_PWM_LEFT2, frequency, 0);         // Make sure the "backward" pin is off
  } else {                                                          // Negative speed means "spin backward"
    m1_speed = -m1_speed;                                           // Flip the sign so it's a positive speed value again
    PWM_Instance[1]->setPWM(PIN_MOTOR_PWM_LEFT2, frequency, m1_speed);  // Drive the "backward" pin instead
    PWM_Instance[0]->setPWM(PIN_MOTOR_PWM_LEFT1, frequency, 0);         // Make sure the "forward" pin is off
  }
  if (m2_speed >= 0) {                                              // Same forward/backward-pin trick as above, now for wheel 2
    PWM_Instance[2]->setPWM(PIN_MOTOR_PWM_LEFT3, frequency, m2_speed);
    PWM_Instance[3]->setPWM(PIN_MOTOR_PWM_LEFT4, frequency, 0);
  } else {
    m2_speed = -m2_speed;
    PWM_Instance[3]->setPWM(PIN_MOTOR_PWM_LEFT4, frequency, m2_speed);
    PWM_Instance[2]->setPWM(PIN_MOTOR_PWM_LEFT3, frequency, 0);
  }
  if (m3_speed >= 0) {                                              // ...and wheel 3
    PWM_Instance[4]->setPWM(PIN_MOTOR_PWM_RIGHT1, frequency, m3_speed);
    PWM_Instance[5]->setPWM(PIN_MOTOR_PWM_RIGHT2, frequency, 0);
  } else {
    m3_speed = -m3_speed;
    PWM_Instance[5]->setPWM(PIN_MOTOR_PWM_RIGHT2, frequency, m3_speed);
    PWM_Instance[4]->setPWM(PIN_MOTOR_PWM_RIGHT1, frequency, 0);
  }
  if (m4_speed >= 0) {                                              // ...and wheel 4
    PWM_Instance[6]->setPWM(PIN_MOTOR_PWM_RIGHT3, frequency, m4_speed);
    PWM_Instance[7]->setPWM(PIN_MOTOR_PWM_RIGHT4, frequency, 0);
  } else {
    m4_speed = -m4_speed;
    PWM_Instance[7]->setPWM(PIN_MOTOR_PWM_RIGHT4, frequency, m4_speed);
    PWM_Instance[6]->setPWM(PIN_MOTOR_PWM_RIGHT3, frequency, 0);
  }
}

// Simple "tank style" driving: give one speed to both LEFT wheels and one
// speed to both RIGHT wheels. Equal speeds = drive straight. Different
// speeds (or opposite signs) = turn or spin. This sketch mostly uses the
// fancier Motor_M_Move() below instead, which controls all 4 wheels separately.
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

// Mecanum-wheel driving: lets each of the 4 wheels get its own speed.
// This is what makes the special "spin in place" and "escape" moves in
// Ultrasonic_Car() possible - e.g. left wheels backward + right wheels forward
// makes the car pivot/turn on the spot instead of just going straight.
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
// Gets the buzzer pin ready to use (not used by the obstacle-avoidance logic
// in this particular sketch, but available if you want the car to beep).
void Buzzer_Setup(void) {
  pinMode(PIN_BUZZER, OUTPUT);
}

// Beeps the buzzer in a pattern: "beat" short beeps in a row, then a pause,
// repeated "rebeat" times total.
void Buzzer_Alert(int beat, int rebeat) {
  beat = constrain(beat, 1, 9);        // Limit how many beeps per burst (1-9)
  rebeat = constrain(rebeat, 1, 255);  // Limit how many times the whole pattern repeats
  for (int j = 0; j < rebeat; j++) {
    for (int i = 0; i < beat; i++) {
      freq(PIN_BUZZER, BUZZER_FREQUENCY, 30);
    }
    delay(500);
  }
  freq(PIN_BUZZER, 0, 30);
}

// Low-level helper that makes a pin buzz at a chosen frequency (freqs, in Hz)
// for a chosen length of time (times). Passing freqs = 0 just turns it off.
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
// These battery functions aren't directly called in this sketch's loop, but
// oa_VoltageCompensationToSpeed (calculated here) is used inside Ultrasonic_Car()
// to nudge the driving speed up as the battery voltage drops, so the car
// doesn't get slower and slower as the battery runs down.
float batteryVoltage = 0;      //Last measured battery voltage, in volts
float batteryCoefficient = 4;  //Scaling number used to convert the raw sensor reading into real volts
int oa_VoltageCompensationToSpeed;  //Extra speed to add to motor commands to compensate for a low battery

// Reads the battery voltage sensor pin 5 times and averages the results,
// to smooth out any noisy/jumpy readings. Returns a raw sensor value (not volts yet).
int Get_Battery_Voltage_ADC(void) {
  pinMode(PIN_BATTERY, INPUT);
  int batteryADC = 0;
  for (int i = 0; i < 5; i++)
    batteryADC += analogRead(PIN_BATTERY);
  return batteryADC / 5;
}

// Converts the raw battery sensor reading into an actual voltage value and
// returns it (also stores it in the global batteryVoltage variable).
float Get_Battery_Voltage(void) {
  int batteryADC = Get_Battery_Voltage_ADC();
  batteryVoltage = (batteryADC / 1023.0 * 3.3) * batteryCoefficient;
  return batteryVoltage;
}

// Lets you change the scaling number used when converting sensor readings to volts,
// in case a particular board's battery sensor needs calibrating.
void Set_Battery_Coefficient(float coefficient) {
  batteryCoefficient = coefficient;
}

// Measures the battery voltage and works out how much extra motor speed
// (oa_VoltageCompensationToSpeed) is needed to make up for it being below
// the "standard" full-battery voltage. A lower battery -> a bigger speed boost.
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


/////////////////////Ultrasonic drive area//////////////////////////////
// Gets the ultrasonic sensor's two pins (trigger + echo) ready to use.
void Ultrasonic_Setup(void) {
  pinMode(PIN_SONIC_TRIG, OUTPUT);  // set trigPin to output mode
  pinMode(PIN_SONIC_ECHO, INPUT);   // set echoPin to input mode
}

// Takes ONE distance measurement and returns it in centimetres.
// How it works (just like a bat using sonar to "see" in the dark):
//  1. Send a short (10 microsecond) sound pulse out of the TRIG pin.
//  2. Wait for that sound to bounce off something and come back to the ECHO pin.
//  3. Measure how long that round trip took.
//  4. Since we know how fast sound travels, time -> distance.
// If no echo comes back within SONIC_TIMEOUT (nothing in range), we just
// report MAX_DISTANCE, meaning "as far as we can tell, nothing's there."
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
    distance = MAX_DISTANCE;  // No echo detected in time -> assume nothing is close (treat as "far away")
  return distance;  // return the distance value
}

// THE MAIN BEHAVIOUR: look around, decide, then drive.
// Every time this function runs (once per loop() call) it:
//   1. Swivels the sensor to 3 positions (left, straight ahead, right) and
//      measures the distance to whatever is in each direction.
//   2. Uses if/else decision-making to pick ONE of several actions based on
//      how close those 3 distances are:
//        - path clear ahead            -> keep cruising forward
//        - something close on one side -> nudge/steer away from it
//        - something blocking ahead    -> STOP, back up, spin toward the
//                                          clearer side, then carry on
//   3. Sends the chosen wheel speeds to the motors via Motor_M_Move().
// This is the "sensor -> decision -> action" loop that makes the robot behave
// like it's making its own choices, instead of just following a fixed path.
void Ultrasonic_Car() {
  int distance[3], tempDistance[3][5], sumDisntance;
  // "static" means these variables keep their value between calls, instead of
  // resetting each time - so the car remembers which way it scanned last time.
  static u8 leftToRight = 0, servoAngle = 0, lastServoAngle = 0;
  // The 3 servo angles to check, in order. We alternate scan direction each
  // time (left-to-right, then right-to-left) so the sensor doesn't have to
  // snap all the way back to the start every time - it's a smoother sweep.
  const u8 scanAngle[2][3] = { { 150, 90, 30 }, { 30, 90, 150 } };
  int speedOffset = oa_VoltageCompensationToSpeed;  // Extra speed to make up for a lower battery
  // --- STEP 1: Scan 3 directions and record the distance seen in each ---
  for (int i = 0; i < 3; i++) {
    servoAngle = scanAngle[leftToRight][i];
    Servo_1_Angle(servoAngle);          // Point the sensor at this angle
    if (lastServoAngle != servoAngle) {
      delay(100);                       // Give the servo time to physically get there before measuring
    }
    lastServoAngle = servoAngle;
    for (int j = 0; j < COUNT_GET_SONAR; j++) {
      tempDistance[i][j] = Get_Sonar();       // Take a distance reading
      delayMicroseconds(2 * SONIC_TIMEOUT);   // Pause briefly so echoes don't overlap between readings
      sumDisntance += tempDistance[i][j];
    }
    // distance[] is always stored in the same order: [0]=left, [1]=centre, [2]=right,
    // no matter which direction we physically scanned in this time.
    if (leftToRight == 0) {
      distance[i] = sumDisntance / COUNT_GET_SONAR;
    } else {
      distance[2 - i] = sumDisntance / COUNT_GET_SONAR;
    }
    sumDisntance = 0;
  }
  leftToRight = (leftToRight + 1) % 2;  // Flip scan direction for next time (0 -> 1 -> 0 -> ...)
  Serial.println("Sonar : " + String(distance[0]) + " " + String(distance[1]) + " " + String(distance[2]));

  // --- STEP 2: Decide what to do, based on the 3 distances just measured ---
  // distance[0] = left, distance[1] = straight ahead (centre), distance[2] = right.
  if (distance[1] < OBSTACLE_DISTANCE) {  //Too little distance ahead: something is blocking our path!
    // Compare the left and right readings to work out which side has more
    // room, and escape that way. The "escape recipe" is always the same 2 steps:
    //   1) reverse (back away from the obstacle)
    //   2) spin on the spot toward the clearer side
    if (distance[0] > distance[2] && distance[0] > OBSTACLE_DISTANCE) {                                //Left distance is greater than right distance
      Motor_M_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Move back
      delay(100);
      Motor_M_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset) ,(SONAR_MODE_CRUISE_SPEED + speedOffset),(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Left wheels keep reversing, right wheels go forward -> car spins toward the more open left side
    } else if (distance[0] < distance[2] && distance[2] > OBSTACLE_DISTANCE) {                         //Right distance is greater than left distance
      Motor_M_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Move back
      delay(100);
      Motor_M_Move((SONAR_MODE_CRUISE_SPEED + speedOffset), (SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Right wheels keep reversing, left wheels go forward -> car spins toward the more open right side
    } else {  //Get into the dead corner, move back, then turn. (Boxed in on both sides - just pick a turn direction and back out anyway.)
      Motor_M_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Move back
      delay(100);
      Motor_M_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset),(SONAR_MODE_CRUISE_SPEED + speedOffset),(SONAR_MODE_CRUISE_SPEED + speedOffset));
    }
  } else {                                                                                             //No obstacles ahead: the centre path is clear, so keep going - just check the sides for close calls
    if (distance[0] < OBSTACLE_DISTANCE_LOW) {                                                         //Obstacles on the left front.
      Motor_M_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Move back
      delay(100);
      Motor_M_Move((70 + speedOffset),(70 + speedOffset), (20 + speedOffset), (20 + speedOffset));  //Drive the left wheels faster than the right ones, so the car curves away to the right, clear of the obstacle
    } else if (distance[2] < OBSTACLE_DISTANCE_LOW) {                                                  //Obstacles on the right front.
      Motor_M_Move(-(SONAR_MODE_CRUISE_SPEED + speedOffset), -(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset),-(SONAR_MODE_CRUISE_SPEED + speedOffset));  //Move back
      delay(100);
      Motor_M_Move((20 + speedOffset),(20 + speedOffset), (70 + speedOffset), (70 + speedOffset));  //Drive the right wheels faster than the left ones, so the car curves away to the left, clear of the obstacle
    } else {  //Cruising: nothing close on any side - drive straight ahead at normal speed
      Motor_M_Move((30 + speedOffset),(30 + speedOffset), (30 + speedOffset), (30 + speedOffset));
    }
  }
}