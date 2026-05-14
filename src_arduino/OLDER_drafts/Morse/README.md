# Morse Arduino Library

A simple Arduino library for flashing Morse code via a digital output pin.

> ⚠️ **Note:** This library is a learning exercise — built to understand how Arduino libraries are structured before developing a custom library for an Arduino Uno + RoboClaw 2x45ST motor controller.

---

## Files

| File | Description |
|------|-------------|
| `Morse.h` | Class declaration |
| `Morse.cpp` | Class implementation |

---

## Installation

Copy the `Morse/` folder into your Arduino libraries directory:

```
Documents/Arduino/libraries/Morse/
    ├── Morse.h
    ├── Morse.cpp
    └── README.md
```

Then restart the Arduino IDE.

---

## Usage

```cpp
#include <Morse.h>

Morse morse(13); // Use pin 13 (built-in LED)

void setup() {
    morse.begin();
}

void loop() {
    // SOS: · · · — — — · · ·
    morse.dot(); morse.dot(); morse.dot();
    morse.dash(); morse.dash(); morse.dash();
    morse.dot(); morse.dot(); morse.dot();
    delay(3000);
}
```

---

## API

### `Morse(int pin)`
Constructor. Pass the digital pin number connected to your LED or output device.

### `void begin()`
Sets the pin to `OUTPUT` mode. Call this in `setup()`.

### `void dot()`
Flashes the pin HIGH for 250ms, then LOW for 250ms.

### `void dash()`
Flashes the pin HIGH for 1000ms, then LOW for 250ms.

---

Arduino IDE implementation without Morse library

```cpp
// basic library building testing
int pin = 13;

void setup() {
  // put your setup code here, to run once:
  pinMode(pin, OUTPUT);
}

void loop()
{
  dot(); dot(); dot();
  dash(); dash(); dash();
  dot(); dot(); dot();
  delay(3000);
}

void dot()
{
  digitalWrite(pin, HIGH);
  delay(250);
  digitalWrite(pin, LOW);
  delay(250);
}

void dash()
{
  digitalWrite(pin, HIGH);
  delay(1000);
  digitalWrite(pin, LOW);
  delay(250);
}

```

## Author
Created by Audrey — April 27, 2026
