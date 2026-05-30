#define RX_pin 5
int pos = 0; // Index for postion of bit in a character
unsigned char CH = 0; // Character in the receiving string
unsigned int bits1 = 0; // Variable to count the length of frequency (in not any particular unit)
boolean capture = false; // HIGH if the received signal hods string else LOW

void setup() 
{
   Serial.begin(115200); // Set the serial monitor to 115200 baud rate
   pinMode(RX_pin, INPUT_PULLUP); // Configure RX_pin to PULLUP resistro (but it should be PULLDOWN XD STILL CONFUSED )
}

void loop() 
{
   if(digitalRead(RX_pin)) // When RX_pin is HIGH execute
   {
      bits1 = 0;
      unsigned long deltaT = millis(); // Returns the number of milliseconds passed since the Arduino board began running the current program
      // Wait for 10 millisecond and record the no. of time for which the RX_pin goes high during that period
      while(millis()-deltaT <= 10) if(digitalRead(RX_pin)) bits1 ++;
      //Serial.println(bits1); 
      if(capture)
      {
         boolean b = 0; // temp variable to store bit info of the signal
         if(bits1 > 290 && bits1 < 600) b = 0;
         if(bits1 > 20 && bits1 < 290) b = 1;
         if(b) bitSet(CH,7-pos); else bitClear(CH,7-pos); // set and reset the bits in the character from lSV
         //Serial.print(b);
         pos++;
         if(pos == 8) // After writing character finished print it
         {
            Serial.print((char)CH);
            pos = 0;
            capture = false;
         }
      }
      if(bits1 > 600)
      {
         capture = true;
         pos = 0;
      }
   }
}
