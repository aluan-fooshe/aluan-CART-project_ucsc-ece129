# Sftwre01_AICamera-testcodefiles

<b>Author:</b> Audrey Luan </br>
<b>Date Written:</b> 2026 June 12, 05:04PM </br>
<b>Directory Path:</b> /aluan-CART-project_ucsc-ece129/src_AIcamera_py

### Notes

Part 3 of the Software subsystem appendices, includes all the test code used to program the basic functionalities of the AI camera system.
</br>
- <b>distance_tracker.py</b>: basic distance to pixels, for square contour around the orange colored object.
- <b>motor_roboclaw2x45.py</b>: the Python equivalent of CART_current_draw_testing.ino for Raspberry Pi 5 implementation. <strong style="color:red">Attempt failed</strong>, Arduino Uno is better suited for commanding the Roboclaw 2x45 motor controllers.
- <b>ai_hat_tracker.py</b>: basic programming for AI camera to track any object that is a shade of saturated orange, with the AI camera viewer capturing real-time video.

## distance_tracker.py

```python
import cv2
import numpy as np
from picamera2 import Picamera2

# --- CALIBRATION CONSTANTS ---
# Adjust these based on your specific orange hat and environment
KNOWN_WIDTH_CM = 20.0  
FOCAL_LENGTH = 500.0   

# HSV Color Boundaries for Orange
LOWER_ORANGE = np.array([110, 110, 140])
UPPER_ORANGE = np.array([120, 160, 200])

def main():
    print("Initializing Raspberry Pi AI Camera on Pi 5...")
    picam2 = Picamera2()
    
    # Configure camera for RGB output
    config = picam2.create_preview_configuration(main={"size": (640, 480), "format": "RGB888"})
    picam2.configure(config)
    picam2.start()
    
    print("Camera active. Press 'q' in the video window to quit.")
    
    try:
        while True:
            frame = picam2.capture_array()
            metadata = picam2.capture_metadata()
            
            # 1. Parse the IMX500 AI metadata
            if "objects" in metadata:
                for obj in metadata["objects"]:
                    label = obj.get("label", "")
                    score = obj.get("score", 0.0)
                    
                    # 2. Check if the AI sees a person with high confidence
                    if label == "person" and score > 0.50:
                        x, y, w, h = obj["bbox"]
                        
                        # 3. Crop to the top 25% of the bounding box (Head area)
                        head_y1 = max(0, y)
                        head_y2 = min(480, int(y + (h * 0.25)))
                        head_x1 = max(0, x)
                        head_x2 = min(640, x + w)
                        
                        head_roi = frame[head_y1:head_y2, head_x1:head_x2]
                        if head_roi.size == 0:
                            continue
                            
                        # 4. Apply OpenCV color mask for Orange
                        hsv_roi = cv2.cvtColor(head_roi, cv2.COLOR_RGB2HSV)
                        mask = cv2.inRange(hsv_roi, LOWER_ORANGE, UPPER_ORANGE)
                        
                        # 5. Find contours of the orange hat to measure its width in pixels
                        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
                        
                        if contours:
                            largest_contour = max(contours, key=cv2.contourArea)
                            
                            # Filter out tiny specks of orange background noise
                            if cv2.contourArea(largest_contour) > 100:
                                cx, cy, cw, ch = cv2.boundingRect(largest_contour)
                                
                                # cw is the perceived pixel-width of the hat 
                                # 6. Calculate Distance
                                distance_cm = (KNOWN_WIDTH_CM * FOCAL_LENGTH) / cw
                                
                                # 7. Draw bounding boxes and text
                                cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
                                
                                hat_x = head_x1 + cx
                                hat_y = head_y1 + cy
                                cv2.rectangle(frame, (hat_x, hat_y), (hat_x + cw, hat_y + ch), (0, 165, 255), 2)
                                
                                text = f"Distance: {distance_cm:.1f} cm"
                                cv2.putText(frame, text, (x, max(20, y - 10)), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
            
            # Convert frame back to BGR so OpenCV displays colors correctly
            # bgr_frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
            cv2.imshow("Orange Hat Distance Tracker", frame)
            
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    except KeyboardInterrupt:
        print("Exiting...")
    finally:
        picam2.stop()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
```

## motor_roboclaw2x45.py

```python
"""
Python equivalent of CART_current_draw_testing.ino
Original authors: Audrey Luan, Noah Lee, Renat Dobornian

Reads current draw, voltage, speed, and battery temperature from a
4-wheel cart driven by two RoboClaw 2x45A controllers (FRONT + BACK).
"""

import threading
import serial
import time
from roboclaw_3 import Roboclaw

RC_ADDRESS_FRONT = 0x80
RC_ADDRESS_BACK = 0x80  #different address, same serial line
BAUDRATE = 38400

speed = 20
roboclaw_front = Roboclaw("/dev/ttyAMA0", BAUDRATE)
roboclaw_back = Roboclaw("/dev/ttyAMA3", BAUDRATE)

# Python implementation of roboclaw rebooting time, yet to be tested.
def wait_for_roboclaw(rc, address, timeout=5.0):
    """Wait until RoboClaw responds or timeout."""
    start = time.time()
    while time.time() - start < timeout:
        version = rc.ReadVersion(address)
        if version[0]:  # returns (success, version_string)
            print(f"RoboClaw ready: {version[1].strip()}")
            return True
        time.sleep(0.1)
    raise TimeoutError("RoboClaw did not respond in time")


def moveForward(roboclaw, address, speed):
    roboclaw.ForwardM1(address, speed)
    roboclaw.ForwardM2(address, speed)

def moveBackward(roboclaw, address, speed):
    roboclaw.BackwardM1(address, speed)
    roboclaw.BackwardM2(address, speed)


def turnLeft(roboclaw, address, speed):
    roboclaw.BackwardM1(address, speed)
    roboclaw.ForwardM2(address, speed)


def turnRight(roboclaw, address, speed):
    roboclaw.ForwardM1(address, speed)
    roboclaw.BackwardM2(address, speed)


def stopAll(roboclaw, address):
    roboclaw.ForwardM1(address, 0)
    roboclaw.ForwardM2(address, 0)
    

# if __name__ == "__main__":
#     if roboclaw_front.Open():
#         print("roboclaw front works!")
#     if roboclaw_back.Open():
#         print("roboclaw back works!")

#     time.sleep(2)

#     moveForward(speed, 5)
#     roboclaw_front.ForwardM1(RC_ADDRESS_FRONT, 0)
#     roboclaw_front.ForwardM2(RC_ADDRESS_FRONT, 0)
#     roboclaw_back.ForwardM1(RC_ADDRESS_BACK, 0)
#     roboclaw_back.ForwardM2(RC_ADDRESS_BACK, 0)
#     time.sleep(5)

```

## ai_camera_headless.py

```python
import cv2
import numpy as np
import time
from picamera2 import Picamera2
from gpiozero import AngularServo
# Replaced ___.pigpio and PiGPIOFactory with ___.lgpio and LGPIOFactory
from gpiozero.pins.lgpio import LGPIOFactory

# 1. INITIALIZE SERVOS
# We use pigpio factory for smooth hardware PWM. (Run 'sudo pigpiod' in terminal first!)
factory = LGPIOFactory()
pan_servo = AngularServo(17, min_angle=-90, max_angle=90, pin_factory=factory)
tilt_servo = AngularServo(27, min_angle=-90, max_angle=90, pin_factory=factory)

# Initial center position
pan_angle = 0
tilt_angle = 0
pan_servo.angle = pan_angle
tilt_servo.angle = tilt_angle

# 2. CAMERA AND FRAME SETTINGS
FRAME_W = 640
FRAME_H = 480
CENTER_X = FRAME_W // 2
CENTER_Y = FRAME_H // 2

def main():
    print("Initializing AI Camera...")
    picam2 = Picamera2()
    
    # Configure camera stream
    config = picam2.create_preview_configuration(main={"size": (FRAME_W, FRAME_H), "format": "RGB888"})
    picam2.configure(config)
    picam2.start()
    
    print("Tracking Started. Press Ctrl+C to exit.")
    
    try:
        while True:
            # Capture frame and AI metadata simultaneously
            frame = picam2.capture_array()
            metadata = picam2.capture_metadata()
            
            # The IMX500 populates 'objects' when it detects COCO classifications
            if "objects" in metadata:
                for obj in metadata["objects"]:
                    label = obj.get("label", "")
                    score = obj.get("score", 0.0)
                    
                    # Check if the AI sees a person with > 50% confidence
                    if label == "person" and score > 0.50:
                        # Extract Bounding Box
                        x, y, w, h = obj["bbox"]
                        
                        # Calculate the top 25% of the bounding box (The Head/Hat area)
                        head_y1 = max(0, y)
                        head_y2 = min(FRAME_H, int(y + (h * 0.25)))
                        head_x1 = max(0, x)
                        head_x2 = min(FRAME_W, x + w)
                        
                        # Crop the head area and convert to HSV color space
                        head_roi = frame[head_y1:head_y2, head_x1:head_x2]
                        if head_roi.size == 0:
                            continue
                            
            # Show the video feed
            # Note: OpenCV uses BGR natively, but Picamera2 outputs RGB. We convert it for display.
            #       The bgr_frame variable creates a blue filter by default, but it can also do;
        #               Greyscale, Hue Saturation & Value, etc.
            # bgr_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

            cv2.imshow("AI Orange Hat Tracker", frame)
            
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    except KeyboardInterrupt:
        print("Exiting cleanly...")
    finally:
        picam2.stop()
        cv2.destroyAllWindows()
        pan_servo.detach()
        tilt_servo.detach()

if __name__ == "__main__":
    main()
```