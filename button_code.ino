/* 
const int buttonAlarmSet = 10;   
const int buttonTimeSet = 11;    
const int buttonHourSet = 12;    
const int buttonMinSet = 13;     
int functionSwitch = 5;          
*/

void update_buttons_state()
{
  alarm_button_state   = digitalRead(buttonAlarmSet); // button for alarm setting
  time_button_state    = digitalRead(buttonTimeSet);  // button for time setting
  sethour_button_state = digitalRead(buttonHourSet);  // button for setting the hour
  setmin_button_state  = digitalRead(buttonMinSet);   // button for setting the minutes
  alarm_switch_state   = digitalRead(alarmSwitch);
  alarm_snooze_state   = digitalRead(buttonSnooze);
  function_swt_state   = digitalRead(functionSwitch);  // slide switch to display/set date & year
}


// "INTERNAL" VARIABLES FOR BUTTONS FUNCTION:
boolean first_time_hour = true;   // these are used to make sure that the hours
boolean first_time_minute = true; // and minutes only is increased once every keypress.
uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

void buttons()
{
     time_t curtime;   // store the current time in time variable t       
     uint8_t tempHour, tempMinute, tempSecond, tempDay, tempMonth;
     int tempYear;
     
  // LOW == button pressed
  // HIGH == button released
  // (this is because pullup resistors are used)
  

  if (function_swt_state == HIGH) {   // setting or displaying alarm & time  
     // Decide if we should set time or alarm:
     // (this also makes the display show the alarm time)
     displayMode = mTime; // display the time, it will be changed below if otherwise

     if ((alarm_button_state==LOW) &&  (time_button_state==HIGH)) // LOW = Set alarm
     {
        displayMode = mAlarm;                  // display the alarm time
        tempHour = alarmHour;
        tempMinute = alarmMinute;
        set_the_time(&alarmHour, &alarmMinute);
        if ((alarmHour != tempHour) || (alarmMinute != tempMinute)) {
           dsSramWrite(RAM_ALARM, alarmHour, alarmMinute);
           //alarmID = Alarm.alarmRepeat(alarmHour,alarmMinute,0, MorningAlarm);
           Alarm.write(alarmID, AlarmHMS(alarmHour,alarmMinute,0));
           Alarm.enable(alarmID);
           Serial.print(AlarmHMS(alarmHour,alarmMinute,0));
           Serial.print(" set alarm ");
           Serial.print(alarmID,DEC); Serial.print("  ");
           Serial.print(Alarm.count()); Serial.print("  ");
           Serial.println(Alarm.read(alarmID));
        }
     } /* set alarm */
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
     } /* set time */
  }
   else {   // switch is in Month and year - setting or displaying month and year
        displayMode = mDate;
        // Serial.print(" button: display date ");
        curtime = now();   // store the current time in time variable t       
        tempHour = hour(curtime);
        tempMinute = minute(curtime);
        tempSecond = second(curtime);
        tempDay = day(curtime);
        tempMonth = month(curtime);
        tempYear = year(curtime);
        
        if ((alarm_button_state==LOW) &&  (time_button_state==HIGH)) // LOW = Set date
        {
           displayMode = mDate;                  // display the alarm time
           set_the_date(&tempMonth, &tempDay);
        } /* set date */
        else if ((time_button_state==LOW) && !(alarm_button_state==LOW)) // LOW = Set year
        {
           displayMode = mYear;                  // display the year  
           set_the_year(&tempYear);     
        } /* set year */
        
        if ((tempYear != year(curtime)) || (tempMonth != month(curtime)) || (tempDay != day(curtime))) 
        {
           setTime(tempHour,tempMinute,tempSecond,
                   tempDay,tempMonth,tempYear);  // sets the arduino system time
            RTC.set(now());                          // sets the RTC clock
        } 
     }  // set month and year
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

} /* set the time */


/*
 * with the switch in date/year mode:
 *    default display is MMDD
 *    push the "alarm" button to set the MMDD
 *       push "hour" button to advance MMDD one month
 *       push "minute" button to advance one day
 *    push the "time" button to display the YYYY
 *       push "hour" button to advance YYYY one year
 *       push "minute" button to go back one year
 */
void set_the_date(uint8_t *month_p, uint8_t *day_p)
{
  // If "hour" button is pressed, increase month:
  if(sethour_button_state==LOW && first_time_hour) // only increase the hours once
  {                                             // every button press.
    if(*month_p < 12)
      (*month_p)++;
    else
      *month_p = 1;
    
    first_time_hour = false;
    
  }
  else if(sethour_button_state==HIGH)
  {
    first_time_hour = true; // reset when button is released, 
  }                         // so that the next press will be registerd.
  
  // If "minute" button is pressed, increase the day:
  // this may not work for Feb 29
  if(setmin_button_state==LOW && first_time_minute) // only increase the minutes
  {                                                 // once every button press.
    if(*day_p < monthDays[*month_p-1])
      (*day_p)++;
    else
      *day_p = 1;
      
    first_time_minute = false;
  }
  else if(setmin_button_state==HIGH)
  {
    first_time_minute = true; // reset when button is released, 
  }                           // so that the next press will be registerd.

} /* set the month and day */


void set_the_year(int *year_p)
{
  // If "hour" button is pressed, increase the year:
  if(sethour_button_state==LOW && first_time_hour) // only increase the hours once
  {                                             // every button press.
    (*year_p)++;    
    first_time_hour = false;
  }
  else if(sethour_button_state==HIGH)
  {
    first_time_hour = true; // reset when button is released, 
  }                         // so that the next press will be registerd.
  
  // If "minute" button is pressed, decrease the year:
  if(setmin_button_state==LOW && first_time_minute) // only increase the minutes
  {                                                 // once every button press.
    (*year_p)--;
      
    first_time_minute = false;
  }
  else if(setmin_button_state==HIGH)
  {
    first_time_minute = true; // reset when button is released, 
  }                           // so that the next press will be registerd.

} /* set the year */

