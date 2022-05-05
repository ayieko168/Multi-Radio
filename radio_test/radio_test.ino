
#include <SoftwareWire.h>


SoftwareWire myWire(2, 3);


void setup() {

  float frequency = 100.3;
  int _address = 0x60;
  
  myWire.begin();
  
  Serial.begin(9600);
  while (!Serial); // Leonardo: wait for Serial Monitor
  Serial.println("\nSetting radio frequency...");

  unsigned int frequencyB = 4 * (frequency * 1000000 + 225000) / 32768; 
  byte frequencyH = frequencyB >> 8;
  byte frequencyL = frequencyB & 0XFF;
  myWire.beginTransmission(_address);  
  myWire.write(frequencyH);
  myWire.write(frequencyL);
  myWire.write(0xB0);
  myWire.write(0x10);
  myWire.write(0x00);
  myWire.endTransmission();
  delay(100);

}

void loop() {
  // put your main code here, to run repeatedly:

}
