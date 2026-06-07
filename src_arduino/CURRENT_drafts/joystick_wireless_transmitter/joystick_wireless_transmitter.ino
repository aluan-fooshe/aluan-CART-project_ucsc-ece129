#include "RoboClaw.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//srituhobby.com for RF stuff
//Include CE and CSN pins respectively (srituhobby.com)
RF24 radio(9, 8);

//Create an address to identify the receiver (srituhobby.com)
const byte address[6] = "00001";

// HW-504 Joystick pins/values
const int joyX = A0;            // A0 = left/right (turn)
const int joyY = A1;            // A1 = forward/backward

struct JoystickData {           //idk makes it easier to define later
  int xValue;
  int yValue;
};

// -------------------------------------------------------
// Setup
// -------------------------------------------------------

void setup() {
  Serial.begin(9600);

  //RF stuff
  radio.begin(); //srituhobby.com

  //set address
  radio.openWritingPipe(address); //srituhobby.com

  radio.setChannel(108); // Match with receiver
  radio.setDataRate(RF24_250KBPS); // Match with receiver

  radio.setPALevel(RF24_PA_LOW); // Match with receiver
  
  //Set module as transmitter
  radio.stopListening(); //srituhobby.com
  
}

// -------------------------------------------------------
// Loop 
// -------------------------------------------------------

void loop() {
  //RF Sensor + joystick 
  //Joystick data needs to be transmittted
  JoystickData data;
  data.xValue = analogRead(joyX);
  data.yValue = analogRead(joyY);                 
  bool success = radio.write(&data, sizeof(data));

  //Test to see if it is transmitting
    if (success) {
    Serial.print("Sent X: ");
    Serial.print(data.xValue);
    Serial.print("  Y: ");
    Serial.println(data.yValue);
  } else {
    Serial.println("Transmit failed — check wiring");
  }

  delay(50);  
}
