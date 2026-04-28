#include <Morse.h>

Morse morse(13);  // pin 13 (built-in LED on most boards)

void setup() {
    morse.begin();
    Serial.begin(9600);  // initialize serial monitor
}

void loop() {
    // SOS: · · · — — — · · ·
    morse.dot(); morse.dot(); morse.dot();  // S
    morse.dash(); morse.dash(); morse.dash(); // O
    morse.dot(); morse.dot(); morse.dot();  // S

    Serial.println("SOS");

    delay(3000);  // pause before repeating
}