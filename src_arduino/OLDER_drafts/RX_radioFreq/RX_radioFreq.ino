#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radioLeft(7,8);
RF24 radioRight(4,5);

const byte address1[6] = "00001";
const byte address2[6] = "00002";

int leftPackets = 0;
int rightPackets = 0;

unsigned long lastMeasure = 0;

void setup() {
  Serial.begin(9600);

  radioLeft.begin();
  radioLeft.setChannel(100);
  radioLeft.openReadingPipe(1, address1);
  radioLeft.setPALevel(RF24_PA_LOW);
  radioLeft.setDataRate(RF24_250KBPS);  // match transmitter
  radioLeft.startListening();

  radioRight.begin();
  radioRight.setChannel(110);
  radioRight.openReadingPipe(1, address2);
  radioRight.setPALevel(RF24_PA_LOW);
  radioRight.setDataRate(RF24_250KBPS);  // match transmitter
  radioRight.startListening();

  // Debug on startup
  Serial.print("radioLeft connected (CE=9, CSN=10): ");
  Serial.println(radioLeft.isChipConnected() ? "YES" : "NO");
  Serial.print("radioRight connected (CE=4, CSN=5): ");
  Serial.println(radioRight.isChipConnected() ? "YES" : "NO");
}

void loop() {
  if(radioLeft.available()){
    char text[32];
    radioLeft.read(&text, sizeof(text));
    leftPackets++;
  }

  if(radioRight.available()){
    char text[32];
    radioRight.read(&text, sizeof(text));
    rightPackets++;
  }

if(millis() - lastMeasure > 1000){
    int difference = leftPackets - rightPackets;
    int totalPackets = leftPackets + rightPackets;

    // Serial Plotter line — must come first, no other prints before it
    Serial.print("Left:");
    Serial.print(leftPackets);
    Serial.print(",");
    Serial.print("Right:");
    Serial.println(rightPackets);

    // Readable output below
    Serial.print("Left packets: ");
    Serial.println(leftPackets);
    Serial.print("Right packets: ");
    Serial.println(rightPackets);

    if(difference >= 1)
      Serial.println("USER IS LEFT");
    else if(difference <= -1)
      Serial.println("USER IS RIGHT");
    else
      Serial.println("USER IS CENTER");

    if(totalPackets > 120)
      Serial.println("VERY CLOSE");
    else if(totalPackets > 60)
      Serial.println("MEDIUM");
    else if(totalPackets > 20)
      Serial.println("FAR");
    else
      Serial.println("VERY FAR");

    Serial.println();

    leftPackets = 0;
    rightPackets = 0;
    lastMeasure = millis();
  }
}
