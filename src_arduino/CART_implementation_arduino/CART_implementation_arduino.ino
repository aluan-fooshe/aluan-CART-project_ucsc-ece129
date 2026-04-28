#include <SoftwareSerial.h>
#include "RoboClaw.h"

SoftwareSerial serial(10, 11); // RX, TX
RoboClaw roboclaw(&serial, 10000);

#define ROBOCLAW_ADDRESS 0x80
#define BAUDRATE 38400

//Velocity PID coefficients.
#define Kp 1.0
#define Ki 0.5
#define Kd 0.25
#define qpps 44000

void setup() {
  //Open Serial and roboclaw serial ports
  Serial.begin(57600);
  roboclaw.begin(38400);
  
  //Set PID Coefficients
  roboclaw.SetM1VelocityPID(ROBOCLAW_ADDRESS,Kd,Kp,Ki,qpps);
  roboclaw.SetM2VelocityPID(ROBOCLAW_ADDRESS,Kd,Kp,Ki,qpps);  
}

void loop() {
  //Set initial speed
  uint8_t speed = 1000;
  roboclaw.SpeedM1(ROBOCLAW_ADDRESS, speed);
  
  for(uint8_t i = 0;i<1000;i++){
    roboclaw.SpeedM1(ROBOCLAW_ADDRESS, i);
    displayspeed();
    delay(100);
  }

  roboclaw.SpeedM1(ROBOCLAW_ADDRESS, speed);
  delay(1000);

  for(uint8_t i = 1000;i>0;i--){
    roboclaw.SpeedM1(ROBOCLAW_ADDRESS, i);
    displayspeed();
    delay(100);
  }
}

void displayspeed(void)
{
  uint8_t status1,status2,status3,status4;
  bool valid1,valid2,valid3,valid4;
  
  int32_t enc1= roboclaw.ReadEncM1(ROBOCLAW_ADDRESS, &status1, &valid1);
  int32_t enc2 = roboclaw.ReadEncM2(ROBOCLAW_ADDRESS, &status2, &valid2);
  int32_t speed1 = roboclaw.ReadSpeedM1(ROBOCLAW_ADDRESS, &status3, &valid3);
  int32_t speed2 = roboclaw.ReadSpeedM2(ROBOCLAW_ADDRESS, &status4, &valid4);
  Serial.print("Encoder1:");
  if(valid1){
    Serial.print(enc1);
    Serial.print(" ");
    Serial.print(status1);
    Serial.print(" ");
  }
  else{
    Serial.print("invalid ");
  }
  Serial.print("Encoder2:");
  if(valid2){
    Serial.print(enc2);
    Serial.print(" ");
    Serial.print(status2);
    Serial.print(" ");
  }
  else{
    Serial.print("invalid ");
  }
  Serial.print("Speed1:");
  if(valid3){
    Serial.print(speed1);
    Serial.print(" ");
  }
  else{
    Serial.print("invalid ");
  }
  Serial.print("Speed2:");
  if(valid4){
    Serial.print(speed2);
    Serial.print(" ");
  }
  else{
    Serial.print("invalid ");
  }
  Serial.println();
}

void moveForward(uint8_t address, uint8_t target_speed){
  roboclaw.ForwardM1(address, target_speed);
  roboclaw.ForwardM2(address, target_speed);
}

void moveBackward(uint8_t address, uint8_t current_speed, uint8_t target_speed){
  roboclaw.BackwardM1(address, target_speed);
  roboclaw.BackwardM2(address, target_speed);
}

void turnForward(uint8_t address, uint8_t speed1, uint8_t speed2) {
  int steps = max(speed1, speed2);  // ramp based on the higher target speed

  for (int i = 0; i <= steps; i++) {
    uint8_t s1 = (speed1 > 0) ? map(i, 0, steps, 0, speed1) : 0;
    uint8_t s2 = (speed2 > 0) ? map(i, 0, steps, 0, speed2) : 0;
    roboclaw.ForwardM1(address, s1);
    roboclaw.ForwardM2(address, s2);
    delay(20);  // adjust for faster/slower ramp
  }
}

void turnBackward(uint8_t address, uint8_t speed1, uint8_t speed2) {
  roboclaw.BackwardM1(address, speed1);
  roboclaw.BackwardM2(address, speed2);
}

void stopMotors(uint8_t address) {
  roboclaw.ForwardM1(address, 0);
  roboclaw.ForwardM2(address, 0);
}