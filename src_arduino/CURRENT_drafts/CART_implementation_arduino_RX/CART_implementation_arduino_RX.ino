#include <SoftwareSerial.h>
#include <RoboClaw.h>
#include <RF24.h>

// --- MOTOR CONTROLLER LOGIC SEGMENT ---
SoftwareSerial frontSerial(3, 4);  // FRONT MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawFront(&frontSerial, 100);

SoftwareSerial backSerial(5, 6);  // BACK MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawBack(&backSerial, 100);

#define RC_ADDRESS 0x80
#define BAUDRATE 38400
//38400

int given_speed = 10;

// --- RF COMMUNICATIONS LOGIC SEGMENT ---
RF24 radioLeft(7, 8); // RF24(ce_pin, csn_pin)
RF24 radioRight(9, 10);

const byte address1[6] = "00001";
const byte address2[6] = "00002";

int leftPackets = 0;
int rightPackets = 0;

unsigned long lastMeasure = 0;

// -------------------------------------------------------------------------------
//      DEFINED FUNCTIONS SECTION
// -------------------------------------------------------------------------------

void moveForward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  // roboclawBack.BackwardM1(address, speed);
  // roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void moveBackward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  // roboclawBack.ForwardM1(address, speed);
  // roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turn1(uint8_t address, uint8_t speed, int delay_time = 0){
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  // roboclawBack.BackwardM1(address, speed);
  // roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turn2(uint8_t address, uint8_t speed, int delay_time = 0){
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  // roboclawBack.ForwardM1(address, speed);
  // roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

// -------------------------------------------------------------------------------
//      MAIN FUNCTION SECTION
// -------------------------------------------------------------------------------

void setup() {
  Serial.begin(57600);
  radioLeft.begin();
  radioRight.begin();
  radioLeft.stopListening();
  radioRight.stopListening();
  frontSerial.begin(BAUDRATE);
  roboclawFront.begin(BAUDRATE);
  backSerial.begin(BAUDRATE);
  roboclawBack.begin(BAUDRATE);
  Serial.println("Ready for testing (with weight)");

  delay(2000);
  moveForward(RC_ADDRESS, 25, 8500);
}

void loop() {
  moveForward(RC_ADDRESS, 0, 2000);
  Serial.println("loop running");
}