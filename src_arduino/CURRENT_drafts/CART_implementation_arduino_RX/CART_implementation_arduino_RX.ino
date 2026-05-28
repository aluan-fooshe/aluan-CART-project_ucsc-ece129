#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

// -------------------------------------------------------------------------------
//      RF RECEIVER SETUP
// -------------------------------------------------------------------------------

RF24 radioLeft(8, 7);   // CE=8, CSN=7
RF24 radioRight(9, 2); // CE=9, CSN=2

const byte address1[6] = "00001";
const byte address2[6] = "00002";

int leftPackets  = 0;
int rightPackets = 0;

unsigned long lastMeasure = 0;
const unsigned long MEASURE_INTERVAL = 50; // ms

int printEveryInstanceOf = 10;
int counter = 0;
int leftTotal  = 0;
int rightTotal = 0;

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
  Serial.print("radioRight connected (CE=9, CSN=2): ");
  Serial.println(radioRight.isChipConnected() ? "YES" : "NO");

  Serial.println("System ready.");
}

// -------------------------------------------------------------------------------
//      MAIN LOOP
// -------------------------------------------------------------------------------

void loop() {
  // Drain entire LEFT buffer
  if (radioLeft.available()) {
    char text[32];
    radioLeft.read(&text, sizeof(text));
    leftPackets++;
  }

  delayMicroseconds(100); // SPI bus settle before switching radios

  // Drain entire RIGHT buffer
  if (radioRight.available()) {
    char text[32];
    radioRight.read(&text, sizeof(text));
    rightPackets++;
  }

  // Cap to prevent overflow
  leftPackets  = min(leftPackets,  9999);
  rightPackets = min(rightPackets, 9999);

  // Every second, print packet counts
  if (millis() - lastMeasure >= MEASURE_INTERVAL) {
    counter++;

    // Accumulate every interval
    leftTotal  += leftPackets;
    rightTotal += rightPackets;

    if (counter >= printEveryInstanceOf) {
      int difference   = leftTotal - rightTotal;
      int totalPackets = leftTotal + rightTotal;

      Serial.print("Left:");
      Serial.print(leftTotal);
      Serial.print("  Right:");
      Serial.print(rightTotal);
      Serial.print("  Diff:");
      Serial.print(difference);
      Serial.print("  Total:");
      Serial.println(totalPackets);
      Serial.println();

      // Reset accumulators and counter
      leftTotal  = 0;
      rightTotal = 0;
      counter    = 0;
      }

      leftPackets  = 0;
      rightPackets = 0;
      lastMeasure  = millis();
    }
}