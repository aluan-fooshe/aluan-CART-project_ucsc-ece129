//See BareMinimum example for a list of library functions
// from filepath "C:\Users\Audrey\OneDrive\Documents\Arduino\libraries\roboclaw_arduino_library-master\examples\PacketSerialEncoderSpeed"

//Includes required to use Roboclaw library
#include <SoftwareSerial.h>
#include "RoboClaw.h"

//See limitations of Arduino SoftwareSerial
SoftwareSerial serial(3,4);	
RoboClaw roboclaw(&serial,100);

#define address 0x80
int speed = 30;
int val = 40;
int oldSpeed1 = 0;

void setup() {
  //Open Serial and roboclaw at 38400bps
  Serial.begin(57600);
  roboclaw.begin(38400);

  roboclaw.SetEncM1(address,val);
  roboclaw.SetEncM2(address,val);

  for (int i = 0; i <= speed; i++) {
    moveForward(address, i);
    delay(100);  // controls how fast it speeds up
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
  int enc1 = roboclaw.ReadEncM1(0x80);
  int enc2 = roboclaw.ReadEncM2(address, &status2);
  int speed1 = roboclaw.ReadSpeedM1(address, &status3);
  int speed2 = roboclaw.ReadSpeedM2(address, &status4);

  // Only update and print if speed1 changed
    if (speed1 != oldSpeed1) {
      oldSpeed1 = speed1;

      Serial.print("UPDATED Encoder1:"); Serial.print(enc1, HEX);
      Serial.print(" ");        Serial.print(status1, HEX);
      Serial.print(" ");
    }
    else {
      oldSpeed1 = speed1;
      Serial.print("Encoder1:"); Serial.print(enc1, DEC);

      Serial.print(" ");
    }

      Serial.print("Speed1:"); Serial.print(speed1, DEC);
      Serial.print(" \n");
  
  delay(1000);
}

void moveForward(uint8_t rc_address, uint8_t speed){
  roboclaw.ForwardM1(rc_address, speed);
  roboclaw.ForwardM2(rc_address, speed);
}