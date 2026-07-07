/**********************************************************************
  Filename    : IR_Receiver_Car.ino
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
// This sketch turns the car into an RC (remote-controlled) car.
// Every time you press a button on the little infrared (IR) remote,
// it sends a unique code through the air as invisible light pulses.
// The IR receiver module on the car "hears" that code, and this
// program looks up what that code means and drives the motors to match.
#include <Arduino.h>
#include <IRremote.hpp>                        // Library that decodes IR remote signals for us
#include "Freenove_4WD_Car_For_Pico_W.h"
#define IR_Pin 3                                // The Pico pin wired to the IR receiver module
#define ENABLE_LED_FEEDBACK true                // (unused option) would flash an LED whenever a signal is received
#define DISABLE_LED_FEEDBACK false               // We use this one - no LED flashing
int motor_speed = 50;                           // How fast the wheels spin when driving (0-100)
int emotionMode = 4;                            // Which face/animation shows on the LED matrix "eyes"
float time_proportion = 5.7;  //If you want to get the best out of the rotation mode, change the value by experimenting
int motor_flag = 0;                             // Toggles between two control layouts: 0 = drive/turn mode, 1 = diagonal strafe mode
int set_angle = 0;  //Set the forward angle of rotation
int rotate_flag = 0;                            // When true, the car spins slowly on the spot (see loop() below)
int around_rotate_flag = 0;                     // When true, the car orbits/rotates around a point
float battery_voltage = 0;                      // Battery reading, used to time the spin-in-place animation

// Runs once at power-on: get everything (motors, "face" display, IR receiver) ready to go.
void setup() {
  Serial.begin(115200);
  Motor_Setup();                                   //Motor initialization
  Emotion_Setup();                                 //Initializes Emotion module
  IrReceiver.begin(IR_Pin, DISABLE_LED_FEEDBACK);  // Start the receiver
}

// Runs over and over, forever. Each pass: check for a new remote button press,
// react to it, update the "face" animation, and if a rotation mode is active,
// keep nudging the car through its spin.
void loop() {
  if (IrReceiver.decode()) {                     // True only when a fresh IR code has just arrived
    unsigned long value = IrReceiver.decodedIRData.decodedRawData;
    handleControl(value);        // Handle the commands from remote control
    Serial.println(value, HEX);  // Print "old" raw data
    Serial.println();
    IrReceiver.resume();  // Enable receiving of the next value
  }
  showEmotion(emotionMode);
  if (rotate_flag) {  //Rotate when the rotate_glag is true
    // Spin-in-place mode: every loop we move the "forward direction" a few
    // degrees further around the compass (set_angle) and drive the mecanum
    // wheels toward that new angle, so the car slowly rotates on the spot.
    int VY = 40 * cos(set_angle * (M_PI / 180));
    int VX = -(40 * sin(set_angle * (M_PI / 180)));
    int W = 40;

    // Standard mecanum-wheel math: combine forward/sideways speed (VX, VY)
    // with a spin speed (W) to get each wheel's individual speed.
    int FR = VY - VX + W;
    int FL = VY + VX - W;
    int BL = VY - VX - W;
    int BR = VY + VX + W;

    Motor_M_Move(FL, BL, BR, FR);
    delay(5 * time_proportion * 8 / battery_voltage);
    set_angle -= 5;
  }
  //The car rotates around a point
  if (around_rotate_flag) {                     //Rotate when the rotate_glag is true 围绕旋转模式//-90 100 90 100
    int LY = 100 * cos(-90 * (M_PI / 180));     //paramters[1] represents the Angle to the the Y-axis,Counterclockwise is 0 to 180 degrees
    int LX = -(100 * sin(-90 * (M_PI / 180)));  //paramters[2] represents the move speed(the first jostick)
    int RX = 100 * sin(90 * (M_PI / 180));      //paramters[3] represents the Angle to the Y-axis,Counterclockwise is 0 to 180 degrees
    int RY = 100 * cos(90 * (M_PI / 180));      //Converts data from the client to its x and y axis positions
    int FR = LY - LX + RX;  //The McNamum wheel chassis motion formula
    int FL = LY + LX - RX;  //LY stands for longitudinal velocity
    int BL = LY - LX - RX;  //LX stands for transverse velocity
    int BR = LY + LX + RX;  //RX stands for angular velocity
    Motor_M_Move(FL, BL, BR, FR);
  }
}

// This is the heart of the remote control: it takes the raw hex code that
// the IR receiver decoded and looks it up in a switch/case table, like a
// big "if this button, then do that movement" list. Each remote button
// sends one fixed 32-bit code (in hex, e.g. 0xBF40FF00), so every case here
// matches exactly one physical button.
void handleControl(unsigned long value) {
  // Handle the commands
  switch (value) {
    case 0xBF40FF00:  // Receive the number '+'
      rotate_flag = 0;
      Motor_M_Move(motor_speed, motor_speed, motor_speed, motor_speed);  // Go forward
      delay(500);
      Motor_M_Move(0, 0, 0, 0);
      break;

    case 0xE619FF00:  // Receive the number '-'
      rotate_flag = 0;
      Motor_M_Move(-motor_speed, -motor_speed, -motor_speed, -motor_speed);  // Back up
      delay(500);
      Motor_M_Move(0, 0, 0, 0);
      break;

    // '|<<' turns left. Which movement it does depends on motor_flag:
    // if motor_flag is 0 (normal mode) it spins the car on the spot;
    // if motor_flag is 1 (strafe mode, toggled by the '▶' button) it
    // instead slides the car sideways to the left without turning.
    case 0xF807FF00:  // Receive the number '|<<'
      rotate_flag = 0;
      if (!motor_flag) {
        Motor_M_Move(-motor_speed, -motor_speed, motor_speed, motor_speed);  // Turn left
        delay(200);
        Motor_M_Move(0, 0, 0, 0);
        break;
      } else {
        Motor_M_Move(-motor_speed, motor_speed, -motor_speed, motor_speed);  // Turn left
        delay(500);
        Motor_M_Move(0, 0, 0, 0);
        break;
      }
    // '>>|' turns right, same motor_flag logic as '|<<' above (turn vs. sideways slide).
    case 0xF609FF00:  // Receive the number '>>|'
      rotate_flag = 0;
      if (!motor_flag) {
        Motor_M_Move(motor_speed, motor_speed, -motor_speed, -motor_speed);  // Turn right
        delay(200);
        Motor_M_Move(0, 0, 0, 0);
        break;
      } else {
        Motor_M_Move(motor_speed, -motor_speed, motor_speed, -motor_speed);  // Turn right
        delay(500);
        Motor_M_Move(0, 0, 0, 0);
        break;
      }
    // These next four buttons only do something while motor_flag is 1
    // (strafe mode) - they drive the mecanum wheels diagonally, which
    // only mecanum wheels can do! In normal mode (motor_flag == 0) they
    // are ignored (the else branch just breaks with no movement).
    case 0xBB44FF00:  //Receive the number 'TEST'
      rotate_flag = 0;
      if (motor_flag) {
        Motor_M_Move(0, motor_speed, 0, motor_speed);  // Move forward and to the left diagonally
        delay(500);
        Motor_M_Move(0, 0, 0, 0);
        break;
      } else {
        break;
      }

    case 0xF20DFF00:  //Receive the number 'c'
      rotate_flag = 0;
      if (motor_flag) {
        Motor_M_Move(0, -motor_speed, 0, -motor_speed);  // Move backward and to the right diagonally
        delay(500);
        Motor_M_Move(0, 0, 0, 0);
        break;
      } else {
        break;
      }

    case 0xBC43FF00:  //Receive the number '↗'
      rotate_flag = 0;
      if (motor_flag) {
        Motor_M_Move(motor_speed, 0, motor_speed, 0);  // Move forward and to the right diagonally
        delay(500);
        Motor_M_Move(0, 0, 0, 0);
        break;
      } else {
        break;
      }

    case 0xE916FF00:  //Receive the number '0'
      rotate_flag = 0;
      if (motor_flag) {
        Motor_M_Move(-motor_speed, 0, -motor_speed, 0);  // Move back and to the left diagonally
        delay(500);
        Motor_M_Move(0, 0, 0, 0);
        break;
      } else {
        break;
      }

    // '1' picks a "random-ish" face animation using the current millisecond
    // clock (millis() % 10 gives a leftover value from 0-9) - a cheap trick
    // for pseudo-randomness without needing a real random-number function.
    case 0xF30CFF00:  // Receive the number '1'
      emotionMode = millis() % 10;
      break;

    // '▶' (play) flips motor_flag between 0 and 1, switching the remote
    // between "turn mode" and "diagonal strafe mode" for the buttons above.
    case 0xEA15FF00:  // Receive the number '▶'
      rotate_flag = 0;
      motor_flag = !motor_flag;
      break;

    // 'MENU' toggles the spin-in-place mode on/off (rotate_flag), resets
    // the spin angle to 0, and re-measures the battery voltage so the
    // spin's timing (delay in loop()) stays accurate as the battery drains.
    case 0xB847FF00:  // Receive the number 'MENU'
      battery_voltage = Get_Battery_Voltage();
      rotate_flag = !rotate_flag;
      set_angle = 0;
      break;

    // '2' toggles the "orbit around a point" mode on/off.
    case 0xE718FF00:  // Receive the number '2'
      around_rotate_flag = !around_rotate_flag;
      break;

    // Any code that isn't one of the buttons above (including the repeat
    // codes some remotes send while a button is held, or noise) falls
    // through to here - and the safe response is simply to stop the motors.
    // This is effectively the car's "no command / unknown command" case.
    default:
      Motor_M_Move(0, 0, 0, 0);  //stop
      break;
  }
}
