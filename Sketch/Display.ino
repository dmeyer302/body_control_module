void buildDisplay(){

      //int line1 = 0;
      //int line2 = 0;

      /*  Display is not currently connected to any variables.
       *  Present code is to get display laid out.
       */
      
      u8g2.clearBuffer();
      
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
        char buf2[3];
        sprintf(buf2,"%d",temp);
        byte w = u8g2.getStrWidth(buf2);
        u8g2.setFont(u8g2_font_logisoso24_tf);
        u8g2.drawStr(118-w,30,buf2);

        if(minute() < 10){ // prints 4:03 rather than 4:3
          sprintf(buf1,"%d:0%d",hourFormat12(),minute());
        }
        else{
          sprintf(buf1,"%d:%d",hourFormat12(),minute());
        }
        //Serial.println(buf1);
        u8g2.drawStr(0,30,buf1);
        u8g2.drawCircle(124,9,3,U8G2_DRAW_ALL); // X,Y,radius
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
        char buf1[5];
        if(minute() < 10){ // prints 4:03 rather than 4:3
          sprintf(buf1,"%d:0%d",hourFormat12(),minute());
        }
        else{
          sprintf(buf1,"%d:%d",hourFormat12(),minute());
        }
        u8g2.setFont(u8g2_font_logisoso24_tf);
        byte w = u8g2.getStrWidth(buf1);
        u8g2.drawStr(128-w,28,buf1);

        u8g2.setFont(u8g2_font_helvB10_tf);
        u8g2.drawStr(0,16,"TRIP");
        u8g2.drawStr(0,30,"HRS");
        }

      else if(displaySelect == 11){ // Engine Hours
        char buf3[4];
        u8g2.setFont(u8g2_font_logisoso24_tf);
        float line2 = engineHrs;
        sprintf(buf3, "%.1f", engineHrs);
        byte w = u8g2.getStrWidth(buf3);
        u8g2.drawStr(128-w,28,buf3);

        u8g2.setFont(u8g2_font_helvB10_tf);
        u8g2.drawStr(0,16,"ENG");
        u8g2.drawStr(0,30,"HRS");
        }

      else if(displaySelect == 12){ // Oil Miles
        u8g2.setFont(u8g2_font_logisoso24_tf);
        byte w = u8g2.getStrWidth("1278");
        u8g2.drawStr(128-w,28,"1278");

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
        if(digitalRead(rightDoorIn) == LOW){
          u8g2.drawTriangle(128,16,120,22,120,10);
        }
      }

      else if(displaySelect == 14){
        u8g2.setFont(u8g2_font_logisoso22_tf);
        u8g2.drawStr(7,28,"LOW FUEL");
      }
        
      
      u8g2.sendBuffer();  

      displaySelect++;
        if(displaySelect == 15){
          displaySelect = 1;
         }
         displaySelectTime = millis();
}

