#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"
#include "Freenove_VK16K33_Lib.h"
#include "Array.h"
#include <Wire.h>
// This file is the shared "driver library" for the whole car - it has code
// for the servo, motors, buzzer, battery check, light sensor, ultrasonic
// sensor, LED face display, AND the line-tracking sensors used by this
// sketch. For sketch 04.1, only the "Track drive area" section further down
// (Track_Setup and Track_Read) matters - everything else is unused here but
// kept because this exact file is reused by every example in the kit.

/////////////////////Servo drive area///////////////////////////////////
uint32_t Servo_Pins[] = { PIN_SERVO1 };
#define NUM_OF_ServoPINS  ( sizeof(Servo_Pins) / sizeof(uint32_t) )
float dutyCycle1[NUM_OF_ServoPINS] = { 0.0f};
float freq1[] = { 50.0f};
RP2040_PWM* Servo_Instance[NUM_OF_ServoPINS];

int servo_1_offset=0; //Define the offset variable for servo 1

// Servo_Setup(): sets up the PWM (Pulse Width Modulation) hardware needed to
// control the steering servo. PWM is how the Pico "talks" to a servo motor -
// it sends rapid on/off pulses whose timing tells the servo what angle to
// move to. Takes no parameters, returns nothing. Not used by this sketch.
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

// Servo_1_Angle(angle): moves the steering servo to the given angle in
// degrees. angle is automatically clamped (limited) to 30-150 degrees so you
// can't ask it to over-rotate and strain the gears. Not used by this sketch.
void Servo_1_Angle(float angle)
{
  angle = constrain(angle, 30, 150);
  angle=map(angle,0.0f,180.0f,2500.0f,12500.0f);
  Servo_Instance[0]->setPWM(PIN_SERVO1, 50.0f, angle/1000.0f);
}

// Set_Servo_1_Offset(offset): stores a small correction angle used to
// straighten the servo if it isn't perfectly centered when built. Not used
// by this sketch.
void Set_Servo_1_Offset(int offset)
{
  servo_1_offset=offset;
}

// Servo_Sweep(servo_id, angle_start, angle_end): smoothly moves a servo one
// degree at a time from angle_start to angle_end (works going up or down),
// pausing 10ms between steps so the motion looks gradual instead of a snap.
// Not used by this sketch.
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
// Motor_Setup(): sets up PWM control for all 8 motor-driver pins (2 pins per
// wheel x 4 wheels), so the wheels' speed and direction can be controlled
// later. Not used by this sketch.
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

// Motor_Move_Init(m1..m4 speed): sets the raw speed (-100 to 100) of each of
// the 4 wheel motors individually. Positive/negative sign controls spin
// direction (forward vs. backward) - each motor has two wires, and driving
// current through one or the other reverses it. Not used by this sketch.
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
// Motor_Move(Left_speed, Right_speed): simpler helper that sets all left-side
// wheels to one speed and all right-side wheels to another (handy for
// tank-style steering). The #ifdef blocks below let you flip an individual
// motor's wiring in software if it was connected backwards. Not used here.
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
// Buzzer_Setup(): configures the buzzer pin as an output so the code can
// switch it on/off to make sound. Not used by this sketch.
void Buzzer_Setup(void)
{
  pinMode(PIN_BUZZER, OUTPUT);
}

// Buzzer_Alert(beat, rebeat): beeps the buzzer "beat" times, then pauses,
// repeating that whole pattern "rebeat" times - like a car alarm. Not used
// by this sketch.
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
    delay(300);
  }
  freq(PIN_BUZZER, 0, 30);
}

// freq(PIN, freqs, times): manually toggles a pin HIGH/LOW at a given
// frequency to produce a tone on a simple (non-PWM-driven) buzzer. Not used
// by this sketch.
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
float batteryVoltage = 0;       //Battery voltage variable
float batteryCoefficient = 4;   //Set the proportional coefficient

// Get_Battery_Voltage_ADC(): reads the raw analog-to-digital (ADC) value
// from the battery sensing pin 5 times and averages them for a steadier
// reading. Returns a raw number, not yet converted to volts. Not used here.
int Get_Battery_Voltage_ADC(void)
{
  pinMode(PIN_BATTERY, INPUT);
  int batteryADC = 0;
  for (int i = 0; i < 5; i++)
    batteryADC += analogRead(PIN_BATTERY);
  return batteryADC / 5;
}

// Get_Battery_Voltage(): converts the raw ADC reading into an actual voltage
// number (in volts) using a scaling formula and the batteryCoefficient
// constant. Returns the battery voltage as a float. Not used by this sketch.
float Get_Battery_Voltage(void)
{
  int batteryADC = Get_Battery_Voltage_ADC();
  batteryVoltage = (batteryADC / 1023.0  * 3.67 ) * batteryCoefficient;
  return batteryVoltage;
}

// Set_Battery_Coefficient(coefficient): lets you adjust the voltage-scaling
// math above, e.g. to calibrate for a different resistor setup. Not used by
// this sketch.
void Set_Battery_Coefficient(float coefficient)
{
  batteryCoefficient = coefficient;
}

/////////////////////Photosensitive drive area//////////////////////////
// Photosensitive_Setup(): configures the light-sensor pin as an input. Not
// used by this sketch.
void Photosensitive_Setup(void)
{
  pinMode(PHOTOSENSITIVE_PIN, INPUT);
}

// Get_Photosensitive(): reads the ambient light level from the photoresistor
// as a raw analog number (bigger/smaller depending on brightness). Not used
// by this sketch.
int Get_Photosensitive(void)
{
  int photosensitiveADC = analogRead(PHOTOSENSITIVE_PIN);
  return photosensitiveADC;
}

/////////////////////Ultrasonic drive area//////////////////////////////
// Ultrasonic_Setup(): configures the two pins used by the HC-SR04 distance
// sensor - one to send a "ping" pulse (Trig) and one to time how long the
// echo takes to come back (Echo). Not used by this sketch.
void Ultrasonic_Setup(void)
{
  pinMode(PIN_SONIC_TRIG, OUTPUT);// set trigPin to output mode
  pinMode(PIN_SONIC_ECHO, INPUT); // set echoPin to input mode
}

// Get_Sonar(): fires an ultrasonic ping and measures how long it takes for
// the echo to return, then uses the speed of sound to work out the distance
// to the nearest object in cm. Returns MAX_DISTANCE if nothing echoes back
// in time. Not used by this sketch.
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

/////////////////////Track drive area//////////////////////////////
// This is the part sketch 04.1 (Tracking_Sensor) actually uses.
unsigned char sensorValue[4];  //define an array : [0]=Left, [1]=Middle, [2]=Right, [3]=all three combined

// Track_Setup(): configures the 3 line-tracking sensor pins as digital
// inputs, so we can later ask each one "are you seeing black or white?"
// Call this once, from setup(), before using Track_Read(). Takes no
// parameters and returns nothing.
void Track_Setup(void)
{
  pinMode(PIN_TRACKING_LEFT, INPUT); //
  pinMode(PIN_TRACKING_RIGHT, INPUT); //
  pinMode(PIN_TRACKING_CENTER, INPUT); //
}

// Track_Read(): reads all 3 line-tracking sensors and stores the results in
// the global sensorValue[] array. Each sensor gives back a single bit: 0
// means "I see light floor", 1 means "I see a dark line". Takes no
// parameters and returns nothing - the results come back through the shared
// sensorValue[] array instead of a normal return value.
void Track_Read(void)
{
  sensorValue[0] = digitalRead(PIN_TRACKING_LEFT);   // Left sensor:   0 or 1
  sensorValue[1] = digitalRead(PIN_TRACKING_CENTER); // Middle sensor: 0 or 1
  sensorValue[2] = digitalRead(PIN_TRACKING_RIGHT);  // Right sensor:  0 or 1
  // Here's the key trick: combine 3 separate 0/1 bits into ONE number from
  // 0-7 using bit shifting ("<<") and bitwise OR ("|").
  //   sensorValue[0] << 2   shifts the Left bit 2 places left  (worth 4 if it's 1)
  //   sensorValue[1] << 1   shifts the Middle bit 1 place left (worth 2 if it's 1)
  //   sensorValue[2]        stays as-is                        (worth 1 if it's 1)
  // OR-ing them together (|) just merges the bits into one binary number,
  // like laying 3 light switches side by side and reading them as one row:
  // binary "LMR" -> decimal 0-7. Example: Left=1, Middle=0, Right=1 gives
  // binary 101, which is 5. This lets you tell instantly, from one number,
  // exactly which combination of sensors is on the black line.
  sensorValue[3] = sensorValue[0] << 2 | sensorValue[1] << 1 | sensorValue[2];
}

//////////////////Emotion drive area////////////////////////////////
// This section drives the little LED matrix "face" on top of the car,
// showing animated eyes/arrows/faces. None of this is used by this sketch -
// it's here because this .cpp file is shared by every sketch in the kit.

Freenove_VK16K33 matrix = Freenove_VK16K33();
int time_before=0;      //Record each non-blocking time
int time_count=0;       //Record the number of non-blocking times
int time_flag=0;        //Record the blink time

// Emotion_Setup(): starts up the LED matrix display over I2C and records the
// current time so the animation timers below have a starting point. Not
// used by this sketch.
void Emotion_Setup()
{
  matrix.init(EMOTION_ADDRESS);
  time_before=millis();
}

// eyesRotate(delay_ms): plays the "eyes looking around" animation, showing
// one new frame every delay_ms milliseconds without using delay() (so the
// rest of the program keeps running). Not used by this sketch.
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

// eyesBlink(delay_ms): plays a blinking-eyes animation, using time_flag to
// switch between "eyes open, waiting" and "playing the blink frames" states.
// Not used by this sketch.
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

// eyesSmile(delay_ms): plays a smiling-face animation on the LED matrix,
// advancing one frame every delay_ms milliseconds. Not used by this sketch.
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

// eyesCry(delay_ms): plays a crying-face animation on the LED matrix. Not
// used by this sketch.
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

// eyesBlink1(delay_ms): an alternate blinking animation (different eye
// artwork/frame count from eyesBlink). Not used by this sketch.
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

// showArrow(arrow_direction, delay_ms): animates a scrolling arrow (up/down/
// left/right depending on arrow_direction) across the LED matrix; any other
// direction value just clears the display. Not used by this sketch.
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

// wheel(mode, delay_ms): animates a spinning-wheel icon, spinning left
// (mode 1) or right (mode 2); any other mode clears the display. Not used
// by this sketch.
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

// carMove(mode, delay_ms): animates a little car icon driving left (mode 1)
// or right (mode 2); any other mode clears the display. Not used by this
// sketch.
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

// expressingLove(delay_ms): shows a "heart"/love message animation on the
// LED matrix. Not used by this sketch.
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

// saveWater(delay_ms): plays a "save water" themed icon animation on the LED
// matrix. Not used by this sketch.
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
