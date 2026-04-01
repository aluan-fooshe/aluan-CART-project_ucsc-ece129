# Filename: RF_reciever_side_v1.py
#
# Notes:    A Python implementation of the Arduino RF circuit for the CART project.
#
# Author(s): Jayant Dharwakar, Audrey Luan
#
# Date Written: 2026-03-31 11:27PM
# -----------------------------------------------------------------

import datetime
from pyrf24 import RF24, RF24_PA_MAX, RF24_1MBPS
import time

address = b"\xe1\xf0\xf0\xf0\xf0"

radio = RF24(22, 0)
if not radio.begin():
    raise RuntimeError("nRF24 hardware not responding - check wiring")

radio.pa_level = RF24_PA_MAX
radio.channel = 0x76
radio.data_rate = RF24_1MBPS
radio.dynamic_payloads = True
radio.set_auto_ack(True)

radio.open_rx_pipe(1, address)
radio.print_details()
radio.start_listening()

print("Listening on channel 0x76...")
while True:
    if radio.available():
        length = radio.get_dynamic_payload_size()
        payload = radio.read(length)
        # string = "".join(chr(n) for n in payload if 32 <= n <= 126)
        string = bytes(payload).decode('utf-8', errors='ignore').rstrip('\x00').strip()
        timestamp = datetime.datetime.now().strftime("%H:%M:%S")
        print(f"{timestamp}   {string}")
    time.sleep(0.01)


