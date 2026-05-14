//See BareMinimum example for a list of library functions
// from filepath "C:\Users\Audrey\OneDrive\Documents\Arduino\libraries\roboclaw_arduino_library-master\examples\PacketSerialEncoderSpeed"

//Includes required to use Roboclaw library
#include <SoftwareSerial.h>
#include "RoboClaw.h"

//See limitations of Arduino SoftwareSerial
SoftwareSerial serial(10,11);	
RoboClaw roboclaw(&serial,10000);

#define address 0x80
int speed = 30;

void setup() {
  //Open Serial and roboclaw at 38400bps
  Serial.begin(57600);
  roboclaw.begin(38400);

  for (int i = 0; i <= speed; i++) {
    moveForward(address, i);
    delay(50);  // controls how fast it speeds up
  }

}

void loop() {
  // Move forward
  moveForward(address, speed);
  uint8_t status1,status2,status3,status4;
  bool valid1,valid2,valid3,valid4;
  
  //Read all the data from Roboclaw before displaying on Serial Monitor window
  //This prevents the hardware serial interrupt from interfering with
  //reading data using software serial.
  int32_t enc1= roboclaw.ReadEncM1(address, &status1, &valid1);
  int32_t enc2 = roboclaw.ReadEncM2(address, &status2, &valid2);
  int32_t speed1 = roboclaw.ReadSpeedM1(address, &status3, &valid3);
  int32_t speed2 = roboclaw.ReadSpeedM2(address, &status4, &valid4);

  Serial.print("Encoder1:");
  if(valid1){
    Serial.print(enc1,HEX);
    Serial.print(" ");
    Serial.print(status1,HEX);
    Serial.print(" ");
  }
  else{
    Serial.print("invalid ");
  }
  Serial.print("Encoder2:");
  if(valid2){
    Serial.print(enc2,HEX);
    Serial.print(" ");
    Serial.print(status2,HEX);
    Serial.print(" ");
  }
  else{
    Serial.print("invalid ");
  }
  Serial.print("Speed1:");
  if(valid3){
    Serial.print(speed1,HEX);
    Serial.print(" ");
  }
  else{
    Serial.print("invalid ");
  }
  Serial.print("Speed2:");
  if(valid4){
    Serial.print(speed2,HEX);
    Serial.print(" ");
  }
  else{
    Serial.print("invalid ");
  }
  Serial.println();
  
  delay(100);
}

void moveForward(uint8_t rc_address, uint8_t speed){
  roboclaw.ForwardM1(rc_address, speed);
  roboclaw.ForwardM2(rc_address, speed);
}