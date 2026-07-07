#ifndef _FREENOVE_4WD_CAR_WIFI_H
#define _FREENOVE_4WD_CAR_WIFI_H

///////////////////WiFi command area////////////////////////////////////////
// Every command sent by the phone app is one line of plain text, split into
// pieces by the '#' character and ended with a newline ('\n'). The very
// first character is always one of the single-letter "command codes"
// defined below, and it tells the car WHAT KIND of command this is. For
// example the app might send:
//     N#45#80#0#0\n
// which decodes as: command 'N' (CMD_M_MOTOR = "drive with mecanum
// mixing"), followed by four number parameters (angle/speed for each
// joystick). See Get_Command() in the main .ino file for the code that
// splits the text apart, and the big if-chain in loop() for what each
// letter below actually makes the car do.

#define ENTER               '\n'                  //Marks the end of a command line
#define INTERVAL_CHAR       "#"                   //The character used to separate the pieces of a command
#define CMD_MOTOR           'M'           //(unused in this sketch) Simple left/right motor speed command
#define CMD_POWER           'P'           //Ask for / report the battery voltage
#define CMD_BUZZER          'B'           //Play a tone on the buzzer
#define CMD_SERVO           'S'           //Move the head servo to an angle
#define CMD_LED_MOD         'D'           //Choose which WS2812 light pattern (mode) to show
#define CMD_LED             'L'           //Set the WS2812 strip's color and brightness
#define CMD_MATRIX_MOD      'T'           //Choose which face/emotion to show on the LED matrix
#define CMD_CAR_MODE        'C'           //Switch the car between manual and an auto-drive mode
#define CMD_SIGN            'A'           //Sent BY the car to tell the app which head module (matrix/sonar) is fitted
#define CMD_M_MOTOR         'N'           //Drive the mecanum wheels using the two virtual joysticks
#define CMD_CAR_ROTATE      'O'           //Drive while spinning on the spot (the "teacup ride" mode)

extern char ssid_Router[]             ;    //Modify according to your router name
extern char password_Router[]         ;    //Modify according to your router password
extern char ssid_AP[]                 ;    //Pico W turns on an AP and calls it Sunshine
extern char password_AP[]             ;    //Set your AP password for Pico W to Sunshine
void WiFi_Setup(bool WiFi_Mode)       ;    //Initialize WiFi function


#endif
