

#include <SoftwareWire.h>

// Using pin 2 (software sda) and 3 (software scl) in this example.
SoftwareWire myWireOne(2, 3);
SoftwareWire myWireTwo(4, 5);


void setup() {
  float frequency1 = 100.3;
  float frequency2 = 98.1;

  int _address = 0x60;

  myWireOne.begin();
  myWireTwo.begin();

//  Set radio One
  unsigned int frequencyB1 = 4 * (frequency1 * 1000000 + 225000) / 32768; 
  byte frequencyH1 = frequencyB1 >> 8;
  byte frequencyL1 = frequencyB1 & 0XFF;
  myWireOne.beginTransmission(_address);  
  myWireOne.write(frequencyH1);
  myWireOne.write(frequencyL1);
  myWireOne.write(0xB0);
  myWireOne.write(0x10);
  myWireOne.write(0x00);
  myWireOne.endTransmission();
  delay(100);

//  Set Radio Two

  unsigned int frequencyB2 = 4 * (frequency2 * 1000000 + 225000) / 32768; 
  byte frequencyH2 = frequencyB2 >> 8;
  byte frequencyL2 = frequencyB2 & 0XFF;
  myWireTwo.beginTransmission(_address);  
  myWireTwo.write(frequencyH2);
  myWireTwo.write(frequencyL2);
  myWireTwo.write(0xB0);
  myWireTwo.write(0x10);
  myWireTwo.write(0x00);
  myWireTwo.endTransmission();
  delay(100);

  
}

void loop() {
}
