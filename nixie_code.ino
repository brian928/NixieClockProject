/******************************************************************
 * print the time to the nixies
 ******************************************************************
 */
 void NixieTimeDisplay(int hours, int minutes){
  
   if (hours >= 12)
      digitalWrite(ampmLED, LOW);
   else     
      digitalWrite(ampmLED, HIGH);

   if (milTime == false) {
     if (hours == 0) 
        hours = 12; // 12 midnight
     else if( hours  > 12)
        hours = hours - 12 ;
   } 

  // 1st digit 
    if (hours < 10)
     writeNixieDigit(DIGIT1, BLANK);  // leading space
  else
     writeNixieDigit(DIGIT1, (hours/10));
  // 2nd digit
  writeNixieDigit(DIGIT2, (hours % 10));

  // write the minutes
  // 1st digit
  if (minutes < 10)
     writeNixieDigit(DIGIT3, 0);  // leading zero
  else
     writeNixieDigit(DIGIT3, (minutes/10));
  // 2nd digit
  writeNixieDigit(DIGIT4, (minutes % 10));
}
/************************************************************************
 * print the date to the nixies
 * get the month and the days
 *************************************************************************
*/
void NixieDateDisplay(){

  int temp;

  temp = month();
  // write the month
  // 1st digit
  if (temp < 10)
     writeNixieDigit(DIGIT1, BLANK);  // leading space
  else
     writeNixieDigit(DIGIT1, (temp/10));
  // 2nd digit
  writeNixieDigit(DIGIT2, (temp % 10));

  // write the day
  // 1st digit
  temp = day();  
  if (temp < 10)
     writeNixieDigit(DIGIT3, BLANK);  // leading space
  else
     writeNixieDigit(DIGIT3, (temp/10));
 // 2nd digit
  writeNixieDigit(DIGIT4, (temp % 10));
}

/************************************************************************
 * print the year to the nixies
 * 
 *************************************************************************
*/
void NixieYearDisplay(){
/* byte tube[4]={0x0D, 0x0C, 0x0B, 0x0A}; */
  int temp;

  temp = year();

  // 1st digit
  writeNixieDigit(DIGIT1, (temp/1000));

  // 2nd digit
  temp = temp % 1000;
  writeNixieDigit(DIGIT2, (temp/100));

  // 3rd digit
  temp = temp % 100;  
  writeNixieDigit(DIGIT3, (temp/10));

 // 4th digit
  writeNixieDigit(DIGIT4, (temp % 10));
 
}
/*
 * Write the number to the given nixie
 * Look at taylor datasheet for setting the numbers
 * spare 1 = 0x0400 -> 10
 * spare 2 = 0x0800 -> 11
 * blank   = 0x1000 -> 12
 */
 
void writeNixieDigit(byte address, byte number) {

  Wire.beginTransmission(address);
  Wire.write(0x00); //register address=Character
  Wire.write(number);
  Wire.endTransmission();

}

/*
 * DimValue needs to be between 0 and 100
 * 100 = 100% bright
 *   0 = 0%
*/
void dimNixie(byte dimValue) {
  int i;
  if (dimValue > 100)
      dimValue = 100;
  for (i= 0; i < 4; i++) {   // dim all the nixies
     Wire.beginTransmission(tube[i]);
     Wire.write(0x0B); //register address=Dimmer
     Wire.write(dimValue);
     Wire.endTransmission();
  }
}

/*
 * Burn In routine
 * 1 = on
 * 2 = off
 *  
*/
void burnInNixie(byte stateValue) {
  int i;
    Serial.print("BURNIN = ");
    Serial.println(stateValue);
  if (stateValue == 0x01) {  // Are we turning this on?
    lastMode = displayMode;  // Save the display mode
    displayMode = mBurnIn;
    dimNixie(100);           // 100 % bright
  }
  else {                      // we're done burning so put things back
     displayMode = lastMode;
     dimNixie(dimNixieVal);
  }

  for (i= 0; i < 4; i++) {    
     Wire.beginTransmission(tube[i]);
     Wire.write(0x0D); //register address= Burn in
     Wire.write(stateValue);
     Wire.endTransmission();
  }

}

/*
 * Flashing routine
 * 0x03 = on
 * 0x04 = off
 *  
*/
void flashNixie(byte stateValue) {
  int i;
    Serial.print("FLASHING = ");
    Serial.println(stateValue);

  for (i= 0; i < 4; i++) {    
     Wire.beginTransmission(tube[i]);
     Wire.write(0x0D); //register address= Burn in and flashing
     Wire.write(stateValue);
     Wire.endTransmission();
  }

}
/*
 * delay routine for burnin and flashing
 * delay in 10ms from 10ms to 2.550s
 * values 0x01 to 0xFF
 *  
*/
void delayNixie(byte stateValue) {
  int i;
    Serial.print("DELAY = ");
    Serial.println(stateValue);

  for (i= 0; i < 4; i++) {    
     Wire.beginTransmission(tube[i]);
     Wire.write(0x0C); //register address= delay time for burnin and flashing
     Wire.write(stateValue);
     Wire.endTransmission();
  }

}
