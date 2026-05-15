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

int given_speed = 30;

// --- RF COMMUNICATIONS LOGIC SEGMENT ---
RF24 radioLeft(7, 8); // RF24(ce_pin, csn_pin)
RF24 radioRight(9, 10);

const byte address1[6] = "00001";
const byte address2[6] = "00002";

int leftPackets = 0;
int rightPackets = 0;

unsigned long lastMeasure = 0;

// -------------------------------------------------------------------------------
//      MAIN FUNCTION SECTION
// -------------------------------------------------------------------------------

void setup() {
  // ROBOCLAW SETUP
  Serial.begin(BAUDRATE);  // USB serial for debugging
  frontSerial.begin(BAUDRATE);
  backSerial.begin(BAUDRATE);
  roboclawFront.begin(BAUDRATE);
  roboclawBack.begin(BAUDRATE);
  Serial.println("RoboClaw sketch started");

  delay(2000);  // Wait for RoboClaw to fully boot before sending any commands

  // RF LEFT MODULE SETUP
  radioLeft.begin();
  radioLeft.setChannel(100);
  radioLeft.setAutoAck(false);
  radioLeft.openReadingPipe(1, address1);
  radioLeft.setPALevel(RF24_PA_LOW);
  radioLeft.setDataRate(RF24_250KBPS);
  radioLeft.startListening();

  // RF RIGHT MODULE SETUP
  radioRight.begin();
  radioRight.setChannel(110);
  radioRight.setAutoAck(false);
  radioRight.openReadingPipe(1, address2);
  radioRight.setPALevel(RF24_PA_LOW);
  radioRight.setDataRate(RF24_250KBPS);
  radioRight.startListening();

  // Stop RF24 before motor commands to prevent SPI interference
  radioLeft.stopListening();
  radioRight.stopListening();

  // PRINT OUT DEBUGGING STATEMENTS
  Serial.print("radioLeft connected (CE=7, CSN=8): ");
  Serial.println(radioLeft.isChipConnected() ? "YES" : "NO");
  Serial.print("radioRight connected (CE=4, CSN=5): ");
  Serial.println(radioRight.isChipConnected() ? "YES" : "NO");

  // function testing
  for (int i=0; i<=given_speed; i++){
    moveForward(RC_ADDRESS, i);
    delay(100);
  }

  delay(2000);
  moveForward(RC_ADDRESS, 0);
  
  // Move backward
  roboclawFront.ForwardM1(RC_ADDRESS, 30);
  roboclawFront.ForwardM2(RC_ADDRESS, 30);
  delay(3000);

  // Move forward
  moveForward(RC_ADDRESS, 30);
  delay(2000);

  moveForward(RC_ADDRESS, 0);
  delay(2000);
}

void loop() {

}

// -------------------------------------------------------------------------------
//      DEFINED FUNCTIONS SECTION
// -------------------------------------------------------------------------------

void moveForward(uint8_t address, uint8_t speed) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
}

void moveBackward(uint8_t address, uint8_t speed) {
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.BackwardM1(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
}
