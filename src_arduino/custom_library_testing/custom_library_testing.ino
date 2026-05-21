#include <SPI.h>
#include <SoftwareSerial.h>
#include <RoboClaw.h>
#include <RF24.h>
#include <nRF24L01.h>

// -------------------------------------------------------------------------------
//      MOTOR CONTROLLER SETUP
// -------------------------------------------------------------------------------

SoftwareSerial frontSerial(3, 4);  // FRONT MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawFront(&frontSerial, 100);

SoftwareSerial backSerial(5, 6);   // BACK MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawBack(&backSerial, 100);

#define RC_ADDRESS 0x80
#define BAUDRATE 38400

// -------------------------------------------------------------------------------
//      RF RECEIVER SETUP
// -------------------------------------------------------------------------------

RF24 radioLeft(7, 8);   // CE=7, CSN=8
RF24 radioRight(9, 10); // CE=9, CSN=10

const byte address1[6] = "00001";
const byte address2[6] = "00002";

int leftPackets  = 0;
int rightPackets = 0;

unsigned long lastMeasure = 0;
const unsigned long MEASURE_INTERVAL = 1000; // ms

// -------------------------------------------------------------------------------
//      SPEED CONFIGURATION
// -------------------------------------------------------------------------------

// Distance thresholds (total packets per second)
// Tune these to match your real-world environment
const int VERY_CLOSE_THRESH  = 120;
const int MEDIUM_THRESH      = 60;
const int FAR_THRESH         = 20;
const int VERY_FAR_THRESH    = 10;

// Speeds for each distance band (closer = slower)
const uint8_t SPEED_VERY_CLOSE = 5;   // nearly stopped — collision avoidance
const uint8_t SPEED_MEDIUM     = 20;
const uint8_t SPEED_FAR        = 50;  // fastest — chase the user
const uint8_t SPEED_VERY_FAR   = 0;  

// Turning speed when user is clearly off-center
const uint8_t SPEED_TURN = 30;

// How many more packets on one side before we consider the user off-center
// (raise this to make the centering zone wider)
const int CENTER_DEADBAND = 10;

// -------------------------------------------------------------------------------
//      MOTOR PRIMITIVES
// -------------------------------------------------------------------------------

void stopAll() {
  roboclawFront.ForwardM1(RC_ADDRESS, 0);
  roboclawFront.ForwardM2(RC_ADDRESS, 0);
  roboclawBack.ForwardM1(RC_ADDRESS, 0);
  roboclawBack.ForwardM2(RC_ADDRESS, 0);
}

void moveForward(uint8_t speed) {
  roboclawFront.ForwardM1(RC_ADDRESS, speed);
  roboclawFront.ForwardM2(RC_ADDRESS, speed);
  roboclawBack.BackwardM1(RC_ADDRESS, speed);
  roboclawBack.BackwardM2(RC_ADDRESS, speed);
}

void moveBackward(uint8_t speed) {
  roboclawFront.BackwardM1(RC_ADDRESS, speed);
  roboclawFront.BackwardM2(RC_ADDRESS, speed);
  roboclawBack.ForwardM1(RC_ADDRESS, speed);
  roboclawBack.ForwardM2(RC_ADDRESS, speed);
}

// Turn LEFT in place (user is to the left)
void turnLeft(uint8_t speed) {
  roboclawFront.BackwardM1(RC_ADDRESS, speed);
  roboclawFront.ForwardM2(RC_ADDRESS, speed);
  roboclawBack.ForwardM1(RC_ADDRESS, speed);
  roboclawBack.BackwardM2(RC_ADDRESS, speed);
}

// Turn RIGHT in place (user is to the right)
void turnRight(uint8_t speed) {
  roboclawFront.ForwardM1(RC_ADDRESS, speed);
  roboclawFront.BackwardM2(RC_ADDRESS, speed);
  roboclawBack.BackwardM1(RC_ADDRESS, speed);
  roboclawBack.ForwardM2(RC_ADDRESS, speed);
}

// -------------------------------------------------------------------------------
//      SPEED FROM DISTANCE HELPER
// -------------------------------------------------------------------------------

uint8_t speedFromTotalPackets(int total) {
if (total < VERY_CLOSE_THRESH)                            return SPEED_VERY_CLOSE;
if (total > VERY_CLOSE_THRESH && total < FAR_THRESH)      return SPEED_MEDIUM;
  if (total > FAR_THRESH && total < VERY_FAR_THRESH)      return SPEED_FAR;
else                                                      return SPEED_VERY_FAR;
}

// -------------------------------------------------------------------------------
//      SETUP
// -------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);

  // Radio — LEFT
  radioLeft.begin();
  radioLeft.setChannel(100);
  radioLeft.setAutoAck(false);
  radioLeft.openReadingPipe(1, address1);
  radioLeft.setPALevel(RF24_PA_LOW);
  radioLeft.setDataRate(RF24_250KBPS);
  radioLeft.startListening();

  // Radio — RIGHT
  radioRight.begin();
  radioRight.setChannel(110);
  radioRight.setAutoAck(false);
  radioRight.openReadingPipe(1, address2);
  radioRight.setPALevel(RF24_PA_LOW);
  radioRight.setDataRate(RF24_250KBPS);
  radioRight.startListening();

  Serial.print("radioLeft  connected (CE=7, CSN=8):  ");
  Serial.println(radioLeft.isChipConnected()  ? "YES" : "NO");
  Serial.print("radioRight connected (CE=9, CSN=10): ");
  Serial.println(radioRight.isChipConnected() ? "YES" : "NO");

  // Motor controllers
  frontSerial.begin(BAUDRATE);
  roboclawFront.begin(BAUDRATE);
  backSerial.begin(BAUDRATE);
  roboclawBack.begin(BAUDRATE);

  stopAll();
  Serial.println("System ready.");
}

// -------------------------------------------------------------------------------
//      MAIN LOOP
// -------------------------------------------------------------------------------

void loop() {
  // --- Continuously drain radio buffers and count packets ---
  if (radioLeft.available()) {
    char text[32];
    radioLeft.read(&text, sizeof(text));
    leftPackets++;
  }

  if (radioRight.available()) {
    char text[32];
    radioRight.read(&text, sizeof(text));
    rightPackets++;
  }

  // --- Every MEASURE_INTERVAL ms, decide what to do ---
  if (millis() - lastMeasure >= MEASURE_INTERVAL) {

    int difference   = leftPackets - rightPackets; // + = user left, - = user right
    int totalPackets = leftPackets + rightPackets;
    uint8_t driveSpeed = speedFromTotalPackets(totalPackets);

    // --- Serial debug ---
    Serial.print("Left:");
    Serial.print(leftPackets);
    Serial.print("  Right:");
    Serial.print(rightPackets);
    Serial.print("  Diff:");
    Serial.print(difference);
    Serial.print("  Total:");
    Serial.println(totalPackets);

    // --- Position label ---
    String position;
    if      (difference >  CENTER_DEADBAND) position = "LEFT";
    else if (difference < -CENTER_DEADBAND) position = "RIGHT";
    else                                    position = "CENTER";

    // --- Distance label ---
    String distance;
    if      (totalPackets < VERY_CLOSE_THRESH) distance = "VERY CLOSE";
    else if (totalPackets > VERY_CLOSE_THRESH && totalPackets < MEDIUM_THRESH)     distance = "MEDIUM";
    else if (totalPackets > MEDIUM_THRESH && totalPackets < FAR_THRESH)        distance = "FAR";
    else                                       distance = "VERY FAR";

    Serial.print("Position: ");
    Serial.print(position);
    Serial.print("  Distance: ");
    Serial.print(distance);
    Serial.print("  DriveSpeed: ");
    Serial.println(driveSpeed);

    // --- Motor decision ---
    if (position == "LEFT") {
      // User is to the left — turn left to face them
      turnLeft(SPEED_TURN);
      Serial.println("Action: TURN LEFT");

    } else if (position == "RIGHT") {
      // User is to the right — turn right to face them
      turnRight(SPEED_TURN);
      Serial.println("Action: TURN RIGHT");

    } else {
      // User is centered
      if (totalPackets == 0) {
        // No signal at all — stop and wait
        stopAll();
        Serial.println("Action: STOP (no signal)");
      } else {
        // Move forward, speed inversely scaled with proximity
        moveForward(driveSpeed);
        Serial.println("Action: FORWARD");
      }
    }

    Serial.println();

    // Reset counters
    leftPackets  = 0;
    rightPackets = 0;
    lastMeasure  = millis();
  }
}