

#include <SoftwareWire.h>

// Using pin 2 (software sda) and 3 (software scl) in this example.
SoftwareWire myWireOne(2, 3);
SoftwareWire myWireTwo(4, 5);


void setup() {
  float frequency1 = 100.3;
  float frequency2 = 98.1;

  

  myWireOne.begin();
  myWireTwo.begin();


  setFrequency(frequency1, myWireOne);


  
}

void loop() {
}

void setFrequency(float frequency, SoftwareWire _wireObj){

  int _address = 0x60;
  
  unsigned int frequencyB = 4 * (frequency * 1000000 + 225000) / 32768; 
  byte frequencyH = frequencyB >> 8;
  byte frequencyL = frequencyB & 0XFF;
  
  _wireObj.beginTransmission(_address);  
  _wireObj.write(frequencyH);
  _wireObj.write(frequencyL);
  _wireObj.write(0xB0);
  _wireObj.write(0x10);
  _wireObj.write(0x00);
  _wireObj.endTransmission();
  
  delay(100);
}
