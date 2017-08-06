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

/*  ================================================
 *  Interrupt Functions
 *  ================================================*/

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
