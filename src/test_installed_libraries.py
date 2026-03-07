# Testing every newly installed library here

from gpiozero import DigitalInputDevice
from gpiozero.pins.lgpio import LGPIOFactory
from gpiozero import Device

# Explicitly set lgpio as backend for Pi 5
Device.pin_factory = LGPIOFactory()

# Listen for LM293 comparator output (via voltage divider)
receiver = DigitalInputDevice(24)  # GPIO 24, Pin 18

def signal_detected():
    print("Ultrasonic signal received!")

receiver.when_activated = signal_detected