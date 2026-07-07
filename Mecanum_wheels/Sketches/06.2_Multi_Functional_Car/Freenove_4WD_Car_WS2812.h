#ifndef _FREENOVE_4WD_CAR_WS2812_H
#define _FREENOVE_4WD_CAR_WS2812_H
// This file/its .cpp control the WS2812 RGB LED strip (sometimes called
// "NeoPixels") on the car. Each of the 8 LEDs can be set to its own
// red/green/blue color independently, and WS2812_Show() plays one of
// several built-in light patterns (solid color, chasing lights, blinking,
// breathing/fading, rainbow) using the Adafruit_NeoPixel library.

#include <Adafruit_NeoPixel.h>

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
int Wheel(byte pos);

#endif

