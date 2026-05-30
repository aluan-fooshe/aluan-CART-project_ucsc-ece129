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
    → SERIAL_PORT_FRONT = "/dev/ttyAMA0"  (or /dev/ttyS0)

  BACK RoboClaw (needs a second UART or USB-serial adapter):
    Pi TX2 / USB adapter     → RoboClaw S1
    Pi RX2 / USB adapter     → RoboClaw S2
    → SERIAL_PORT_BACK  = "/dev/ttyUSB0"  (adjust as needed)

Dependencies:
  pip install pyserial
  # For NRF24L01 radio (optional):
  # pip install pyrf24   (https://github.com/nRF24/pyRF24)
"""

import struct
import time
import serial

# ── Configuration ─────────────────────────────────────────────────────────────
SERIAL_PORT_FRONT = "/dev/ttyAMA0"
SERIAL_PORT_BACK  = "/dev/ttyUSB0"
BAUDRATE          = 38400
RC_ADDRESS        = 0x80

# Velocity PID coefficients (same as #define values in the sketch)
Kp   = 1.0
Ki   = 0.5
Kd   = 0.25
QPPS = 100          # quadrature pulses per second at full speed

# Test parameters
GIVEN_SPEED = 20    # 0-127
GIVEN_TIME  = 23.0  # seconds  (Arduino used 23000 ms)
# ──────────────────────────────────────────────────────────────────────────────


# ── CRC-16 (RoboClaw packet serial) ──────────────────────────────────────────
def crc16(data: bytes) -> int:
    crc = 0
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if (crc & 0x8000) else (crc << 1)
        crc &= 0xFFFF
    return crc


# ── Low-level serial helpers ──────────────────────────────────────────────────
def _write_cmd(ser: serial.Serial, address: int, cmd: int,
               *args: int) -> None:
    """Send a write command (no response expected beyond ACK)."""
    payload  = bytes([address, cmd] + list(args))
    checksum = crc16(payload)
    ser.write(payload + struct.pack(">H", checksum))


def _read_cmd(ser: serial.Serial, address: int, cmd: int,
              n_bytes: int) -> bytes | None:
    """
    Send a read command and return n_bytes of response data
    (excluding the 2-byte CRC that the RoboClaw appends).
    Returns None on timeout or CRC mismatch.
    """
    payload = bytes([address, cmd])
    ser.write(payload)
    raw = ser.read(n_bytes + 2)          # data + 2-byte CRC
    if len(raw) < n_bytes + 2:
        return None                       # timeout
    data, recv_crc = raw[:n_bytes], struct.unpack(">H", raw[n_bytes:])[0]
    calc_crc = crc16(payload + data)
    return data if calc_crc == recv_crc else None


# ── RoboClaw command wrappers ─────────────────────────────────────────────────
# Command numbers from the RoboClaw serial reference manual.

CMD_FORWARD_M1  = 0
CMD_FORWARD_M2  = 4
CMD_BACKWARD_M1 = 1
CMD_BACKWARD_M2 = 5
CMD_READ_ENC_M1 = 16
CMD_READ_ENC_M2 = 17
CMD_READ_SPD_M1 = 18
CMD_READ_SPD_M2 = 19
CMD_SET_VEL_PID_M1 = 28
CMD_SET_VEL_PID_M2 = 29


def forward_m1(ser, addr, speed):
    _write_cmd(ser, addr, CMD_FORWARD_M1, max(0, min(127, speed)))

def forward_m2(ser, addr, speed):
    _write_cmd(ser, addr, CMD_FORWARD_M2, max(0, min(127, speed)))

def backward_m1(ser, addr, speed):
    _write_cmd(ser, addr, CMD_BACKWARD_M1, max(0, min(127, speed)))

def backward_m2(ser, addr, speed):
    _write_cmd(ser, addr, CMD_BACKWARD_M2, max(0, min(127, speed)))


def read_enc_m1(ser, addr) -> int | None:
    data = _read_cmd(ser, addr, CMD_READ_ENC_M1, 5)   # 4-byte count + status
    return struct.unpack(">i", data[:4])[0] if data else None

def read_enc_m2(ser, addr) -> int | None:
    data = _read_cmd(ser, addr, CMD_READ_ENC_M2, 5)
    return struct.unpack(">i", data[:4])[0] if data else None

def read_speed_m1(ser, addr) -> int | None:
    data = _read_cmd(ser, addr, CMD_READ_SPD_M1, 5)   # 4-byte speed + status
    return struct.unpack(">i", data[:4])[0] if data else None

def read_speed_m2(ser, addr) -> int | None:
    data = _read_cmd(ser, addr, CMD_READ_SPD_M2, 5)
    return struct.unpack(">i", data[:4])[0] if data else None


def set_velocity_pid(ser, addr, cmd, kd, kp, ki, qpps):
    """Pack four 32-bit floats as required by SetM1/M2VelocityPID."""
    args = struct.pack(">IIII",
                       int(kd   * 65536),
                       int(kp   * 65536),
                       int(ki   * 65536),
                       int(qpps))
    _write_cmd(ser, addr, cmd, *args)


# ── High-level motion helpers ─────────────────────────────────────────────────

def move_forward(front, back, addr, speed, duration=0.0):
    forward_m1(front, addr, speed)
    forward_m2(front, addr, speed)
    forward_m1(back,  addr, speed)
    forward_m2(back,  addr, speed)
    if duration > 0:
        time.sleep(duration)

def move_backward(front, back, addr, speed, duration=0.0):
    backward_m1(front, addr, speed)
    backward_m2(front, addr, speed)
    backward_m1(back,  addr, speed)
    backward_m2(back,  addr, speed)
    if duration > 0:
        time.sleep(duration)

def turn_right(front, back, addr, speed, duration=0.0):
    """Left-side motors forward, right-side motors backward → turns right."""
    forward_m1(front,  addr, speed)
    backward_m2(front, addr, speed)
    forward_m1(back,   addr, speed)
    backward_m2(back,  addr, speed)
    if duration > 0:
        time.sleep(duration)

def turn_left(front, back, addr, speed, duration=0.0):
    backward_m1(front, addr, speed)
    forward_m2(front,  addr, speed)
    backward_m1(back,  addr, speed)
    forward_m2(back,   addr, speed)
    if duration > 0:
        time.sleep(duration)

def stop_all(front, back, addr):
    forward_m1(front, addr, 0)
    forward_m2(front, addr, 0)
    forward_m1(back,  addr, 0)
    forward_m2(back,  addr, 0)


# ── Telemetry display (mirrors displayspeed() in the sketch) ──────────────────

def display_speed(front, back, addr):
    enc1  = read_enc_m1(front, addr)
    enc2  = read_enc_m2(front, addr)
    enc3  = read_enc_m1(back,  addr)
    enc4  = read_enc_m2(back,  addr)
    spd1  = read_speed_m1(front, addr)
    spd2  = read_speed_m2(front, addr)
    spd3  = read_speed_m1(back,  addr)
    spd4  = read_speed_m2(back,  addr)
    print(
        f"EncM1, SpeedM1: {enc1}, {spd1}\t\t"
        f"EncM2, SpeedM2: {enc2}, {spd2}\t\t"
        f"EncM3, SpeedM3: {enc3}, {spd3}\t\t"
        f"EncM4, SpeedM4: {enc4}, {spd4}"
    )


# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    print(f"Opening FRONT serial: {SERIAL_PORT_FRONT}")
    front = serial.Serial(SERIAL_PORT_FRONT, BAUDRATE, timeout=0.1)

    print(f"Opening BACK  serial: {SERIAL_PORT_BACK}")
    back  = serial.Serial(SERIAL_PORT_BACK,  BAUDRATE, timeout=0.1)

    # Set velocity PID on all four channels (mirrors setup())
    set_velocity_pid(front, RC_ADDRESS, CMD_SET_VEL_PID_M1, Kd, Kp, Ki, QPPS)
    set_velocity_pid(front, RC_ADDRESS, CMD_SET_VEL_PID_M2, Kd, Kp, Ki, QPPS)
    set_velocity_pid(back,  RC_ADDRESS, CMD_SET_VEL_PID_M1, Kd, Kp, Ki, QPPS)
    set_velocity_pid(back,  RC_ADDRESS, CMD_SET_VEL_PID_M2, Kd, Kp, Ki, QPPS)

    time.sleep(2.0)   # boot-up settle (mirrors delay(2000))
    print("Ready for testing (with weight)")

    time.sleep(6.0)   # mirrors delay(6000) before first move
    turn_right(front, back, RC_ADDRESS, GIVEN_SPEED, GIVEN_TIME)
    time.sleep(1.0)
    turn_right(front, back, RC_ADDRESS, 0, 1.0)   # coast / stop

    # ── Loop (mirrors loop()) ──────────────────────────────────────────────
    print("Entering loop …  Press Ctrl-C to exit.")
    try:
        while True:
            turn_right(front, back, RC_ADDRESS, 0, 1.0)
            time.sleep(2.0)
            print("loop running")
    except KeyboardInterrupt:
        print("\nStopping all motors …")
        stop_all(front, back, RC_ADDRESS)
    finally:
        front.close()
        back.close()
        print("Serial ports closed.")


if __name__ == "__main__":
    main()