#include <SoftwareSerial.h>
#include <RoboClaw.h>
#include <RF24.h>

// --- MOTOR CONTROLLER LOGIC SEGMENT ---
SoftwareSerial frontSerial(3, 4); // FRONT MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawFront(&frontSerial, 10000);

SoftwareSerial backSerial(5, 6);  // BACK MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawBack(&backSerial, 10000);

#define RC_ADDRESS 0x80
#define BAUDRATE 38400

int speed = 30;

// --- RF COMMUNICATIONS LOGIC SEGMENT ---
RF24 radioLeft(7,8);
RF24 radioRight(9 ,10);

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

  // PRINT OUT DEBUGGING STATEMENTS
  Serial.print("radioLeft connected (CE=7, CSN=8): ");
  Serial.println(radioLeft.isChipConnected() ? "YES" : "NO");
  Serial.print("radioRight connected (CE=4, CSN=5): ");
  Serial.println(radioRight.isChipConnected() ? "YES" : "NO");

  for (int i = 0; i <= speed; i++) {
    moveForward(RC_ADDRESS, i);
    delay(50);  // controls how fast it speeds up
  }

  // Move forward
  moveForward(RC_ADDRESS, speed);
  delay(4000);
  Serial.print("loop completed\n");

  for (int i = speed; i >= 0; i--) {
    moveForward(RC_ADDRESS, i);
    delay(50);  // controls how fast it slows down
  }
}

void loop() {
  moveForward(RC_ADDRESS, 0);
  delay(2000);

  if(radioLeft.available()){
    char text[32];
    radioLeft.read(&text, sizeof(text));
    leftPackets++;
  }

  if(radioRight.available()){
    char text[32];
    radioRight.read(&text, sizeof(text));
    rightPackets++;
  }

  if(millis() - lastMeasure > 1000){
    int difference = leftPackets - rightPackets;
    int totalPackets = leftPackets + rightPackets;

    Serial.print("Left:");
    Serial.print(leftPackets);
    Serial.print(",");
    Serial.print("Right:");
    Serial.println(rightPackets);

    Serial.print("Left packets: ");
    Serial.println(leftPackets);
    Serial.print("Right packets: ");
    Serial.println(rightPackets);

    if(difference >= 1)
      Serial.println("USER IS LEFT");
    else if(difference <= -1)
      Serial.println("USER IS RIGHT");
    else
      Serial.println("USER IS CENTER");

    if(totalPackets > 120)
      Serial.println("VERY CLOSE");
    else if(totalPackets > 60)
      Serial.println("MEDIUM");
    else if(totalPackets > 20)
      Serial.println("FAR");
    else
      Serial.println("VERY FAR");

    Serial.println();

    leftPackets = 0;
    rightPackets = 0;
    lastMeasure = millis();
  }
}

// -------------------------------------------------------------------------------
//      DEFINED FUNCTIONS SECTION
// -------------------------------------------------------------------------------

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