//Includes required to use Roboclaw library
#include <SoftwareSerial.h>
#include "RoboClaw.h"

//Uncomment if Using Hardware Serial port
//RoboClaw roboclaw(&Serial,10000);

//Uncomment if using SoftwareSerial. See limitations of Arduino SoftwareSerial
// SoftwareSerial serial(10,11);	
SoftwareSerial serial(3,4);	// FRONT MOTORS PAIR: RX (yellow), TX (orange)
RoboClaw roboclaw(&serial,10000);

#define FR_ADDRESS 0x80
#define BK_ADDRESS 0x80
#define BAUDRATE 38400

//Velocity PID coefficients
#define Kp 1.0
#define Ki 0.5
#define Kd 0.25
#define QPPS 100

void displayspeed(){
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
}

void moveForward(uint8_t rc_address, uint8_t speed){
  roboclaw.ForwardM1(rc_address, speed);
  roboclaw.ForwardM2(rc_address, speed);
}

// ----------------- MAIN FUNCTION BLOCK -----------------

void setup() {
  //Communicate at 38400bps
  Serial.begin(57600);
  
  // Must explicitly start the SoftwareSerial port
  roboclaw.begin(BAUDRATE);

  //Set PID Coefficients
  roboclaw.SetM1VelocityPID(FR_ADDRESS, Kd, Kp, Ki, QPPS);
  roboclaw.SetM2VelocityPID(FR_ADDRESS, Kd, Kp, Ki, QPPS); 

  delay(2000); // time needed to boot up the roboclaws

  // SpeedAccelMx(address, accel, speed) — accel and speed are separate args
  // Forward
  roboclaw.SpeedAccelM1(FR_ADDRESS, 500, 1000);
  roboclaw.SpeedAccelM2(FR_ADDRESS, 500, 1000);

  moveForward(FR_ADDRESS, 20);
  displayspeed();
  delay(7500);
}

void loop()
{  
  roboclaw.ForwardM1(FR_ADDRESS, 0); // explicit stop
  roboclaw.ForwardM2(FR_ADDRESS, 0);
  delay(1000);

  // // Backward (negative speed)
  // roboclaw.SpeedAccelM1(FR_ADDRESS, 500, -1000);
  // roboclaw.SpeedAccelM2(FR_ADDRESS, 500, -1000);
  // delay(500);

}
