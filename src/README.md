# aluan-CART-project_ucsc-ece129
## src directory

---
### basic_cart_statemachine.py

The first draft of the CART state machine. It does not include any programming for any IR sensors, motors or weight sensors yet.

This draft includes an implementation of the ultrasonic sensor (will be replaced with an IR sensor later) and the states simulated by either the key

<a href="https://learn.voltaat.com/tutorials/how-to-use-ultrasonic-sensor-with-raspberry-pi-5">How to use Ultrasonic Sensor with Raspberry pi 5 ? (voltaat.com)</a>

---
### version 1 of Microcontrollers Pugh Chart

A table of what coding libraries, sensors, components, etc. that are best suited for either ESP32 or Raspberry Pi. The hex file can be uploaded to any microcontroller.

| Microcontroller | Programming Language(s) | Coding Libraries | Sensors | Voltage | Ports |
|---|---|---|---|----|----|
| Raspberry Pi 3 Model B+ | Python (pre-installed)*, Scratch, C/C++, Bash/Shell | MFRC522 (Arduino C++),  |  | ~5 Volts| 
| Raspberry Pi (different model) | - | - | - | - | - |
| ESP32 | C/C++, MicroPython, CircuitPython, Arduino C++ | - | - | ~3 Volts | 3 UART ports |
| - | - | - | - |

Sources:

https://thelinuxcode.com/programming-languages-esp32-use/
https://raspberrytips.com/best-languages-raspberry-pi/

https://www.arduinolibraries.info/libraries/mfrc522
https://github.com/miguelbalboa/rfid
