 /*  
  *  Body Control Module
  *  Copyright (c) 2017 Daniel Meyer
  *  dmeyer302@gmail.com
  *  
  *  Project started August 2017
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
#include <FreqMeasure.h>  // https://github.com/PaulStoffregen/FreqMeasure
#include <EEPROM.h>       // https://github.com/PaulStoffregen/EEPROM
#include <Math.h>
#include <Wire.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA); // OLED Constructor, see u8g2 library or examples to find your specific display

// EEPROM

  const byte instrumentEEPROM = 10;
  const byte hoursEEPROM = 75;
  byte minutesEEPROM = 80;
  const byte engineDecMinutesEEPROM = 105;

// Constants

  const int blinkerHold = 350; // millis before blinker will go into constant on mode, ideally slightly less than blinkerTime
  const int blinkerTime = 400; // millis on and off; 1/2 standard cycle time


// Program Variables

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
    
      // Windshield Wipers
      unsigned long washTime = 0;
      unsigned long lastWipe = 0;
    
      // Speedometer
      float speed = 0;
      volatile double sum = 0;    // freqmeasure
      volatile int count = 0;     // freqmeasure
      const byte resolution = 19; // pulses per revolution: 17 for PB, else for test

      // Lights
      bool fogStatus = 0;
      bool domeStatus = 0;
      unsigned int domeBrightness = 0;
      unsigned long domeTime = 0;
      unsigned long instTime = 0;
      unsigned int instrumentBrightness = 0;

      // Odometer, Engine Hours
      unsigned long incrementalMillis = 0;
      unsigned int minutes = 0;
      unsigned int hours = EEPROM.read(hoursEEPROM);
      float engineHrs = 0;

      // Display
      byte displaySelect = 7;
      unsigned long displaySelectTime = 0;




// Input Pins

  const byte hazard = 3;
  const byte leftTurn = 4;
  const byte rightTurn = 5;
  const byte brakePin = 6;

  const byte washPin = 7;
  const byte wipePin = 8;
  const byte intermittentPin = 9;

  const byte headlightIn = 24;
  const byte parkIn = 25;
  const byte fogIn = 26;
  const byte autoLampIn = 27;
  const byte domeIn = 28;
  const byte instBrighter = 29;
  const byte instDimmer = 30;

  const byte runIn = 40;
  //const byte 


// Output Pins

  const byte statusLED = 13; // Onboard Teensy

  const byte speedOut = 20;

  const byte lfOut = 14;
  const byte rfOut = 15;
  const byte lrOut = 16;
  const byte rrOut = 17;
  
  const byte wipeOut = 41;
  const byte washOut = 42;

  const byte headlightOut = 31;
  const byte parkOut = 32;
  const byte fogOut = 33;
  const byte domeOut = 35;
  const byte instOut = 36;

  

// Function Declarations

  void flash();
  void brake();
  void wipe();
  void speedometer();
  void domeInterrupt();
  void instBright();
  void instDim();
  void fogInterrupt();
  void status();
  void getHours();


void setup() {
  Serial.begin(9600);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.drawBox(0,0,128,32);
  //u8g2.drawBox(0,0,20,30);
  u8g2.sendBuffer();
  delay(100);

  // Uncomment only to reset engine hrs to 0
  //EEPROM.write(hoursEEPROM,0);
  //EEPROM.write(minutesEEPROM,0);

  FreqMeasure.begin(); // Must be pin 3
  
  attachInterrupt(digitalPinToInterrupt(leftTurn), leftInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(rightTurn), rightInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(hazard), hazardInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(brakePin), brakeInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(washPin), washInterrupt, RISING);

  pinMode(intermittentPin, INPUT);

  pinMode(headlightIn, INPUT);
  pinMode(autoLampIn, INPUT);
  attachInterrupt(digitalPinToInterrupt(parkIn), parkInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(fogIn), fogInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(domeIn), domeInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(instBrighter), instBright, RISING);
  attachInterrupt(digitalPinToInterrupt(instDimmer), instDim, RISING);

  pinMode(statusLED, OUTPUT);
  
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
  
  getHours();
  buildDisplay();
  
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
            }
            analogWrite(instOut,instrumentBrightness);
            //Serial.println(instrumentBrightness);
          }
      }
      else {
        if(instrumentBrightness > 0){
              instrumentBrightness--;
              delay(5);
              analogWrite(instOut,instrumentBrightness);
        }
        digitalWrite(parkOut,LOW);
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
    Serial.println(engineDecMinutes);
    engineHrs = EEPROM.read(hoursEEPROM);
    engineHrs += engineDecMinutes;
    if(engineDecMinutes != EEPROM.read(engineDecMinutesEEPROM)){
      EEPROM.write(engineDecMinutesEEPROM,engineDecMinutes);
    }
    //engineHrs = round( engineHrs * 10 ) / 10;
    Serial.println(engineHrs);
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
    sum = sum + FreqMeasure.read();
    count = count + 1;
    if (count > 17) {
      float frequency = FreqMeasure.countToFrequency(sum / count);
      Serial.println(frequency);
      sum = 0;
      count = 0;
      speed = ((float)frequency * 180.0)/resolution;
      Serial.print("Speed is ");
      Serial.println(speed);
      if(speed<10){
        speed = 0;
      }
    }
  }

  tone(speedOut,speed);

  
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


void instBright(){
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


void instDim(){
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

      /*  Display is not currently connected to any variables.
       *  Present code is to get display laid out.
       */
      
      u8g2.clearBuffer();

      if(millis() - displaySelectTime > 2000){
        displaySelect++;
        if(displaySelect == 13){
          displaySelect = 7;
         }
         displaySelectTime = millis();
      }
    
    // Small text
      
      //u8g2.setFont(u8g2_font_helvB08_tr);
    
      if(displaySelect == 1){ // Park
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(0,32,"P");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(22,32,"RND21");
        }
    
      else if(displaySelect == 2){ // R
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(18,32,"R");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(0,32,"P");
        u8g2.drawStr(38,32,"ND21");
        }
    
      else if(displaySelect == 3){ // N
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(32,32,"N");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(0,32,"PR");
        u8g2.drawStr(54,32,"D21");
        }

      else if(displaySelect == 4){ // D
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(48,32,"D");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(0,32,"PRN");
        u8g2.drawStr(70,32,"21");
        }

      else if(displaySelect == 5){ // 2
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(63,32,"2");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(0,32,"PRND");
        u8g2.drawStr(84,32,"1");
        }

      else if(displaySelect == 6){ // 1
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(74,32,"1");
  
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(0,32,"PRND2");
        }

      else if(displaySelect == 7){ // Clock and Temp
        u8g2.setFont(u8g2_font_logisoso24_tf);
        u8g2.drawStr(85,32,"72");
        u8g2.drawStr(0,32,"10:42");
        u8g2.drawCircle(124,11,3,U8G2_DRAW_ALL); // X,Y,radius
        }

      else if(displaySelect == 8){ // Odometer
        u8g2.setFont(u8g2_font_logisoso24_tf);
        u8g2.drawStr(50,28,"10531");
        
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
        u8g2.setFont(u8g2_font_logisoso24_tf);
        u8g2.drawStr(50,28,"175");

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
        
      
      u8g2.sendBuffer();  
  
}

