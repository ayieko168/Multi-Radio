
#include <SoftwareWire.h>
#include <LiquidCrystal_I2C.h>

// Rotary Encoder Inputs
#define CLK 10
#define DT 11
#define SW 12

float frequency = 100.30;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
unsigned long lastButtonPress = 0;

boolean cursorTriggerd = false;
unsigned long cursorTriggeredTime = 0;
int cursorLoc = -1;
bool updateScreen = true;
float freqDx = 0.01;

int currentRadio = 0;
bool makeChange = false;
//int updateTime;


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

      delay(10);
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

    void toggleMute() {
      getValues();
      _muted = !_muted;

      sendValues();
    }


    //  Get Functions
    char isMuted() {
      getValues();

      if (_muted) {
        return 'L';
      }
      else {
        return 'M';
      }
    }

    String getSignalLevel() {
      getValues();
      //      return String(_siglvl, HEX);
      return String(_siglvl, DEC);
    }

    float getFrequency() {
      getValues();
      double freqI = (_frequencyH << 8) | _frequencyL;
      return (freqI * 8192 - 225000) / 1000000.00;
    }

    char isStereo() {
      getValues();
      if (_stereo) {
        return 'S';
      }
      else {
        return 'M';
      }
    }


};


// sdaPin, sclPin
//Radio radio1(2, 3);
//Radio radio2(4, 5);

LiquidCrystal_I2C lcd(0x3F, 16, 2);

Radio myRadios[] = {Radio(2, 3), Radio(4, 5)};

#define radiosLength (sizeof(myRadios) / sizeof(myRadios[0]))

void setup() {

  Serial.begin(115200);

  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Read the initial state of Rotary encoder CLK
  lastStateCLK = digitalRead(CLK);

  // Setup the radios
  myRadios[0].begin();
  myRadios[0].setFrequency(100.3);
  myRadios[1].begin();
  myRadios[1].setFrequency(96.0);

  initScreen();
  delay(2000);

}

void loop() {


  readRotation();
  readButn();

  displayMenu();
  checkCursor();
}


void readButn() {

  // Read the button state and debounce
  int btnState = digitalRead(SW);
  if (btnState == LOW) {
    if (millis() - lastButtonPress > 50) {
      Serial.println("Button pressed!");

      cursorLoc += 1;
      if (cursorLoc > 3) {
        cursorLoc = 0;
      }

      setCursorPos(cursorLoc);
    }
    // Remember last button press event
    lastButtonPress = millis();
  }

  // Put in a slight delay to help debounce the reading
  delay(1);
}

void readRotation() {

  // Only get readings if in triggered mode
  if (!cursorTriggerd) {
    return;
  }

  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {


      currentDir = "SUB";
      makeChange = true;



    } else {
      // Encoder is rotating CW so increment

      currentDir = "ADD";
      makeChange = true;

    }

    // Function calling logic
    if (makeChange) {
      // Function calling logic
      if (cursorLoc == 0) {
        changeRadioIndex(currentDir);
      } else if (cursorLoc == 1) {
        changeRadioFrequency(currentDir);
      } else if (cursorLoc == 2) {
        changeRadioFrequency(currentDir);
      }else if (cursorLoc == 3) {
        changeRadioMute();
      }

      serialDisplayInfo();
      updateScreen = true;
      makeChange = false;
    }

    // Reset the cursor active time whenever a rotation is triggered while cursor is still active
    if (cursorTriggerd) {
      cursorTriggeredTime = millis();
    }


  }
  // Remember last CLK state
  lastStateCLK = currentStateCLK;


}

void changeRadioMute() {
  myRadios[currentRadio].toggleMute();
}

void changeRadioFrequency(String dir) {

  if(cursorLoc == 1){
    freqDx = 1;
  }else if(cursorLoc == 2){
    freqDx = 0.01;
  }
  
  if (dir == "ADD") {

    frequency = myRadios[currentRadio].getFrequency() + freqDx;
    myRadios[currentRadio].setFrequency(frequency);

    //    Serial.print("Change radio ");
    //    Serial.print(currentRadio);
    //    Serial.print("Frequency to ");
    //    Serial.println(frequency);
  } else if (dir == "SUB") {

    frequency = myRadios[currentRadio].getFrequency() - freqDx;
    myRadios[currentRadio].setFrequency(frequency);

    //    Serial.print("Change radio ");
    //    Serial.print(currentRadio);
    //    Serial.print("Frequency to ");
    //    Serial.println(frequency);
  }
}

void changeRadioIndex(String dir) {
  if (dir == "ADD") {
    currentRadio += 1;
    if (currentRadio >= radiosLength) {
      currentRadio = 0;
    }
    Serial.print("Adding radio index to ");
    Serial.println(currentRadio);

  } else {
    currentRadio -= 1;
    if (currentRadio < 0) {
      currentRadio = radiosLength - 1;
    }
    Serial.print("Subtracting radio index to ");
    Serial.println(currentRadio);
  }
}

void serialDisplayInfo() {
  Serial.print("Frequency: ");
  Serial.print(myRadios[currentRadio].getFrequency());
  Serial.print(" MHz :: ");

  Serial.print("Is Muted: ");
  Serial.print(myRadios[currentRadio].isMuted());
  Serial.print(" :: ");

  Serial.print("Is Stereo: ");
  Serial.print(myRadios[currentRadio].isStereo());
  Serial.print(" :: ");

  Serial.print("Signal Level: ");
  Serial.print(myRadios[currentRadio].getSignalLevel());
  Serial.println(" ");

  Serial.print("Direction: ");
  Serial.print(currentDir);
  Serial.print(" | Counter: ");
  Serial.println(cursorLoc);
}

void displayMenu() {
  if (updateScreen) {
    lcd.clear();
    lcd.print("Radio ");
    lcd.print("0");
    lcd.print(currentRadio + 1);

    lcd.setCursor(0, 1);
    lcd.print("Freq ");
    lcd.print(myRadios[currentRadio].getFrequency());

    lcd.setCursor(12, 1);
    lcd.print(myRadios[currentRadio].isMuted());
    lcd.print(myRadios[currentRadio].isStereo());
    lcd.print(myRadios[currentRadio].getSignalLevel());

    setCursorPos(cursorLoc);
    updateScreen = false;
  }
}

void initScreen() {

  // double init to clear any previous text
  lcd.init();
  lcd.init();

  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Globetrack INT");
  lcd.setCursor(4, 1);
  lcd.print("RADIOS");

}

void setCursorPos(int index) {
  if (index == 0) {
    lcd.setCursor(0, 0);
    lcd.blink();
    cursorTriggerd = true;
    cursorTriggeredTime = millis();
  }
  else if (index == 1) {
    lcd.setCursor(0, 1);
    lcd.blink();
    cursorTriggerd = true;
    cursorTriggeredTime = millis();
  }
  else if (index == 2) {
    lcd.setCursor(3, 1);
    lcd.blink();
    cursorTriggerd = true;
    cursorTriggeredTime = millis();
  }
  else if (index == 3) {
    lcd.setCursor(12, 1);
    lcd.blink();
    cursorTriggerd = true;
    cursorTriggeredTime = millis();
  }

}

void checkCursor() {
  if (cursorTriggerd) {
    // Wait for 5 seconds from being triggered then turn off
    if (millis() >= cursorTriggeredTime + 5000) {
      lcd.noBlink();
      cursorTriggerd = false;
      cursorLoc = -1;
      lcd.setCursor(0, 0);
    }
  }
}
