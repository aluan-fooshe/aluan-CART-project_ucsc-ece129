#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE=7, CSN=8

const byte address1[6] = "00001";
const byte address2[6] = "00002";

void setup() {
  pinMode(10, OUTPUT);   // ← add this as the very first line
  digitalWrite(10, HIGH);
  Serial.begin(9600);

while (1){
  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!");
    while (1) {}
  }
}

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();

  Serial.println("Transmitter ready");
}

void loop() {
  const char text[] = "USER";

  // --- Send to LEFT receiver (channel 100) ---
  radio.setChannel(100);
  radio.openWritingPipe(address1);
  delay(2);  // let channel settle

  bool ok1 = radio.write(&text, sizeof(text));
  Serial.print("Left send: ");
  Serial.println(ok1 ? "OK" : "FAIL");

  delay(5);

  // --- Send to RIGHT receiver (channel 110) ---
  radio.setChannel(110);
  radio.openWritingPipe(address2);
  delay(2);  // let channel settle

  bool ok2 = radio.write(&text, sizeof(text));
  Serial.print("Right send: ");
  Serial.println(ok2 ? "OK" : "FAIL");

  delay(5);
}