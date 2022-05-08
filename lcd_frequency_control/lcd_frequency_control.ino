
#include <SoftwareWire.h>
#include <LiquidCrystal_I2C.h>

// Rotary Encoder Inputs
#define CLK 10
#define DT 11
#define SW 12

float counter = 100.30;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
unsigned long lastButtonPress = 0;

boolean cursorTriggerd = false;
unsigned long cursorTriggeredTime = 0;
int cursorLoc = 0;
bool updateScreen = true;


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
      _muted = !_muted;
    }


    //  Get Functions
    char isMuted() {
      getValues();
      
      if (_muted){return 'L';}
      else {return 'M';}
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
      if (_stereo){return 'S';}
      else {return 'M';}
    }


};


// sdaPin, sclPin
Radio radio1(2, 3);

LiquidCrystal_I2C lcd(0x3F, 16, 2);


void setup() {

  Serial.begin(115200);

  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Read the initial state of Rotary encoder CLK
  lastStateCLK = digitalRead(CLK);

  radio1.begin();
  radio1.setFrequency(100.3);

  initScreen();
  delay(2000);
}

void loop() {

  
  readRotation();
  readButn();
  

  displayMenu();
  checkCursor();

}

void readButn(){

  // Read the button state and debounce
  int btnState = digitalRead(SW);
  if (btnState == LOW) {
    if (millis() - lastButtonPress > 50) {
      Serial.println("Button pressed!");
      setCursorPos(cursorLoc);
      
      if (cursorLoc == 2){
        cursorLoc = 0;
      }else{
        cursorLoc +=1;
      }
    }
    // Remember last button press event
    lastButtonPress = millis();
  }

  // Put in a slight delay to help debounce the reading
  delay(1);
}

void readRotation(){

  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
    
    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      counter = counter - 0.05;
      radio1.setFrequency(counter);
      currentDir = "CCW";
      
      serialDisplayInfo();
      updateScreen = true;
      
    } else {
      // Encoder is rotating CW so increment
      counter = counter + 0.05;
      radio1.setFrequency(counter);
      currentDir = "CW";
      
      serialDisplayInfo();
      updateScreen = true;
      
    }

    Serial.print("Direction: ");
    Serial.print(currentDir);
    Serial.print(" | Counter: ");
    Serial.println(counter);
  }
  // Remember last CLK state
  lastStateCLK = currentStateCLK;
  
}

void serialDisplayInfo() {
  Serial.print("Frequency: ");
  Serial.print(radio1.getFrequency());
  Serial.print(" MHz :: ");

  Serial.print("Is Muted: ");
  Serial.print(radio1.isMuted());
  Serial.print(" :: ");

  Serial.print("Is Stereo: ");
  Serial.print(radio1.isStereo());
  Serial.print(" :: ");

  Serial.print("Signal Level: ");
  Serial.print(radio1.getSignalLevel());
  Serial.println(" ");

}

void displayMenu() {
  if(updateScreen) {
    lcd.clear();
    lcd.print("Radio ");
    lcd.print("01");
    
    lcd.setCursor(0,1);
    lcd.print("Freq ");
    lcd.print(radio1.getFrequency());

    lcd.setCursor(12,1);
    lcd.print(radio1.isMuted());
    lcd.print(radio1.isStereo());
    lcd.print(radio1.getSignalLevel());
    
    updateScreen = false;
  }
}

void initScreen() {
  
  lcd.init();
  lcd.init(); // double init clears any previous text
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("Globetrack INT");
  lcd.setCursor(4,1);
  lcd.print("RADIOS");

}

void setCursorPos(int index){
  if(index == 0){
    lcd.setCursor(0, 0);
    lcd.blink();
    cursorTriggerd = true;
    cursorTriggeredTime = millis();
  }
  else if(index == 1){
    lcd.setCursor(0, 1);
    lcd.blink();
    cursorTriggerd = true;
    cursorTriggeredTime = millis();
  }
  else if(index == 2){
    lcd.setCursor(12, 1);
    lcd.blink();
    cursorTriggerd = true;
    cursorTriggeredTime = millis();
  }

}

void checkCursor(){
  if(cursorTriggerd){
    // Wait for 5 seconds from being triggered then turn off
    if(millis() >= cursorTriggeredTime + 5000){
      lcd.noBlink();
      cursorTriggerd = false;
      cursorLoc = 0;
    }
  }
}
