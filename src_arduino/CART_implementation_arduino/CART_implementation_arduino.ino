#include <SoftwareSerial.h>
#include "RoboClaw.h"

SoftwareSerial serial(11, 10); // RX, TX
RoboClaw roboclaw(&serial, 10000);

#define RC_ADDRESS 0x80
#define BAUDRATE 38400

void setup() {
  Serial.begin(BAUDRATE);  // USB serial for debugging
  serial.begin(BAUDRATE);
  roboclaw.begin(BAUDRATE);
  Serial.println("RoboClaw sketch started");
}

void loop() {
  int speed = 30;

  for (int i = 0; i <= speed; i++) {
    moveForward(RC_ADDRESS, i);
    delay(50);  // controls how fast it speeds up
  }

  // Move forward
  moveForward(RC_ADDRESS, speed);
  delay(2000);


  for (int i = speed; i >= 0; i--) {
    moveForward(RC_ADDRESS, i);
    delay(50);  // controls how fast it slows down
  }
}

void moveForward(uint8_t address, uint8_t target_speed){
  roboclaw.ForwardM1(address, target_speed);
  roboclaw.ForwardM2(address, target_speed);
}

void moveBackward(uint8_t address, uint8_t current_speed, uint8_t target_speed){
  roboclaw.BackwardM1(address, target_speed);
  roboclaw.BackwardM2(address, target_speed);
}

void turnForward(uint8_t address, uint8_t speed1, uint8_t speed2) {
  int steps = max(speed1, speed2);  // ramp based on the higher target speed

  for (int i = 0; i <= steps; i++) {
    uint8_t s1 = (speed1 > 0) ? map(i, 0, steps, 0, speed1) : 0;
    uint8_t s2 = (speed2 > 0) ? map(i, 0, steps, 0, speed2) : 0;
    roboclaw.ForwardM1(address, s1);
    roboclaw.ForwardM2(address, s2);
    delay(20);  // adjust for faster/slower ramp
  }
}

void turnBackward(uint8_t address, uint8_t speed1, uint8_t speed2) {
  roboclaw.BackwardM1(address, speed1);
  roboclaw.BackwardM2(address, speed2);
}

void stopMotors(uint8_t address) {
  roboclaw.ForwardM1(address, 0);
  roboclaw.ForwardM2(address, 0);
}