/**********************************************************************
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
#include "Freenove_4WD_Car_For_Pico_W.h"

// setup() runs ONCE when the board powers on or is reset.
// It's the perfect place to prepare things before the main program starts.
void setup() {
  Serial.begin(9600);   //Start the serial connection so we could print debug messages to a computer if we wanted to (not used in this sketch)
  Servo_Setup();        //Get the steering/pan servo ready (not driven in this sketch, but must still be initialized)
  Motor_Setup();  //Motor Initializes
}

// loop() runs over and over again, forever, right after setup() finishes.
// This is where we tell all 4 mecanum wheels what to do, one movement at a time.
//
// Motor_M_Move(M1, M2, M3, M4) sets the speed of each wheel independently:
//   M1 = front-left wheel   M2 = back-left wheel
//   M3 = front-right wheel  M4 = back-right wheel
// Each value is a PWM speed from -100 (full speed backward) to 100 (full speed forward),
// where 0 means "stopped". PWM (Pulse Width Modulation) is how we get "in-between" speeds
// out of a motor that's really just an on/off switch: the code flips the motor on and off
// very fast, and the fraction of time it's "on" controls how fast it spins.
//
// Mecanum wheels have rollers mounted at 45 degrees around their rim (instead of a normal
// flat tyre). That means a spinning mecanum wheel doesn't just push the car forward/back —
// part of its force also pushes the car sideways. By choosing different speeds and
// directions for each of the 4 wheels, those sideways and forward/back forces can be made
// to cancel out or add up in different combinations, which is how the car can drive
// forward, spin on the spot, slide sideways, or glide diagonally, all without turning
// the wheels themselves like a steering wheel would.
void loop() {
  Motor_M_Move(50, 50, 50, 50);  //Farward
  // All 4 wheels spin forward at the same speed. Each wheel's small sideways push is
  // cancelled out by the wheel diagonally opposite it, leaving only forward motion.
  delay(1000);  //Keep driving for 1000 milliseconds (1 second) before the next command
  Motor_M_Move(0, 0, 0, 0);  //Stop
  delay(1000);
  Motor_M_Move(-50, -50, -50, -50);  //Backward
  // Same idea as forward, but every wheel spins the opposite way.
  delay(1000);
  Motor_M_Move(0, 0, 0, 0);  //Stop
  delay(1000);
  Motor_M_Move(-50, -50, 50, 50);  //Turn left
  // Left wheels (M1, M2) spin backward while right wheels (M3, M4) spin forward.
  // Their forward/backward pushes now oppose each other instead of adding up, so the car
  // doesn't travel forward at all — instead it rotates on the spot, turning left, like
  // paddling a canoe backward on one side and forward on the other.
  delay(1000);
  Motor_M_Move(0, 0, 0, 0);  //Stop
  delay(1000);
  Motor_M_Move(50, 50, -50, -50);  //Turn right
  // The mirror image of "Turn left": left wheels forward, right wheels backward,
  // spinning the car the opposite way on the spot.
  delay(1000);
  Motor_M_Move(0, 0, 0, 0);  //Stop
  delay(1000);
  Motor_M_Move(-50, 50, -50, 50);  //Move left
  // Front wheels (M1, M3) spin one way and back wheels (M2, M4) spin the other way.
  // This cancels out any forward/backward motion, but the sideways force from every
  // wheel's angled rollers points the same way, so the car slides left without turning
  // to face a new direction — this is called "strafing".
  delay(1000);
  Motor_M_Move(0, 0, 0, 0);  //Stop
  delay(1000);
  Motor_M_Move(50, -50, 50, -50);  //Move right
  // The exact opposite pattern to "Move left", so the sideways forces add up pointing
  // right instead, making the car strafe right.
  delay(1000);
  Motor_M_Move(0, 0, 0, 0);  //Stop
  delay(1000);
  Motor_M_Move(-50, 50, -50, 50);  //Move diagonally upward to the left
  // Note: this uses the exact same wheel speeds as the "Move left" strafe above — on this
  // kit's wheel layout, this pattern produces a sideways/diagonal glide rather than a pure
  // forward diagonal, so the visible movement will look similar to strafing left.
  delay(1000);
  Motor_M_Move(0, 0, 0, 0);  //Stop
  delay(1000);
  Motor_M_Move(0, -50, 0, -50);  //Move diagonally downward to the right
  // Only the two back wheels (M2, M4) are driven; the front wheels are left stopped.
  // Driving just one diagonal pair of wheels means their forward/back push and sideways
  // push don't get cancelled by a partner wheel, so the car glides diagonally instead of
  // moving in a straight line.
  delay(1000);
  Motor_M_Move(0, 0, 0, 0);  //Stop
  delay(1000);
  Motor_M_Move(50, 0, 50, 0);  //Move diagonally upward to the right
  // Only the two front wheels (M1, M3) are driven forward; the back wheels are left
  // stopped, sending the car gliding diagonally forward-and-sideways.
  delay(1000);
  Motor_M_Move(0, 0, 0, 0);  //Stop
  delay(1000);
  Motor_M_Move(-50, 0, -50, 0);  //Move diagonally downward to the left
  // Same idea as above, but the front wheels spin backward instead, sending the car
  // gliding diagonally the opposite way.
  delay(1000);
  Motor_M_Move(0, 0, 0, 0);  //Stop
  delay(1000);
}
