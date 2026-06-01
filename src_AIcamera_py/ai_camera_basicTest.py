# custom created python libraries
# Author: Audrey Luan
import motor_roboclaw2x45s

# imported libraries
import collections
import cv2
import numpy as np
from picamera2 import Picamera2
from picamera2.devices.imx500 import IMX500

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


# Add before main()
box_history = collections.deque(maxlen=10)

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

            if contours:
                largest = max(contours, key=cv2.contourArea)
                if cv2.contourArea(largest) > 30:
                    (cx, cy), radius = cv2.minEnclosingCircle(largest)
                    cx, cy, radius = int(cx), int(cy), int(radius)

                    # Always accumulate raw detections into the history buffer
                    box_history.append((cx, cy, radius))

                    # Only compute and display the moving average once
                    # the deque has filled up to its full capacity of 10 samples
                    if len(box_history) == box_history.maxlen:
                        smoothed["cx"]     = int(sum(b[0] for b in box_history) / len(box_history))
                        smoothed["cy"]     = int(sum(b[1] for b in box_history) / len(box_history))
                        smoothed["radius"] = int(sum(b[2] for b in box_history) / len(box_history))

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
                
                # state machine implementation goes here


            # Show how many samples have been collected while warming up
            if len(box_history) < box_history.maxlen:
                cv2.putText(frame, f"Warming up: {len(box_history)}/{box_history.maxlen}",
                            (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 2)

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