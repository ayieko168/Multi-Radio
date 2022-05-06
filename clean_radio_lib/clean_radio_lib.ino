
#include <SoftwareWire.h>


class Radio {

  private:

    uint8_t _sdaPin;
    uint8_t _sclPin;
    SoftwareWire _myWire;
    int _address = 0x60;
    bool _muted, _stereo;
    byte _frequencyH, _frequencyL;
    short _siglvl, _rdy;


  public:

    Radio(uint8_t sdaPin, uint8_t sclPin) {
      _sdaPin = sdaPin;
      _sclPin = sclPin;
      _muted = false;
      _frequencyH = 0x00;
      _frequencyL = 0x00;
      _siglvl = 2;

    };

    void begin() {

      SoftwareWire myWire(_sdaPin, _sclPin);
      _myWire = myWire;

      _myWire.begin();

    }

    void sendValues() {

      _myWire.beginTransmission(_address);
      _myWire.write((_muted << 7) | (false << 6) | _frequencyH);
      _myWire.write(_frequencyL);
      _myWire.write((true << 7) | (_siglvl & 0x3 << 5) | 0x10);
      _myWire.write(0x10 | (false << 6));
      _myWire.write(0x00);
      _myWire.endTransmission();
      delay(100);
    }

    void getValues() {
      byte wIn;
      _myWire.requestFrom(_address, 5);
      while (_myWire.available() < 5);

      wIn = _myWire.read();
      bool rf = wIn & 0x80;
      bool blf = wIn & 0x40;
      if (!rf)_rdy = 0;
      else if (rf && !blf)_rdy = 1;
      else _rdy = 2;
      _frequencyH = wIn & 0x3F;

      wIn = _myWire.read();
      _frequencyL = wIn;

      wIn = _myWire.read();
      _stereo = wIn & 0x80;

      wIn = _myWire.read();
      _siglvl = wIn >> 4;

      wIn = _myWire.read();

      delay(50);
    }


//  Set Functions
    bool setFrequency(float frequency) {
      if (frequency < 87.50 || frequency > 108.00) return false;

      unsigned int frequencyB = (frequency * 1000000 + 225000) / 8192;
      _frequencyH = frequencyB >> 8;
      _frequencyL = frequencyB & 0XFF;
      
      sendValues();
      
      return true;
    }

    void toggleMute(){
      _muted = !_muted
    }


//  Get Functions
    bool isMuted(){
      getValues();
      return _muted;
    }

    short getSignalLevel(){
      getValues();
      return _siglvl;
    }

    float getFrequency(){
      getValues();
      double freqI = (_frequencyH << 8) | _frequencyL;
      return (freqI * 8192 - 225000) / 1000000.00; 
    }

    bool isStereo(){
    getValues();
    return _stereo;
    }


};


Radio radio1(2, 3);

void setup() {

  Serial.begin(9600);

  radio1.begin();
  radio1.setFrequency(106.7);
}

void loop() {

  Serial.print("Frequency: ");
  Serial.print(radio1.getFrequency());
  Serial.print(" MHz - ");

  Serial.print("Is Muted: ");
  Serial.print(radio1.isMuted());
  Serial.print(" - ");

  Serial.print("Is Stereo: ");
  Serial.print(radio1.isStereo());
  Serial.print(" - ");

  Serial.print("Signal Level: ");
  Serial.print(radio1.getSignalLevel());
  Serial.println(" ");

  delay(1500);
  
}
