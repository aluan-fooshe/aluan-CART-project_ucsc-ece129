"""
Python equivalent of CART_current_draw_testing.ino
Original authors: Audrey Luan, Noah Lee, Renat Dobornian

Reads current draw, voltage, speed, and battery temperature from a
4-wheel cart driven by two RoboClaw 2x45A controllers (FRONT + BACK),
with optional NRF24L01 radio support.

Hardware wiring (Raspberry Pi GPIO → RoboClaw):
  FRONT RoboClaw:
    Pi TX  (GPIO 14, pin 8)  → RoboClaw S1
    Pi RX  (GPIO 15, pin 10) → RoboClaw S2
    → SERIAL_PORT_FRONT = -

  BACK RoboClaw (needs a second UART or USB-serial adapter):
    Pi TX2 / USB adapter     → RoboClaw S1
    Pi RX2 / USB adapter     → RoboClaw S2
    → SERIAL_PORT_BACK  = -

Dependencies:
  -
"""

import serial
import time
from roboclaw_3 import Roboclaw

RC_ADDRESS_FRONT = 0x80
RC_ADDRESS_BACK = 0x81  #different address, same serial line
BAUDRATE = 38400

speed = 20
roboclaw_front = Roboclaw("/dev/ttyAMA0", BAUDRATE)
roboclaw_back = Roboclaw("/dev/ttyAMA3", BAUDRATE)

if __name__ == "__main__":
    if roboclaw_front.Open():
        print("roboclaw front works!")
    roboclaw_front.ForwardM1(RC_ADDRESS_FRONT, speed)
    roboclaw_front.ForwardM2(RC_ADDRESS_FRONT, speed)
    time.sleep(5)
    roboclaw_front.ForwardM1(RC_ADDRESS_FRONT, 0)
    roboclaw_front.ForwardM2(RC_ADDRESS_FRONT, 0)
    time.sleep(5)
