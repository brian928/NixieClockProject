/*
Lots of code for serial input processing

*/
/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

/*


*/
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void processCLI(String inputString) 
{
  String tempString;
  char tempstr[5];
  int tval[6];
  int i;  
  inputString.toUpperCase();
  
  if (inputString.startsWith("SA")) {   // Set Alarm format "SA hh mm"
     Serial.print("CMD: Set Alarm to ");
     tempString = getValue(inputString, ' ', 1);
     tempString.toCharArray(tempstr,3);
     alarmHour = atoi(tempstr);

     tempString = getValue(inputString, ' ', 2);
     tempString.toCharArray(tempstr,3);
     alarmMinute = atoi(tempstr);
     
     dsSramWrite(alarmHour, alarmMinute);    
     Alarm.alarmRepeat(alarmHour,alarmMinute,0, MorningAlarm);
     Serial.print(alarmHour);
     printDigits(alarmMinute);
     Serial.println();
      
  }
  else
  if (inputString.startsWith("ST")) {   // Set Time
     Serial.println("CMD: Set Time");
      // get the system time so not all parameters need to be set
     time_t t = now(); // store the current time in time variable t 
     tval[0] = hour(t);          // returns the hour for the given time t
     tval[1] = minute(t);        // returns the minute for the given time t
     tval[2] = second(t);        // returns the second for the given time t 
     tval[3] = day(t);           // the day for the given time t 
     tval[4] = month(t);         // the month for the given time t 
     tval[5] = year(t);          // the year for the given time t  
     for (i = 0; i <6; i++) {
       tempString = getValue(inputString, ' ', i+1);
       if (tempString != "") {
          tempString.toCharArray(tempstr,5);
          tval[i] = atoi(tempstr);
       }

       Serial.print(tval[i]);
       Serial.print(" ");
    }
    Serial.println();
    /*  
     setTime(hr,min,sec,day,month,yr);  // sets the arduino system time
    */
    setTime(tval[0],tval[1],tval[2],tval[3],tval[4],tval[5]);  // sets the arduino system time
    RTC.set(now());                    // sets the RTC clock
  }
  else
  if (inputString.startsWith("DT")) {   // Display Time
     Serial.print("CMD: Display Time ");
     displayMode = mTime;
     digitalClockDisplay();
  }
  else
  if (inputString.startsWith("DA")) {   // Display Alarm
     Serial.print("CMD: Display Alarm ");
     displayMode = mAlarm;
     Serial.print(alarmHour);
     printDigits(alarmMinute);
     Serial.println();
  }
  else
  if (inputString.startsWith("DD")) {   // Display Date
     Serial.println("CMD: Display Date");
     displayMode = mDate;
 }
  else
  if (inputString.startsWith("DY")) {   // Display Year
     Serial.println("CMD: Display Year");
     displayMode = mYear;
 }
  else
  if (inputString.startsWith("DN")) {   // Dimming
     Serial.println("CMD: Dimming");
     tempString = getValue(inputString, ' ', 1);
     if (tempString != "") {
          tempString.toCharArray(tempstr,5);
          // dimNixieVal = atoi(tempstr);
          // dimNixie(dimNixieVal);
     }
 }

 else
  if (inputString.startsWith("MT")) {   // Toggle 12 24 hr format
     Serial.println("CMD: Military Time");
     if (milTime == false)
         milTime = true;
     else
         milTime = false;
  }
 else
  if (inputString.startsWith("BI")) {   // Burn In counter
     Serial.println("CMD: Burn In Counter");
     // burnInNixie(0x01);
     // Alarm.timerOnce(15, burnTimer);  // set the burn in timer for 15 seconds
  }
 else
  if (inputString.startsWith("DL")) {   // set delay for flash and burnin
     Serial.println("CMD: Delay - value must be 1 to 255");
     tempString = getValue(inputString, ' ', 1);
     if (tempString != "") {
          tempString.toCharArray(tempstr,5);
          // delayNixie(atoi(tempstr));
     }

  }
 else
  if (inputString.startsWith("FL")) {   // turn flashing on
     Serial.println("CMD: FLASH - 3 = on, 4 = off");
     tempString = getValue(inputString, ' ', 1);
     if (tempString != "") {
          tempString.toCharArray(tempstr,5);
          // flashNixie(atoi(tempstr));
     }

  }
  else {
     Serial.println("CMD: Error");
  }
} /* processCLI */

