#ifndef _FREENOVE_4WD_CAR_EMOTION_H
#define _FREENOVE_4WD_CAR_EMOTION_H

// ============================================================================
// This header describes the "face": a small LED matrix (HT16K33/VK16K33
// driven display, two 8x8 grids of dots side by side) that can show simple
// animated eyes and emotions, like a robot face. The .cpp file has all the
// actual pixel-pattern data and drawing logic; this file just lists the
// pieces of it that other files (like the main .ino) are allowed to use.
// ============================================================================

#define EMOTION_ADDRESS 0x71  // I2C address the matrix driver chip answers to
#define EMOTION_SDA     4     // I2C data pin
#define EMOTION_SCL     5     // I2C clock pin

// Each of these draws one animation frame on the matrix, based on how much
// time (delay_ms) should pass between frames. Call them repeatedly (e.g.
// once per loop()) and they will animate themselves over time.
void clearEmtions(void);           //clear all
void eyesRotate(int delay_ms);     //rotate eyes
void eyesBlink(int delay_ms);      //blink eyes
void eyesSmile(int delay_ms);      //smile
void eyesCry(int delay_ms);        //cry
void wheel(int mode, int delay_ms);//wheel

extern int emotion_task_mode;  // Which animation Emotion_Show() is currently drawing
extern int emotion_flag;       // Which face the remote control most recently selected

void Emotion_Setup();              //Initializes the Led Matrix
void Emotion_Show(int mode);       //Display:0-Display off,1-Turn the eyes,2-blink eyes,3-smile,4-cry,5-left-wheel,6-right-wheel
void Emotion_SetMode(int mode);    //set the emotion show mode
void staticEmtions(int emotion);   //show static emotion

#endif
