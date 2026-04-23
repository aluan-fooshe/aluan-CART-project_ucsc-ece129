# README

these files were made by Jayant D.

receiver side
user side send

## Basic testing code for Arduino Uno R3 + Roboclaw ST 2x45A

code written on 04/22/2026.

```cpp
#include <SoftwareSerial.h>
#include "RoboClaw.h"

SoftwareSerial serial(11, 10); // RX, TX
RoboClaw roboclaw(&serial, 10000);

#define RC_ADDRESS 0x80
#define BAUDRATE 9600

void setup() {
  Serial.begin(BAUDRATE);  // USB serial for debugging
  serial.begin(BAUDRATE);
  roboclaw.begin(BAUDRATE);
  Serial.println("RoboClaw sketch started");
}

void loop() {
  // Move forward
  roboclaw.ForwardM1(RC_ADDRESS, 64);
  roboclaw.ForwardM2(RC_ADDRESS, 64);
  delay(2000);

  // Stop
  roboclaw.ForwardM1(RC_ADDRESS, 0);
  roboclaw.ForwardM2(RC_ADDRESS, 64);
  delay(2000);

  // Move backward
  roboclaw.BackwardM1(RC_ADDRESS, 64);
  roboclaw.BackwardM2(RC_ADDRESS, 64);
  delay(2000);
}
```