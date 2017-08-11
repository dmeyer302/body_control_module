void lightsInMCPSetup(){
  lightsInMCP.begin(1);
  lightsInMCP.setupInterrupts(true,false,HIGH);
  pinMode(lightsInMCPInterrupt,INPUT);
}

void lightsOutMCPSetup(){
  lightsOutMCP.begin(2);
  lightsOutMCP.setupInterrupts(true,false,HIGH);
  pinMode(lightsOutMCPInterrupt,INPUT);
}

void doors(){
  if(digitalRead(leftDoorIn) == LOW || digitalRead(rightDoorIn) == LOW){
    domeStatus = 1;
    displaySelect = 13;
    buildDisplay(); // Remove in final version?
    doorCloseTime = millis();
    //displaySelectTime -= 3000;
    // play tone
  }
  else{
    displaySelect = EEPROM.read(storedDisplaySelect);
    buildDisplay();
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
              EEPROM.update(instrumentEEPROM,instrumentBrightness);
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
              EEPROM.update(instrumentEEPROM,instrumentBrightness);
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
    EEPROM.update(instrumentEEPROM,instrumentBrightness);
    analogWrite(instOut,instrumentBrightness);
    //Serial.println(instrumentBrightness);
  }

  else if(instrumentBrightness >= 246){
    instrumentBrightness = 255;
    instTime = millis();
    EEPROM.update(instrumentEEPROM,instrumentBrightness);
    analogWrite(instOut,instrumentBrightness);
    //Serial.println(instrumentBrightness);
  }}
  
}

void instDimInterrupt(){
  if(digitalRead(parkIn) == HIGH){
  if((millis() - instTime > 100) && instrumentBrightness > 10){
    instrumentBrightness -= 10;
    instTime = millis();
    EEPROM.update(instrumentEEPROM,instrumentBrightness);
    analogWrite(instOut,instrumentBrightness);
    //Serial.println(instrumentBrightness);
  }

  else if(instrumentBrightness <= 10){
    instrumentBrightness = 0;
    instTime = millis();
    EEPROM.update(instrumentEEPROM,instrumentBrightness);
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

