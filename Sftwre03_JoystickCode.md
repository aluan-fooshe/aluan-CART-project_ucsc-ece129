# Sftwre03_JoystickCode

<b>Author (s):</b> Audrey Luan, Noah Lee </br>
<b>Date Written:</b> 2026 June 12, 06:54PM </br>
<b>Directory Path:</b>
- /aluan-CART-project_ucsc-ece129/src_arduino/CURRENT_drafts
- /aluan-CART-project_ucsc-ece129/src_arduino/OLDER_drafts

### Notes

Part 3 of the Software subsystem appendices, includes all the code used to program the joystick implementation of the CART

## joystick_wireless_receiver.ino

```cpp
#include "RoboClaw.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//RF Stuff
RF24 radio(2, 10); //ce=2, csn=10
const byte address[6] = "00001";

// --- MOTOR CONTROLLER LOGIC SEGMENT (Audrey) ---
SoftwareSerial frontSerial(3, 4);  // FRONT MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawFront(&frontSerial, 100);
SoftwareSerial backSerial(5, 6);   // BACK MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawBack(&backSerial, 100);

#define RC_ADDRESS 0x80
#define BAUDRATE 38400

// Velocity PID coefficients
#define Kp 1.0
#define Ki 0.5
#define Kd 0.25
#define QPPS 100

uint8_t given_speed = 40;
int given_time = 23000;

//Joystick Data
struct JoystickData {
  int xValue;
  int yValue;
};


// -------------------------------------------------------
// Movement functions (Audrey)
// -------------------------------------------------------

void moveForward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void moveBackward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.BackwardM1(address, speed);  // FIX: was missing
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turnRight(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turnLeft(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void stopAll() {
  roboclawFront.ForwardM1(RC_ADDRESS, 0);
  roboclawFront.ForwardM2(RC_ADDRESS, 0);
  roboclawBack.ForwardM1(RC_ADDRESS, 0);
  roboclawBack.ForwardM2(RC_ADDRESS, 0);
}

//setup function
void setup() {
  Serial.begin(9600);
  
//RF Stuff
  radio.begin();
  radio.openReadingPipe(0, address); 
  
  radio.setChannel(108);           // Match with transmitter
  radio.setDataRate(RF24_250KBPS); // Match with transmitter
 
  radio.setPALevel(RF24_PA_LOW); // Match with transmitter
  radio.startListening();        // RECEIVER
  
//Motorcontroller Stuff (Audrey)
  frontSerial.begin(BAUDRATE);  
  roboclawFront.begin(BAUDRATE);
  backSerial.begin(BAUDRATE);
  roboclawBack.begin(BAUDRATE);

  // Set PID Coefficients
  roboclawFront.SetM1VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
  roboclawFront.SetM2VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
  roboclawBack.SetM1VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
  roboclawBack.SetM2VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);

  delay(2000);  // Wait for RoboClaws to boot
}


void loop() {
  
  // 0 - 511 - 1023
  bool got_signal = radio.available();
  
  if (got_signal) {

    JoystickData data;
    radio.read(&data, sizeof(data));

    Serial.print("X: ");
    Serial.print(data.xValue);
    Serial.print(" Y: ");
    Serial.println(data.yValue);

    if (data.xValue > 600 && data.xValue < 800) {
      Serial.println("Forward");
      moveForward(RC_ADDRESS, given_speed);
    } else if (data.xValue > 800) {
      Serial.println("FAST Forward");
      moveForward(RC_ADDRESS, (uint8_t)(given_speed * 1.5));
    } else if (data.xValue < 400 && data.xValue > 200) {
      Serial.println("Backward");
      moveBackward(RC_ADDRESS, given_speed);
    } else if (data.xValue < 200) {
      Serial.println("FAST Backward");
      moveBackward(RC_ADDRESS, (uint8_t)(given_speed * 1.5));
    } else if (data.yValue < 400) {
      Serial.println("Turn Left");
      turnLeft(RC_ADDRESS, given_speed);
    } else if (data.yValue > 600) {
      Serial.println("Turn Right");
      turnRight(RC_ADDRESS, given_speed);
    } else {
      Serial.println("Stopped");
      stopAll();
    }

  } else {                                 // If no signal then also stop

    Serial.println("NO SIGNAL");
    stopAll();
  }

  delay(50);
}

```

## joystick_wireless_transmitter.ino

```cpp
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

```

## noahs_joystick_testing.ino

```cpp
#include "RoboClaw.h"
#include <SoftwareSerial.h>

// --- MOTOR CONTROLLER LOGIC SEGMENT ---
SoftwareSerial frontSerial(3, 4);  // FRONT MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawFront(&frontSerial, 100);
SoftwareSerial backSerial(5, 6);   // BACK MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawBack(&backSerial, 100);

#define RC_ADDRESS 0x80
#define BAUDRATE 38400

// Velocity PID coefficients
#define Kp 1.0
#define Ki 0.5
#define Kd 0.25
#define QPPS 100

uint8_t given_speed = 20;
int given_time = 23000;

// HW-504 Joystick pins/values
const int joyX = A0;  // A0 = left/right (turn)
const int joyY = A1;  // A1 = forward/backward

int xValue;
int yValue;

// -------------------------------------------------------
// Movement functions — speed is now a parameter, not 0
// -------------------------------------------------------

void moveForward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void moveBackward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.BackwardM1(address, speed);  // FIX: was missing
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turnRight(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turnLeft(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void stopAll() {
  roboclawFront.ForwardM1(RC_ADDRESS, 0);
  roboclawFront.ForwardM2(RC_ADDRESS, 0);
  roboclawBack.ForwardM1(RC_ADDRESS, 0);
  roboclawBack.ForwardM2(RC_ADDRESS, 0);
}

// -------------------------------------------------------
// Setup
// -------------------------------------------------------

void setup() {
  Serial.begin(9600);

  frontSerial.begin(BAUDRATE);  
  roboclawFront.begin(BAUDRATE);
  backSerial.begin(BAUDRATE);
  roboclawBack.begin(BAUDRATE);

  // Set PID Coefficients
  roboclawFront.SetM1VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
  roboclawFront.SetM2VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
  roboclawBack.SetM1VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
  roboclawBack.SetM2VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);

  delay(2000);  // Wait for RoboClaws to boot
  // -- End test sequence --
  
}

// -------------------------------------------------------
// Loop — joystick maps to actual speed, axes corrected
// -------------------------------------------------------

void loop() {
  xValue = analogRead(joyX);                  // Controls forward/backward movement
  yValue = analogRead(joyY);                  // Controls left/right movement

  if (xValue > 600) {                         //If joystick is pushed up, it moves forward
    Serial.println("Forward");
    moveForward(RC_ADDRESS, given_speed);        

  } else if (xValue < 400) {                  //If joystick is pushed down, it moves backward
    Serial.println("Backward");
    moveBackward(RC_ADDRESS, given_speed);      

  } else if (yValue < 400) {                  //If joystick is pushed left, it moves left
    Serial.println("Turn Left");
    turnLeft(RC_ADDRESS, given_speed);          

  } else if (yValue > 600) {                 //If joystick is pushed right, it moves right
    Serial.println("Turn Right");
    turnRight(RC_ADDRESS, given_speed);         

  } else {                                    //If joystick is not touched, CART will stay still 
    Serial.println("Center — Stopped");
    stopAll();
  }

  delay(100);
}

```