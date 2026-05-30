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
UPPER_ORANGE1 = np.array([140, 210, 250])

# HSV Color Boundaries for Orange2 (Audrey's hat)
LOWER_ORANGE2 = np.array([100, 130, 120])
UPPER_ORANGE2 = np.array([130, 210, 250])

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

                    box_history.append((cx, cy, radius))
                    avg_cx     = int(sum(b[0] for b in box_history) / len(box_history))
                    avg_cy     = int(sum(b[1] for b in box_history) / len(box_history))
                    avg_radius = int(sum(b[2] for b in box_history) / len(box_history))

                    x = avg_cx - avg_radius      # use smoothed values for the box
                    y = avg_cy - avg_radius
                    size = avg_radius * 2

                    # Distance estimation
                    distance_cm = calculate_distance(size)

                    cv2.rectangle(frame, (x, y), (x + size, y + size), (0, 255, 0), 2)
                    cv2.putText(frame, f"Dist: {distance_cm:.1f} cm", (x, y - 30),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
                    cv2.putText(frame, f"W: {size}px  H: {size}px", (x, y + size + 20),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
                    cv2.putText(frame, "Orange detected", (x, y - 10),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
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