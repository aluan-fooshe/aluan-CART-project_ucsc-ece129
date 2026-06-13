# Sftwre04_ArduinoUno-roboclaw2x45

<b>Author:</b> Audrey Luan </br>
<b>Date Written:</b> 2026 June 12, 06:37PM </br>
<b>Directory Path (s):</b> 
- /aluan-CART-project_ucsc-ece129/src_arduino/CURRENT_drafts/audrey_rpi5-to-arduinouno-logic
- /aluan-CART-project_ucsc-ece129/src_arduino/CURRENT_drafts/CART_current_draw_testing

### Notes

Part 4 of the Software subsystem appendices, includes the code used to program the Roboclaw 2x45 with the Arduino Uno and the current draw test code.
</br>
- <b>audrey_rpi5-to-arduinouno-logic.ino</b>: Basic code for sending 3-bit commands from Raspberry Pi 5 to Arduino Uno, and commanding the Roboclaw 2x45 motor controllers to move based on that command.
- <b>CART_current_draw_testing.ino</b>: Code file to record data on current draw from Basicmicro Motion Studio from skid steering.

## audrey_rpi5-to-arduinouno-logic.ino

```cpp
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
      return "turnLeft";
  } else if (cmd == 0b010) {
      turnRight(RC_ADDRESS, clampedSpeed(1.5));
      return "turnRight";
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

```

```cpp
// THE CODE USED TO GET DATA FROM CART ON CURRENT DRAW, VOLTAGE, SPEED, AND BATTERY TEMPERATURE.
// THIS WAS DONE IN COLLABORATION WITH NOAH LEE AND RENAT DOBORNIAN.
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

uint8_t given_speed = 20;
int given_time = 23000;

// --- RF COMMUNICATIONS LOGIC SEGMENT ---
RF24 radioLeft(7, 8); // RF24(ce_pin, csn_pin)
RF24 radioRight(9, 10);

const byte address1[6] = "00001";
const byte address2[6] = "00002";

int leftPackets = 0;
int rightPackets = 0;

unsigned long lastMeasure = 0;

// -------------------------------------------------------------------------------
//      DEFINED FUNCTIONS SECTION
// -------------------------------------------------------------------------------

void displayspeed(){
  int motor_1_count = roboclawFront.ReadEncM1(0x80);
  int motor_2_count = roboclawFront.ReadEncM2(0x80);
  int motor_3_count = roboclawBack.ReadEncM1(0x80);
  int motor_4_count = roboclawBack.ReadEncM2(0x80);
  int motor_1_speed = roboclawFront.ReadSpeedM1(0x80);
  int motor_2_speed = roboclawFront.ReadSpeedM2(0x80);
  int motor_3_speed = roboclawBack.ReadSpeedM1(0x80);
  int motor_4_speed = roboclawBack.ReadSpeedM2(0x80);
  Serial.print("EncM1, SpeedM1: "); 
  Serial.print(motor_1_count); 
  Serial.print(", "); 
  Serial.print(motor_1_speed);
  Serial.print("\t\t");
  Serial.print("EncM2, SpeedM2: "); 
  Serial.print(motor_2_count); 
  Serial.print(", "); 
  Serial.print(motor_2_speed);
  Serial.print("\t\t");
  Serial.print("EncM3, SpeedM3: "); 
  Serial.print(motor_3_count); 
  Serial.print(", "); 
  Serial.print(motor_3_speed);
  Serial.print("\t\t");
  Serial.print("EncM4, SpeedM4: "); 
  Serial.print(motor_4_count); 
  Serial.print(", "); 
  Serial.print(motor_4_speed);
  Serial.println("");
}

void moveForward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void moveBackward(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turnRight(uint8_t address, uint8_t speed, int delay_time = 0){
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.BackwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.BackwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void turnLeft(uint8_t address, uint8_t speed, int delay_time = 0){
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

// -------------------------------------------------------------------------------
//      MAIN FUNCTION SECTION
// -------------------------------------------------------------------------------

void setup() {
  Serial.begin(57600);
  radioLeft.begin();
  radioRight.begin();
  radioLeft.stopListening();
  radioRight.stopListening();
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

  delay(6000);
  turnRight(RC_ADDRESS, given_speed, given_time);
  delay(1000);
  turnRight(RC_ADDRESS, 0, 1000);
}

void loop() {
  turnRight(RC_ADDRESS, 0, 1000);
  delay(2000);
  Serial.println("loop running");
}
```