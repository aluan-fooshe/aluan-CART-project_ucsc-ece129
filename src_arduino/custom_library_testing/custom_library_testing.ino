#include <RF24.h>
#include <SPI.h>

// RF24(CE_pin, CSN_pin)
RF24 radio(8, 7);
// RF24 radio(9, 2);

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial on Uno

  if (radio.begin()) {
    Serial.println("radio.begin() = true (SPI OK)");
  } else {
    Serial.println("radio.begin() = false — check SPI wiring & power!");
    while (1);
  }

  if (radio.isChipConnected()) {
    Serial.println("isChipConnected() = true — NRF24L01 detected!");
  } else {
    Serial.println("isChipConnected() = false — module not found");
  }

  radio.printDetails(); // Print all register values
}

void loop() {}