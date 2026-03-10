# Testing every newly installed library here

import lgpio
import time

GREEN_LED = 17   # GPIO 17 (Physical Pin 11)
RED_LED   = 27   # GPIO 27 (Physical Pin 13)

h = lgpio.gpiochip_open(4)
lgpio.gpio_claim_output(h, GREEN_LED)
lgpio.gpio_claim_output(h, RED_LED)

print("Alternating LEDs every 5 seconds... (Ctrl+C to stop)")

try:
    while True:
        # Green ON, Red OFF
        lgpio.gpio_write(h, GREEN_LED, 1)
        lgpio.gpio_write(h, RED_LED,   0)
        print("🟢 Green ON")
        time.sleep(5)

        # Red ON, Green OFF
        lgpio.gpio_write(h, GREEN_LED, 0)
        lgpio.gpio_write(h, RED_LED,   1)
        print("🔴 Red ON")
        time.sleep(5)

except KeyboardInterrupt:
    print("\nStopping...")
    lgpio.gpio_write(h, GREEN_LED, 0)
    lgpio.gpio_write(h, RED_LED,   0)
    lgpio.gpiochip_close(h)