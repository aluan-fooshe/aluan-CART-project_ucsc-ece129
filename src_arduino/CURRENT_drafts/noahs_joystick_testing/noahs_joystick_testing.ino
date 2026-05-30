#include "RoboClaw.h"
#include <SoftwareSerial.h>

// --- MOTOR CONTROLLER LOGIC SEGMENT ---
SoftwareSerial frontSerial(3, 4);  // FRONT MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawFront(&frontSerial, 100);
SoftwareSerial backSerial(5, 6);   // BACK MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawBack(&backSerial, 100);

#define RC_ADDRESS 0x80
#define BAUDRATE 38400

// Velocity PID coefficients
#define Kp 1.0
#define Ki 0.5
#define Kd 0.25
#define QPPS 100

uint8_t given_speed = 20;
int given_time = 23000;

// HW-504 Joystick pins/values
const int joyX = A0;  // A0 = left/right (turn)
const int joyY = A1;  // A1 = forward/backward

int xValue;
int yValue;

// -------------------------------------------------------
// Movement functions — speed is now a parameter, not 0
// -------------------------------------------------------

void moveForward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void moveBackward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.BackwardM1(address, speed);  // FIX: was missing
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turnRight(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turnLeft(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void stopAll() {
  roboclawFront.ForwardM1(RC_ADDRESS, 0);
  roboclawFront.ForwardM2(RC_ADDRESS, 0);
  roboclawBack.ForwardM1(RC_ADDRESS, 0);
  roboclawBack.ForwardM2(RC_ADDRESS, 0);
}

// -------------------------------------------------------
// Setup
// -------------------------------------------------------

void setup() {
  Serial.begin(9600);

  frontSerial.begin(BAUDRATE);  
  roboclawFront.begin(BAUDRATE);
  backSerial.begin(BAUDRATE);
  roboclawBack.begin(BAUDRATE);

  // Set PID Coefficients
  roboclawFront.SetM1VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
  roboclawFront.SetM2VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
  roboclawBack.SetM1VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
  roboclawBack.SetM2VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);

  delay(2000);  // Wait for RoboClaws to boot
  // -- End test sequence --
  
}

// -------------------------------------------------------
// Loop — joystick maps to actual speed, axes corrected
// -------------------------------------------------------

void loop() {
  xValue = analogRead(joyX);                  // Controls forward/backward movement
  yValue = analogRead(joyY);                  // Controls left/right movement

  if (xValue > 600) {                         //If joystick is pushed up, it moves forward
    Serial.println("Forward");
    moveForward(RC_ADDRESS, given_speed);        

  } else if (xValue < 400) {                  //If joystick is pushed down, it moves backward
    Serial.println("Backward");
    moveBackward(RC_ADDRESS, given_speed);      

  } else if (yValue < 400) {                  //If joystick is pushed left, it moves left
    Serial.println("Turn Left");
    turnLeft(RC_ADDRESS, given_speed);          

  } else if (yValue > 600) {                 //If joystick is pushed right, it moves right
    Serial.println("Turn Right");
    turnRight(RC_ADDRESS, given_speed);         

  } else {                                    //If joystick is not touched, CART will stay still 
    Serial.println("Center — Stopped");
    stopAll();
  }

  delay(100);
}
