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
  *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  *  
  *  Enough of that, I'm getting back to coding.
  *  
  */

#include <Arduino.h>
#include <FreqMeasure.h>        // https://github.com/PaulStoffregen/FreqMeasure
#include <EEPROM.h>             // https://github.com/PaulStoffregen/EEPROM
#include <TimeLib.h>            // https://www.pjrc.com/teensy/td_libs_Time.html#teensy3
#include <Adafruit_MCP23017.h>  // https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library
//#include <Math.h>
#include <Wire.h>
#include <U8g2lib.h>


U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA); // OLED Constructor, see u8g2 library or examples to find your specific display
Adafruit_MCP23017 lightsMCP;


// EEPROM

  const byte instrumentEEPROM = 10;
  const byte hoursEEPROM = 75;
  byte minutesEEPROM = 80;
  const byte engineDecMinutesEEPROM = 105;
  const byte odometerThousandsEEPROM = 200;
  const byte odometerHundredsEEPROM = 201;
  const byte odometerOnesEEPROM = 202;
  const byte odometerTenthsEEPROM = 203;
  const byte fuelAlertEEPROM = 300;


// Program Variables

    // General
      bool statusBool = 0;
      unsigned long statusOn = 0;
      unsigned long statusOff = 0;
  
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




// Input Pins

  const byte hazard = 3;
  const byte leftTurn = 4;
  const byte rightTurn = 5;
  const byte brakePin = 6;

  const byte washPin = 7;
  const byte wipePin = 8;
  const byte intermittentPin = 9;

  //lightsMCP
  const byte headlightIn =  1;
  const byte parkIn =       2;
  const byte fogIn =        3;
  const byte autoLampIn =   4;
  const byte domeIn =       5;
  const byte instBrighter = 6;
  const byte instDimmer =   7;

  const byte runIn = 40;
  const byte startIn = 44;
  
  const byte leftDoorIn = 41;
  const byte rightDoorIn = 42;
  const byte fuelIn = 43;


// Output Pins

  const byte statusLED = 13; // Onboard Teensy

  const byte wipeOut = 41;
  const byte washOut = 42;

  // Turn indicators
  const byte lfOut = 14;
  const byte rfOut = 15;
  const byte lrOut = 16;
  const byte rrOut = 17;

  const byte headlightOut = 31;
  const byte parkOut = 32;
  const byte fogOut = 33;
  const byte domeOut = 35;
  const byte instOut = 36;

  // Idiot lights
  /*
   * Low fuel
   * Door ajar
   * E-brake
   * Cruise
   */

  // Gauges
  const byte speedoOut = 10;
  /*
   * Oil pressure
   * Temp
   * Tach
   * Fuel
   */


// Function Declarations

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
  void fuel();
  void doors();


void setup() {
  Serial.begin(9600);
  lightsMCP.begin();
  u8g2.begin();
  u8g2.setContrast(EEPROM.read(instrumentEEPROM));
  u8g2.clearBuffer();
  u8g2.drawBox(0,0,128,32); // Test display on startup
  //u8g2.drawBox(0,0,20,30);
  u8g2.sendBuffer();
  delay(10);

  // Built In RTC
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);
  /*if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }*/

  // Uncomment to set parameters; run sketch only once and re-upload
  //EEPROM.write(hoursEEPROM,0);
  //EEPROM.write(minutesEEPROM,0);
  //EEPROM.write(odometerTenthsEEPROM, 0);
  //EEPROM.write(odometerOnesEEPROM, 0);
  //EEPROM.write(odometerHundredsEEPROM, 0);
  //EEPROM.write(odometerThousandsEEPROM, 0);
  //EEPROM.write(fuelAlertEEPROM,128);

  FreqMeasure.begin(); // Must be pin 3
  
  attachInterrupt(digitalPinToInterrupt(leftTurn), leftInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(rightTurn), rightInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(hazard), hazardInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(brakePin), brakeInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(washPin), washInterrupt, RISING);

  pinMode(intermittentPin, INPUT);

  // lightsMCP
  mcp.pinMode(headlightIn, INPUT);
  mcp.pinMode(autoLampIn, INPUT);
  attachInterrupt(digitalPinToInterrupt(parkIn), parkInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(fogIn), fogInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(domeIn), domeInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(instBrighter), instBrightInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(instDimmer), instDimInterrupt, RISING);

  attachInterrupt(digitalPinToInterrupt(startIn), startInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(runIn), runInterrupt, RISING);

  pinMode(statusLED, OUTPUT);
  pinMode(speedoOut, OUTPUT);
  
  pinMode(lfOut, OUTPUT);
  pinMode(rfOut, OUTPUT);
  pinMode(lrOut, OUTPUT);
  pinMode(rrOut, OUTPUT);

  if(digitalRead(parkIn) == HIGH){
    parkInterrupt();
  }
  //analogWrite(instOut,EEPROM.read(instrumentEEPROM));
// Gauge sweep here

}


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



void fuel(){
  if(analogRead(fuelIn) < EEPROM.read(fuelAlertEEPROM)){
    displaySelect = 14;
    // play tone
    // idiot light on
  }
}

void doors(){
  if(digitalRead(leftDoorIn) == LOW || digitalRead(rightDoorIn) == LOW){
    domeStatus = 1;
    displaySelect = 13;
    // play tone
  }
}

void lights(){
        
          if((domeStatus) && (domeBrightness < 255) && (millis() - domeTime > 20)){
            domeBrightness++;
            analogWrite(domeOut, domeBrightness);
            domeTime = millis();
          }
        
          else if((!domeStatus) && (domeBrightness > 0) && (millis() - domeTime > 25)){
            domeBrightness--;
            analogWrite(domeOut, domeBrightness);
            domeTime = millis();
          }


      if(digitalRead(parkIn) == HIGH || digitalRead(headlightIn) == HIGH){

        digitalWrite(parkOut,HIGH);

          // Instrument cluster
          if(digitalRead(instBrighter) && (millis() - instTime > 400)){
            if(instrumentBrightness < 255){
              instrumentBrightness++;
              EEPROM.write(instrumentEEPROM,instrumentBrightness);
              delay(5);
            }
            else if(instrumentBrightness >= 255){
              instrumentBrightness = 255;
            }
            analogWrite(instOut,instrumentBrightness);
            u8g2.setContrast(instrumentBrightness);
            //Serial.println(instrumentBrightness);
          }

          else if(digitalRead(instDimmer) && (millis() - instTime > 400)){
            if(instrumentBrightness > 0){
              instrumentBrightness--;
              EEPROM.write(instrumentEEPROM,instrumentBrightness);
              delay(5);
            }
            else if(instrumentBrightness <= 0){
              instrumentBrightness = 0;
              u8g2.setContrast(instrumentBrightness);
            }
            analogWrite(instOut,instrumentBrightness);
            u8g2.setContrast(instrumentBrightness);
            //Serial.println(instrumentBrightness);
          }
      }
      else {
        if(instrumentBrightness > 0){
              instrumentBrightness--;
              delay(5);
              analogWrite(instOut,instrumentBrightness);
              u8g2.setContrast(instrumentBrightness);
        }
        digitalWrite(parkOut,LOW);
        }
        
}

void startInterrupt(){
  
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


void speedometer(){

  if (FreqMeasure.available()) {
    // average several reading together
    unsigned int correctedFreq = 0;
    sum = sum + FreqMeasure.read();
    count = count + 1;
    milesCount++;
    //Serial.print("milesCount ");
    //Serial.println(milesCount);
    //Serial.print("Sum is ");
    //Serial.println(sum);
    //Serial.print("Count is ");
    //Serial.println(count);
    if (count > 8) {
      float frequency = FreqMeasure.countToFrequency(sum / count);
      Serial.print("Freq is ");
      Serial.println(frequency);
      sum = 0;
      count = 0;
      correctedFreq = ((float)frequency * 100)/100; // Adjust this ratio for speed correction
      if(correctedFreq < 20){                       // Adjust this number to be the max frequency at which the speedometer rests on the needle post
        noTone(speedoOut);
      }
      Serial.print("corrFreq is ");
      Serial.println(correctedFreq);
      tone(speedoOut,correctedFreq);
    }
    
  }

  

  
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

void brake(){

    if(digitalRead(brakePin) == HIGH && flashCount < 3){
      if(blinkMode == 1){
        digitalWrite(rrOut, HIGH);
      }
  
      else if(blinkMode == 2){
        digitalWrite(lrOut, HIGH);
      }
    }
  
    else if(digitalRead(brakePin) == HIGH){
  
      digitalWrite(lrOut, HIGH);
      digitalWrite(rrOut, HIGH);
      
    }  
}




void wipe(){
    if(digitalRead(wipePin) == HIGH){
      
      unsigned int intermittent = analogRead(intermittentPin);
      unsigned int intermittentDelay = map(intermittent, 100, 900, 2000, 30000);
      
      if(intermittent > 900){
        digitalWrite(wipeOut, HIGH);
        }
  
      else if(intermittent < 900 && intermittent > 100 && (millis() - lastWipe > intermittentDelay)){
        digitalWrite(wipeOut, HIGH);
        }
      
      else{
        digitalWrite(wipeOut, LOW);
        }
      
      }
  
  
    if(digitalRead(washPin) == HIGH){
      digitalWrite(wipeOut, HIGH);
      digitalWrite(washOut, HIGH);
    }
  
    // if...else runs wipers when washing. Courtesy wipe after x seconds.
    if(millis() - washTime < 4000){
      digitalWrite(wipeOut, HIGH);
    }
  
    // courtesy wipe
    else if(millis() - washTime > 8000 && millis() - washTime < 9000){
      digitalWrite(wipeOut, HIGH);
    }
  
    else{
      digitalWrite(wipeOut, LOW);
      digitalWrite(washOut, LOW);
    }

}


void washInterrupt(){
  washTime = millis();
}


void flash(){

    if(digitalRead(leftTurn) == HIGH || digitalRead(rightTurn) == HIGH || digitalRead(hazard) == HIGH){
      if(millis() - timePressed > blinkerHold){
        flashCount = 2;
        //Serial.println(flashCount);
      }
    }
    
    if(flasherOn){
      if(millis() - flashOnTime > blinkerTime){
        if(digitalRead(brakePin) != HIGH){
          digitalWrite(lrOut, LOW);
          digitalWrite(rrOut, LOW);
        }
        flasherOn = false;
        flashCount++;
        //Serial.println(flashCount);
      }
      flashOffTime = millis();
      
    }
    
    if(!flasherOn && flashCount <= 2 && (millis() - flashOffTime > blinkerTime)){
      flasherOn = true;
      flashOnTime = millis();
    }
    
    if(blinkMode == 3){
        digitalWrite(lfOut,flasherOn); 
        digitalWrite(rfOut,flasherOn);
        digitalWrite(lrOut,flasherOn);
        digitalWrite(rrOut,flasherOn);
                 }
    else if(blinkMode == 1){
        digitalWrite(lfOut,flasherOn);
        digitalWrite(lrOut,flasherOn);
        }
    else if(blinkMode == 2){
        digitalWrite(rfOut,flasherOn);
        digitalWrite(rrOut,flasherOn);
        }  
}

void brakeInterrupt(){
  //if(flashCount > 2){
    digitalWrite(lrOut, LOW);
    digitalWrite(rrOut, LOW);
  //}
}

void leftInterrupt(){
  blinkMode = 1;
  if(flashCount != 2){
    flashCount = 0;
  }
  //Serial.println(flashCount);
  timePressed = millis();
  digitalWrite(rfOut,LOW);
}

void rightInterrupt(){
  blinkMode = 2;
  if(flashCount != 2){
    flashCount = 0;
  }
  //Serial.println(flashCount);
  timePressed = millis();
  digitalWrite(lfOut,LOW);
}

void hazardInterrupt(){
  blinkMode = 3;
  if(flashCount != 2){
    flashCount = 0;
  }
  //Serial.println(flashCount);
  timePressed = millis();
}

void fogInterrupt(){
  fogStatus = !fogStatus;
}

void domeInterrupt(){
  domeStatus = !domeStatus;
}


void instBrightInterrupt(){
  if(digitalRead(parkIn) == HIGH){
  if((millis() - instTime > 100) && instrumentBrightness < 246){
    instrumentBrightness += 10;
    instTime = millis();
    EEPROM.write(instrumentEEPROM,instrumentBrightness);
    analogWrite(instOut,instrumentBrightness);
    //Serial.println(instrumentBrightness);
  }

  else if(instrumentBrightness >= 246){
    instrumentBrightness = 255;
    instTime = millis();
    EEPROM.write(instrumentEEPROM,instrumentBrightness);
    analogWrite(instOut,instrumentBrightness);
    //Serial.println(instrumentBrightness);
  }}
  
}


void instDimInterrupt(){
  if(digitalRead(parkIn) == HIGH){
  if((millis() - instTime > 100) && instrumentBrightness > 10){
    instrumentBrightness -= 10;
    instTime = millis();
    EEPROM.write(instrumentEEPROM,instrumentBrightness);
    analogWrite(instOut,instrumentBrightness);
    //Serial.println(instrumentBrightness);
  }

  else if(instrumentBrightness <= 10){
    instrumentBrightness = 0;
    instTime = millis();
    EEPROM.write(instrumentEEPROM,instrumentBrightness);
    analogWrite(instOut,instrumentBrightness);
    //Serial.println(instrumentBrightness);
  }}
  
}

void parkInterrupt(){
  for(int x = instrumentBrightness; x < EEPROM.read(instrumentEEPROM); x++){
  analogWrite(instOut,x);
  instrumentBrightness = x;
  delay(5);
  }
}

void runInterrupt(){
  incrementalMillis = millis();
}

void buildDisplay(){

      //int line1 = 0;
      //int line2 = 0;

      /*  Display is not currently connected to any variables.
       *  Present code is to get display laid out.
       */
      
      u8g2.clearBuffer();

      if(millis() - displaySelectTime > 2000){
        displaySelect++;
        if(displaySelect == 15){
          displaySelect = 1;
         }
         displaySelectTime = millis();
      }
    
    // Small text
      
      //u8g2.setFont(u8g2_font_helvB08_tr);
      byte displayOffset = 18;
      
      if(displaySelect == 1){ // Park
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(0+displayOffset,32,"P");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(22+displayOffset,32,"RND21");
        }
    
      else if(displaySelect == 2){ // R
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(18+displayOffset,32,"R");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(0+displayOffset,32,"P");
        u8g2.drawStr(38+displayOffset,32,"ND21");
        }
    
      else if(displaySelect == 3){ // N
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(32+displayOffset,32,"N");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(0+displayOffset,32,"PR");
        u8g2.drawStr(54+displayOffset,32,"D21");
        }

      else if(displaySelect == 4){ // D
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(48+displayOffset,32,"D");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(0+displayOffset,32,"PRN");
        u8g2.drawStr(70+displayOffset,32,"21");
        }

      else if(displaySelect == 5){ // 2
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(63+displayOffset,32,"2");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(0+displayOffset,32,"PRND");
        u8g2.drawStr(84+displayOffset,32,"1");
        }

      else if(displaySelect == 6){ // 1
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(74+displayOffset,32,"1");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(0+displayOffset,32,"PRND2");
        }

      else if(displaySelect == 7){ // Clock and Temp
        char buf1[8];
        u8g2.setFont(u8g2_font_logisoso24_tf);
        u8g2.drawStr(85,32,"72");
        
        sprintf(buf1,"%d:%d",hourFormat12(),minute());
        //Serial.println(buf1);
        u8g2.drawStr(0,32,buf1);
        u8g2.drawCircle(124,11,3,U8G2_DRAW_ALL); // X,Y,radius
        }

      else if(displaySelect == 8){ // Odometer
        char buf1[12];
        
        u8g2.setFont(u8g2_font_logisoso22_tf); // Note change to smaller font due to longer number
        if(EEPROM.read(odometerThousandsEEPROM) < 1){  // Do not print leading thousands zero
          sprintf(buf1,"%d%d",EEPROM.read(odometerHundredsEEPROM),EEPROM.read(odometerOnesEEPROM));
          }
        else{
          sprintf(buf1,"%d%d%d",EEPROM.read(odometerThousandsEEPROM),EEPROM.read(odometerHundredsEEPROM),EEPROM.read(odometerOnesEEPROM));
        }
        
        u8g2.drawStr(50,28,buf1);
        
        u8g2.setFont(u8g2_font_helvB10_tf);
        u8g2.drawStr(0,16,"ODO");
        u8g2.drawStr(0,30,"MI");
        }

      else if(displaySelect == 9){ // Trip Miles
        u8g2.setFont(u8g2_font_logisoso24_tf);
        u8g2.drawStr(50,28,"430.7");

        u8g2.setFont(u8g2_font_helvB10_tf);
        u8g2.drawStr(0,16,"TRIP");
        u8g2.drawStr(0,30,"MI");
        }

      else if(displaySelect == 10){ // Trip Hours
        u8g2.setFont(u8g2_font_logisoso24_tf);
        u8g2.drawStr(50,28,"9:56");

        u8g2.setFont(u8g2_font_helvB10_tf);
        u8g2.drawStr(0,16,"TRIP");
        u8g2.drawStr(0,30,"HRS");
        }

      else if(displaySelect == 11){ // Engine Hours
        char buf3[4];
        u8g2.setFont(u8g2_font_logisoso24_tf);
        float line2 = engineHrs;
        sprintf(buf3, "%.1f", engineHrs);
        u8g2.drawStr(60,28,buf3);

        u8g2.setFont(u8g2_font_helvB10_tf);
        u8g2.drawStr(0,16,"ENG");
        u8g2.drawStr(0,30,"HRS");
        }

      else if(displaySelect == 12){ // Oil Miles
        u8g2.setFont(u8g2_font_logisoso24_tf);
        u8g2.drawStr(50,28,"1278");

        u8g2.setFont(u8g2_font_helvB10_tf);
        u8g2.drawStr(0,16,"OIL");
        u8g2.drawStr(0,30,"MI");
        }

      else if(displaySelect == 13){
        u8g2.setFont(u8g2_font_logisoso16_tf);
        u8g2.drawStr(19,24,"DOOR AJAR");
        if(digitalRead(leftDoorIn) == LOW){
          u8g2.drawTriangle(0,16,8,22,8,10);
        }
        u8g2.drawTriangle(0,16,5,20,5,12);
        u8g2.drawTriangle(128,16,120,22,120,10);
      }

      else if(displaySelect == 14){
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(7,28,"LOW FUEL");
      }
        
      
      u8g2.sendBuffer();  
  
}

//time_t variable = now();

// Set system time to RTC time
time_t getTeensy3Time(){ return Teensy3Clock.get();}
