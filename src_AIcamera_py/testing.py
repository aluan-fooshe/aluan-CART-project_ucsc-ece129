import lgpio
import time

# Open the GPIO chip (Pi 5 uses chip 4)
chip = lgpio.gpiochip_open(4)
print("[gpio] Chip opened successfully")

# --- Declare pins ---
# Pi GPIO 14 (pin 8)  → Arduino pin 7
# Pi GPIO 15 (pin 10) → Arduino pin 8
# Pi GPIO 9  (pin 21) → Arduino pin 9
GPIO_TO_ARDUINO_7 = 14
GPIO_TO_ARDUINO_8 = 15
GPIO_TO_ARDUINO_9 = 9

lgpio.gpio_claim_output(chip, GPIO_TO_ARDUINO_7)
print(f"[gpio] GPIO {GPIO_TO_ARDUINO_7} claimed as OUTPUT → Arduino pin 7")

lgpio.gpio_claim_output(chip, GPIO_TO_ARDUINO_8)
print(f"[gpio] GPIO {GPIO_TO_ARDUINO_8} claimed as OUTPUT → Arduino pin 8")

lgpio.gpio_claim_output(chip, GPIO_TO_ARDUINO_9)
print(f"[gpio] GPIO {GPIO_TO_ARDUINO_9} claimed as OUTPUT → Arduino pin 9")

# --- Send signals ---
def send_signal(arduino_pin, gpio_pin, value):
    lgpio.gpio_write(chip, gpio_pin, value)
    print(f"[gpio] GPIO {gpio_pin} → Arduino pin {arduino_pin} set {'HIGH' if value == 1 else 'LOW'}")

if __name__ == "__main__":
    send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=1)
    send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=1)
    send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=1)
    time.sleep(1)

    send_signal(arduino_pin=7, gpio_pin=GPIO_TO_ARDUINO_7, value=0)
    send_signal(arduino_pin=8, gpio_pin=GPIO_TO_ARDUINO_8, value=0)
    send_signal(arduino_pin=9, gpio_pin=GPIO_TO_ARDUINO_9, value=0)
    time.sleep(1)

    # --- Cleanup ---
    lgpio.gpiochip_close(chip)
    print("[gpio] Chip closed, cleanup done")