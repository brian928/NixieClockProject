/* 
const int buttonAlarmSet = 10;   // button for alarm setting
const int buttonTimeSet = 11;    // button for time setting
const int buttonHourSet = 12;    // button for setting the hour
const int buttonMinSet = 13;     // button for setting the minutes
*/

void update_buttons_state()
{
  alarm_button_state   = digitalRead(buttonAlarmSet);
  time_button_state    = digitalRead(buttonTimeSet); 
  sethour_button_state = digitalRead(buttonHourSet);
  setmin_button_state  = digitalRead(buttonMinSet);
  alarm_switch_state   = digitalRead(alarmSwitch);
  alarm_snooze_state   = digitalRead(buttonSnooze);
  function_swt_state   = digitalRead(functionSwitch);
}


// "INTERNAL" VARIABLES FOR BUTTONS FUNCTION:
boolean first_time_hour = true;   // these are used to make sure that the hours
boolean first_time_minute = true; // and minutes only is increased once every keypress.

void buttons()
{
     time_t curtime;   // store the current time in time variable t       
     int tempSecond, tempDay, tempMonth, tempYear;
     uint8_t tempHour, tempMinute;
     
  // LOW == button pressed
  // HIGH == button released
  // (this is because pullup resistors are used)
  
  displayMode = mTime; // display the time, it will be changed below if otherwise
  
  // Decide if we should set time or alarm:
  // (this also makes the display show the alarm time)
  if ((alarm_button_state==LOW) && !(time_button_state==LOW)) // LOW = Set alarm
  {
     displayMode = mAlarm;                  // display the alarm time
     tempHour = alarmHour;
     tempMinute = alarmMinute;
     set_the_time(&alarmHour, &alarmMinute);
     if ((alarmHour != tempHour) || (alarmMinute != tempMinute)) {
        dsSramWrite(alarmHour, alarmMinute);
        //alarmID = Alarm.alarmRepeat(alarmHour,alarmMinute,0, MorningAlarm);
        Alarm.write(alarmID, AlarmHMS(alarmHour,alarmMinute,0));
        Alarm.enable(alarmID);
        Serial.print(AlarmHMS(alarmHour,alarmMinute,0));
        Serial.print(" set alarm ");
        Serial.print(alarmID,DEC); Serial.print("  ");
        Serial.print(Alarm.count()); Serial.print("  ");
        Serial.println(Alarm.read(alarmID));
     }
  }
  else if ((time_button_state==LOW) && !(alarm_button_state==LOW)) // LOW = Set time
  {
     displayMode = mTime;                  // display the time  
     curtime = now();   // store the current time in time variable t       
     tempHour = hour(curtime);
     tempMinute = minute(curtime);
     tempSecond = 0;               // ability to make the second exact
     tempDay = day(curtime);
     tempMonth = month(curtime);
     tempYear = year(curtime);
     set_the_time(&tempHour, &tempMinute);
     
     if ((tempHour != hour(curtime)) || (tempMinute != minute(curtime))) {
        setTime(tempHour,tempMinute,tempSecond,
                tempDay,tempMonth,tempYear);  // sets the arduino system time
        RTC.set(now());                       // sets the RTC clock
     }
  }
} // buttons()

void set_the_time(uint8_t *hours_p, uint8_t *minutes_p)
{
  // If hour button is pressed, increase hours:
  if(sethour_button_state==LOW && first_time_hour) // only increase the hours once
  {                                             // every button press.
    if(*hours_p < 23)
      (*hours_p)++;
    else
      *hours_p = 0;
    
    first_time_hour = false;
    
  }
  else if(sethour_button_state==HIGH)
  {
    first_time_hour = true; // reset when button is released, 
  }                         // so that the next press will be registerd.
  
  // If minute button is pressed, increase minutes:
  if(setmin_button_state==LOW && first_time_minute) // only increase the minutes
  {                                                 // once every button press.
    if(*minutes_p < 59)
      (*minutes_p)++;
    else
      *minutes_p = 0;
      
    first_time_minute = false;
  }
  else if(setmin_button_state==HIGH)
  {
    first_time_minute = true; // reset when button is released, 
  }                           // so that the next press will be registerd.

}

