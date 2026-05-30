from pyrf24 import RF24, RF24_PA_LOW
import lgpio
import time

# ============================================================
# Raspberry Pi 5 RF Receiver
# ============================================================
# Wiring:
#   GREEN LED     --> GPIO 2  (Physical Pin 3)  + 330Ω to GND
#   RED LED       --> GPIO 4  (Physical Pin 7)  + 330Ω to GND
#   nRF24L01 CE   --> GPIO 22 (Physical Pin 15)
#   nRF24L01 CSN  --> GPIO 8  (Physical Pin 24)
#   nRF24L01 SCK  --> GPIO 11 (Physical Pin 23)
#   nRF24L01 MOSI --> GPIO 10 (Physical Pin 19)
#   nRF24L01 MISO --> GPIO 9  (Physical Pin 21)
#   nRF24L01 VCC  --> 3.3V    (Physical Pin 1)
#   nRF24L01 GND  --> GND     (Physical Pin 6)
# ============================================================

GREEN_LED = 17   # GPIO 17 (Physical Pin 11) - ON when signal is CLOSE
RED_LED   = 27   # GPIO 27 (Physical Pin 13) - ON when signal is FAR

address = b"00001"  # Must match transmitter

# Setup GPIO
h = lgpio.gpiochip_open(4)
lgpio.gpio_claim_output(h, GREEN_LED)
lgpio.gpio_claim_output(h, RED_LED)

# Both LEDs off at start
lgpio.gpio_write(h, GREEN_LED, 0)
lgpio.gpio_write(h, RED_LED, 0)

# Setup radio - CE=GPIO22, CSN=SPI bus 0
radio = RF24(22, 0)

if not radio.begin():
    print("ERROR: nRF24L01 not responding - check wiring")
    lgpio.gpiochip_close(h)
    exit()

radio.open_rx_pipe(0, address)
radio.set_pa_level(RF24_PA_LOW)
radio.listen = True

print("Listening for transmissions...")
print("GREEN = CLOSE  |  RED = FAR")

def set_leds(strong_signal):
    """Green ON + Red OFF if close, Green OFF + Red ON if far"""
    if strong_signal:
        lgpio.gpio_write(h, GREEN_LED, 1)
        lgpio.gpio_write(h, RED_LED,   0)
        print("Distance: CLOSE")
    else:
        lgpio.gpio_write(h, GREEN_LED, 0)
        lgpio.gpio_write(h, RED_LED,   1)
        print("Distance: FAR")

def no_signal_leds(red, green):
    if red == 1:
        red = 0
    elif red == 0:
        red = 1

    if green == 1:
        green = 0
    elif green == 0:
        green = 1

    lgpio.gpio_write(h, RED_LED,   red)
    lgpio.gpio_write(h, GREEN_LED, green)
    return red, green

red = 1
green = 0

try:
    while True:
        if radio.available():
            # Check signal strength BEFORE reading (True = close, False = far)
            strong_signal = radio.rpd

            # Read incoming payload
            received = radio.read(32)
            text = received.decode("utf-8").rstrip("\x00")  # Strip null bytes
            print(f"Received: {text}")

            if text == "USER":
                print("User detected")
                set_leds(strong_signal)  # ✅ Fixed - now actually calls LED function
        else:
            print("No transmit signal recieved")
            red, green = no_signal_leds(red, green)
            time.sleep(1)

        time.sleep(0.01)  # Small delay to avoid hammering the CPU

except KeyboardInterrupt:
    print("\nStopping...")
    lgpio.gpio_write(h, GREEN_LED, 0)
    lgpio.gpio_write(h, RED_LED,   0)
    radio.listen = False
    lgpio.gpiochip_close(h)


'''
**LED behavior:**

Signal CLOSE  -->  🟢 Green ON  + Red OFF
Signal FAR    -->  🔴 Green OFF + Red ON
No signal     -->  Both OFF
'''