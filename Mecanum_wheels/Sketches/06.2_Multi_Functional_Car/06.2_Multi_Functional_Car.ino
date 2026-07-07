/**********************************************************************
  Filename    : Camera Car
  Product     : Freenove 4WD Car for Pico W
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
// ======================================================================
// THE "EVERYTHING" SKETCH — Multi Functional Car
// ======================================================================
// This is the biggest, most complete program in the whole course. It ties
// together every part of the robot into one program that a phone app can
// control over WiFi:
//   - Driving the mecanum wheels (forward/back/sideways/spin, all at once)
//   - Showing emotions (a face) or reading distance on the LED matrix
//   - Lighting up the WS2812 RGB strip in different patterns
//   - Reading the light sensors, line-tracking sensors and ultrasonic
//     sensor so the car can also drive itself in three "auto" modes
//
// THE BIG IDEA: your phone sends short text messages like "N#45#80#...\n"
// over WiFi. This program listens for those messages, figures out what
// they mean (this is called "parsing"), and then calls the right function
// to make the car do it. Read Get_Command() at the bottom of this file
// and Freenove_4WD_Car_WiFi.cpp to see exactly how a line of text turns
// into a robot doing something.
//
// Overall flow of the program:
//   1. setup()  - turns everything on once when the Pico W boots up.
//   2. loop()   - runs forever. It waits for the phone app to connect,
//                 then repeatedly: reads one command, does what it says,
//                 updates the face/lights, and sends sensor data back.
// ======================================================================
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>  //Library for controlling WS2812 RGB LEDs
#include <WiFi.h>               //Lets the Pico W join a WiFi network
#include <WiFiClient.h>         //Lets us talk to the phone app over a WiFi connection
#include <Wire.h>               //I2C library, used to talk to the LED matrix chip
#include <math.h>               //Gives us cos()/sin()/M_PI for the mecanum-wheel maths
#include "Freenove_4WD_Car_WiFi.h"
#include "Freenove_4WD_Car_Emotion.h"
#include "Freenove_4WD_Car_WS2812.h"
#include "Freenove_4WD_Car_For_Pico_W.h"

// These control how often (in milliseconds) we send different types of
// sensor data back to the phone app, so we don't flood the WiFi link.
#define UPLOAD_VOL_TIME 3000          //How often to send battery voltage (every 3 seconds)
long int lastUploadVoltageTime;       //Remembers the last time we sent it
#define UPLOAD_LIGHTVOL_TIME 500      //How often to send light-sensor readings
long int lastUploadLIGHTADCTime;
#define UPLOAD_LINEVOL_TIME 200       //How often to send line-tracking sensor readings
long int lastUploadLINEVOLTime;
#define UPLOAD_SONARVOL_TIME 20       //How often to send ultrasonic distance readings
long int lastUploadSONARVOLTime;
float time_proportion = 5.5;  //If you want to get the best out of the rotation mode, change the value by experimenting
int buzzer_frequency = 0;
int CAR_MODE_VOL = 0;      //The car's current auto-drive mode (0=manual, 1=light, 2=line, 3=sonar)
int LASt_CAR_MODE_VOL = 0; //The mode we just switched to, used to detect when the mode changes

// CmdArray/paramters hold the most recently decoded command from the app.
// CmdArray[0] is the single letter that says WHAT to do (see the CMD_*
// defines in Freenove_4WD_Car_WiFi.h), and paramters[1..4] are the numbers
// that came after it (e.g. angle and speed for a drive command).
char CmdArray[8];
int paramters[8];
int sendOnceModuleCheck = 1;  //1 = we still need to tell the app which head module (matrix/sonar) is fitted
int cmdFlag;


//WiFi parameter setting — this is where you type in YOUR home WiFi details.
//Replace the "********" placeholders with your own network name and password.
char ssid_Router[] = "********";         //Modify according to your router name
char password_Router[] = "********";  //Modify according to your router password
char ssid_AP[] = "Sunshine";             //Pico W turns on an AP and calls it Sunshine
char password_AP[] = "Sunshine";         //Set your AP password for Pico W to Sunshine
WiFiServer server_Cmd(4002);  //A TCP server on port 4002 that the phone app connects to for sending commands

// setup() runs ONCE when the Pico W is powered on or reset.
// It turns on every subsystem before the car starts listening for commands.
void setup() {
  Buzzer_Setup();  //Buzzer initialization
  Serial.begin(115200);
  WiFi_Setup(0);                //Set mode. When the parameter is 1, AP mode is enabled. If the parameter is 0, the STA mode is enabled.
  server_Cmd.begin(4002);       //Start the command server
  server_Cmd.setNoDelay(true);  //Set no delay in sending and receiving data
  Wire.begin();
  Motor_Setup();                   //Motor initialization
  Servo_Setup();                   //Servo initialization
  WS2812_Setup();                  //WS2812 initialization
  Emotion_and_Ultrasonic_Setup();  //Emotion initialization or Ultrasonic initialization
  Photosensitive_Setup();          //Light initialization
  Track_Setup();                   //Track initialization
  Servo_1_Angle(90);
  delay(500);
}

// loop() runs over and over, forever, as fast as the Pico W can manage.
// Each time round it: accepts a phone connection if there is one, reads
// one line of text (a command) if one has arrived, works out what that
// command means, does it, and then updates the face/lights/sensors
// before looping back round to check for the next command.
void loop() {
  WiFiClient client = server_Cmd.accept();  //listen for incoming clients
  if (client) {                                //if you get a client
    Serial.println("Cmd_Server connected to a client.");
    while (client.connected()) {
      if (sendOnceModuleCheck == 1) {
        // The very first time we talk to the app, tell it whether this car
        // has an LED matrix (face) or an ultrasonic sensor plugged into its
        // "head" connector — the app shows different buttons depending on
        // which one is fitted.
        if (Check_Module_value == SONAR_IS_ESIST)
          cmdFlag = SONAR_IS_ESIST;
        else
          cmdFlag = MATRIX_IS_EXIST;
        sendOnceModuleCheck = 0;
        client.print("A#" + String(cmdFlag) + "\n");
      }                                                         //loop while the client's connected
      if (client.available()) {                                 //if there's bytes to read from the client
        // A command from the app is one line of text ending in '\n', for
        // example "N#45#80#0#0\n". Read it, copy it into a plain C string
        // (char array) because that's what Get_Command()/strtok() need,
        // then hand it to Get_Command() to decode into CmdArray/paramters.
        String inputStringTemp = client.readStringUntil('\n');  //Read the command by WiFi
        Serial.println(inputStringTemp);                        //Print out the command received by WiFi
        int string_length = inputStringTemp.length() + 1;
        char str[string_length];
        inputStringTemp.toCharArray(str, string_length);
        Get_Command(str);
        if (CmdArray[0] == CMD_LED_MOD)  //Set the display mode of car colored lights
        {
          WS2812_SetMode(paramters[1]);
        }
        if (CmdArray[0] == CMD_LED)  //Set the color and brightness of the car lights
        {
          WS2812_Set_Color_1(paramters[1], paramters[2], paramters[3], paramters[4]);
        }
        if (CmdArray[0] == CMD_MATRIX_MOD)  //Set the display mode of the LED matrix
        {
          if (Check_Module_value == MATRIX_IS_EXIST) {
            Emotion_SetMode(paramters[1]);  //Display
            Serial.print(" \n matrix is exist !!! \n");
          }
          if (Check_Module_value != MATRIX_IS_EXIST) {
            // Buzzer_Variable(2000, 50, 2);
            client.print("A#" + String(SONAR_IS_ESIST) + "\n");
            Serial.print(" \n sonar is exist !!! \n");
          }
        }
        if (CmdArray[0] == CMD_BUZZER)  //Buzzer control command
        {
          tone(2, paramters[1]);  //Play a tone on pin 2 at the requested frequency (Hz); paramters[1]==0 turns it off
          ;
        }
        if (CmdArray[0] == CMD_POWER) {  //Power query command
          // The app asked "how much battery is left?" — read it and send an
          // answer straight back in the same "LETTER#value\n" style as the
          // commands we receive, e.g. "P#7.90\n".
          float battery_voltage = Get_Battery_Voltage();
          client.print(CMD_POWER);
          client.print(INTERVAL_CHAR);
          client.print(battery_voltage);
          client.print(ENTER);
        }

        if (CmdArray[0] == CMD_M_MOTOR) {  //Network control car movement command
          // The app has two virtual joysticks. The LEFT one sends its angle
          // and distance-from-centre as paramters[1]/paramters[2] and steers
          // the car's direction/speed; the RIGHT one sends paramters[3]/[4]
          // and controls how fast the car spins on the spot. We turn each
          // joystick's (angle, distance) into (X, Y) speeds using basic
          // trigonometry (cos/sin), exactly like converting a compass
          // direction and distance into "how far across, how far up".
          Car_SetMode(0);
          int LY = paramters[2] * cos(paramters[1] * (M_PI / 180));     //paramters[1] represents the Angle to the the Y-axis,Counterclockwise is 0 to 180 degrees
          int LX = -(paramters[2] * sin(paramters[1] * (M_PI / 180)));  //paramters[2] represents the move speed(the first jostick)
          int RX = paramters[4] * sin(paramters[3] * (M_PI / 180));     //paramters[3] represents the Angle to the Y-axis,Counterclockwise is 0 to 180 degrees
          int RY = paramters[4] * cos(paramters[3] * (M_PI / 180));     //Converts data from the client to its x and y axis positions

          // Mecanum wheels have rollers set at 45 degrees, so mixing the
          // forward (LY), sideways (LX) and turning (RX) speeds together
          // with plus/minus like this lets all four wheels spin at just
          // the right speed to drive in ANY direction, even sideways,
          // without turning the car's body at all!
          int FR = LY - LX + RX;  //The McNamum wheel chassis motion formula
          int FL = LY + LX - RX;  //LY stands for longitudinal velocity
          int BL = LY - LX - RX;  //LX stands for transverse velocity
          int BR = LY + LX + RX;  //RX stands for angular velocity
          Motor_M_Move(FL, BL, BR, FR);
        }
        if (CmdArray[0] == CMD_CAR_ROTATE)  //Rotate car command
        {
          // This is the "spin while driving in a circle" trick mode. If the
          // right joystick isn't pushed, it just drives normally (same maths
          // as CMD_M_MOTOR above). But if it IS pushed, we enter a small
          // loop of our own (a "while(1)") that keeps recalculating the
          // wheel speeds every few milliseconds, slowly walking set_angle
          // around a full circle, so the car drives round and round while
          // also spinning — like a fairground teacup ride! It keeps checking
          // for a new command each time round so it can be told to stop.
          Car_SetMode(0);
          float battery_voltage = Get_Battery_Voltage();
          int LY = paramters[2] * cos(paramters[1] * (M_PI / 180));
          int LX = -(paramters[2] * sin(paramters[1] * (M_PI / 180)));
          int RX = paramters[4] * sin(paramters[3] * (M_PI / 180));
          int RY = paramters[4] * cos(paramters[3] * (M_PI / 180));  //Converts data from the client to its x and y axis positions

          if (paramters[4] == 0) {  //If we don't move joystick of the right
            int FR = LY - LX + RX;
            int FL = LY + LX - RX;
            int BL = LY - LX - RX;
            int BR = LY + LX + RX;
            Motor_M_Move(FL, BL, BR, FR);  //
          } else {
            int W = (RX > 0) ? 40 : -40;  //Spin direction/speed: +40 one way, -40 the other
            int VY;
            int VX;
            int set_angle = paramters[3];
            while (1) {  //Keep driving round the circle until told to stop
              VY = 40 * cos(set_angle * (M_PI / 180));
              VX = -(40 * sin(set_angle * (M_PI / 180)));
              int FR = VY - VX + W;
              int FL = VY + VX - W;
              int BL = VY - VX - W;
              int BR = VY + VX + W;
              Motor_M_Move(FL, BL, BR, FR);
              delay(5 * time_proportion * 8 / battery_voltage);  //Pause briefly; battery voltage nudges the timing to keep speed consistent
              if (W == 40) {
                set_angle -= 5;  //Step the circle angle round a bit each time
              } else {
                set_angle += 5;
              }
              if (client.available()) {  //Check if a new command has arrived while we're stuck in this loop
                String inputStringTemp = client.readStringUntil('\n');
                Serial.println(inputStringTemp);
                int string_length = inputStringTemp.length() + 1;
                char str[string_length];
                inputStringTemp.toCharArray(str, string_length);
                Get_Command(str);
                if (CmdArray[0] == CMD_CAR_ROTATE && paramters[3] == 0) {  //App says "stop rotating"
                  Motor_M_Move(0, 0, 0, 0);
                  break;  //Exit the while(1) loop and return to the normal command loop
                }
              }
            }
          }
        }
        if (CmdArray[0] == CMD_SERVO) {  //Network control servo motor movement command
          Servo_1_Angle(paramters[1]);
        }
        if (CmdArray[0] == CMD_CAR_MODE) {  //Car command Mode
          // This is how the app switches the car between manual driving and
          // one of the three "self-driving" auto modes. Car_SetMode() just
          // records which mode we're in (checked later, at the bottom of
          // loop(), by Car_Select() which actually runs the self-driving
          // behaviour every time round the loop).
          oa_CalculateVoltageCompensation();
          if (paramters[1] == CAR_MODE_LIGHT_TRACING)  //Light seeking car command
          {
            LASt_CAR_MODE_VOL = 1;
            Car_SetMode(1);
          } else if (paramters[1] == CAR_MODE_LINE_TRACKING)  //Tracking car command
          {
            LASt_CAR_MODE_VOL = 1;
            Car_SetMode(2);
          } else if (paramters[1] == CAR_MODE_SONAR)  //Ultrasonic car command
          {
            if (Check_Module_value == SONAR_IS_ESIST) {
              LASt_CAR_MODE_VOL = 1;
              Car_SetMode(CAR_MODE_SONAR);
            }
            if (Check_Module_value != SONAR_IS_ESIST) {
              Car_SetMode(CAR_MODE_MANUAL);
              // Buzzer_Variable(2000, 50, 2);
              client.print("A#" + String(MATRIX_IS_EXIST) + "\n");
            }
          } else if (paramters[1] == 0) {
            LASt_CAR_MODE_VOL = 2;
            Car_SetMode(0);
          }
          if (CAR_MODE_VOL != LASt_CAR_MODE_VOL) {
            if (CAR_MODE_VOL == 1) {
              emotion_task_mode = 0;  //Display
              Motor_Move(0, 0);       //Stop the car
              Servo_1_Angle(90);
              delay(100);
              isLightModeFirstStarting = true;
            }
            CAR_MODE_VOL = LASt_CAR_MODE_VOL;
          }
        }
        //Clears the command array and parameter array
        memset(CmdArray, 0, sizeof(CmdArray));
        memset(paramters, 0, sizeof(paramters));
      }
      // --- From here on we run EVERY time round loop(), whether or not a
      // new command just arrived, so the face/lights/sensors keep updating
      // smoothly even while nothing new has been said over WiFi. ---
      if (millis() - lastUploadVoltageTime > UPLOAD_VOL_TIME) {  //Time to send another battery reading?
        float battery_voltage = Get_Battery_Voltage();
        client.print("P#" + String(battery_voltage) + "\n");
        lastUploadVoltageTime = millis();
      }
      Emotion_and_Ultrasonic_Setup();  //Re-check which "head" module (matrix or sonar) is plugged in
      if (Check_Module_value == MATRIX_IS_EXIST) {
        Emotion_Show(emotion_task_mode);  //Led matrix display function
      } else if (Check_Module_value == SONAR_IS_ESIST) {
      }
      WS2812_Show(ws2812_task_mode);  //Car color lights display function
      Car_Select(carFlag);            //Pico W Car mode selection function — runs the self-driving behaviour if we're in an auto mode
      if (carFlag == CAR_MODE_LIGHT_TRACING) {  //Send light-sensor readings back to the app while in light-seeking mode
        if (millis() - lastUploadLIGHTADCTime > UPLOAD_LIGHTVOL_TIME) {
          int leftADCVoltage = getLeftPhotosensitiveADCValue();
          int rightADCVoltage = getRightPhotosensitiveADCValue();
          client.print("C#1#" + String(leftADCVoltage) + "#" + String(rightADCVoltage) + "\n");
          lastUploadLIGHTADCTime = millis();
        }
      }
      if (carFlag == CAR_MODE_LINE_TRACKING) {  //Send line-tracking sensor readings back to the app while in line-tracking mode
        if (millis() - lastUploadLINEVOLTime > UPLOAD_LINEVOL_TIME) {
          Track_Read();
          client.print("C#2#" + String(sensorValue[0]) + String(sensorValue[1]) + String(sensorValue[2]) + "\n");
          lastUploadLINEVOLTime = millis();
        }
      }
      if (carFlag == CAR_MODE_SONAR && Check_Module_value == SONAR_IS_ESIST) {  //Send ultrasonic distance readings back to the app while in sonar mode
        if(millis() - lastUploadSONARVOLTime > UPLOAD_SONARVOL_TIME){
          client.print("C#3#" + String(sonar_distance) + "\n");
          lastUploadSONARVOLTime = millis();
        }
      }
    }
    client.stop();  //close the connection:
    Serial.println("Command Client Disconnected.");
    sendOnceModuleCheck = 1;  //Next time a phone connects, tell it which head module we have again
    carFlag = CAR_MODE_MANUAL;  //Safety: if the app disconnects, stop any auto-driving mode
  }
}

// Get_Command() is the heart of "turning text into action". It takes one
// command line (already split from the '\n'), e.g. "N#45#80#0#0", and
// breaks it apart at each '#' character using strtok() — a C function that
// chops a string into pieces ("tokens") around a separator, a bit like
// Python's str.split("#"). The first piece is a single letter saying WHAT
// to do (stored in CmdArray[0]); the rest are turned from text into numbers
// with atoi() ("ASCII to integer") and stored in paramters[1..5] for the
// code in loop() to use.
void Get_Command(char *string) {
  char *token = strtok(string, INTERVAL_CHAR);
  CmdArray[0] = token[0];                    // Put the command into an array
  for (int i = 0; i < 5; i++) {
    if (token != NULL) {
      token = strtok(NULL, INTERVAL_CHAR);
    }
    paramters[i + 1] = atoi(token);
  }
}
