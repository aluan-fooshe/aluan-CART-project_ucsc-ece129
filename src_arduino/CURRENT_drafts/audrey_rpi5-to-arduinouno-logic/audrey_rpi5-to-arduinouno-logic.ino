// THIS WAS DONE IN COLLABORATION WITH RENAT DOBORNIAN.
// 
// AUTHOR: AUDREY LUAN

#include <SoftwareSerial.h>
#include <RoboClaw.h>
#include <RF24.h>

// --- MOTOR CONTROLLER LOGIC SEGMENT ---
SoftwareSerial frontSerial(3, 4);  // FRONT MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawFront(&frontSerial, 10000);

SoftwareSerial backSerial(5, 6);  // BACK MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclawBack(&backSerial, 10000);

#define RC_ADDRESS 0x80
#define BAUDRATE 38400

//Velocity PID coefficients
#define Kp 1.0
#define Ki 0.5
#define Kd 0.25
#define QPPS 100

#define PI_INPUT_PIN_7 7   // receives from Pi GPIO 14
#define PI_INPUT_PIN_8 8   // receives from Pi GPIO 15
#define PI_INPUT_PIN_9 9   // receives from Pi GPIO 9

struct ThreeBit {
  uint8_t val7 : 1;
  uint8_t val8 : 1;
  uint8_t val9 : 1;
};

uint8_t speed = 12;
const char* cartAction = "";

void moveForward(uint8_t address, uint8_t speed) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
}

void moveBackward(uint8_t address, uint8_t speed) {
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
}

void turnRight(uint8_t address, uint8_t speed){
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
}

void turnLeft(uint8_t address, uint8_t speed){
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
}

void stopAll() {
  roboclawFront.ForwardM1(RC_ADDRESS, 0);
  roboclawFront.ForwardM2(RC_ADDRESS, 0);
  roboclawBack.ForwardM1(RC_ADDRESS, 0);
  roboclawBack.ForwardM2(RC_ADDRESS, 0);
}

uint8_t clampedSpeed(float multiplier) {
    return (uint8_t)(min((int)(speed * multiplier), 127));
}

const char* rpi_to_motors(int val7, int val8, int val9) {
    uint8_t cmd = (val7 << 2) | (val8 << 1) | val9;
    //  cmd = 0b000 (0) → val7=0, val8=0, val9=0 → stopAll
    //  cmd = 0b001 (1) → val7=0, val8=0, val9=1 → turnLeft(speed*1.5)
    //  cmd = 0b010 (2) → val7=0, val8=1, val9=0 → turnRight(speed*1.5)
    //  cmd = 0b011 (3) → val7=0, val8=1, val9=1 → moveForward(speed)
    //  cmd = 0b100 (4) → val7=1, val8=0, val9=0 → moveForward(speed*1.5)
    //  cmd = 0b101 (5) → val7=1, val8=0, val9=1 → moveForward(speed*2.0)

    if (cmd == 0b000) {
      stopAll();
      return "stopAll";
  } else if (cmd == 0b001) {
      turnLeft(RC_ADDRESS, clampedSpeed(1.5));
      return "dir1";
  } else if (cmd == 0b010) {
      turnRight(RC_ADDRESS, clampedSpeed(1.5));
      return "dir2";
  } else if (cmd == 0b011) {
      moveForward(RC_ADDRESS, speed);
      return "moveForward";
  } else if (cmd == 0b100) {
      moveForward(RC_ADDRESS, clampedSpeed(1.5));
      return "moveForward_1.5x";
  } else if (cmd == 0b101) {
      moveForward(RC_ADDRESS, clampedSpeed(2.0));
      return "moveForward_2.0x";
  } else {
      stopAll();
      return "stopAll";
  }
}

void setup() {
    Serial.begin(9600);
    pinMode(PI_INPUT_PIN_7, INPUT);
    pinMode(PI_INPUT_PIN_8, INPUT);
    pinMode(PI_INPUT_PIN_9, INPUT);

    frontSerial.begin(BAUDRATE);
    roboclawFront.begin(BAUDRATE);
    backSerial.begin(BAUDRATE);
    roboclawBack.begin(BAUDRATE);

    //Set PID Coefficients
    roboclawFront.SetM1VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
    roboclawFront.SetM2VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
    roboclawBack.SetM1VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);
    roboclawBack.SetM2VelocityPID(RC_ADDRESS, Kd, Kp, Ki, QPPS);

    delay(2000); // time needed to boot up the roboclaws 
    Serial.println("Ready for testing (with weight)");
}

void loop() {
    int val7 = digitalRead(PI_INPUT_PIN_7);
    int val8 = digitalRead(PI_INPUT_PIN_8);
    int val9 = digitalRead(PI_INPUT_PIN_9);

    cartAction = rpi_to_motors(val7, val8, val9);

    Serial.print("Pin7-8-9: "); Serial.print(val7); 
    Serial.print(val8); Serial.print(val9); Serial.print("   "); Serial.println(cartAction);

    delay(100);
}
