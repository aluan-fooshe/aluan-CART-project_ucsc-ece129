# Filename: RF_reciever_side_v1.py
#
# Notes:    A Python implementation of the Arduino RF circuit for the CART project.
#
# Author(s): Jayant Dharwakar, Audrey Luan
# -----------------------------------------------------------------

import time
from pyrf24 import RF24, RF24_PA_LOW, RF24_DRIVER
import lgpio

h = lgpio.gpiochip_open(4)
for pin in [8, 9, 10, 11, 22]:  # nRF24L01 pins
    mode = lgpio.gpio_get_mode(h, pin)
    print(f'GPIO {pin}: mode={mode}')
lgpio.gpiochip_close(h)

radio = RF24(22, 0)  # CE=GPIO22, CSN=SPI bus 0
if not radio.begin():
    print('ERROR: nRF24L01 not responding - check wiring')
else:
    print('SUCCESS: nRF24L01 connected!')
    radio.print_details()
