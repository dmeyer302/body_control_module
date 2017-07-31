 /*  
  *  Body Control Module by Daniel Meyer
  *  Project started August 2017
  *  
  *  Optimized for Teensy 3.5
  *  
  */

#include <FreqMeasure.h>
#include <Arduino.h>
#include <EEPROM.h>


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
      unsigned int instrumentBrightness = EEPROM.read(10);

// EEPROM

  const byte instrumentEEPROM = 10;


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


// Output Pins

  const byte statusLED = 13; // Onboard Teensy

  const byte speedOut = 20;

  const byte lfOut = 14;
  const byte rfOut = 15;
  const byte lrOut = 16;
  const byte rrOut = 17;
  
  const byte wipeOut = 18;
  const byte washOut = 19;

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
  void fog();


void setup() {
  Serial.begin(9600);

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
  attachInterrupt(digitalPinToInterrupt(fogIn), fog, RISING);
  attachInterrupt(digitalPinToInterrupt(domeIn), domeInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(instBrighter), instBright, RISING);
  attachInterrupt(digitalPinToInterrupt(instDimmer), instDim, RISING);

  pinMode(statusLED, OUTPUT);
  
  pinMode(lfOut, OUTPUT);
  pinMode(rfOut, OUTPUT);
  pinMode(lrOut, OUTPUT);
  pinMode(rrOut, OUTPUT);

  analogWrite(instOut,EEPROM.read(instrumentEEPROM));
// Gauge sweep here

}

void loop() {

flash();
brake();
wipe();
speedometer();
status();
lights();

}


void lights(){
        
          if((domeStatus) && (domeBrightness < 255) && (millis() - domeTime > 15)){
            domeBrightness++;
            analogWrite(domeOut, domeBrightness);
            domeTime = millis();
          }
        
          else if((!domeStatus) && (domeBrightness > 0) && (millis() - domeTime > 20)){
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
              delay(5);
            }
            else if(instrumentBrightness >= 255){
              instrumentBrightness = 255;
            }
            EEPROM.write(instrumentEEPROM,instrumentBrightness);
            analogWrite(instOut,instrumentBrightness);
            Serial.println(instrumentBrightness);
          }

          else if(digitalRead(instDimmer) && (millis() - instTime > 400)){
            if(instrumentBrightness > 0){
              instrumentBrightness--;
              delay(5);
            }
            else if(instrumentBrightness <= 0){
              instrumentBrightness = 0;
            }
            EEPROM.write(instrumentEEPROM,instrumentBrightness);
            analogWrite(instOut,instrumentBrightness);
            Serial.println(instrumentBrightness);
          }
      }
      else {
        analogWrite(instOut, 0);
        digitalWrite(parkOut,LOW);
        }
        
}



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
    
    int intermittent = analogRead(intermittentPin);
    
    if(intermittent > 900){
      digitalWrite(wipeOut, HIGH);
      }

    else if(intermittent < 900 && intermittent > 100 && (millis() - lastWipe > (intermittent * 100))){
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

void fog(){
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
    Serial.println(instrumentBrightness);
  }

  else if(instrumentBrightness >= 245){
    instrumentBrightness = 255;
    instTime = millis();
    EEPROM.write(instrumentEEPROM,instrumentBrightness);
    analogWrite(instOut,instrumentBrightness);
    Serial.println(instrumentBrightness);
  }}
  
}

void instDim(){
  if(digitalRead(parkIn) == HIGH){
  if((millis() - instTime > 100) && instrumentBrightness > 10){
    instrumentBrightness -= 10;
    instTime = millis();
    EEPROM.write(instrumentEEPROM,instrumentBrightness);
    analogWrite(instOut,instrumentBrightness);
    Serial.println(instrumentBrightness);
  }

  else if(instrumentBrightness <= 11){
    instrumentBrightness = 0;
    instTime = millis();
    EEPROM.write(instrumentEEPROM,instrumentBrightness);
    analogWrite(instOut,instrumentBrightness);
    Serial.println(instrumentBrightness);
  }}
  
}

void parkInterrupt(){
  analogWrite(instOut,instrumentBrightness);
}

