/**********************************************************************
  Filename    : IR_Receiver_Car.ino
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
// ============================================================================
// THE "MULTI-FUNCTIONAL CAR" SKETCH
// ----------------------------------------------------------------------------
// This is the "everything at once" project. It ties together three systems
// you may have already met in earlier lessons:
//   1. IR REMOTE CONTROL   - reading button presses from the little remote
//   2. LED MATRIX "FACE"   - showing emotions/eyes on the small 8x8x2 display
//   3. WS2812 RGB LIGHTS   - colourful light strip effects on the car body
//
// The big idea in this file is a classic pattern used in almost every
// Arduino program: setup() runs ONCE to get everything ready, then loop()
// runs OVER AND OVER FOREVER, each time checking "did anything happen?" and
// reacting to it. Think of loop() like a video game's main loop: check
// controller input -> update the world -> draw the screen -> repeat.
// ============================================================================
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <IRremote.hpp>
#include "Freenove_4WD_Car_For_Pico_W.h"
#include "Freenove_4WD_Car_Emotion.h"
#include "Freenove_VK16K33_Lib.h"
#include "Freenove_4WD_Car_WS2812.h"

#define IR_Pin 3  // Infrared receiving pin
#define ENABLE_LED_FEEDBACK true
#define DISABLE_LED_FEEDBACK false

static int servo_1_angle = 90;  // Current angle of the head/camera servo (90 = pointing straight ahead)
int emotion_flag = 0;   // Which "face" the LED matrix is currently showing (0-7)
int ws2812_flag = 0;    // Which light effect the RGB strip is currently showing (0-5)

int CAR_MODE_VOL = 0;      // (not actively used below, left over from other sketches in this family)
int LASt_CAR_MODE_VOL = 0;

// setup() runs exactly once, right after the board powers on or is reset.
// This is where we turn on and configure every piece of hardware before the
// main loop starts using it.
void setup() {
  Serial.begin(115200);  //Turn on the serial port monitor and set the baud rate to 115200
  Motor_Setup();         //Initialize motor
  Servo_Setup();         //Initialize servo
  Buzzer_Setup();        //Initialize the buzzer
  WS2812_Setup();        //WS2812 initialization
  Emotion_and_Ultrasonic_Setup();  // Detect whether the LED matrix or the ultrasonic sensor is plugged onto the head
  IrReceiver.begin(IR_Pin, DISABLE_LED_FEEDBACK);  // Start the receiver
  Servo_1_Angle(servo_1_angle);  // Point the head servo straight ahead to start
  delay(100);
}

// loop() runs forever, as fast as the board can manage. Each pass through it:
//   1. Checks if a new IR remote signal arrived, and if so, reacts to it.
//   2. Re-checks which "head" accessory (matrix or ultrasonic) is attached.
//   3. Updates the battery-voltage speed compensation.
//   4. Draws one "frame" of the emotion display (if the matrix is present).
//   5. Lets the currently selected car mode (manual/light/track/sonar) drive.
//   6. Draws one "frame" of the WS2812 light effect.
// Nothing in here uses delay() to wait around (except brief motor pulses),
// which is what lets the remote control, the face and the lights all appear
// to run "at the same time" even though the microcontroller can only do one
// thing at a time.
void loop() {
  if (IrReceiver.decode()) {                      // Did a new remote signal just arrive?
    unsigned long value = IrReceiver.decodedIRData.decodedRawData;
    handleControl(value);        // Handle the commands from remote control
    Serial.println(value, HEX);  // Print "old" raw data
    Serial.println();
    IrReceiver.resume();  // Enable receiving of the next value
  }
  Emotion_and_Ultrasonic_Setup();       // Re-check which head module is plugged in
  oa_CalculateVoltageCompensation();    // Adjust motor speeds as the battery drains
  if (Check_Module_value == MATRIX_IS_EXIST) {
    Emotion_Show(emotion_task_mode);  //Led matrix display function
  } else if (Check_Module_value == SONAR_IS_ESIST) {
    // No matrix attached (ultrasonic sensor instead) - nothing to draw on a face.
  }
  Car_Select(carFlag);             // Let the currently active driving mode take control of the motors
  WS2812_Show(ws2812_task_mode);  //Car color lights display function
}

// handleControl() is called every time the IR remote sends a new code.
// Each button on the remote sends its own unique 32-bit hex number - the
// switch statement below matches that number and decides what to do.
// This is a common way to turn "raw sensor data" into "meaningful actions".
void handleControl(unsigned long value) {
  // Handle the commands
  switch (value) {
    case 0xBF40FF00:  // Receive the number '+'
      // Drive all 4 wheels forward briefly, then stop - one "nudge" forward.
      Motor_M_Move(50, 50, 50, 50);
      delay(300);
      Motor_M_Move(0, 0, 0, 0);
      break;
    case 0xE619FF00:  // Receive the number '-'
      // Same idea, but all wheels spin backwards - a "nudge" backward.
      Motor_M_Move(-50, -50, -50, -50);
      delay(300);
      Motor_M_Move(0, 0, 0, 0);
      break;
    case 0xF807FF00:  // Receive the number '|<<'
      // Left wheels backward, right wheels forward - the car strafes/spins left.
      Motor_M_Move(-50, -50, 50, 50);
      delay(300);
      Motor_M_Move(0, 0, 0, 0);
      break;
    case 0xF609FF00:  // Receive the number '>>|'
      // Opposite of above - spins/strafes right.
      Motor_M_Move(50, 50, -50, -50);
      delay(300);
      Motor_M_Move(0, 0, 0, 0);
      break;
    case 0xEA15FF00:  // Receive the number '▶'
      Motor_M_Move(0, 0, 0, 0);  // "Play" button used here simply as an emergency stop
      break;
    case 0xE916FF00:  // Receive the number '0'
      // Tilt the head servo up a bit (add 10 degrees) and move it there.
      servo_1_angle = servo_1_angle + 10;
      Servo_1_Angle(servo_1_angle);
      break;
    case 0xF30CFF00:  // Receive the number '1'
      // Tilt the head servo down a bit (subtract 10 degrees).
      servo_1_angle = servo_1_angle - 10;
      Servo_1_Angle(servo_1_angle);
      break;
    case 0xF708FF00:  // Receive the number '4'
      // Recentre the head servo back to looking straight ahead.
      servo_1_angle = 90;
      Servo_1_Angle(servo_1_angle);
      break;
    case 0xF20DFF00:  // Receive the number 'C'
      // Switch into "light tracing" mode (car follows a light source).
      isLightModeFirstStarting = true;
      Servo_1_Angle(90);
      Car_SetMode(1);
      break;
    case 0xA15EFF00:  // Receive the number '3'
      // Switch into "line tracking" mode (car follows a black line on the floor).
      Servo_1_Angle(90);
      Car_SetMode(2);
      break;
    case 0xA55AFF00:  // Receive the number '6'
      // Switch into "sonar/obstacle avoidance" mode - but only if an
      // ultrasonic sensor is actually plugged in! If it isn't, beep a
      // warning instead and fall back to manual mode.
      Servo_1_Angle(90);
      if (Check_Module_value == SONAR_IS_ESIST) {
        LASt_CAR_MODE_VOL = 1;
        Car_SetMode(3);
      }
      if (Check_Module_value != SONAR_IS_ESIST) {
        Buzzer_Variable(2000, 50, 2);
        Car_SetMode(0);
        Motor_M_Move(0, 0, 0, 0);
      }
      break;
    case 0xB54AFF00:  // Receive the number '9'
      // Reset everything back to plain manual driving: clear the face,
      // recentre the head, stop the motors.
      emotion_flag = 0;
      Emotion_SetMode(emotion_flag);
      Servo_1_Angle(90);
      Car_SetMode(0);
      Motor_M_Move(0, 0, 0, 0);
      break;
    case 0xBB44FF00:  // Receive the number 'TEST'
      Buzzer_Variable(2000, 100, 1);  // Just beep once, to confirm the remote works
      break;
    case 0xE718FF00:  // Receive the number '2'
      // Cycle through the LED matrix face animations (0-7), but only if a
      // matrix is actually attached - otherwise beep to say "no matrix here".
      if (Check_Module_value == MATRIX_IS_EXIST) {
        emotion_flag = emotion_flag + 1;
        if (emotion_flag > 7) {
          emotion_flag = 0;
        }
        Emotion_SetMode(emotion_flag);  //Display
        Serial.print(" \n matrix is exist !!! \n");
      }
      if (Check_Module_value != MATRIX_IS_EXIST) {
        Buzzer_Variable(2000, 50, 2);
        Serial.print(" \n sonar is exist !!! \n");
      }
      break;
    case 0xE31CFF00:  // Receive the number '5'
      // Turn the face display off (blank).
      emotion_flag = 0;
      Emotion_SetMode(emotion_flag);
      break;
    case 0xBD42FF00:  // Receive the number '7'
      // Cycle through the WS2812 light effects (0-5).
      ws2812_flag = ws2812_flag + 1;
      if (ws2812_flag >= 6) {
        ws2812_flag = 0;
      }
      WS2812_SetMode(ws2812_flag);
      break;
    case 0xAD52FF00:  // Receive the number '8'
      // Turn the RGB lights off.
      ws2812_flag = 0;
      WS2812_SetMode(ws2812_flag);
      break;
    case 0xFFFFFFFF:  // Remain unchanged
      // This special code means "the same button is still held down" (a
      // repeat code), so we deliberately do nothing here.
      break;
    default:
      // Any button we don't recognise - just ignore it.
      break;
  }
}
