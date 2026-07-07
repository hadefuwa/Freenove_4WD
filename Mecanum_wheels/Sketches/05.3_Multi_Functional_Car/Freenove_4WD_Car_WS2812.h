#ifndef _FREENOVE_4WD_CAR_WS2812_H
#define _FREENOVE_4WD_CAR_WS2812_H

#include <Adafruit_NeoPixel.h>

// ============================================================================
// This header describes the WS2812 RGB light strip - the colourful
// individually-addressable LEDs on the car's body. WS2812 LEDs are "smart":
// you send each one its own Red/Green/Blue value over a single data wire,
// and the Adafruit_NeoPixel library handles the tricky timing for you.
// The .cpp file has the actual light-pattern logic (solid colour, chasing
// lights, blink, breathing fade, rainbow); this file lists what other files
// (like the main .ino) are allowed to call.
// ============================================================================

#define WS2812_PIN 16        //Control pins for Pico W
#define LEDS_COUNT 8
extern int ws2812_task_mode; //WS2812 run mode

void WS2812_Setup(void);                                                                                   //WS2812 initialization function
void WS2812_Show(int mode);                                                                                //WS2812 non-blocking display function
void WS2812_Set_Color_1(int number, unsigned char color_1,unsigned char color_2,unsigned char color_3);    //Set the display color1 for WS2812
void WS2812_Set_Color_2(int number, unsigned char color_1,unsigned char color_2,unsigned char color_3);    //Set the display color2 for WS2812
void WS2812_SetMode(int mode);                                                                             //WS2812 set mode
void ws2812_close(void);
void ws2812_following(void);
void ws2812_rgb(void);
void ws2812_rgb(void);
void ws2812_breathe(void);
void ws2812_rainbow(void);
int Wheel(byte pos);        // Converts a 0-255 position into a rainbow colour (used by ws2812_rainbow)
void WS2812_Show1(int ws2812_task_mode1);

#endif
