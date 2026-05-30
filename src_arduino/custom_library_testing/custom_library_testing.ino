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