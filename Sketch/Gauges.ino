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

void readFuel(){

  fuel.update();
  analogWrite(fuelOut,fuel.getValue());
  if(fuel.getValue() < EEPROM.read(fuelAlertEEPROM)){
    displaySelect = 14;
    // play tone
    // idiot light on
  }
}

