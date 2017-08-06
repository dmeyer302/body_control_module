 /*  
  *  Body Control Module
  *  Copyright (c) 2017 Daniel Meyer
  *  dmeyer302@gmail.com
  *  
  *  Project started July 2017
  *  
  *  Built for Teensy 3.5
  *  https://www.pjrc.com/store/teensy35.html
  *  
  *  Disclaimer: I can't think of one other than I'm not responsible for your wreck due to using this software.
  *  You'd have to try really hard to wreck because of this software anyway.
  *  
  *  Ok, you insisted, the guys at MIT get paid way too much to think this stuff up anyway so I'll just use it:
  *  
  *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  *  
  *  Enough of that, I'm getting back to coding.
  *  
  */

#include <Arduino.h>
#include <FreqMeasure.h>          // https://github.com/PaulStoffregen/FreqMeasure
#include <EEPROM.h>               // https://github.com/PaulStoffregen/EEPROM
#include <TimeLib.h>              // https://www.pjrc.com/teensy/td_libs_Time.html#teensy3
#include <ResponsiveAnalogRead.h> // https://github.com/dxinteractive/ResponsiveAnalogRead
#include <Adafruit_MCP23017.h>    // https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library
//#include <Math.h>
#include <Wire.h>
#include <U8g2lib.h>


U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA); // OLED Constructor, see u8g2 library or examples to find your specific display
Adafruit_MCP23017 lightsInMCP;
Adafruit_MCP23017 lightsOutMCP;


/*  ================================================
 *  EEPROM
 *  ================================================*/

  const byte instrumentEEPROM = 10;
  const byte hoursEEPROM = 75;
  const byte minutesEEPROM = 80;
  const byte engineDecMinutesEEPROM = 105;
  const byte odometerThousandsEEPROM = 200;
  const byte odometerHundredsEEPROM = 201;
  const byte odometerOnesEEPROM = 202;
  const byte odometerTenthsEEPROM = 203;
  const byte fuelAlertEEPROM = 300;


/*  ================================================
 *  Program Variables
 *  ================================================*/
 

    // General
      bool statusBool = 0;
      unsigned long statusOn = 0;
      unsigned long statusOff = 0;

      long errorTime = -50000;
      byte numberOfErrors = 0;
      unsigned long errorLockoutTime = 30000;

    unsigned long tempTime = 0;
    int temp = 0;
  
    // Turn signals
      unsigned long flashTime = 0;
      unsigned long flashOnTime = 0;
      unsigned long flashOffTime = 0;
      unsigned long timePressed = 0;
      bool flasherOn = false;
      byte flashCount = 3;
      byte blinkMode = 1;
      const int blinkerHold = 350; // millis before blinker will go into constant on mode, ideally slightly less than blinkerTime
      const int blinkerTime = 400; // millis on and off; 1/2 standard cycle time
  
    // Windshield Wipers
      unsigned long washTime = 0;
      unsigned long lastWipe = 0;
  
    // Speedometer
      float speed = 0;
      volatile double sum = 0;    // freqmeasure
      volatile int count = 0;     // freqmeasure
      const byte resolution = 19; // pulses per revolution: 17 for PB, else for test
      volatile unsigned int milesCount = 0;

    // Lights
      bool fogStatus = 0;
      bool domeStatus = 0;
      unsigned int domeBrightness = 0;
      unsigned long domeTime = 0;
      unsigned long instTime = 0;
      unsigned int instrumentBrightness = 0;
      unsigned long doorCloseTime = 0;

    // Odometer, Engine Hours
      const unsigned int pulsesPerTenth = 500; // VSS Pulses required in 1/10 mile of driving
      unsigned long incrementalMillis = 0;
      unsigned int minutes = 0;
      unsigned int hours = EEPROM.read(hoursEEPROM);
      float engineHrs = EEPROM.read(hoursEEPROM) + EEPROM.read(engineDecMinutesEEPROM);
      unsigned long odometer = 0;
      unsigned long odometerThousands = 0;
      int odometerOnes = 0;
    

    // Display
      byte displaySelect = 0;
      unsigned long displaySelectTime = 0;

    // Securicode
      bool keypadLightStatus = 0;
      unsigned long keypadLightTime = 0;
      byte enteredDigits = 0;
      byte codePosition = 0;
      byte enteredCode[5] = {0,0,0,0,0};
      const byte code1[5] = {1,2,3,4,5};
      const byte code2[5] = {5,4,3,2,1};
      const byte code3[5] = {1,2,3,4,3};


/*  ================================================
 *  Input Pins
 *  ================================================*/

  const byte washPin = 7;
  const byte wipePin = 8;
  const byte intermittentPin = 9;

  const byte fuelIn = 123;
  const byte tempIn = A0;
  ResponsiveAnalogRead fuel(fuelIn,false);
  ResponsiveAnalogRead responsiveTemp(tempIn,false);

  //const byte MCPinterrupt = 

  //lightsInMCP
  const byte headlightIn =  1;
  const byte parkIn =       2;
  const byte fogIn =        3;
  const byte autoLampIn =   4;
  const byte domeIn =       5;
  const byte instBrighter = 6;
  const byte instDimmer =   7;
  const byte hazard =       9;
  const byte leftTurn =     10;
  const byte rightTurn =    11;
  const byte brakePin =     12;

  const byte runIn = 40;
  const byte startIn = 44;
  
  const byte leftDoorIn = 41;
  const byte rightDoorIn = 42;

  // SecuriCode
  const byte key1 = 28;
  const byte key2 = 29;
  const byte key3 = 30;
  const byte key4 = 31;
  const byte key5 = 32;


/*  ================================================
 *  Output Pins
 *  ================================================*/

  const byte statusLED = 13; // Onboard Teensy

  const byte wipeOut = 41;
  const byte washOut = 42;

  // lightsOutMCP
    // Turn indicators
    const byte lfOut = 1;
    const byte rfOut = 2;
    const byte lrOut = 3;
    const byte rrOut = 4;
  
    const byte headlightOut = 9;
    const byte parkOut =      10;
    const byte fogOut =       11;
    const byte domeOut =      12;
    const byte instOut =      13;
    const byte keypadLight =  33;

  // Idiot lights
  /*
   * Low fuel
   * Door ajar
   * E-brake
   * Cruise
   */

  // Gauges
  const byte speedoOut = 10;
  const byte fuelOut = 124;
  /*
   * Oil pressure
   * Temp
   * Tach
   * Fuel
   */


/*  ================================================
 *  Function Declarations
 *  ================================================*/
 
  void flash();
  void brake();
  void wipe();
  void speedometer();
  void domeInterrupt();
  void instBrightInterrupt();
  void instDimInterrupt();
  void fogInterrupt();
  void status();
  void getHours();
  void readFuel();
  void doors();
  void securicode();


/*  ================================================
 *  Setup
 *  ================================================*/

void setup() {
  Serial.begin(9600);
  lightsInMCP.begin();
  u8g2.begin();
  u8g2.setContrast(EEPROM.read(instrumentEEPROM));
  u8g2.clearBuffer();
  u8g2.drawBox(0,0,128,32); // Test display on startup
  //u8g2.drawBox(0,0,20,30);
  u8g2.sendBuffer();
  delay(10);

  setSyncProvider(getTeensy3Time); // set the Time library to use Teensy 3.0's RTC to keep time

  // Uncomment to set parameters; run sketch only once and re-upload
  //EEPROM.write(hoursEEPROM,0);
  //EEPROM.write(minutesEEPROM,0);
  //EEPROM.write(odometerTenthsEEPROM, 0);
  //EEPROM.write(odometerOnesEEPROM, 0);
  //EEPROM.write(odometerHundredsEEPROM, 0);
  //EEPROM.write(odometerThousandsEEPROM, 0);
  //EEPROM.write(fuelAlertEEPROM,128);

  FreqMeasure.begin(); // Must be pin 3

  attachInterrupt(digitalPinToInterrupt(key1), sc1int, RISING);
  attachInterrupt(digitalPinToInterrupt(key2), sc2int, RISING);
  attachInterrupt(digitalPinToInterrupt(key3), sc3int, RISING);
  attachInterrupt(digitalPinToInterrupt(key4), sc4int, RISING);
  attachInterrupt(digitalPinToInterrupt(key5), sc5int, RISING);
  
  attachInterrupt(digitalPinToInterrupt(leftTurn), leftInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(rightTurn), rightInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(hazard), hazardInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(brakePin), brakeInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(washPin), washInterrupt, RISING);

  pinMode(intermittentPin, INPUT);

  // lightsInMCP
  lightsInMCP.pinMode(headlightIn, INPUT);
  lightsInMCP.pinMode(autoLampIn, INPUT);
  attachInterrupt(digitalPinToInterrupt(parkIn),        parkInterrupt,        RISING);
  attachInterrupt(digitalPinToInterrupt(fogIn),         fogInterrupt,         RISING);
  attachInterrupt(digitalPinToInterrupt(domeIn),        domeInterrupt,        RISING);
  attachInterrupt(digitalPinToInterrupt(instBrighter),  instBrightInterrupt,  RISING);
  attachInterrupt(digitalPinToInterrupt(instDimmer),    instDimInterrupt,     RISING);

  attachInterrupt(digitalPinToInterrupt(startIn),       startInterrupt,       RISING);
  attachInterrupt(digitalPinToInterrupt(runIn),         runInterrupt,         RISING);

  pinMode(statusLED, OUTPUT);
  pinMode(speedoOut, OUTPUT);
  
  pinMode(lfOut, OUTPUT);
  pinMode(rfOut, OUTPUT);
  pinMode(lrOut, OUTPUT);
  pinMode(rrOut, OUTPUT);

  pinMode(keypadLight, OUTPUT);

  if(digitalRead(parkIn) == HIGH){
    parkInterrupt();
  }
  //analogWrite(instOut,EEPROM.read(instrumentEEPROM));
// Gauge sweep here

}

/*  ================================================
 *  Main
 *  ================================================*/

void loop() {

  flash();
  brake();
  wipe();
  speedometer();
  status();
  lights();
  
  getHours(); // Replace with if(key on){keyOn()}
  getMiles();
  buildDisplay();
  exteriorTemp();
  securicode();
  //fuel();



  // Uncomment to set Teensy clock to PC clock (unconfirmed)
  /*if (Serial.available()) {
    time_t t = processSyncMessage();
    if (t != 0) {
      Teensy3Clock.set(t); // set the RTC
      setTime(t);
    }
  }*/
  //digitalClockDisplay();  
  
}


/*  ================================================
 *  Functions
 *  ================================================*/


void exteriorTemp(){
  if(millis() - tempTime > 1000){
    tempTime = millis();

    responsiveTemp.update();
    //Serial.println(analogRead(tempIn));
    temp = map(responsiveTemp.getValue(),700,750,70,90); // Calibrate temp readout here
    //Serial.println(temp);
  }
  
}

void keyOn(){
  getHours();
}

void getHours(){
    if(millis() - incrementalMillis > 60000){
      incrementalMillis = millis();
      minutes = EEPROM.read(minutesEEPROM);
      /*for(int i = 80; i < 99; i++){
        if(EEPROM.read(minutesEEPROM) < EEPROM.read(minutesEEPROM + 1)){
          minutes = minutesEEPROM + 1;
        }*/
        
      minutes++;
      
      if(minutes >= 60){
        hours = EEPROM.read(hoursEEPROM);
        hours++;
        EEPROM.write(hoursEEPROM,hours);
        minutes = 0;
        
      }
      
      EEPROM.write(minutesEEPROM,minutes);
      
      //Serial.println(minutes);
  
      float engineDecMinutes = round(minutes * 0.166666666)/10.0;
      //Serial.println(engineDecMinutes);
      engineHrs = EEPROM.read(hoursEEPROM);
      engineHrs += engineDecMinutes;
      if(engineDecMinutes != EEPROM.read(engineDecMinutesEEPROM)){
        EEPROM.write(engineDecMinutesEEPROM,engineDecMinutes);
      }
      //engineHrs = round( engineHrs * 10 ) / 10;
      //Serial.println(engineHrs);
    }
}

// Built-In LED indicates program has not frozen
void status(){
  if(statusBool && millis() - statusOn > 200){
    statusOff = millis();
    statusBool = 0;
  }
  else if(!statusBool && millis() - statusOff > 2000){
    statusOn = millis();
    statusBool = 1;
  }
  digitalWrite(statusLED, statusBool);
}


// TODO: Optimize EEPROM write cycles to reduce significance of EEPROM failure
void getMiles(){
  if(milesCount > pulsesPerTenth){
    
    EEPROM.write(odometerTenthsEEPROM, EEPROM.read(odometerOnesEEPROM) + 1);
    

    if(EEPROM.read(odometerTenthsEEPROM) != milesCount){
      EEPROM.write(odometerTenthsEEPROM,milesCount);
      }
    if(EEPROM.read(odometerTenthsEEPROM) == 10){
      EEPROM.write(odometerOnesEEPROM, EEPROM.read(odometerOnesEEPROM) + 1);
      EEPROM.write(odometerTenthsEEPROM, 0);
      }
    if(EEPROM.read(odometerOnesEEPROM) == 100){
      EEPROM.write(odometerHundredsEEPROM, EEPROM.read(odometerHundredsEEPROM) + 1);
      EEPROM.write(odometerOnesEEPROM, 0);
      }
    if(EEPROM.read(odometerHundredsEEPROM) == 10){
      EEPROM.write(odometerThousandsEEPROM, EEPROM.read(odometerThousandsEEPROM) + 1);
      EEPROM.write(odometerHundredsEEPROM, 0);
      }

    milesCount = 0;
  }
}

/*  ================================================
 *  Interrupt Functions
 *  ================================================*/

void runInterrupt(){
  incrementalMillis = millis();
}

void startInterrupt(){
  
}


//time_t variable = now();

// Set system time to RTC time
time_t getTeensy3Time(){ return Teensy3Clock.get();}
