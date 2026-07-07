#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"
#include "Freenove_VK16K33_Lib.h"
#include "Array.h"
#include <Wire.h>

/////////////////////Servo drive area///////////////////////////////////
uint32_t Servo_Pins[] = { PIN_SERVO1 };
#define NUM_OF_ServoPINS  ( sizeof(Servo_Pins) / sizeof(uint32_t) )
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f};
float freq1[] = { 50.0f};
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];

int servo_1_offset=0; //Define the offset variable for servo 1

// Sets up the PWM (Pulse Width Modulation) hardware needed to control the
// servo motor. PWM is how we tell a servo "turn to this angle" - the Pico
// sends rapid on/off pulses, and the servo reads the pulse timing as an angle.
// Call this once, from setup(), before using Servo_1_Angle().
void Servo_Setup(void)
{
  for (uint8_t index = 0; index < NUM_OF_ServoPINS; index++)
  {   
    Servo_Instance[index] = new RP2040_PWM(Servo_Pins[index], freq1[index], dutyCycle1[index]);
    if (Servo_Instance[index])
    {
      Servo_Instance[index]->setPWM();
      uint32_t div = Servo_Instance[index]->get_DIV();
      uint32_t top = Servo_Instance[index]->get_TOP();
      PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", Servo_Instance[index]->get_freq_CPU());
    }
  }
}

// Turns servo 1 to a specific angle (0-180 degrees on paper, but this sketch
// clamps it to a safer 30-150 degree range with constrain() so it can't
// strain itself). The angle is then converted ("mapped") into a pulse-width
// number the PWM hardware understands, because servos don't speak "degrees".
void Servo_1_Angle(float angle)
{
  angle = constrain(angle, 30, 150);
  angle=map(angle,0.0f,180.0f,2500.0f,12500.0f);
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle/1000.0f);
}

// Stores a small correction ("offset") for servo 1, in case it doesn't sit
// perfectly straight when told to go to its middle angle. Not used by the
// light-tracking sketch, but kept here since it's shared code.
void Set_Servo_1_Offset(int offset)
{
  servo_1_offset=offset;
}

// Smoothly moves servo 1 from angle_start to angle_end, one degree at a time,
// with a short delay between each step - this makes the servo glide instead
// of snapping instantly to the new angle. Works whether the end angle is
// bigger or smaller than the start angle.
void Servo_Sweep(int servo_id, int angle_start, int angle_end)
{
  if (servo_id == 1)
  {
    angle_start = constrain(angle_start, 0, 180);
    angle_end = constrain(angle_end, 0, 180);
  }
  if (angle_start > angle_end)
  {
    for (int i = angle_start; i >= angle_end; i--)
    {
      if (servo_id == 1)
        Servo_1_Angle(i);
      delay(10);
    }
  }
  if (angle_start < angle_end)
  {
    for (int i = angle_start; i <= angle_end; i++)
    {
      if (servo_id == 1)
        Servo_1_Angle(i);
      delay(10);
    }
  }
}

/////////////////////Motor drive area///////////////////////////////////
uint32_t PWM_Pins[] = { PIN_MOTOR_PWM_RIGHT1, PIN_MOTOR_PWM_RIGHT2, PIN_MOTOR_PWM_RIGHT3, PIN_MOTOR_PWM_RIGHT4, PIN_MOTOR_PWM_LEFT1, PIN_MOTOR_PWM_LEFT2, PIN_MOTOR_PWM_LEFT3, PIN_MOTOR_PWM_LEFT4 };
#define NUM_OF_PINS  ( sizeof(PWM_Pins) / sizeof(uint32_t) )
float dutyCycle2[NUM_OF_PINS] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
float freq2[] = { 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f, 500.0f };
RP2040_PWM* PWM_Instance[NUM_OF_PINS];
// Sets up PWM control for all 8 motor pins (2 pins per wheel: one for
// "forward" and one for "backward"). Must run once in setup() before any
// other Motor_ function is called, or the wheels won't respond.
void Motor_Setup(void)
{
  for (uint8_t index = 0; index < NUM_OF_PINS; index++)
  {   
    PWM_Instance[index] = new RP2040_PWM(PWM_Pins[index], freq2[index], dutyCycle2[index]);
    if (PWM_Instance[index])
    {
      PWM_Instance[index]->setPWM();
      uint32_t div = PWM_Instance[index]->get_DIV();
      uint32_t top = PWM_Instance[index]->get_TOP();
      PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", PWM_Instance[index]->get_freq_CPU());
    }
  }
}

// The lowest-level motor function - it directly sets the speed of all 4
// wheel motors (m1-m4), each ranging from -100 (full speed backward) to
// +100 (full speed forward). Each motor has two wires ("forward" and
// "backward"); a positive speed sends power one way and a negative speed
// sends power the other way, so only one of each motor's two pins is ever
// powered at a time. Other Motor_ functions call this one to actually move
// the wheels - you normally won't call this directly.
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
// A simpler way to drive a normal (non-mecanum) car: give it one speed for
// the whole left side and one speed for the whole right side. Behind the
// scenes it just copies each side's speed to both of that side's motors,
// then flips the sign for any motor marked as reversed with the
// REVERSE_MOTOR defines near the top of the header file.
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
// Like Motor_Move(), but lets you set all four mecanum wheels independently
// (M1-M4). This is the function this sketch actually uses in Light_Car(),
// since it can drive the two sides at different speeds to steer.
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
// Prepares the buzzer pin so the Pico can send it signals. Call this once
// in setup() before using Buzzer_Alert().
void Buzzer_Setup(void)
{
  pinMode(PIN_BUZZER, OUTPUT);
}

// Beeps the buzzer "beat" times in a row, then repeats that whole burst
// "rebeat" times, pausing half a second between repeats. For example,
// Buzzer_Alert(1, 1) used in setup() means "beep once, one time" - a simple
// startup chirp to let you know the car is ready.
void Buzzer_Alert(int beat, int rebeat)
{
  beat = constrain(beat, 1, 9);
  rebeat = constrain(rebeat, 1, 255);
  for (int j = 0; j < rebeat; j++)
  {
    for (int i = 0; i < beat; i++)
    {
      freq(PIN_BUZZER, BUZZER_FREQUENCY, 30);
    }
    delay(500);
  }
  freq(PIN_BUZZER, 0, 30);
}

// Makes the given pin buzz at a certain frequency (in Hz - vibrations per
// second) for a certain duration (in milliseconds), by rapidly switching the
// pin HIGH and LOW by hand. Passing freqs = 0 just turns the buzzer off.
// This is a manual way of making "sound" without any special music library.
void freq(int PIN, int freqs, int times) {
  if (freqs == 0) {
    digitalWrite(PIN, LOW);
  }
  else {
    for (int i = 0; i < times * freqs / 500; i ++) {
      digitalWrite(PIN, HIGH);
      delayMicroseconds(500000 / freqs );
      digitalWrite(PIN, LOW);
      delayMicroseconds(500000 / freqs );
    }
  }
}

////////////////////Battery drive area/////////////////////////////////////
float batteryVoltage = 0;      //Battery voltage variable
float batteryCoefficient = 4;  //Set the proportional coefficient
int oa_VoltageCompensationToSpeed;

// Reads the battery voltage sensor pin 5 times in a row and averages the
// results. ADC stands for "Analog-to-Digital Converter" - it's the hardware
// that turns a real-world voltage into a number (0-1023) the Pico can use.
// Averaging several readings helps smooth out electrical noise.
int Get_Battery_Voltage_ADC(void)
{
  pinMode(PIN_BATTERY, INPUT);
  int batteryADC = 0;
  for (int i = 0; i < 5; i++)
    batteryADC += analogRead(PIN_BATTERY);
  return batteryADC / 5;
}

// Converts the raw ADC number from Get_Battery_Voltage_ADC() into an actual
// voltage (in volts), using the batteryCoefficient to account for the
// voltage-divider circuit on the board. Returns the calculated voltage.
float Get_Battery_Voltage(void) {
  int batteryADC = Get_Battery_Voltage_ADC();
  batteryVoltage = (batteryADC / 1023.0 * 3.3) * batteryCoefficient;
  return batteryVoltage;
}

// Lets you override the proportional coefficient used when converting raw
// ADC readings into real voltage - useful if your specific board's resistors
// aren't exactly the expected values.
void Set_Battery_Coefficient(float coefficient)
{
  batteryCoefficient = coefficient;
}

// Works out how much extra motor speed to add to make up for a battery that
// has drained a bit (a lower battery voltage means less power to the
// motors, so this nudges the speed up to compensate). Stores the result in
// oa_VoltageCompensationToSpeed for other code to use.
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

/////////////////////Photosensitive drive area//////////////////////////
// SENSOR CALIBRATION NOTE (measured on the real hardware): a bigger ADC
// number here means MORE light (brighter), and a smaller number means LESS
// light (darker) - roughly: under 10 is pitch black, under 50 is very dark,
// around 250 is bright, and 1000+ is super bright. So throughout this file:
// a bigger ADC number = it's brighter there, and a smaller ADC number = it's
// darker there. Keep that in mind below!
int light_init_value = 0;  //Set the car's initial environment ADC value
//Photosensitive initialization
// Configures both photoresistor pins as inputs so we can read light levels
// from them. Call this once in setup() before reading either sensor.
void Photosensitive_Setup(void) {
  pinMode(Left_PHOTOSENSITIVE_PIN, INPUT);
  pinMode(Right_PHOTOSENSITIVE_PIN, INPUT);
}

// Reads the LEFT photoresistor and returns its raw ADC value (0-1023).
// Remember: a bigger number here means the left sensor is seeing MORE light.
int getLeftPhotosensitiveADCValue(void) {
  int photosensitiveADCValue = analogRead(Left_PHOTOSENSITIVE_PIN);
  return photosensitiveADCValue;
}
// Reads the RIGHT photoresistor and returns its raw ADC value (0-1023).
// Same rule as the left sensor: bigger number = more light on the right.
int getRightPhotosensitiveADCValue(void) {
  int photosensitiveADCValue = analogRead(Right_PHOTOSENSITIVE_PIN);
  return photosensitiveADCValue;
}

#define LIGHT_MIN_MOVED 50
#define LIGHT_MODE_CRUISE_SPEED (25+oa_VoltageCompensationToSpeed)     //0-100
bool isLightModeFirstStarting = true;  //is_light_mode_first_starting
// THE MAIN LIGHT-CHASING LOGIC. Called every time through loop(). It reads
// both light sensors, compares them, and speeds up the wheels on whichever
// side is brighter while slowing the wheels on the darker side, steering
// the car one way or the other depending on which side sees more light.
void Light_Car() {
    // The very first time this function runs, give the car a quick little
    // nudge forward. This "un-sticks" the motors/gears so the very first
    // steering command afterwards actually moves the car instead of getting
    // lost to friction. isLightModeFirstStarting flips to false right after,
    // so this block only ever runs once.
    if (isLightModeFirstStarting) {
      isLightModeFirstStarting = false;
      Motor_M_Move(40, 40, 40, 40);
      delay(200);
    }
    int leftLightValue = getLeftPhotosensitiveADCValue();
    int rightLightValue = getRightPhotosensitiveADCValue();
    // diffLightValue tells us which side is brighter and by how much.
    // Since bigger ADC = brighter, (right - left) is POSITIVE when the RIGHT
    // side is brighter (right's bigger "brighter" number minus left's smaller
    // "darker" number), and NEGATIVE when the LEFT side is brighter.
    // We divide by 8 just to shrink the raw sensor difference down to a
    // gentler number that's more sensible to add/subtract from a motor speed.
    int diffLightValue = (rightLightValue - leftLightValue) / 8;
    // Only actually drive if BOTH sensors are seeing at least a little bit of
    // light (their ADC value is above LIGHT_MIN_MOVED). If the car is picked
    // up, in a dark box, or a wire is disconnected, both readings can stay
    // near 0, and we don't want the car driving blindly in that case.
    if (leftLightValue > LIGHT_MIN_MOVED && rightLightValue > LIGHT_MIN_MOVED) {
      // Start from a steady "cruise" speed for both sides, then steer by
      // giving the two sides opposite adjustments based on diffLightValue:
      //  - If the RIGHT is brighter, diffLightValue is positive, so the left
      //    wheels (lsp) get SLOWER and the right wheels (rsp) get FASTER -
      //    i.e. the wheels on the BRIGHTER side speed up and the wheels on
      //    the DARKER side slow down.
      //  - If the LEFT is brighter, diffLightValue is negative, so it's the
      //    opposite: left wheels speed up and right wheels slow down - again,
      //    the brighter side's wheels get faster.
      // Whether "speed up the brighter side's wheels" makes the car curve
      // TOWARD or AWAY FROM the light depends on exactly how your wheels and
      // motors are wired up - the best way to know for sure is to shine a
      // torch at one sensor and watch which way your own car actually turns!
      // constrain() just makes sure the final speed never goes outside the
      // motors' valid -100 to 100 range.
      int lsp = constrain(LIGHT_MODE_CRUISE_SPEED - diffLightValue, -100, 100);
      int rsp = constrain(LIGHT_MODE_CRUISE_SPEED + diffLightValue, -100, 100);
      Serial.println("sp: " + String(lsp) + " " + String(rsp) + " " + String(diffLightValue));
      Motor_M_Move(lsp, lsp, rsp, rsp);
    } else {
      // Too dark on at least one side to make a sensible decision - stop
      // moving rather than guess.
      Motor_M_Move(0, 0, 0, 0);
    }
}

/////////////////////Ultrasonic drive area//////////////////////////////
// Sets up the two pins used by the ultrasonic distance sensor: one to send
// a "ping" (Trig) and one to listen for its echo bouncing back (Echo). Not
// used by this light-tracking sketch, but shared with other sketches.
void Ultrasonic_Setup(void)
{
  pinMode(PIN_SONIC_TRIG, OUTPUT);// set trigPin to output mode
  pinMode(PIN_SONIC_ECHO, INPUT); // set echoPin to input mode
}

// Measures distance to the nearest object using sound, like a bat's sonar:
// it sends a short ultrasonic "chirp" and times how long it takes to hear
// the echo bounce back, then uses the speed of sound to work out the
// distance in centimeters. Returns MAX_DISTANCE if nothing echoes back
// in time (e.g. no obstacle in range).
float Get_Sonar(void) 
{
  unsigned long pingTime;
  float distance;
  digitalWrite(PIN_SONIC_TRIG, HIGH); // make trigPin output high level lasting for 10μs to triger HC_SR04,
  delayMicroseconds(10);
  digitalWrite(PIN_SONIC_TRIG, LOW);
  pingTime = pulseIn(PIN_SONIC_ECHO, HIGH, SONIC_TIMEOUT); // Wait HC-SR04 returning to the high level and measure out this waitting time
  if (pingTime != 0)
    distance = (float)pingTime * SOUND_VELOCITY / 2 / 10000; // calculate the distance according to the time
  else
    distance = MAX_DISTANCE;
  return distance; // return the distance value
}

//////////////////Emotion drive area////////////////////////////////
// Everything below here controls the little 8x8 LED "face" matrix, showing
// animations like blinking eyes or arrows (defined in Array.h). This
// light-tracking sketch never calls any of these functions, but they're
// part of the shared car library, so they're documented here too.
//
// A trick used throughout this section: instead of using delay() (which
// would freeze the whole program), these functions check "has enough time
// passed since the last frame?" using millis() (milliseconds since the Pico
// booted). This is called "non-blocking" timing - it lets an animation play
// frame-by-frame while other code (like Light_Car()) keeps running too.

Freenove_VK16K33 matrix = Freenove_VK16K33();
int time_before=0;      //Record each non-blocking time
int time_count=0;       //Record the number of non-blocking times
int time_flag=0;        //Record the blink time

// Starts up the LED matrix display over I2C and records the current time as
// our first animation "checkpoint". Call this once in setup() before using
// any of the eyes/arrow/wheel/etc. animation functions below.
void Emotion_Setup()
{
  matrix.init(EMOTION_ADDRESS);
  time_before=millis();
}

// Plays a rotating-eyes animation: every delay_ms milliseconds, it shows the
// next frame from the eyeRotate1/eyeRotate2 arrays, looping back to the
// start once it reaches the last frame.
void eyesRotate(int delay_ms)
{
  int count = sizeof(eyeRotate1) / sizeof(eyeRotate1[0]);
  if(millis()-time_before>=delay_ms)
  {
    matrix.showStaticArray(eyeRotate1[time_count], eyeRotate2[time_count]);
    time_before=millis();
    time_count++;
    if(time_count>=count)
      time_count=0;
  }
}

// Plays a blinking-eyes animation: the eyes stay open (frame 0) for a while,
// then once time_count reaches 25 "ticks", it plays through the rest of the
// eyeBlink frames once (a quick blink) before going back to staying open.
void eyesBlink(int delay_ms)
{
  int count = sizeof(eyeBlink) / sizeof(eyeBlink[0]);
  if(millis()-time_before>=delay_ms)
  {    
    time_before=millis();
    time_count++;
    if(time_count>=25)
    {
      time_count=0;
      time_flag=1;
    }
    if(time_flag==0)
      matrix.showStaticArray(eyeBlink[0], eyeBlink[0]);
    else if(time_flag==1)
    {
      matrix.showStaticArray(eyeBlink[time_count], eyeBlink[time_count]);
      if(time_count>=(count-1))
      {
        time_flag=0;
        time_count=0;
      }
    }
  }
}

// Plays a smiling-eyes animation, advancing through the eyeSmile frames
// once every delay_ms milliseconds and looping back to the start.
void eyesSmile(int delay_ms)
{
  int count = sizeof(eyeSmile) / sizeof(eyeSmile[0]);
  if(millis()-time_before>=delay_ms)
  {
    matrix.showStaticArray(eyeSmile[time_count], eyeSmile[time_count]);
    time_before=millis();
    time_count++;
    if(time_count>=count)
      time_count=0;
  }
}

// Plays a crying-eyes animation (tears falling), advancing through the
// eyeCry1/eyeCry2 frames once every delay_ms milliseconds and looping.
void eyesCry(int delay_ms)
{
  int count = sizeof(eyeCry1) / sizeof(eyeCry1[0]);
  if(millis()-time_before>=delay_ms)
  {
    matrix.showStaticArray(eyeCry1[time_count], eyeCry2[time_count]);
    time_before=millis();
    time_count++;
    if(time_count>=count)
      time_count=0;
  }
}

// A second style of blinking-eyes animation (different eye shape), same
// idea as eyesBlink() but blinks every 15 ticks instead of 25.
void eyesBlink1(int delay_ms)
{
  int count = sizeof(eyeBlink1) / sizeof(eyeBlink1[0]);
  if(millis()-time_before>=delay_ms)
  {    
    time_before=millis();
    time_count++;
    if(time_count>=15)
    {
      time_count=0;
      time_flag=1;
    }
    if(time_flag==0)
      matrix.showStaticArray(eyeBlink1[0], eyeBlink1[0]);
    else if(time_flag==1)
    {
      matrix.showStaticArray(eyeBlink1[time_count], eyeBlink1[time_count]);
      if(time_count>=(count-1))
      {
        time_flag=0;
        time_count=0;
      }
    }
  }
}

// Shows a scrolling arrow pointing up, down, left, or right, chosen by
// arrow_direction (1=up, 2=down, 3=left, 4=right; anything else clears the
// display). Each direction slides its arrow shape across the matrix by
// changing which offset/column it's drawn at as time_count increases.
void showArrow(int arrow_direction,int delay_ms)
{
  if (arrow_direction == 1)
  {
    if(millis()-time_before>=delay_ms)
    {
      matrix.showLedMatrix(arrow_up, 4, time_count-8);
      time_before=millis();
      time_count++;
      if(time_count>16)
        time_count=0;
    }
  }
  else if (arrow_direction == 2)
  {
    if(millis()-time_before>=delay_ms)
    {
      matrix.showLedMatrix(arrow_dowm, 4, 8-time_count);
      time_before=millis();
      time_count++;
      if(time_count>16)
        time_count=0;
    }
  }
  else if (arrow_direction == 3)
  {
    if(millis()-time_before>=delay_ms)
    {
      matrix.showLedMatrix(arrow_left, 8-time_count, 0);
      time_before=millis();
      time_count++;
      if(time_count>8)
        time_count=0;
    }
  }
  else if (arrow_direction == 4)
  {
    if(millis()-time_before>=delay_ms)
    {
      matrix.showLedMatrix(arrow_right, time_count, 0);
      time_before=millis();
      time_count++;
      if(time_count>8)
        time_count=0;
    }
  }
  else
    matrix.clear();
}

// Shows a spinning wheel animation. mode 1 spins the "wheel_left" pattern,
// mode 2 spins "wheel_right" (a mirror-image pattern), and anything else
// clears the display.
void wheel(int mode, int delay_ms)
{
  if (mode == 1)
  {
    int count = sizeof(wheel_left) / sizeof(wheel_left[0]);
    if(millis()-time_before>=delay_ms)
    {
      matrix.showStaticArray(wheel_left[time_count], wheel_left[time_count]);
      time_before=millis();
      time_count++;
      if(time_count>=count)
        time_count=0;
    }
  }
  else if (mode == 2)
  {
    int count = sizeof(wheel_right) / sizeof(wheel_right[0]);
    if(millis()-time_before>=delay_ms)
    {
      matrix.showStaticArray(wheel_right[time_count], wheel_right[time_count]);
      time_before=millis();
      time_count++;
      if(time_count>=count)
        time_count=0;
    }
  }
  else
    matrix.clear();
}

// Shows a little car icon sliding across the display. mode 1 slides the
// car_left pattern, mode 2 slides car_right, and anything else clears it.
void carMove(int mode,int delay_ms)
{
  if (mode == 1)
  {
    if(millis()-time_before>=delay_ms)
    {
      matrix.showLedMatrix(car_left, 8-time_count, 0);
      time_before=millis();
      time_count++;
      if(time_count>=8)
        time_count=0;
    }
  }
  else if (mode == 2)
  {
    if(millis()-time_before>=delay_ms)
    {
      matrix.showLedMatrix(car_right, time_count, 0);
      time_before=millis();
      time_count++;
      if(time_count>=8)
        time_count=0;
    }
  }
  else
    matrix.clear();
}

// Shows a heart / "I love you" pattern on the display, refreshing it every
// delay_ms milliseconds. (Note: the header file declares this function with
// no parameters, but here it takes delay_ms - a small mismatch in the
// original code that was left as-is since we're only adding comments.)
void expressingLove(int delay_ms)
{
    int count = sizeof(I_love_you) / sizeof(I_love_you[0]);
    if(millis()-time_before>=delay_ms)
    {
      matrix.showStaticArray(I_love_you[0], I_love_you[1]);
      time_before=millis();
      time_count++;
      if(time_count>=count)
        time_count=0;
    }
}

// Shows a "save water" themed animation, cycling through the
// save_water_left/save_water_right frames every delay_ms milliseconds.
void saveWater(int delay_ms)
{
    int count = sizeof(save_water_left) / sizeof(save_water_left[0]);
    if(millis()-time_before>=delay_ms)
    {
      matrix.showStaticArray(save_water_left[time_count], save_water_right[time_count]);
      time_before=millis();
      time_count++;
      if(time_count>=count)
        time_count=0;
    }
}
