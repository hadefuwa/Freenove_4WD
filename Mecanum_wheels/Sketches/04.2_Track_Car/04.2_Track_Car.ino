/**********************************************************************
  Filename    : Tracking_Car.ino
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/

#include <Arduino.h>
#include "Freenove_4WD_Car_For_Pico_W.h"

// This sketch is the "brain" of a line-following robot. Every trip around
// loop() the car does three things, over and over, forever:
//   1. LOOK  - read the 3 floor sensors (Track_Read)
//   2. DECIDE - use a switch/case to turn that sensor pattern into a plan
//   3. ACT   - spin the 4 wheels (Motor_M_Move) and show a face/arrow
//
// The floor sensors shine light down and check whether it bounces back.
// A dark line (usually black tape) absorbs light, so a sensor sitting
// over the line reads differently than one sitting over the light floor.
// We combine the 3 separate on/off sensor readings into a single 3-bit
// binary number (0-7) so the switch/case below can handle every possible
// combination of "line / no line" in one place.

// Speed "levels" - just friendly names for motor power percentages, so the
// switch/case below reads like "go at SPEED_LV1" instead of a mystery 45.
// (100 = full power, 0 = stopped). Higher numbers here do NOT mean the
// wheel spins faster automatically - they're just labels we chose.
#define SPEED_LV4   ( 90 )
#define SPEED_LV3   ( 75 )
#define SPEED_LV2   ( 60 )
#define SPEED_LV1   ( 45 )

// setup() runs exactly once, right after the Pico powers on or resets.
// It wakes up the three hardware systems this sketch needs: the line
// sensors, the drive motors, and the little LED "face" on top of the car.
void setup()
{
  Track_Setup();      //Trace module initialization
  Motor_Setup();      //Motor initialization
  Emotion_Setup();    //Emotion initialization
}

// loop() runs forever, thousands of times per second. Each pass reads the
// sensors fresh, decides what that reading means, and drives the motors
// to match - this is what makes the car react to the line in real time.
void loop()
{
  Track_Read();
  // sensorValue[3] packs the left/center/right sensor bits into ONE number
  // from 0 to 7, e.g. binary 010 (left=0, center=1, right=0) becomes the
  // number 2. Writing the switch this way means every one of the 8
  // possible sensor combinations gets its own clearly labelled case below.
  // Sensor bit = 1 usually means "line detected under this sensor".
  switch (sensorValue[3])
  {
    case 2:   //010 - only the CENTER sensor sees the line: dead on target.
    case 5:   //101 - LEFT and RIGHT sensors both see line, center doesn't.
              //      This happens when the line is thicker than the
              //      center sensor alone, or at a crossing - still counts
              //      as "centered", so we treat it the same as case 2.
      showArrow(1, 100);                                          //Show a "straight ahead" arrow on the LED face
      Motor_M_Move(SPEED_LV1 , SPEED_LV1, SPEED_LV1, SPEED_LV1);  //Move Forward - all 4 wheels spin the same speed, same direction
      break;
    case 0:   //000 - NO sensor sees the line. The car has drifted off the
              //      track entirely, or is between line segments. Keep
              //      creeping forward slowly and hope to find it again.
      Motor_M_Move(SPEED_LV1 , SPEED_LV1 ,SPEED_LV1 ,SPEED_LV1);  //Move Forward
      break;
    case 7:   //111 - ALL THREE sensors see the line: this means the whole
              //      width of the track is dark, which this course uses
              //      as a "finish line" / stop marker rather than a turn.
      eyesBlink1(100);           //Blink the LED eyes to celebrate reaching the stop line
      Motor_M_Move(0, 0, 0, 0);  //Stop - zero power to every wheel
      break;
    case 4:   //100 - only the LEFT sensor sees the line: the line has
              //      drifted to the car's left, so the car needs to
              //      steer LEFT to get back on top of it.
    case 6:   //110 - LEFT and CENTER both see the line: line is drifting
              //      left but not as far off as case 4 - same fix applies.
      wheel(2, 100);  //Spin the LED "wheel" icon to show we're turning left
      // To pivot left, the two LEFT wheels spin BACKWARDS (negative speed)
      // while the two RIGHT wheels spin FORWARDS (positive speed). Wheels
      // turning opposite directions makes the car rotate on the spot
      // instead of driving in a straight line - a "tank turn".
      Motor_M_Move(-SPEED_LV4, -SPEED_LV4, SPEED_LV3, SPEED_LV3);  //Turn Left
      break;
    case 1:   //001 - only the RIGHT sensor sees the line: line has
              //      drifted to the car's right, so steer RIGHT.
    case 3:   //011 - CENTER and RIGHT both see the line: same idea as
              //      case 1, line is off to the right.
      wheel(1, 100);  //Spin the LED "wheel" icon to show we're turning right
      // Mirror image of the left turn above: LEFT wheels spin FORWARDS
      // (positive) and RIGHT wheels spin BACKWARDS (negative), pivoting
      // the car the other way to chase the line back to center.
      Motor_M_Move(SPEED_LV3, SPEED_LV3, -SPEED_LV4, -SPEED_LV4);  //Turn Right
      break;
    default:
      // sensorValue[3] can only ever be 0-7, and every one of those 8
      // values is already handled above, so this branch should never
      // actually run. It's here just in case, so the car simply does
      // nothing rather than crashing if something unexpected happens.
      break;
  }
}
