// Info/Setup/Reset button set commonly found on Ford vehicles
// Controls vehicle display and settings


void infoInterrupt(){
  if(millis() - infoTime > 200){ // debouncing
    infoTime = millis();
    
    if(displaySelect < 20 && displaySelect > 9){ // < x should be the greatest number in the driving menu series
      displaySelect ++;
      if(displaySelect == 16){
        displaySelect = 10;
      }
      EEPROM.update(storedDisplaySelect,displaySelect);
    }
  
    else if(displaySelect < 30 && displaySelect > 19){
      if(EEPROM.read(storedDisplaySelect) < 20 && EEPROM.read(storedDisplaySelect) > 9){
        displaySelect = EEPROM.read(storedDisplaySelect);
      }
      else{
        displaySelect = 10;
      }
    }
}}


void setupInterrupt(){
  if(millis() - infoTime > 200){ // debouncing
    infoTime = millis();
  displaySelectTime = millis();
 
  if(displaySelect < 21 && displaySelect > 19){ // < x should be the greatest number in the Setup menu series
    displaySelect ++;
  }
  else if(displaySelect == 22){ // This number should be 1 + the greatest number in the Setup menu series
    displaySelect = EEPROM.read(storedDisplaySelect);
  }
  else{
    displaySelect = 20;
  }
  Serial.println(displaySelect);
}}


void resetInterrupt(){
  
  if(millis() - resetTime > 200){ // debouncing
    resetTime = millis();
    displaySelectTime = millis();
  
    if(displaySelect == 20){
        byte c = EEPROM.read(clockAdjustEEPROM);
        c++;
        if(c > 12){
          c = 8;
        }
        Serial.println(c);
        EEPROM.update(clockAdjustEEPROM,c);
      }

      else if(displaySelect == 21){ // Increment fuel alert setting
        byte f = EEPROM.read(fuelAlertEEPROM);
        f += 5;
        if(f > 116){
          f = 12;
        }
        EEPROM.update(fuelAlertEEPROM,f);
      }
  }
}


void resetButtonAction(){
  
  if(digitalRead(resetIn) == HIGH && millis() - resetTime > 2000){
    resetTime = millis();
    switch (displaySelect){
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
        break;
  
      case 12:
        //tripMiles = 0;
        //tripHours = 0;
        break;
  
      case 13:
        //tripMiles = 0;
        //tripHours = 0;
        break;
  
      case 14:
        break;
  
      case 15:
        //oilMiles = 0;
  
      case 16:
      case 17:
      case 18:
      case 19:
        break;
  
      case 20:{
        byte c = EEPROM.read(clockAdjustEEPROM);
        c++;
        if(c > 2){
          c = -2;
        }
        EEPROM.update(clockAdjustEEPROM,c);
        break;
      }
  
      case 51:{
        fuelReset = 1;
        displaySelect = EEPROM.read(storedDisplaySelect);
        break;
      }
  }
  }}
