#include <SoftwareSerial.h>
#include "RoboClaw.h"

SoftwareSerial serial(11, 10); // RX, TX
RoboClaw roboclaw(&serial, 10000);

void setup() {
  serial.begin(38400);
  roboclaw.begin(38400);
}

void loop() {
  roboclaw.ForwardM1(0x80, 64);
}