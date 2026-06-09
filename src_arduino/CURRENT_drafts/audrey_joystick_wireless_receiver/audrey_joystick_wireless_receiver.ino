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

uint8_t given_speed = 30;

//Joystick Data
struct JoystickData {
  int xValue;
  int yValue;
};

#define PI_INPUT_PIN_7 7
#define PI_INPUT_PIN_8 8
#define PI_INPUT_PIN_9 9
const char* cartAction = "";
bool aiCameraMode = false;  // false = joystick, true = AI camera

// -------------------------------------------------------
//   Triple-Back Pattern Detection (Audrey)
// -------------------------------------------------------

#define PATTERN_LENGTH 4         // back, stop, back, stop
#define PATTERN_WINDOW_MS 2000   // must complete within 2 seconds

// Encoded states for the pattern: 1 = BACK, 0 = STOP
const int BACK_PATTERN[PATTERN_LENGTH] = {1, 0, 1, 0};

int    patternStep       = 0;
int    lastCommandState  = -1;  // -1 = uninitialized, 0 = STOP, 1 = BACK
unsigned long patternStartTime = 0;

bool checkDoubleBackPattern(int currentState) {
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


// -------------------------------------------------------
// AI Camera Mode functions
// -------------------------------------------------------

uint8_t clampedSpeed(float multiplier) {
    return (uint8_t)(min((int)(given_speed * multiplier), 127));
}

// just decode, don't move
const char* decode_pi_pins(int val7, int val8, int val9) {
  uint8_t cmd = (val7 << 2) | (val8 << 1) | val9;
  if (cmd == 0b000) return "stopAll";
  else if (cmd == 0b001) return "turnLeft";
  else if (cmd == 0b010) return "turnRight";
  else if (cmd == 0b011) return "moveForward";
  else if (cmd == 0b100) return "moveForward_1.5x";
  else if (cmd == 0b101) return "moveForward_2.0x";
  else return "stopAll";
}

// Returns the speed value
uint8_t rpi_to_motors(int val7, int val8, int val9) {
    uint8_t cmd = (val7 << 2) | (val8 << 1) | val9;
    //  cmd = 0b000 (0) → val7=0, val8=0, val9=0 → stopAll
    //  cmd = 0b001 (1) → val7=0, val8=0, val9=1 → turnLeft(speed*1.5)
    //  cmd = 0b010 (2) → val7=0, val8=1, val9=0 → turnRight(speed*1.5)
    //  cmd = 0b011 (3) → val7=0, val8=1, val9=1 → moveForward(speed)
    //  cmd = 0b100 (4) → val7=1, val8=0, val9=0 → moveForward(speed*1.5)
    //  cmd = 0b101 (5) → val7=1, val8=0, val9=1 → moveForward(speed*2.0)

    if (cmd == 0b000) {
      stopAll();
      return 0;
  } else if (cmd == 0b001) {
      turnLeft(RC_ADDRESS, clampedSpeed(1.5));
      return clampedSpeed(1.5);
  } else if (cmd == 0b010) {
      turnRight(RC_ADDRESS, clampedSpeed(1.5));
      return clampedSpeed(1.5);
  } else if (cmd == 0b011) {
      moveForward(RC_ADDRESS, given_speed);
      return given_speed;
  } else if (cmd == 0b100) {
      moveForward(RC_ADDRESS, clampedSpeed(1.5));
      return clampedSpeed(1.5);
  } else if (cmd == 0b101) {
      moveForward(RC_ADDRESS, clampedSpeed(2.0));
      return clampedSpeed(2.0);
  } else {
      stopAll();
      return 0;
  }
}

// -------------------------------------------------------
// DATA COLLECTION AND DEBUG PRINT STATEMENT
// -------------------------------------------------------

// TIMESTAMP VARIABLE
unsigned long startTime = 0;

void logTelemetry(unsigned long timestamp, const char* action, uint8_t speed, int val7, int val8, int val9, JoystickData data, int patternDetected, bool aiCameraMode) {
    float seconds = timestamp / 1000.0;
    uint8_t cmd = (val7 << 2) | (val8 << 1) | val9;
    Serial.print(seconds, 1);       Serial.print(", ");
    Serial.print(aiCameraMode ? "AI_CAM" : "JOYSTICK"); Serial.print(", ");
    Serial.print(speed);            Serial.print(", ");
    Serial.print((cmd >> 2) & 1);
    Serial.print((cmd >> 1) & 1);
    Serial.print((cmd >> 0) & 1);   Serial.print(", ");
    Serial.print(action);           Serial.print(", ");
    Serial.print(data.xValue);      Serial.print(", ");
    Serial.print(data.yValue);      Serial.print(", ");
    Serial.println(patternDetected);
}


//setup function
void setup() {
  pinMode(PI_INPUT_PIN_7, INPUT);
  pinMode(PI_INPUT_PIN_8, INPUT);
  pinMode(PI_INPUT_PIN_9, INPUT);

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
  startTime = millis();  // ← add this as the last line
  Serial.println("Time,Mode,Speed,PinCmd,Action,xValue,yValue,PatternState");
}


// -------------------------------------------------------
// MAIN FUNCTION
// -------------------------------------------------------

void loop() {
  bool got_signal = radio.available();
  unsigned long timestamp = (millis() - startTime);
  JoystickData data = {0, 0};  // safe default
  int currentState = -1;
  cartAction = decode_pi_pins(val7, val8, val9);

  // Run Pi GPIO regardless of joystick signal
  int val7 = digitalRead(PI_INPUT_PIN_7);
  int val8 = digitalRead(PI_INPUT_PIN_8);
  int val9 = digitalRead(PI_INPUT_PIN_9);

  // ---------------- AI CAMERA MODE ---------------------
  if (aiCameraMode) {
    uint8_t CART_speed = rpi_to_motors(val7, val8, val9);
    // Only check joystick for mode switch if signal is available
    if (got_signal) {
      radio.read(&data, sizeof(data));
      // void logTelemetry(unsigned long timestamp, const char* action, uint8_t speed, 
      // int val7, int val8, int val9, JoystickData data, int patternDetected, bool aiCameraMode)
      logTelemetry(timestamp, cartAction, CART_speed, 
      val7, val8, val9, data, currentState, aiCameraMode);

      currentState = -1;
      if (data.xValue <= 100) {
        Serial.println("SWITCH STATE CASE");
        currentState = 1;
      } else {
        currentState = 0;
      }

      if (checkDoubleBackPattern(currentState)) {
        aiCameraMode = !aiCameraMode;
        Serial.print("*** MODE SWITCH: ");
        Serial.println(aiCameraMode ? "AI CAMERA" : "JOYSTICK");
        stopAll();
      }
    } else {
      logTelemetry(timestamp, "NO_SIGNAL", 0, 
      val7, val8, val9, data, currentState, aiCameraMode);
    }

  // ---------------- JOYSTICK MODE ---------------------
  } else if (got_signal) {
    radio.read(&data, sizeof(data));
    
    // void logTelemetry(unsigned long timestamp, const char* action, uint8_t speed, 
    // int val7, int val8, int val9, JoystickData data, int patternDetected, bool aiCameraMode)

    currentState = -1;

    // FORWARD range 600 - 900: range width is 300
    if (data.xValue > 600 && data.xValue < 900) {
      uint8_t slow_FWDspeed = (uint8_t)(
        0 + ((long)(data.xValue - 600) * given_speed / 300)
      );
      logTelemetry(timestamp, "moveForward", slow_FWDspeed, val7, val8, val9, data, currentState, aiCameraMode);
      moveForward(RC_ADDRESS, slow_FWDspeed);

    // FAST FORWARD range 900 - 1023: range width is 123
    } else if (data.xValue >= 900) {
      uint8_t fast_FWDspeed = (uint8_t)(
        given_speed + ((long)(data.xValue - 900) * given_speed / 123)
      );
      logTelemetry(timestamp, "moveForward_2.0x", fast_FWDspeed, val7, val8, val9, data, currentState, aiCameraMode);
      moveForward(RC_ADDRESS, fast_FWDspeed);

    // BACKWARD range 100 - 400: range width is 300
    } else if (data.xValue < 400 && data.xValue > 100) {
      uint8_t BKWD_speed = (uint8_t)(0 + ((long)(400 - data.xValue) * given_speed / 300));
      logTelemetry(timestamp, "moveBackward", BKWD_speed, val7, val8, val9, data, currentState, aiCameraMode);
      moveBackward(RC_ADDRESS, BKWD_speed);
      // NOTE: currentState intentionally NOT set here

    // SWITCH STATE CASE
    } else if (data.xValue <= 100) {
      currentState = 1;
      logTelemetry(timestamp, "SwitchState", 0, val7, val8, val9, data, currentState, aiCameraMode);

    // BACKWARD range 100 - 400: range width is 300
    } else if (data.yValue < 400) {
      uint8_t turn_speed = (uint8_t)((long)(400 - data.yValue) * given_speed / 400);
      logTelemetry(timestamp, "turnLeft", turn_speed, val7, val8, val9, data, currentState, aiCameraMode);
      turnLeft(RC_ADDRESS, turn_speed);

    } else if (data.yValue > 600) {
      uint8_t turn_speed = (uint8_t)((long)(data.yValue - 600) * given_speed / 400);
      logTelemetry(timestamp, "turnRight", turn_speed, val7, val8, val9, data, currentState, aiCameraMode);
      turnRight(RC_ADDRESS, turn_speed);

    } else {
      stopAll();
      currentState = 0;
      logTelemetry(timestamp, "stopAll", 0, val7, val8, val9, data, currentState, aiCameraMode);
    }

    if (checkDoubleBackPattern(currentState)) {
      aiCameraMode = !aiCameraMode;
      Serial.print("*** MODE SWITCH: ");
      Serial.println(aiCameraMode ? "AI CAMERA" : "JOYSTICK");
      stopAll();
    }

  } else {
    logTelemetry(timestamp, "NO_SIGNAL", 0, val7, val8, val9, data, currentState, aiCameraMode);
    stopAll();
  }

  delay(100);
}
