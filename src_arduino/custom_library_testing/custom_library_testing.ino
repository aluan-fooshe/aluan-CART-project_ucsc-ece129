//Includes required to use Roboclaw library
#include <SoftwareSerial.h>
#include "RoboClaw.h"

//Uncomment if Using Hardware Serial port
//RoboClaw roboclaw(&Serial,10000);

//Uncomment if using SoftwareSerial. See limitations of Arduino SoftwareSerial
// SoftwareSerial serial(10,11);	
SoftwareSerial serial(3,4);	// FRONT MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclaw(&serial,10000);

#define address 0x80

void setup() {
  //Communicate at 38400bps
  Serial.begin(9600);
  roboclaw.begin(38400);
}

void loop()
{
  int motor_1_count = roboclaw.ReadEncM1(0x80);
  int motor_2_count = roboclaw.ReadEncM2(0x80);
  int motor_1_speed = roboclaw.ReadSpeedM1(0x80);
  int motor_2_speed = roboclaw.ReadSpeedM2(0x80);
  Serial.print("EncM1, SpeedM1: "); 
  Serial.print(motor_1_count); 
  Serial.print(", "); 
  Serial.print(motor_1_speed);
  Serial.print("\t\t");
  Serial.print("EncM2, SpeedM2: "); 
  Serial.print(motor_2_count); 
  Serial.print(", "); 
  Serial.print(motor_2_speed);
  Serial.println("");
  delay(1000);
}
