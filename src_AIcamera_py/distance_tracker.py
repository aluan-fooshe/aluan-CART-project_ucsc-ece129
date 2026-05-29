import cv2
import numpy as np
from picamera2 import Picamera2

# --- CALIBRATION CONSTANTS ---
# Adjust these based on your specific orange hat and environment
KNOWN_WIDTH_CM = 20.0  
FOCAL_LENGTH = 500.0   

# HSV Color Boundaries for Orange
LOWER_ORANGE = np.array([5, 100, 100])
UPPER_ORANGE = np.array([25, 255, 255])

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