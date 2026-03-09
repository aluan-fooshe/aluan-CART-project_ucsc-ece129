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

      if (strongSignal) {
        Serial.println("Distance: CLOSE");
      } 
      else {
        Serial.println("Distance: FAR");
      }
    }
  }
}