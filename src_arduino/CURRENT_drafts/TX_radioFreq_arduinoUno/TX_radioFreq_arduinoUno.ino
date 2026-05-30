#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

RF24 radio(7, 8);
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(100);
  radio.enableCRC();               // ← add
  radio.setCRCLength(RF24_CRC_16); // ← add
  radio.setAutoAck(true);          // ← change from false
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address);
  radio.stopListening();
  Serial.println("Transmitter ready.");
}

unsigned long lastReport = 0;
int packetsSent   = 0;
int packetsFailed = 0;

void loop() {
  char payload[32] = "PING";
  bool ok = radio.write(&payload, sizeof(payload));
  if (ok) packetsSent++;
  else    packetsFailed++;

  if (millis() - lastReport >= 1000) {
    Serial.print("Sent: ");    Serial.print(packetsSent);
    Serial.print("  Failed: "); Serial.println(packetsFailed);
    packetsSent   = 0;
    packetsFailed = 0;
    lastReport    = millis();
  }
  delay(20);
}