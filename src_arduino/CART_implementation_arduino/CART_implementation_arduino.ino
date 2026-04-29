#include <SoftwareSerial.h>
#include "RoboClaw.h"

// FRONT MOTORS PAIR
SoftwareSerial frontSerial(10, 11); // RX (yellow), TX (orange)
RoboClaw roboclawFront(&frontSerial, 10000);

// BACK MOTORS PAIR
SoftwareSerial backSerial(8, 9); // RX (yellow), TX (orange)
RoboClaw roboclawBack(&backSerial, 10000);

#define RC_ADDRESS 0x80
#define BAUDRATE 38400

void setup() {
  Serial.begin(BAUDRATE);  // USB serial for debugging
  frontSerial.begin(BAUDRATE);
  backSerial.begin(BAUDRATE);
  roboclawFront.begin(BAUDRATE);
  roboclawBack.begin(BAUDRATE);
  Serial.println("RoboClaw sketch started");

  int speed = 30;

  for (int i = 0; i <= speed; i++) {
    moveForward(RC_ADDRESS, i);
    delay(50);  // controls how fast it speeds up
  }

  // Move forward
  moveForward(RC_ADDRESS, speed);
  delay(4000);
  Serial.print("loop completed\n");

  // for (int i = speed; i >= 0; i--) {
  //   moveForward(RC_ADDRESS, i);
  //   delay(50);  // controls how fast it slows down
  // }
}

void loop() {
  Serial.print("running\n");
  moveForward(RC_ADDRESS, 30);
  delay(2000);
}

void moveForward(uint8_t address, uint8_t speed) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
}

void moveBackward(uint8_t address, uint8_t speed) {
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
}

void turnForward(uint8_t address, uint8_t speed1, uint8_t speed2) {
  int steps = max(speed1, speed2);  // ramp based on the higher target speed

  for (int i = 0; i <= steps; i++) {
    uint8_t s1 = (speed1 > 0) ? map(i, 0, steps, 0, speed1) : 0;
    uint8_t s2 = (speed2 > 0) ? map(i, 0, steps, 0, speed2) : 0;
    roboclawFront.ForwardM1(address, s1);
    roboclawFront.ForwardM2(address, s2);
    roboclawBack.ForwardM1(address, s1);
    roboclawBack.ForwardM2(address, s2);
    delay(20);  // adjust for faster/slower ramp
  }
}

void turnBackward(uint8_t address, uint8_t speed1, uint8_t speed2) {
  roboclawFront.BackwardM1(address, speed1);
  roboclawFront.BackwardM2(address, speed2);
  roboclawBack.BackwardM1(address, speed1);
  roboclawBack.BackwardM2(address, speed2);
}

void stopMotors(uint8_t address) {
  roboclawFront.ForwardM1(address, 0);
  roboclawFront.ForwardM2(address, 0);
  roboclawBack.ForwardM1(address, 0);
  roboclawBack.ForwardM2(address, 0);
}