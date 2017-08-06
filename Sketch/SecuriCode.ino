void securicode(){
  
  if(millis() - keypadLightTime < 5000){
    
    //keypadLightStatus = 1;
    
    if(codePosition == 5 && (memcmp(enteredCode,code1,5) == 0 || memcmp(enteredCode,code2,5) == 0 || memcmp(enteredCode,code3,5) == 0)){
      
      unlock();
      domeStatus = 1;
      keypadLightStatus = 0;
      keypadLightTime = 0;
      codePosition = 0;
      
      enteredCode[0] = 0;
      enteredCode[1] = 0;
      enteredCode[2] = 0;
      enteredCode[3] = 0;
      enteredCode[4] = 0;

      numberOfErrors = 0;
    }

    else if(codePosition == 5 && (memcmp(enteredCode,code1,5) != 0 || memcmp(enteredCode,code2,5) != 0 || memcmp(enteredCode,code3,5) != 0) && numberOfErrors == 4){
      for(byte i = 0; i < 40; i++){
        keypadLightStatus = !keypadLightStatus;
        digitalWrite(keypadLight,keypadLightStatus);
        delay(80);
      }
      keypadLightStatus = 0;
      
      errorTime = millis();
      numberOfErrors = 0;
      keypadLightStatus = 0;
      codePosition = 0;

    }
    
    else if(codePosition == 5 && (memcmp(enteredCode,code1,5) != 0 || memcmp(enteredCode,code2,5) != 0 || memcmp(enteredCode,code3,5) != 0) && numberOfErrors < 4){

      numberOfErrors++;
      
      enteredCode[0] = 0;
      enteredCode[1] = 0;
      enteredCode[2] = 0;
      enteredCode[3] = 0;
      enteredCode[4] = 0;

      // Blink light to indicate bad code
      digitalWrite(keypadLight,0);
      delay(150);
      digitalWrite(keypadLight,1);
      delay(150);
      digitalWrite(keypadLight,0);
      delay(150);
      digitalWrite(keypadLight,1);
      
      keypadLightTime = millis();
      codePosition = 0;
    }
  }

  else{
    keypadLightStatus = 0;
      enteredCode[0] = 0;
      enteredCode[1] = 0;
      enteredCode[2] = 0;
      enteredCode[3] = 0;
      enteredCode[4] = 0;
    codePosition = 0;
  }

  if(digitalRead(key4) == HIGH && digitalRead(key5) == HIGH){ // 7/8 + 9/0 to lock
    lock();
    keypadLightStatus = 0;
    domeStatus = 0;
    keypadLightTime = 0;
  }

      /*Serial.println();
      for(byte i = 0; i < 5; i++){
        Serial.print(enteredCode[i]);
      }
      Serial.println();
      Serial.println(codePosition);*/

  
  digitalWrite(keypadLight,keypadLightStatus);
}


/*  ================================================
 *  Interrupt Functions
 *  ================================================*/

void sc1int(){
  if(millis() - keypadLightTime > 200 && millis() - errorTime > errorLockoutTime){
  keypadLightStatus = 1;
  keypadLightTime = millis();
  
  enteredCode[codePosition] = 1;
  codePosition++;
  }
}

void sc2int(){
  if(millis() - keypadLightTime > 200 && millis() - errorTime > errorLockoutTime){
  keypadLightStatus = 1;
  keypadLightTime = millis();

  enteredCode[codePosition] = 2;
  codePosition++;
  }
}

void sc3int(){
  if(millis() - keypadLightTime > 200 && millis() - errorTime > errorLockoutTime){
  keypadLightStatus = 1;
  keypadLightTime = millis();
  
  enteredCode[codePosition] = 3;
  codePosition++;
  }
}

void sc4int(){
  if(millis() - keypadLightTime > 200 && millis() - errorTime > errorLockoutTime){
  keypadLightStatus = 1;
  keypadLightTime = millis();

  enteredCode[codePosition] = 4;
  codePosition++;
  }
}

void sc5int(){
  if(millis() - keypadLightTime > 200 && millis() - errorTime > errorLockoutTime){
  keypadLightStatus = 1;
  keypadLightTime = millis();

  enteredCode[codePosition] = 5;
  codePosition++;
  }
}

