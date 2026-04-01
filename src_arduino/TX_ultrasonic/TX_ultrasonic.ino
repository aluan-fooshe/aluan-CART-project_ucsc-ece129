#define TX_PIN 3  // Connect ultrasonic transducer to pin 3
#define FREQ 40000  // 40kHz frequency — matches Python's 40kHz detection

void setup() {
  Serial.begin(115200);
  pinMode(TX_PIN, OUTPUT);
}

void loop() {
  sendMessage("Hello World!!!\n");  // \n tells Python receiver end of message
  delay(500);  // pause between transmissions
}

void sendMessage(String msg) {
  for (int i = 0; i < msg.length(); i++) {
    sendChar(msg.charAt(i));
  }
}

void sendChar(byte ch) {
  Serial.print((char)ch);  // debug: print to serial monitor

  // START PULSE — 10ms tone
  // Python detects this as >= 7ms → triggers read_byte()
  tone(TX_PIN, FREQ);
  delay(10);
  noTone(TX_PIN);

  // SEND 8 BITS — MSB first
  for (int i = 7; i >= 0; i--) {
    bool b = bitRead(ch, i);

    if (b) {
      // BIT 1 — 2ms tone
      // Python: duration < 3ms threshold → reads as 1
      tone(TX_PIN, FREQ);
      delay(2);
    } else {
      // BIT 0 — 4ms tone
      // Python: duration >= 3ms threshold → reads as 0
      tone(TX_PIN, FREQ);
      delay(4);
    }

    // GAP — 11ms silence between bits
    // Python: sleeps 5ms through this gap
    noTone(TX_PIN);
    delay(11);
  }
}