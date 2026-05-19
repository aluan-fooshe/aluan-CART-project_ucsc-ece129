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
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void moveBackward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turn1(uint8_t address, uint8_t speed, int delay_time = 0){
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turn2(uint8_t address, uint8_t speed, int delay_time = 0){
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);  // was BackwardM2
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

// -------------------------------------------------------------------------------
//      MAIN FUNCTION SECTION
// -------------------------------------------------------------------------------

void setup() {
  Serial.begin(BAUDRATE);
  radioLeft.begin();
  radioRight.begin();
  radioLeft.stopListening();
  radioRight.stopListening();
  frontSerial.begin(BAUDRATE);
  roboclawFront.begin(BAUDRATE);
  backSerial.begin(BAUDRATE);
  roboclawBack.begin(BAUDRATE);
  Serial.println("Ready. Format: 1 1200 or 2 1200");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    int spaceIndex = input.indexOf(' ');
    if (spaceIndex != -1) {
      int turnDir = input.substring(0, spaceIndex).toInt();
      int delay_ms = input.substring(spaceIndex + 1).toInt();

      Serial.print("Running turn");
      Serial.print(turnDir);
      Serial.print(" for ");
      Serial.print(delay_ms);
      Serial.println("ms...");

      if (turnDir == 1) {
        turn1(RC_ADDRESS, given_speed, delay_ms);
        turn1(RC_ADDRESS, 0, 400);
      } else if (turnDir == 2) {
        turn2(RC_ADDRESS, given_speed, delay_ms);
        turn2(RC_ADDRESS, 0, 400);
      }
      else if (turnDir == 0) {
        turn2(RC_ADDRESS, 0, delay_ms);
        turn2(RC_ADDRESS, 0, 400);
      }

      Serial.println("Done. Send next value.");
    } else {
      Serial.println("Bad format. Use: 1 1200");
    }
  }
}
