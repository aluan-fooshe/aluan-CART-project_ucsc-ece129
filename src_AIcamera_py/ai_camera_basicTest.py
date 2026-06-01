import threading
import collections
import cv2
import numpy as np
from picamera2 import Picamera2
from picamera2.devices.imx500 import IMX500
import motor_roboclaw2x45s as RC
import time

# --- Shared state ---
shared_state = {"action": "stopAll", "distance_cm": 0, "zone": "MIDDLE"}
state_lock = threading.Lock()
stop_event = threading.Event()  # <-- signals both threads to exit cleanly

# --- Constants ---
KNOWN_WIDTH_CM = 20.0
FOCAL_LENGTH   = 500.0
LOWER_ORANGE2  = np.array([100, 130, 120])
UPPER_ORANGE2  = np.array([120, 210, 250])
FRAME_W, FRAME_H = 700, 400
THIRD_LEFT  = FRAME_W // 3
THIRD_RIGHT = (FRAME_W // 3) * 2
speed = 20
box_history = collections.deque(maxlen=10)


def get_zone(cx):
    if cx < THIRD_LEFT:   return "LEFT"
    elif cx < THIRD_RIGHT: return "MIDDLE"
    else:                  return "RIGHT"


def CART_statemachine(distance_cm, zone):
    if zone == "LEFT":   return "turnLeft"
    elif zone == "RIGHT": return "turnRight"
    if distance_cm <= 100:   return "stopAll"
    elif distance_cm <= 200: return "moveForward"
    elif distance_cm <= 300: return "moveForward_1.5x"
    elif distance_cm <= 400: return "moveForward_2x"
    else:                    return "stopAll"


def vision_thread():
    # IMX500 must be created INSIDE the thread, alongside Picamera2
    imx500 = IMX500("/usr/share/imx500-models/imx500_network_ssd_mobilenetv2_fpnlite_320x320_pp.rpk")
    imx500.show_network_fw_progress_bar()

    picam2 = Picamera2()
    config = picam2.create_preview_configuration(
        main={"size": (FRAME_W, FRAME_H), "format": "RGB888"}
    )
    picam2.configure(config)
    picam2.start()
    print("[vision] Camera started")  # <-- add prints so you can see what's alive

    smoothed = {"cx": 0, "cy": 0, "radius": 0}

    while not stop_event.is_set():
        frame = picam2.capture_array()
        hsv   = cv2.cvtColor(frame, cv2.COLOR_RGB2HSV)
        mask  = cv2.inRange(hsv, LOWER_ORANGE2, UPPER_ORANGE2)
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        action, distance_cm, zone = "stopAll", 0, "MIDDLE"

        if contours:
            largest = max(contours, key=cv2.contourArea)
            if cv2.contourArea(largest) > 30:
                (cx, cy), radius = cv2.minEnclosingCircle(largest)
                box_history.append((int(cx), int(cy), int(radius)))

                if len(box_history) == box_history.maxlen:
                    smoothed["cx"]     = int(sum(b[0] for b in box_history) / len(box_history))
                    smoothed["cy"]     = int(sum(b[1] for b in box_history) / len(box_history))
                    smoothed["radius"] = int(sum(b[2] for b in box_history) / len(box_history))

        if smoothed["radius"] > 0:
            size        = smoothed["radius"] * 2
            distance_cm = int(round((KNOWN_WIDTH_CM * FOCAL_LENGTH) / size))
            zone        = get_zone(smoothed["cx"])
            action      = CART_statemachine(distance_cm, zone)

        with state_lock:
            shared_state["action"]      = action
            shared_state["distance_cm"] = distance_cm
            shared_state["zone"]        = zone

        cv2.imshow("Orange Hat Tracker", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            stop_event.set()  # <-- tell motor thread to stop too
            break

    picam2.stop()
    cv2.destroyAllWindows()
    print("[vision] Thread exited")


motor_tick = threading.Event()

def motor_clock():
    """Single clock thread that pulses both motor threads in sync."""
    while not stop_event.is_set():
        motor_tick.set()    # wake both motor threads simultaneously
        motor_tick.clear()
        time.sleep(0.05)


def front_motor_thread():
    print("[front motor] Thread started")
    while not stop_event.is_set():
        motor_tick.wait()   # block until the clock fires

        with state_lock:
            action = shared_state["action"]

        print(f"[front motor] action={action}")

        if action == "moveForward":
            RC.moveForward(RC.roboclaw_front, RC.RC_ADDRESS_FRONT, speed)
        elif action == "moveForward_1.5x":
            RC.moveForward(RC.roboclaw_front, RC.RC_ADDRESS_FRONT, int(speed * 1.5))
        elif action == "moveForward_2x":
            RC.moveForward(RC.roboclaw_front, RC.RC_ADDRESS_FRONT, int(speed * 2))
        elif action == "turnLeft":
            RC.turnLeft(RC.roboclaw_front, RC.RC_ADDRESS_FRONT, int(speed * 1.3))
        elif action == "turnRight":
            RC.turnRight(RC.roboclaw_front, RC.RC_ADDRESS_FRONT, int(speed * 1.3))
        else:
            RC.stopAll(RC.roboclaw_front, RC.RC_ADDRESS_FRONT)

    RC.stopAll(RC.roboclaw_front, RC.RC_ADDRESS_FRONT)
    print("[front motor] Thread exited")


def back_motor_thread():
    print("[back motor] Thread started")
    while not stop_event.is_set():
        motor_tick.wait()   # block until the clock fires

        with state_lock:
            action = shared_state["action"]

        print(f"[back motor] action={action}")

        if action == "moveForward":
            RC.moveForward(RC.roboclaw_back, RC.RC_ADDRESS_BACK, speed)
        elif action == "moveForward_1.5x":
            RC.moveForward(RC.roboclaw_back, RC.RC_ADDRESS_BACK, int(speed * 1.5))
        elif action == "moveForward_2x":
            RC.moveForward(RC.roboclaw_back, RC.RC_ADDRESS_BACK, int(speed * 2))
        elif action == "turnLeft":
            RC.turnLeft(RC.roboclaw_back, RC.RC_ADDRESS_BACK, int(speed * 1.3))
        elif action == "turnRight":
            RC.turnRight(RC.roboclaw_back, RC.RC_ADDRESS_BACK, int(speed * 1.3))
        else:
            RC.stopAll(RC.roboclaw_back, RC.RC_ADDRESS_BACK)

    RC.stopAll(RC.roboclaw_back, RC.RC_ADDRESS_BACK)
    print("[back motor] Thread exited")


if __name__ == "__main__":
    RC.roboclaw_front.Open()
    RC.roboclaw_back.Open()

    t_vision      = threading.Thread(target=vision_thread,      daemon=True)
    t_front_motor = threading.Thread(target=front_motor_thread, daemon=True)
    t_back_motor  = threading.Thread(target=back_motor_thread,  daemon=True)
    t_clock       = threading.Thread(target=motor_clock,        daemon=True)

    t_vision.start()
    t_front_motor.start()
    t_back_motor.start()
    t_clock.start()         # clock starts last so motors are ready to receive ticks

    t_vision.join()
    stop_event.set()
    motor_tick.set()        # unblock motor threads so they can see stop_event and exit
    t_front_motor.join(timeout=2)
    t_back_motor.join(timeout=2)
    t_clock.join(timeout=2)