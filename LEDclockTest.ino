/*
 * 
 * 
 * TODO:
    How to set time, alarm, date, military time, dim
       4 buttons? selector switch, alarm switch, snooze
     X AM/PM indicator light
    Set up buzzer - pitch, cadence, loudness, song?
    Dim functions:
      Add way to dim the nixies
      X Add burn in timer
      dim/off at certain times? 50% at night
                               100% morning and evening
                               off while at work
    Store timers in RTC memory: alarm, burn-in, dimming
 */

#include <Time.h>  
#include <TimeAlarms.h>
#include <Wire.h>  
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

#define DS1307_CTRL_ID 0x68  // ds1307 RTC I2C address
#define RAM_ALARM 8    // ds1307 memory location for saving alarm time
#define RAM_BURNALARM 10
#define RAM_BURN  10   // ds1307 memory location for saving burnin time

// map logical ram addresses (0-55) to physical addresses (8-63)
// Nixie I2C addresses and constants
#define DIGIT1 0x0D    // tens digit of hours
#define DIGIT2 0x0C    // single digit of hours
#define DIGIT3 0x0B    // tens digit of minutes
#define DIGIT4 0x0A    // single digit of minutes

#define BURNON 0x01    // value to turn burn in On
#define BURNOFF 0x02   // value to turn burn in Off
#define BLANK 0x10
#define SPARE1 0x40
#define SPARE2 0x80
#define TRUE  1
#define FALSE 0

enum Modes {mTime, mAlarm, mDate, mYear, mBurnIn};
Modes displayMode, lastMode;

byte tube[4]={DIGIT1, DIGIT2, DIGIT3, DIGIT4};  // array of nixie addresses
byte dimNixieVal = 100;          // initial dimmer value
byte delayNixieVal = 10;         // initial delay value

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
  
int alarmLED = 3;                // our alarm for devel purposes
int alarmSwitch = 8;             // slide switch pin
const int buttonSnooze = 7;      // snooze timer pin
boolean alarmActivated = false;  // is the alarm on?
boolean milTime = false;         // display 12 or 24 hour format, 12 hr = false
int ampmLED = 6;                 // Turn LED on for PM

// buttons for clock setting
const int buttonAlarmSet = 10;   // button for alarm setting
const int buttonTimeSet = 11;    // button for time setting
const int buttonHourSet = 12;    // button for setting the hour
const int buttonMinSet = 9;     // button for setting the minutes
// button and switch states
int alarm_button_state;
int time_button_state; 
int sethour_button_state;
int setmin_button_state;
int alarm_switch_state;
int alarm_snooze_state;

int functionSwitch = 5;          // switch to display/set date & year
int function_swt_state;

// Set a default alarm time of 6am
uint8_t alarmHour = 6;
uint8_t alarmMinute = 0;
uint8_t alarmID;
int aH = 1;  // temp variables
int aM = 2;  // temp variables
int timerSnooze = 15;             // 15 seconds for our snooze timer
  
  // Set a burn in time at 12 noon for 300 seconds (5 minutes)
int burnHour = 12;
int burnMinute = 0;
int burnTime = 300;  // 5 minutes
uint8_t burnAlarmID;

void setup()  {
  Serial.begin(9600);
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time"); 

  displayMode = mTime;        // start off displaying the time     
  lastMode = mTime;
  inputString.reserve(80);    // reserve 80 bytes for the inputString:
    
  pinMode(alarmLED, OUTPUT);  // initialize the digital pin as an output.
  pinMode(alarmSwitch, INPUT_PULLUP);  // alarm slide switch
  pinMode(buttonSnooze, INPUT_PULLUP); // Snooze button
  pinMode(ampmLED, OUTPUT);  // initialize the digital pin as an output.
  
  pinMode(buttonAlarmSet, INPUT_PULLUP);   // button for alarm setting
  pinMode(buttonTimeSet, INPUT_PULLUP);    // button for time setting
  pinMode(buttonHourSet, INPUT_PULLUP);    // button for setting the hour
  pinMode(buttonMinSet, INPUT_PULLUP);     // button for setting the minutes
  pinMode(functionSwitch, INPUT_PULLUP);   // switch to display/set date & year

  // lets get alarm info from RAM
  dsSramRead(RAM_ALARM, &aH, &aM);
     // Serial.print(aH,DEC);
     //Serial.print(" ");
     //Serial.println(aM,DEC);
  if ((aH == 0) && (aM == 0)) {           // ds1307 has been re-initialized
     dsSramWrite(RAM_ALARM, alarmHour, alarmMinute); // set the default time
  } 
  else {
    alarmHour = aH;
    alarmMinute = aM;
  }

/*     Serial.print(aH,DEC);
     Serial.print(" ");
     Serial.print(aM,DEC);
     Serial.print(" ");
     Serial.println(af,DEC);
     Serial.print(alarmHour,DEC);
     Serial.print(" ");
     Serial.print(alarmMinute,DEC);
     Serial.print(" ");
     Serial.println(alarmflag,DEC);
*/
  alarmID = Alarm.alarmRepeat(alarmHour,alarmMinute,0, MorningAlarm);  // set alarm for default time
  burnAlarmID = Alarm.alarmRepeat(burnHour,burnMinute,0, burnAlarm);  // set alarm for Burn In
     // Serial.print(alarmID,DEC);
     // Serial.println(Alarm.read(alarmID));
}  /* setup */

void loop()
{
   switch (displayMode)
   {
      case mTime:
           NixieTimeDisplay(hour(), minute());
           break;            
      case mAlarm:
           NixieTimeDisplay(alarmHour, alarmMinute);
           break;
      case mDate:
           NixieDateDisplay();
           break;
      case mYear:
           NixieYearDisplay();
           break;
      case mBurnIn:
           break;
      default:
           Serial.print("Invalid DisplayMode encountered: ");
           Serial.print(displayMode);
           break;
    }
    
  Alarm.delay(100);
  update_buttons_state();  // see if any buttons have been pushed
  buttons();               // go set the clock if pushing buttons
  alarm();                 // see if the alarm is sounding
     
 //
 // print the string when a newline arrives:
 //
   if (stringComplete) {
     Serial.print("RCVD: ");
     Serial.println(inputString);
     processCLI(inputString);
     // clear the string:
     inputString = "";
     stringComplete = false;
   }
   
}  /* loop */


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

// Serial digital clock display of the time
void digitalClockDisplay(){
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

