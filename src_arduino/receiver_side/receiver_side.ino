#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);

const byte address[6] = "00001";

const int LED_PIN = 13;

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_HIGH);   // stronger receive
  radio.startListening();
}

void loop() {

  // static: persists across loop() calls
  static int  hysteresisCount   = 0;
  static bool reportedClose     = false;

  // Thresholds: must accumulate N consecutive reads to flip state
  const int CLOSE_THRESHOLD =  3;   // N strong reads  → report CLOSE
  const int FAR_THRESHOLD   = -3;   // N weak reads    → report FAR

  if (radio.available()) {

    // check signal strength BEFORE reading
    bool strongSignal = radio.testRPD();

    char text[32] = "";
    radio.read(&text, sizeof(text));

    Serial.println(text);

    if (String(text) == "USER") {

      Serial.println("User detected");

      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);

      // accumulate evidence of strong / weak signal
      if (strongSignal) {
        hysteresisCount++;
      } else {
        hysteresisCount--;
      }

      // clamp so the counter doesn't grow unbounded
      hysteresisCount = constrain(hysteresisCount, FAR_THRESHOLD, CLOSE_THRESHOLD);

      // only flip the reported state once a threshold is reached
      if (hysteresisCount >= CLOSE_THRESHOLD) {
        reportedClose = true;
      } else if (hysteresisCount <= FAR_THRESHOLD) {
        reportedClose = false;
      }

      if (reportedClose) {
        Serial.println("Distance: CLOSE");
      } else {
        Serial.println("Distance: FAR");
      }
    }
  }
}