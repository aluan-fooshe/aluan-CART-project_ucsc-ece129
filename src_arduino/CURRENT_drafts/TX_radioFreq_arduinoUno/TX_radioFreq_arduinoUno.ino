#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);

const byte address1[6] = "00001";
const byte address2[6] = "00002";

void setup() {
  Serial.begin(9600);

  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!");
    while (1) {}
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(false);   // CRITICAL — no ACK expected
  radio.setRetries(0, 0);    // CRITICAL — don't retry
  radio.stopListening();

  Serial.println("Transmitter ready");
}

// void loop() {
//   const char text[] = "USER";

//   radio.setChannel(100);
//   radio.openWritingPipe(address1);
//   radio.write(&text, sizeof(text));

//   radio.setChannel(110);
//   radio.openWritingPipe(address2);
//   radio.write(&text, sizeof(text));
// }

int packetsSent = 0;
unsigned long lastMeasure = 0;

void loop() {
  const char text[] = "USER";

  radio.setChannel(100);
  radio.openWritingPipe(address1);
  radio.write(&text, sizeof(text));
  packetsSent++;

  // delay(10);

  radio.setChannel(110);
  radio.openWritingPipe(address2);
  radio.write(&text, sizeof(text));
  packetsSent++;

  if (millis() - lastMeasure > 1000) {
    Serial.print("Packets sent: ");
    Serial.println(packetsSent);
    packetsSent = 0;
    lastMeasure = millis();
  }
}