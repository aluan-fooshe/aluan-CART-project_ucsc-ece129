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

# Open the GPIO chip (Pi 5 uses chip 4)
chip = lgpio.gpiochip_open(4)
print("[gpio] Chip opened successfully")

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
LOWER_ORANGE2 = np.array([100, 130, 120])
UPPER_ORANGE2 = np.array([120, 210, 250])

# 2. CAMERA AND FRAME SETTINGS
FRAME_W = 700
FRAME_H = 400
CENTER_X = FRAME_W // 2
CENTER_Y = FRAME_H // 2

speed = 20

# Add before main()
box_history = collections.deque(maxlen=10)

# --- Add these constants near your other frame settings ---
THIRD_LEFT  = FRAME_W // 3          # x = 0   to 233
THIRD_RIGHT = (FRAME_W // 3) * 2   # x = 233  to 466
# Right zone: THIRD_RIGHT to FRAME_W (466 to 700)


lgpio.gpio_claim_output(chip, GPIO_TO_ARDUINO_7)
print(f"[gpio] GPIO {GPIO_TO_ARDUINO_7} claimed as OUTPUT → Arduino pin 7")

lgpio.gpio_claim_output(chip, GPIO_TO_ARDUINO_8)
print(f"[gpio] GPIO {GPIO_TO_ARDUINO_8} claimed as OUTPUT → Arduino pin 8")

lgpio.gpio_claim_output(chip, GPIO_TO_ARDUINO_9)
print(f"[gpio] GPIO {GPIO_TO_ARDUINO_9} claimed as OUTPUT → Arduino pin 9")

# --- Send signals ---
def send_signal(arduino_pin, gpio_pin, value):
    lgpio.gpio_write(chip, gpio_pin, value)
    print(f"[gpio] GPIO {gpio_pin} → Arduino pin {arduino_pin} set {'HIGH' if value == 1 else 'LOW'}")


def get_zone(cx: int) -> str:
    """Return which horizontal third of the frame the target is in."""
    if cx < THIRD_LEFT:
        return "LEFT"
    elif cx < THIRD_RIGHT:
        return "MIDDLE"
    else:
        return "RIGHT"


def CART_statemachine(distance_cm, zone):
    # Steering takes priority — correct heading first
    if zone == "LEFT":
        # RC.turnLeft(int(speed * 1.3))
        send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
        send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
        send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
        print("")
        return "turnLeft"
    elif zone == "RIGHT":
        # RC.turnRight(int(speed * 1.3))
        send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
        send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=1)
        send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
        print("")
        return "turnRight"
    else:
        # Only reach here if zone == "MIDDLE"
        if distance_cm <= 100:
            # RC.stopAll()
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
            print("")
            return "stopAll"
        elif distance_cm <= 200:
            # RC.moveForward(speed)
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=1)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
            print("")
            return "moveForward"
        elif distance_cm <= 300:
            # RC.moveForward(speed * 1.5)
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=1)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
            print("")
            return "moveForward_1.5x"
        elif distance_cm <= 400:
            # RC.moveForward(speed * 2)
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=1)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
            print("")
            return "moveForward_2x"
        else:
            # RC.stopAll()
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
            print("")
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


def main():
    print("Initializing Raspberry Pi AI Camera on Pi 5...")
    picam2 = Picamera2()

    # Configure camera for RGB output
    config = picam2.create_preview_configuration(main={"size": (FRAME_W, FRAME_H), "format": "RGB888"})
    picam2.configure(config)
    picam2.start()

    print("Camera active. Press 'q' in the video window to quit.")

    # Smoothed display values — only updated once deque is full (10 samples)
    smoothed = {"cx": 0, "cy": 0, "radius": 0}

    try:
        while True:
            frame = picam2.capture_array()

            # Scan the ENTIRE frame for orange, no AI needed
            hsv_full = cv2.cvtColor(frame, cv2.COLOR_RGB2HSV)
            mask = cv2.inRange(hsv_full, LOWER_ORANGE2, UPPER_ORANGE2)

            contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

            # it returns a list of contours (outlines of orange blobs found in the frame). This just 
            # checks that list isn't empty — i.e. that at least one orange blob was found at all.
            if contours:
                # There could be multiple orange blobs detected (a reflection, a second object, noise). 
                # This picks the single largest one by area, so you're always tracking the most prominent 
                # orange shape and ignoring small distractions.
                largest = max(contours, key=cv2.contourArea)

                # A noise filter. Even the largest contour could just be a few stray orange pixels. If its 
                # area is 30 pixels or less, it's ignored entirely. 30 is a tunable threshold — raise it if 
                # you're getting false detections.
                if cv2.contourArea(largest) > 30:
                    # Fits the smallest possible circle around the contour shape. 
                    # It returns the center point (cx, cy) and the radius.
                    (cx, cy), radius = cv2.minEnclosingCircle(largest)
                    cx, cy, radius = int(cx), int(cy), int(radius)

                    # Pushes this frame's raw detection into the deque (sliding window buffer). The deque was 
                    # defined as maxlen=10, so once it has 10 entries, adding a new one automatically drops 
                    # the oldest — it self-manages.
                    box_history.append((cx, cy, radius))

                    # the smoothing step only runs once the buffer is fully "warmed up" (all 10 slots filled). 
                    # It computes a simple moving average across the last 10 frames
                    if len(box_history) == box_history.maxlen:
                        # b[0] is the cx from each stored tuple, b[1] is cy, b[2] is radius. Averaging them reduces jitter.
                        smoothed["cx"]     = int(sum(b[0] for b in box_history) / len(box_history))
                        smoothed["cy"]     = int(sum(b[1] for b in box_history) / len(box_history))
                        smoothed["radius"] = int(sum(b[2] for b in box_history) / len(box_history))

            # Draw zone dividers every frame regardless of detection
            cv2.line(frame, (THIRD_LEFT, 0),  (THIRD_LEFT, FRAME_H),  (255, 0, 0), 1)
            cv2.line(frame, (THIRD_RIGHT, 0), (THIRD_RIGHT, FRAME_H), (255, 0, 0), 1)

            # Draw using the last fully-averaged values (stays blank until first 10 samples)
            if smoothed["radius"] > 0:
                avg_cx     = smoothed["cx"]
                avg_cy     = smoothed["cy"]
                avg_radius = smoothed["radius"]

                x    = avg_cx - avg_radius
                y    = avg_cy - avg_radius
                size = avg_radius * 2

                distance_cm = int(round(calculate_distance(size)))

                cv2.rectangle(frame, (x, y), (x + size, y + size), (0, 255, 0), 2)
                cv2.putText(frame, f"Dist: {distance_cm:.1f} cm", (x, y - 30),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
                cv2.putText(frame, "Orange detected", (x, y - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                
                # Zone detection — now avg_cx is valid
                zone = get_zone(avg_cx)

                # CART state machine implementation
                action = CART_statemachine(distance_cm, zone)
                
                cv2.putText(frame, f"Action: {action}", (10, 90),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 165, 255), 2)

                cv2.putText(frame, f"Zone: {zone}", (10, 60),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 0, 255), 2)


            # # Show how many samples have been collected while warming up
            # if len(box_history) < box_history.maxlen:
            #     cv2.putText(frame, f"Warming up: {len(box_history)}/{box_history.maxlen}",
            #                 (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 2)

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