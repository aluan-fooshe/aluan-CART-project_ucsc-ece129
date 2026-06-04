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
LOWER_ORANGE2 = np.array([100, 175, 120])
UPPER_ORANGE2 = np.array([120, 255, 250])

# 2. CAMERA AND FRAME SETTINGS
FRAME_W = 700
FRAME_H = 400
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
    # Steering takes priority — correct heading first
    if zone == "LEFT" and distance_cm < 300:
        send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
        send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
        send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
        return "turnLeft"
    elif zone == "RIGHT" and distance_cm < 300:
        send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
        send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=1)
        send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
        return "turnRight"
    else:
        # Only reach here if zone == "MIDDLE"
        if distance_cm < 100:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
            return "stopAll"
        elif 100 <= distance_cm < 200:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=1)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
            return "moveForward"
        elif 200 <= distance_cm < 300:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=1)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
            return "moveForward_1.5x"
        elif 300 <= distance_cm < 400:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=1)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
            return "moveForward_2x"
        else:
            send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
            send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
            send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
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