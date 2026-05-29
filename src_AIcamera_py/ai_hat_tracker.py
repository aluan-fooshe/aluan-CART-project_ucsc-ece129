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