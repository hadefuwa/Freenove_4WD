/**********************************************************************
  Filename    : WiFi Server
  Description : Use Pico W's WiFi server feature to wait for other WiFi devices to connect.
                And communicate with them once a connection has been established.
  Auther      : www.freenove.com
  Modification: 2022/09/27
**********************************************************************/
// This library gives the Pico W all its WiFi powers: connecting to a
// network, and acting as a server that other devices can connect to.
#include <WiFi.h>

// A "port" is like a door number on the Pico W. Your WiFi network is the
// street address (the IP address below), and the port picks which door
// on that address to knock on. Port 4002 is just a number we picked that
// isn't already used by something else - the phone app needs to know
// this exact number to find us.
#define port 4002

// Replace the "********" text below with the name (SSID) and password of
// YOUR WiFi network, keeping the quote marks. The Pico W can only join a
// network it has been told the name and password for, just like you
// would type them into a phone or laptop.
const char *ssid_Router      = "********";  //input your wifi name
const char *password_Router  = "********";  //input your wifi passwords

// This creates the actual TCP server object, listening on the port number
// above. Think of a TCP server like a phone that only answers calls - it
// never dials out by itself, it just waits for someone (the app) to call it.
WiFiServer  server(port);

/**
 * setup()
 * Runs once when the Pico W powers on or resets.
 * Connects to your WiFi network, waits until the connection succeeds,
 * prints the Pico's network address/port to the Serial Monitor so you
 * can find it, then starts the TCP server so it's ready for a client
 * (like the phone app) to connect.
 */
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.printf("\nConnecting to ");
    Serial.println(ssid_Router);
    WiFi.disconnect();
    // Ask the WiFi chip to start joining the network using the name and
    // password we set above. This doesn't wait for success - it just
    // starts the process running in the background.
    WiFi.begin(ssid_Router, password_Router);
    // Keep checking WiFi.status() over and over until it reports
    // "connected". This is called "polling" - like repeatedly checking
    // if a cake is done instead of just guessing. Each loop we wait half
    // a second (delay(500)) and print a dot so you can see it's working.
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    // WiFi.localIP() is the Pico's unique address on your home network -
    // the phone app will need this address (plus the port number) to
    // know where to send its "call".
    Serial.println(WiFi.localIP());
    Serial.printf("IP port: %d\n",port);
    // Actually start the server listening for incoming connections on
    // our chosen port. Before this line, nobody could connect yet.
    server.begin(port);
}

/**
 * loop()
 * Runs over and over forever after setup() finishes. Each pass, it
 * checks whether a client (like the phone app) has connected, and if
 * so, relays text back and forth between that client and the Serial
 * Monitor until the client disconnects. This is a simple two-way
 * "chat bridge" between your network connection and your USB cable.
 */
void loop(){
 // Ask the server "has anyone tried to connect since last time?"
 // If yes, this gives us a WiFiClient object representing that
 // connection. If no one has connected, client will be "empty".
 WiFiClient client = server.accept();            // listen for incoming clients
  if (client) {                                     // if you get a client,
    Serial.println("Client connected.");
    // client.connected() stays true for as long as the phone/PC app is
    // still connected to us. As soon as it disconnects, this becomes
    // false and we drop out of the loop below.
    while (client.connected()) {                    // loop while the client's connected
      // client.available() tells us how many bytes the client has sent
      // that we haven't read yet. If it's more than 0, there's a
      // message waiting for us to pick up.
      if (client.available()) {                     // if there's bytes to read from the client,
        // readStringUntil('\n') reads everything the client sent up to
        // (and including) a newline character, and gives it back as text.
        Serial.println(client.readStringUntil('\n')); // print it out the serial monitor
        while(client.read()>0);                     // clear the wifi receive area cache
      }
      // Same idea, but the other direction: if you type something into
      // the Serial Monitor on your computer, send that text over WiFi
      // to whatever app is connected to the Pico.
      if(Serial.available()){                       // if there's bytes to read from the serial monitor,
        client.print(Serial.readStringUntil('\n')); // print it out the client.
        while(Serial.read()>0);                     // clear the wifi receive area cache
      }
    }
    // The client disconnected (or we asked it to stop), so free up the
    // connection. After this, loop() will run again and wait for the
    // next client to connect.
    client.stop();                                  // stop the client connecting.
    Serial.println("Client Disconnected.");
  }
}