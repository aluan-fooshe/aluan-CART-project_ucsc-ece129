#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);  // CE=9, CSN=10

// Matches Python: address = b"\xe1\xf0\xf0\xf0\xf0"
// RF24 stores LSByte first, so 0xF0F0F0F0E1LL = same address
const uint64_t address = 0xF0F0F0F0E1LL;

void setup() {
  Serial.begin(115200);

  if (!radio.begin()) {
    Serial.println("ERROR: nRF24L01 not responding - check wiring!");
    while (1) {}
  }

  radio.setPALevel(RF24_PA_MAX);      // matches: radio.pa_level = RF24_PA_MAX
  radio.setChannel(0x76);             // matches: radio.channel = 0x76
  radio.setDataRate(RF24_1MBPS);      // matches: radio.data_rate = RF24_1MBPS
  radio.enableDynamicPayloads();      // matches: radio.dynamic_payloads = True
  radio.setAutoAck(true);             // matches: radio.set_auto_ack(True)
  radio.openWritingPipe(address);     // matches: radio.open_rx_pipe(1, address)
  radio.stopListening();              // TX mode

  Serial.println("nRF24L01 ready. Transmitting...");
}

void loop() {
  const char text[] = "Hello World!!!";

  Serial.print("Sending: ");
  Serial.println(text);

  bool success = radio.write(&text, sizeof(text));

  if (success) {
    Serial.println("OK - ACK received");
  } else {
    Serial.println("FAILED - No ACK (check Pi receiver is running)");
  }

  delay(1000);
}
