#include <RF24.h>
#include <SPI.h>

RF24 radio(7, 8);  // CE=7, CSN=8

const byte address1[6] = "00001";  // radioLeft  — channel 100
const byte address2[6] = "00002";  // radioRight — channel 110

char payload[32] = "ping";

void setup() {
  Serial.begin(9600);

  pinMode(8, OUTPUT); digitalWrite(8, HIGH);

  if (radio.begin()) {
    Serial.println("begin(): OK");
  } else {
    Serial.println("begin(): FAILED — check wiring & power!");
    while (1);
  }

  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN); // TO MAKE THE RADIO "WORSE"!!!
  radio.setAutoAck(false);
  radio.stopListening(); // Transmit mode

  Serial.println("Transmitter ready.");
}

void loop() {
  // --- Broadcast to radioLeft (channel 100, address 00001) ---
  radio.setChannel(100);
  radio.openWritingPipe(address1);
  bool ok1 = radio.write(&payload, sizeof(payload));
  Serial.print("Sent to LEFT  (ch100): ");
  Serial.println(ok1 ? "OK" : "FAIL");

  delay(10); // Short gap before switching

  // --- Broadcast to radioRight (channel 110, address 00002) ---
  radio.setChannel(110);
  radio.openWritingPipe(address2);
  bool ok2 = radio.write(&payload, sizeof(payload));
  Serial.print("Sent to RIGHT (ch100): ");
  Serial.println(ok2 ? "OK" : "FAIL");

  delay(10); // ~50 packets/sec to each receiver
}