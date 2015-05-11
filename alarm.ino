/*************************************************************************
 * Alarm and sleep timer functions
 *************************************************************************
*/
void MorningAlarm(){
  Serial.println("Morning Alarm: - WAKE UP");    
  alarmActivated = true;
}
void Snooze()  {
  Serial.println("Snooze alarm");
  alarmActivated = true;  
}

void burnAlarm(){
  Serial.println("Activated Burn in function");    
  burnInNixie(BURNON);
  Alarm.timerOnce(burnTime, burnTimer);  // set the burn in timer
}
void burnTimer()  {
  Serial.println("Burn In timer expired");
  burnInNixie(BURNOFF);         // turn it off  
} 

/*************************************************************************
 * 
 *************************************************************************
*/
void alarm()
{
  //
  // check for alarm going off
  //     Alarm.alarmRepeat(alarmHour,alarmMinute,0, MorningAlarm);
   if (alarm_switch_state == LOW) {          // if the alarm switch is on
      if (alarmActivated == true) {          // and time for the alarm them 
         digitalWrite(alarmLED, HIGH);       // Sound the alarm
         alarmActivated = false;
      }
      if (alarm_snooze_state == HIGH) {         // check for snoozin'(swt is Normally On
           digitalWrite(alarmLED, LOW);         // Turn the alarm off
           alarmActivated = false;
           int snoozeId = Alarm.timerOnce(timerSnooze, Snooze);  // set the snooze timer
           Serial.print("snooze alarm: ");
           Serial.println(Alarm.read(snoozeId));
         }
   }   
   else {
       digitalWrite(alarmLED, LOW);          // Turn the alarm off
       alarmActivated = false; 
   }  
}   

/*
  read and write to RAM routines
*/

//#define DS1307_ADDR 0x68
//#define DS1307_CTRL_ID 0x68 // from DS1307RTC.cpp
// this is hardcoded for this application
void dsSramWrite(int location, uint8_t hour, uint8_t minute)
{
    Wire.beginTransmission(DS1307_CTRL_ID);
    Wire.write(location);
    Wire.write(hour);
    Wire.write(minute);
    Wire.endTransmission();    
}

//read from DS1307 RAM where addr>=0 and addr<56
// this is hardcoded for this application
void dsSramRead(int location, int *hour_p, int *minute_p)
{
    Wire.beginTransmission(DS1307_CTRL_ID);
    Wire.write(location); 
    Wire.endTransmission();
    Wire.requestFrom( DS1307_CTRL_ID, 2 );
    *hour_p = Wire.read();
    *minute_p = Wire.read();
    
}

