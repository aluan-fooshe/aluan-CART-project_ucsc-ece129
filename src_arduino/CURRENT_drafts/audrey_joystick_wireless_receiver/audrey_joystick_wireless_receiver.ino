#include "RoboClaw.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//RF Stuff
RF24 radio(2, 10); //ce=2, csn=10
const byte address[6] = "00001";

// -------------------------------------------------------
//  MOTOR CONTROLLER LOGIC SEGMENT (Audrey) 
// -------------------------------------------------------
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

//Joystick Data
struct JoystickData {
  int xValue;
  int yValue;
};

// -------------------------------------------------------
//   Triple-Back Pattern Detection (Audrey)
// -------------------------------------------------------

#define PATTERN_LENGTH 6         // back, stop, back, stop, back, stop
#define PATTERN_WINDOW_MS 2000   // must complete within 2 seconds

// Encoded states for the pattern: 1 = BACK, 0 = STOP
const int BACK_PATTERN[PATTERN_LENGTH] = {1, 0, 1, 0, 1, 0};

int    patternStep       = 0;
int    lastCommandState  = -1;  // -1 = uninitialized, 0 = STOP, 1 = BACK
unsigned long patternStartTime = 0;

bool checkTripleBackPattern(int currentState) {
  unsigned long now = millis();

  // Ignore if same command is still held (only trigger on state *changes*)
  if (currentState == lastCommandState) return false;
  lastCommandState = currentState;

  // Reset if we've exceeded the 2-second window
  if (patternStep > 0 && (now - patternStartTime) > PATTERN_WINDOW_MS) {
    patternStep = 0;
  }

  // Check if this state matches the next expected step
  if (currentState == BACK_PATTERN[patternStep]) {
    if (patternStep == 0) patternStartTime = now;  // start the timer
    patternStep++;
    if (patternStep == PATTERN_LENGTH) {
      patternStep = 0;  // reset for next detection
      return true;      // PATTERN COMPLETE!
    }
  } else {
    // Mismatch — reset and try again from step 0
    patternStep = 0;
    // Re-check from step 0 in case this input starts a new pattern
    if (currentState == BACK_PATTERN[0]) {
      patternStartTime = now;
      patternStep = 1;
    }
  }

  return false;
}

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

    int currentState = -1;  // -1 = neither back nor stop (forward/turn)

    if (data.xValue > 600 && data.xValue < 900) {
      Serial.print("Forward | Speed: ");
      Serial.println(given_speed);
      moveForward(RC_ADDRESS, given_speed);
  } else if (data.xValue >= 900) {
      uint8_t fast_speed = (uint8_t)(
        given_speed + ((long)(data.xValue - 900) * given_speed / 123)
      );
      Serial.print("FAST Forward | Speed: ");
      Serial.println(fast_speed);
      moveForward(RC_ADDRESS, fast_speed);
  } else if (data.xValue < 400) {
      Serial.println("Backward");
      moveBackward(RC_ADDRESS, given_speed);
      currentState = 1;  // BACK
  } else if (data.yValue < 400) {
      Serial.println("Turn Left");
      turnLeft(RC_ADDRESS, given_speed);
  } else if (data.yValue > 600) {
      Serial.println("Turn Right");
      turnRight(RC_ADDRESS, given_speed);
  } else {
      Serial.println("Stopped");
      stopAll();
      currentState = 0;  // STOP
  }

    if (checkTripleBackPattern(currentState)) {
      Serial.println("*** TRIPLE BACK PATTERN DETECTED ***");
      // >>> YOUR ACTION HERE <
      // transitions between the two modes AI_camera and this joystick code script
    }

  } else {
    Serial.println("NO SIGNAL");
    stopAll();
  }

  delay(100);
}
