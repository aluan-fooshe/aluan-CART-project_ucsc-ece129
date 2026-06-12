# Sftwre01_AICamera-headless

<b>Author:</b> Audrey Luan </br>
<b>Date Written:</b> 2026 June 12, 03:39PM </br>
<b>Directory Path:</b> /aluan-CART-project_ucsc-ece129/src_AIcamera_py

### Notes

Part 1 of the Software subsystem appendices, includes all the code used to program the AI camera with headless mode for the Raspberry Pi 5.

## ai_camera_headless.py

```python
# custom created python libraries
# Author: Audrey Luan
import lgpio
import motor_roboclaw2x45s as RC

# imported libraries
import collections
import cv2
import numpy as np
from picamera2 import Picamera2
from picamera2.devices.imx500 import IMX500
import signal

# sudo systemctl enable ai_camera_headless.service
# sudo journalctl -u ai_camera_headless.service -f

# # Open the GPIO chip (Pi 5 uses chip 4)
# chip = lgpio.gpiochip_open(4)
# print("[gpio] Chip opened successfully")

chip = lgpio.gpiochip_open(0)
lgpio.gpio_claim_output(chip, 14, 0)  # drive LOW immediately
lgpio.gpio_claim_output(chip, 15, 0)
lgpio.gpio_claim_output(chip, 9, 0)

# --- Declare pins ---
# Pi GPIO 14 (pin 8)  → Arduino pin 7
# Pi GPIO 15 (pin 10) → Arduino pin 8
# Pi GPIO 9  (pin 21) → Arduino pin 9
GPIO_TO_ARDUINO_7 = 14
GPIO_TO_ARDUINO_8 = 15
GPIO_TO_ARDUINO_9 = 9

# Open roboclaw serial ports
RC.roboclaw_front.Open()
RC.roboclaw_back.Open()

# Load the COCO object detection model onto the IMX500 chip
imx500 = IMX500("/usr/share/imx500-models/imx500_network_ssd_mobilenetv2_fpnlite_320x320_pp.rpk")
imx500.show_network_fw_progress_bar()

# --- CALIBRATION CONSTANTS ---
# Adjust these based on your specific orange hat and environment
KNOWN_WIDTH_CM = 20.0  
FOCAL_LENGTH = 500.0   

# HSV Color Boundaries for Orange1 (Renat's hat)
LOWER_ORANGE1 = np.array([110, 130, 120])
UPPER_ORANGE1 = np.array([130, 210, 250])

# HSV Color Boundaries for Orange2 (Audrey's hat)
LOWER_ORANGE2 = np.array([100, 175, 120])
UPPER_ORANGE2 = np.array([120, 255, 250])

# 2. CAMERA AND FRAME SETTINGS
FRAME_W = 1280 #700
FRAME_H = 720 #400
CENTER_X = FRAME_W // 2
CENTER_Y = FRAME_H // 2

speed = 20

box_history = collections.deque(maxlen=10)

THIRD_LEFT  = FRAME_W // 3          # x = 0   to 233
THIRD_RIGHT = (FRAME_W // 3) * 2    # x = 233 to 466
# Right zone: THIRD_RIGHT to FRAME_W (466 to 700)

lgpio.gpio_claim_output(chip, GPIO_TO_ARDUINO_7)
print(f"[gpio] GPIO {GPIO_TO_ARDUINO_7} claimed as OUTPUT → Arduino pin 7")

lgpio.gpio_claim_output(chip, GPIO_TO_ARDUINO_8)
print(f"[gpio] GPIO {GPIO_TO_ARDUINO_8} claimed as OUTPUT → Arduino pin 8")

lgpio.gpio_claim_output(chip, GPIO_TO_ARDUINO_9)
print(f"[gpio] GPIO {GPIO_TO_ARDUINO_9} claimed as OUTPUT → Arduino pin 9")

print(GPIO_TO_ARDUINO_7, GPIO_TO_ARDUINO_8, GPIO_TO_ARDUINO_9)


# --- Send signals ---
def send_signal(arduino_pin, gpio_pin, value):
    lgpio.gpio_write(chip, gpio_pin, value)
    # print(f"[gpio] GPIO {gpio_pin} → Arduino pin {arduino_pin} set {'HIGH' if value == 1 else 'LOW'}")


def get_zone(cx: int) -> str:
    """Return which horizontal third of the frame the target is in."""
    if cx < THIRD_LEFT:
        return "LEFT"
    elif cx < THIRD_RIGHT:
        return "MIDDLE"
    else:
        return "RIGHT"



def CART_statemachine(distance_cm, zone):
    d1 = 75
    d2 = 200
    d3 = 325
    d4 = 400

    # If target is too far regardless of zone, stop
    if distance_cm > d4:
        send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
        send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
        send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
        print("stopAll - out of range\n")
        return "stopAll"

    # Steering takes priority — correct heading first
    if zone == "LEFT":
        if distance_cm < d2:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
            print("turnLeft")
            return "turnLeft"

        else:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=1)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=1)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
            print("ForwardLeft")
            return "ForwardLeft"

    elif zone == "RIGHT":
        if distance_cm < d2:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=1)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
            print("turnRight")
            return "turnRight"

        else:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=1)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=1)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
            print("ForwardRight\n")
            return "ForwardRight"

    elif zone == "MIDDLE":
        if distance_cm < d1:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
            print("stopAll\n")
            return "stopAll"
        elif distance_cm < d2:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=1)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
            print("moveForward\n")
            return "moveForward"
        elif distance_cm < d3:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=1)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
            print("moveForward_1.5x\n")
            return "moveForward_1.5x"
        else:  # d3 <= distance_cm <= d4
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=1)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
            print("moveForward_2x\n")
            return "moveForward_2x"

    # Unknown zone
    send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
    send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
    send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
    print("stopAll - unknown zone\n")
    return "stopAll"


def calculate_distance(apparent_width_px: int) -> float:
    """
    Estimate distance to target using similar triangles.

    distance = (known_real_width_cm * focal_length_px) / apparent_width_px

    Args:
        apparent_width_px: The detected object's width in pixels
    Returns:
        Estimated distance in centimetres
    """
    if apparent_width_px <= 0:
        return 0.0
    return (KNOWN_WIDTH_CM * FOCAL_LENGTH) / apparent_width_px


running = True

def handle_signal(sig, frame):
    global running
    running = False

signal.signal(signal.SIGTERM, handle_signal)
signal.signal(signal.SIGINT, handle_signal)


def main():
    print("Initializing Raspberry Pi AI Camera on Pi 5...")
    picam2 = Picamera2()

    # Configure camera for RGB output
    config = picam2.create_video_configuration(main={"size": (FRAME_W, FRAME_H), "format": "RGB888"})
    picam2.configure(config)
    picam2.start()

    print("Camera active (headless mode).")

    smoothed = {"cx": 0, "cy": 0, "radius": 0}

    try:
        while running:
            frame = picam2.capture_array()

            # Scan the ENTIRE frame for orange, no AI needed
            hsv_full = cv2.cvtColor(frame, cv2.COLOR_RGB2HSV)
            mask = cv2.inRange(hsv_full, LOWER_ORANGE2, UPPER_ORANGE2)

            contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

            if contours:
                largest = max(contours, key=cv2.contourArea)

                if cv2.contourArea(largest) > 30:
                    (cx, cy), radius = cv2.minEnclosingCircle(largest)
                    cx, cy, radius = int(cx), int(cy), int(radius)

                    box_history.append((cx, cy, radius))

                    if len(box_history) == box_history.maxlen:
                        smoothed["cx"]     = int(sum(b[0] for b in box_history) / len(box_history))
                        smoothed["cy"]     = int(sum(b[1] for b in box_history) / len(box_history))
                        smoothed["radius"] = int(sum(b[2] for b in box_history) / len(box_history))
                else:
                    # No contours at all — clear history and reset
                    box_history.clear()
                    smoothed = {"cx": 0, "cy": 0, "radius": 0}
            else:
                # Hat left the frame entirely
                box_history.clear()
                smoothed = {"cx": 0, "cy": 0, "radius": 0} 

            if smoothed["radius"] > 0:
                size = smoothed["radius"] * 2
                distance_cm = int(round(calculate_distance(size)))
                zone = get_zone(smoothed["cx"])
                action = CART_statemachine(distance_cm, zone)
                print(f"Zone: {zone} | Dist: {distance_cm} cm | Action: {action}")
            else:
                # No target detected — explicitly stop motors
                send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
                send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
                send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
                print("No target detected | Action: stopAll")

    except KeyboardInterrupt:
        print("Exiting...")
    finally:
        picam2.stop()
        lgpio.gpiochip_close(chip)
        print("Cleanup done.")


if __name__ == "__main__":
    main()
```

## audrey_joystick_wireless_receiver.ino

```cpp
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

void ForwardRight(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.ForwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed/2);
  roboclawBack.ForwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed/2);
  if (delay_time > 0) delay(delay_time);
}

void turnLeft(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.BackwardM1(address, speed);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.BackwardM1(address, speed);
  roboclawBack.ForwardM2(address, speed);
  if (delay_time > 0) delay(delay_time);
}

void ForwardLeft(uint8_t address, uint8_t speed, int delay_time = 0) {
  roboclawFront.ForwardM1(address, speed/2);
  roboclawFront.ForwardM2(address, speed);
  roboclawBack.ForwardM1(address, speed/2);
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

// Ramping state
uint8_t currentSpeed = 0;
int8_t  currentDir   = 0;   // 1=fwd, -1=bwd, 0=stop, 2=left, -2=right

#define RAMP_STEP 2          // speed units added per loop iteration (tune this)

uint8_t rampToward(uint8_t current, uint8_t target) {
    if (current < target) return min((int)current + RAMP_STEP, (int)target);
    if (current > target) return max((int)current - RAMP_STEP, (int)target);
    return current;
}

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
  //  cmd = 0b110 (6) → val7=1, val8=1, val9=0 → ForwardLeft(speed)
  //  cmd = 0b111 (7) → val7=1, val8=1, val9=1 → ForwardRight(speed)

  uint8_t targetSpeed;
  int8_t  targetDir;

  switch (cmd) {
    case 0b001: targetSpeed = clampedSpeed(1.5); targetDir =  2; break;  // turnLeft
    case 0b010: targetSpeed = clampedSpeed(1.5); targetDir = -2; break;  // turnRight
    case 0b011: targetSpeed = given_speed;        targetDir =  1; break;  // fwd
    case 0b100: targetSpeed = clampedSpeed(1.5);  targetDir =  1; break;  // fwd 1.5x
    case 0b101: targetSpeed = clampedSpeed(2.0);  targetDir =  1; break;  // fwd 2.0x
    default:    targetSpeed = 0;                  targetDir =  0; break;  // stop
  }

  // Direction changed or stopping — ramp down first
  if (targetDir != currentDir || targetSpeed == 0) {
    if (currentSpeed > 0) {
      currentSpeed = rampToward(currentSpeed, 0);
      // Re-issue current direction at reduced speed (don't call stopAll yet)
      switch (currentDir) {
        case  1: moveForward (RC_ADDRESS, currentSpeed); break;
        case -1: moveBackward(RC_ADDRESS, currentSpeed); break;
        case  2: turnLeft    (RC_ADDRESS, currentSpeed); break;
        case -2: turnRight   (RC_ADDRESS, currentSpeed); break;
      }
      return currentSpeed;  // still ramping down
    }
    // Only fully stop once speed has reached 0
    currentDir   = targetDir;
    currentSpeed = 0;
    stopAll();
    return 0;
  }

    // Only reach here once currentSpeed has hit 0, or direction is unchanged
    currentDir   = targetDir;
    currentSpeed = rampToward(currentSpeed, targetSpeed);

  switch (currentDir) {
      case  1: moveForward (RC_ADDRESS, currentSpeed); return currentSpeed;
      case -1: moveBackward(RC_ADDRESS, currentSpeed); return currentSpeed;
      case  2: turnLeft    (RC_ADDRESS, currentSpeed); return currentSpeed;
      case -2: turnRight   (RC_ADDRESS, currentSpeed); return currentSpeed;
      default: stopAll();                              return currentSpeed;
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

  // Run Pi GPIO regardless of joystick signal
  int val7 = digitalRead(PI_INPUT_PIN_7);
  int val8 = digitalRead(PI_INPUT_PIN_8);
  int val9 = digitalRead(PI_INPUT_PIN_9);

  cartAction = decode_pi_pins(val7, val8, val9);

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

    // SWITCH STATE
    if (data.xValue <= 100) {
      currentState = 1;
      logTelemetry(timestamp, "SwitchState", 0, val7, val8, val9, data, currentState, aiCameraMode);

    // FORWARD-LEFT: both x and y active
    } else if ((data.xValue > 600) && data.yValue < 400) {
      uint8_t speed = (uint8_t)((long)(400 - data.yValue) * given_speed / 400);
      logTelemetry(timestamp, "ForwardLeft", speed, val7, val8, val9, data, currentState, aiCameraMode);
      ForwardLeft(RC_ADDRESS, speed);

    // FORWARD-RIGHT: both x and y active
    } else if ((data.xValue > 600) && data.yValue > 600) {
      uint8_t speed = (uint8_t)((long)(data.yValue - 600) * given_speed / 423);
      logTelemetry(timestamp, "ForwardRight", speed, val7, val8, val9, data, currentState, aiCameraMode);
      ForwardRight(RC_ADDRESS, speed);

    // FORWARD slow range 600-900
    } else if (data.xValue > 600 && data.xValue < 900) {
      uint8_t slow_FWDspeed = (uint8_t)((long)(data.xValue - 600) * given_speed / 300);
      logTelemetry(timestamp, "moveForward", slow_FWDspeed, val7, val8, val9, data, currentState, aiCameraMode);
      moveForward(RC_ADDRESS, slow_FWDspeed);

    // FORWARD fast range 900-1023
    } else if (data.xValue >= 900) {
      uint8_t fast_FWDspeed = (uint8_t)(given_speed + ((long)(data.xValue - 900) * given_speed / 123));
      logTelemetry(timestamp, "moveForward_2.0x", fast_FWDspeed, val7, val8, val9, data, currentState, aiCameraMode);
      moveForward(RC_ADDRESS, fast_FWDspeed);

    // BACKWARD range 100-400
    } else if (data.xValue < 400 && data.xValue > 100) {
      uint8_t BKWD_speed = (uint8_t)((long)(400 - data.xValue) * given_speed / 300);
      logTelemetry(timestamp, "moveBackward", BKWD_speed, val7, val8, val9, data, currentState, aiCameraMode);
      moveBackward(RC_ADDRESS, BKWD_speed);

    // TURN LEFT
    } else if (data.yValue < 400) {
      uint8_t turn_speed = (uint8_t)((long)(400 - data.yValue) * given_speed / 400);
      logTelemetry(timestamp, "turnLeft", turn_speed, val7, val8, val9, data, currentState, aiCameraMode);
      turnLeft(RC_ADDRESS, turn_speed);

    // TURN RIGHT
    } else if (data.yValue > 600) {
      uint8_t turn_speed = (uint8_t)((long)(data.yValue - 600) * given_speed / 423);
      logTelemetry(timestamp, "turnRight", turn_speed, val7, val8, val9, data, currentState, aiCameraMode);
      turnRight(RC_ADDRESS, turn_speed);

    // STOP
    } else {
      stopAll();
      currentState = 0;
      logTelemetry(timestamp, "stopAll", 0, val7, val8, val9, data, currentState, aiCameraMode);
    }

    // checks for JOYSTICK to AI CAMERA state
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
```