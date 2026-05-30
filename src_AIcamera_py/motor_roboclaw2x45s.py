"""
Python equivalent of Motor_Roboclaw2x45S.ino
Controls a RoboClaw 2x45A motor controller via serial (Packet Serial mode).

Hardware setup (mirrors Arduino sketch):
  - Arduino used pins 10 (TX) / 11 (RX) via SoftwareSerial at 38400 baud
  - On a Raspberry Pi use a UART port, e.g. /dev/ttyS0 or /dev/ttyAMA0
  - RoboClaw S1 pin → Pi TX  (GPIO 14, physical pin 8)
  - RoboClaw S2 pin → Pi RX  (GPIO 15, physical pin 10)
  - Shared GND between Pi and RoboClaw

Dependencies:
  pip install pyserial
  # Basicmicro's official Python library (optional but recommended):
  # pip install basicmicro
  # or clone: https://github.com/basicmicro/roboclaw_python
"""

import time
import struct
import serial

# ── Configuration ────────────────────────────────────────────────────────────
SERIAL_PORT = "/dev/ttyS0"   # change to your UART device
BAUD_RATE   = 38400
ADDRESS     = 0x80           # default RoboClaw packet-serial address

# Motor speed (0–127 = forward, matches Arduino sketch value of 64 ≈ 50 % fwd)
MOTOR1_SPEED = 64
# ─────────────────────────────────────────────────────────────────────────────


def crc16(data: bytes) -> int:
    """
    CRC-16 as required by RoboClaw packet serial protocol.
    Polynomial 0x1021, initial value 0.
    """
    crc = 0
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
        crc &= 0xFFFF
    return crc


def send_command(ser: serial.Serial, address: int, command: int,
                 *args: int) -> None:
    """
    Build and send a RoboClaw packet-serial command with CRC-16 checksum.

    Packet layout:  [address] [command] [data bytes …] [CRC high] [CRC low]
    """
    payload = bytes([address, command] + list(args))
    checksum = crc16(payload)
    packet = payload + struct.pack(">H", checksum)   # big-endian 16-bit CRC
    ser.write(packet)


def forward_m1(ser: serial.Serial, address: int, speed: int) -> None:
    """
    Drive Motor 1 forward at the given speed (0–127).
    RoboClaw command 0 = Drive M1 Forward.
    """
    speed = max(0, min(127, speed))   # clamp to valid range
    send_command(ser, address, 0, speed)


def main() -> None:
    print(f"Opening {SERIAL_PORT} at {BAUD_RATE} baud …")
    ser = serial.Serial(
        port=SERIAL_PORT,
        baudrate=BAUD_RATE,
        timeout=0.1,
    )
    time.sleep(0.1)   # brief settle, mirrors roboclaw.begin() delay

    print(f"Sending ForwardM1(address=0x{ADDRESS:02X}, speed={MOTOR1_SPEED}) — "
          "press Ctrl-C to stop.")
    try:
        while True:
            forward_m1(ser, ADDRESS, MOTOR1_SPEED)
            time.sleep(0.02)   # ~50 Hz, keeps the watchdog happy
    except KeyboardInterrupt:
        print("\nStopping motor …")
        forward_m1(ser, ADDRESS, 0)   # send speed=0 before exit
    finally:
        ser.close()
        print("Serial port closed.")


if __name__ == "__main__":
    main()
