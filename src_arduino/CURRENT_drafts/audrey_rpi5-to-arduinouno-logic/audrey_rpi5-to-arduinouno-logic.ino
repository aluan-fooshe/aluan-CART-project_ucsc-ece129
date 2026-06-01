#define PI_INPUT_PIN_7 7   // receives from Pi GPIO 14
#define PI_INPUT_PIN_8 8   // receives from Pi GPIO 15

void setup() {
    Serial.begin(9600);
    pinMode(PI_INPUT_PIN_7, INPUT);
    pinMode(PI_INPUT_PIN_8, INPUT);
}

void loop() {
    int val7 = digitalRead(PI_INPUT_PIN_7);
    int val8 = digitalRead(PI_INPUT_PIN_8);
    Serial.print("Pin 7: "); Serial.print(val7);
    Serial.print(" | Pin 8: "); Serial.println(val8);
    delay(100);
}