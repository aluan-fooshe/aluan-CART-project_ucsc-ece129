# Sftwre05_RFsystemCode

<b>Author:</b> Audrey Luan </br>
<b>Date Written:</b> 2026 June 12, 05:20PM </br>
<b>Directory Path (s):</b> 
- /aluan-CART-project_ucsc-ece129/src_ultrasonic_py
- /aluan-CART-project_ucsc-ece129/src_arduino/CURRENT_drafts/RX_CART_implementation_arduino
- /aluan-CART-project_ucsc-ece129/src_arduino/CURRENT_drafts/TX_radioFreq_arduinoUno

### Notes

Part 4 of the Software subsystem appendices, includes all the test code used to program the basic functionalities of the RF system.
</br>
- <b>RF_simple_test.py</b>: A Python implementation of the Arduino RF circuit for the CART project, on the receiver side.
</br></br>
- <b>RF_reciever_side_v1.py</b>: A Python implementation of the Arduino RF circuit for the CART project, on the receiver side. Uses LEDs to confirm if transmitted signal has been received or not. <strong style="color:red">Result not desirable</strong>, only 1 out of every 100 packets sent were received.
</br></br>
- <b>RX_CART_implementation_arduino.ino</b>: Dual-radio receiver that listens on two nRF24L01 channels simultaneously, counts incoming packets, and prints diagnostics (RPD, carrier detect, ARC, and per-channel packet totals) to Serial every 500 ms.
</br></br>
- <b>TX_radioFreq_arduinoUno.ino</b>: Single-radio transmitter that repeatedly sends a `"PING"` payload over nRF24L01 and reports packets sent vs. failed to Serial every second.


## RF_simple_test.py

```python
# Filename: RF_reciever_side_v1.py
#
# Notes:    A Python implementation of the Arduino RF circuit for the CART project.
#
# Author(s): Audrey Luan
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



```

## RF_reciever_side_v1.py

```python
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
```

## RX_CART_implementation_arduino.ino

```cpp
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>


// -------------------------------------------------------------------------------
//      RF RECEIVER SETUP
// -------------------------------------------------------------------------------

// RF24 radioLeft(8, 7);   // CE=8, CSN=7
// RF24 radioRight(9, 2); // CE=9, CSN=2
RF24 radioLeft(9, 2);
RF24 radioRight(8, 7);

const byte address1[6] = "00001";
const byte address2[6] = "00002";

int leftPackets  = 0;
int rightPackets = 0;

unsigned long lastMeasure = 0;
const unsigned long MEASURE_INTERVAL = 50; // ms

int printEveryInstanceOf = 10;
int counter = 0;
int leftTotal  = 0;
int rightTotal = 0;

// -------------------------------------------------------------------------------
//      SETUP
// -------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);

  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);   // CE low before init
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);  // CSN high before init

  // Radio — LEFT
  radioLeft.begin();
  radioLeft.setChannel(100);
  radioLeft.setAutoAck(false);
  radioLeft.openReadingPipe(1, address1);
  radioLeft.setPALevel(RF24_PA_LOW);
  radioLeft.setDataRate(RF24_250KBPS);
  radioLeft.startListening();

  // Radio — RIGHT
  radioRight.begin();
  radioRight.setChannel(110);
  radioRight.setAutoAck(false);
  radioRight.openReadingPipe(1, address2);
  radioRight.setPALevel(RF24_PA_LOW);
  radioRight.setDataRate(RF24_250KBPS);
  radioRight.startListening();

  Serial.print("radioLeft  connected (CE=7, CSN=8):  ");
  Serial.println(radioLeft.isChipConnected()  ? "YES" : "NO");
  Serial.print("radioRight connected (CE=9, CSN=2): ");
  Serial.println(radioRight.isChipConnected() ? "YES" : "NO");

  Serial.println("System ready.");
}

// -------------------------------------------------------------------------------
//      MAIN LOOP
// -------------------------------------------------------------------------------

void loop() {
  // Drain entire LEFT buffer
  while (radioLeft.available()) {
    char text[32];
    radioLeft.read(&text, sizeof(text));
    leftPackets++;
  }

  delayMicroseconds(100); // SPI bus settle before switching radios

  // Drain entire RIGHT buffer
  while (radioRight.available()) {
    char text[32];
    radioRight.read(&text, sizeof(text));
    rightPackets++;
  }

  // Cap to prevent overflow
  leftPackets  = min(leftPackets,  9999);
  rightPackets = min(rightPackets, 9999);

  // Every second, print packet counts
  if (millis() - lastMeasure >= MEASURE_INTERVAL) {
    counter++;

    // Accumulate every interval
    leftTotal  += leftPackets;
    rightTotal += rightPackets;

    if (counter >= printEveryInstanceOf) {
      int difference   = leftTotal - rightTotal;
      int totalPackets = leftTotal + rightTotal;

      bool rpd_L = radioLeft.testRPD();
      bool rpd_R = radioRight.testRPD();

      bool tc_L = radioLeft.testCarrier();
      bool tc_R = radioRight.testCarrier();

      uint8_t arc_L = radioLeft.getARC();
      uint8_t arc_R = radioRight.getARC();

      Serial.println("=== Radio Diagnostics ===");

      Serial.print("RPD     — Left: "); Serial.print(rpd_L ? "SIGNAL" : "none");
      Serial.print("  |  Right: ");     Serial.println(rpd_R ? "SIGNAL" : "none");

      Serial.print("Carrier — Left: "); Serial.print(tc_L ? "DETECTED" : "none");
      Serial.print("  |  Right: ");     Serial.println(tc_R ? "DETECTED" : "none");

      Serial.print("ARC     — Left: "); Serial.print(arc_L);
      Serial.print("  |  Right: ");     Serial.println(arc_R);

      Serial.print("Left:");
      Serial.print(leftTotal);
      Serial.print("  Right:");
      Serial.print(rightTotal);
      Serial.print("  Diff:");
      Serial.print(difference);
      Serial.print("  Total:");
      Serial.println(totalPackets);
      Serial.println();

      // Reset accumulators and counter
      leftTotal  = 0;
      rightTotal = 0;
      counter    = 0;
      }

      leftPackets  = 0;
      rightPackets = 0;
      lastMeasure  = millis();
    }
}
```

## TX_radioFreq_arduinoUno.ino

```cpp
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

RF24 radio(7, 8);
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(100);
  radio.enableCRC();               // ← add
  radio.setCRCLength(RF24_CRC_16); // ← add
  radio.setAutoAck(true);          // ← change from false
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address);
  radio.stopListening();
  Serial.println("Transmitter ready.");
}

unsigned long lastReport = 0;
int packetsSent   = 0;
int packetsFailed = 0;

void loop() {
  char payload[32] = "PING";
  bool ok = radio.write(&payload, sizeof(payload));
  if (ok) packetsSent++;
  else    packetsFailed++;

  if (millis() - lastReport >= 1000) {
    Serial.print("Sent: ");    Serial.print(packetsSent);
    Serial.print("  Failed: "); Serial.println(packetsFailed);
    packetsSent   = 0;
    packetsFailed = 0;
    lastReport    = millis();
  }
  delay(20);
}
```