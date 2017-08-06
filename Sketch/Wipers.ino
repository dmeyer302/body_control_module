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

