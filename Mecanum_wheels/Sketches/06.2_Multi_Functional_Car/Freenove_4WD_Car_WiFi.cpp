#include "Freenove_4WD_Car_WiFi.h"
#include "Freenove_4WD_Car_For_Pico_W.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>


///////////////////WiFi drive area////////////////////////////////////////
// This file only handles GETTING the Pico W onto a WiFi network — it does
// not read or parse any commands itself (that all happens back in the main
// .ino file, in loop() and Get_Command()). Once WiFi_Setup() has connected,
// the .ino file opens a WiFiServer on port 4002 and the phone app connects
// to that to send its text commands.
bool WiFi_MODE = 1;  //Remembers which mode we ended up in: 0 = joined your router (STA), 1 = made our own hotspot (AP)
int times = 6;

// These addresses are only used in "AP mode" (see below), where the car
// creates its own tiny WiFi network instead of joining your home one.
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Initialize WiFi function.
// WiFi_Mode chooses HOW the car goes online:
//   0 = "STA mode" — the car joins YOUR home WiFi network (ssid_Router/
//       password_Router), just like a phone or laptop would. This is the
//       normal way to use the car, and needs your router's real name/
//       password filled in above in the .ino file.
//   1 = "AP mode" — the car creates its OWN small WiFi network (called
//       "Sunshine") that your phone connects to directly. Handy if you
//       don't have a router nearby, but only the phone can be on it.
// This function blocks (waits) here until the connection succeeds, printing
// dots to the Serial monitor so you can see it's still trying.
void WiFi_Setup(bool WiFi_Mode) {
  if (WiFi_Mode == 0) {
    WiFi_MODE = 0;
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid_Router, password_Router);
    Serial.print("\nWaiting for WiFi... ");
    while (WiFi.connected() != true) {
      Serial.print(".");
      delay(500);
    }
    IPAddress local_ip = WiFi.localIP();
    Serial.println("");
    Serial.println("\nWiFi connected");
    Serial.print("Use your phone to connect to WiFi: ");
    Serial.println(ssid_Router);
    Serial.print("\nThe password for WiFi is: ");
    Serial.println(password_Router);
    Serial.print("\nThen you can enter: '");
    Serial.print(local_ip);
    Serial.println("' to connect the car in Freenove app.");
    
    Buzzer_Alarm(1);  //Beep to let you know WiFi connected successfully
    delay(100);
    Buzzer_Alarm(0);
  } else {
    WiFi_MODE = 1;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ssid_AP, password_AP);
    while (times--) {
      Serial.print(".");
      delay(500);
    }
    Serial.print("\nUse your phone to connect to WiFi: ");
    Serial.println(ssid_AP);
    Serial.print("\nThe password for WiFi is: ");
    Serial.println(password_AP);
    Serial.print("\nThen you can enter: '");
    Serial.print(local_IP);
    Serial.println("' to connect the car in Freenove app.");

    Buzzer_Alarm(1);
    delay(100);
    Buzzer_Alarm(0);
  }
}
