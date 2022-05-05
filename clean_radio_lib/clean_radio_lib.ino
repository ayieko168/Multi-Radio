
#include <SoftwareWire.h>


class Radio {

  private:

    uint8_t _sdaPin;
    uint8_t _sclPin;
    SoftwareWire _myWire;
    int _address = 0x60;

  public:

    Radio(uint8_t sdaPin, uint8_t sclPin) {
      _sdaPin = sdaPin;
      _sclPin = sclPin;
    };

    void begin(int baud) {

      SoftwareWire myWire(_sdaPin, _sclPin);
      _myWire = myWire;
      
      _myWire.begin();

    }

    void setFrequency(float frequency) {

      unsigned int frequencyB = 4 * (frequency * 1000000 + 225000) / 32768;
      byte frequencyH = frequencyB >> 8;
      byte frequencyL = frequencyB & 0XFF;
      _myWire.beginTransmission(_address);
      _myWire.write(frequencyH);
      _myWire.write(frequencyL);
      _myWire.write(0xB0);
      _myWire.write(0x10);
      _myWire.write(0x00);
      _myWire.endTransmission();
      delay(100);

      Serial.println("Set The Freq");
    }

};


Radio radio1(2, 3);

void setup() {
  // put your setup code here, to run once:

  radio1.begin(9600);
  radio1.setFrequency(103.5);
}

void loop() {
  // put your main code here, to run repeatedly:

}
