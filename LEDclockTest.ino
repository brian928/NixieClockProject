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
#include <LiquidTWI.h>

#define RAM_OFFSET 8 
#define DS1307_CTRL_ID 0x68

//map logical ram addresses (0-55) to physical addresses (8-63)
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

LiquidTWI lcd(0);            // Initialize our 16x2 lcd display
byte tube[4]={DIGIT1, DIGIT2, DIGIT3, DIGIT4};  // array of nixie addresses
byte dimNixieVal = 100;          // initial dimmer value
byte delayNixieVal = 10;         // initieal delay valu

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
  
int alarmLED = 7;                // our alarm for devel purposes
int alarmSwitch = 8;             // slide switch pin
const int buttonSnooze = 9;      // snooze timer pin
boolean alarmActivated = false;  // is the alarm on?
boolean milTime = false;         // display 12 or 24 hour format, 12 hr = false
int ampmLED = 6;                 // Turn LED on for PM

// buttons for clock setting
const int buttonAlarmSet = 10;   // button for alarm setting
const int buttonTimeSet = 11;    // button for time setting
const int buttonHourSet = 12;    // button for setting the hour
const int buttonMinSet = 13;     // button for setting the minutes
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
int burnTime = 300;


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
  
  lcd.begin(16, 2);           // set up the LCD's number of rows and columns:
  lcd.setBacklight(HIGH);     // Turn the lcd backlight off or on
    
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
  dsSramRead(&aH, &aM);
Serial.print(aH,DEC);
     Serial.print(" ");
     Serial.println(aM,DEC);
     if ((aH == 0) && (aM == 0)) {           // ds1307 has been re-initialized
     dsSramWrite(alarmHour, alarmMinute); // set the default time
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
  // alarmID = Alarm.alarmRepeat(alarmHour,alarmMinute,0, MorningAlarm);  // set alarm for default time
  alarmID = Alarm.alarmRepeat(22,25,0, MorningAlarm);  // set alarm for default time
  Alarm.alarmRepeat(burnHour,burnMinute,0, burnAlarm);  // set alarm for Burn In
     Serial.print(alarmID,DEC);
     Serial.println(Alarm.read(alarmID));
}  /* setup */

void loop()
{
   switch (displayMode)
   {
      case mTime:
           // digitalClockDisplay();
           lcdTimeDisplay(hour(), minute());
           break;            
      case mAlarm:
           lcdTimeDisplay(alarmHour, alarmMinute);
           break;
      case mDate:
           // NixieDateDisplay();
           lcdDateDisplay();
           break;
      case mYear:
           // NixieYearDisplay();
           lcdYearDisplay();
           break;
      case mBurnIn:
           break;
      default:
           Serial.print("Invalid DisplayMode encountered: ");
           Serial.print(displayMode);
           break;
    }
    
  lcdClockDisplay();
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
  // burnInNixie(BURNON);
  Alarm.timerOnce(burnTime, burnTimer);  // set the burn in timer
}
void burnTimer()  {
  Serial.println("Burn In timer expired");
  // burnInNixie(BURNOFF);         // turn it off  
} 
/******************************************************************
 * print the time to the lcd
 ******************************************************************
 */
 void lcdTimeDisplay(int hours, int minutes){
  
   lcd.setCursor(12, 1);   // set the cursor to column 12, line 1
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
     lcd.print(' ');  // leading space
  else
     lcd.print(hours/10);

  // 2nd digit
  lcd.print((hours % 10));

  // write the minutes
  // 1st digit
  if (minutes < 10)
     lcd.print('0');  // leading zero
  else
     lcd.print((minutes/10));
  // 2nd digit
  lcd.print((minutes % 10));
}
/************************************************************************
 * print the date to the nixies
 * get the month and the days
 *************************************************************************
*/
void lcdDateDisplay(){

  int temp;

  temp = month();
  lcd.setCursor(12, 1);   // set the cursor to column 12, line 1    
  // write the month
  // 1st digit
  if (temp < 10)
     lcd.print(' ');  // leading space
  else
     lcd.print(temp/10);

  // 2nd digit
  lcd.print((temp % 10));

  // write the day
  // 1st digit
  temp = day();  
  if (temp < 10)
     lcd.print(' ');  // leading space
  else
     lcd.print(temp/10);
 // 2nd digit
  lcd.print((temp % 10));
}

/************************************************************************
 * print the year to the nixies
 * 
 *************************************************************************
*/
void lcdYearDisplay(){

  int temp;

  temp = year();
  lcd.setCursor(12, 1);   // set the cursor to column 12, line 1 
  // 1st digit
  lcd.print( (temp/1000));

  // 2nd digit
  temp = temp % 1000;
  lcd.print((temp/100));

  // 3rd digit
  temp = temp % 100;  
  lcd.print((temp/10));

 // 4th digit
  lcd.print((temp % 10));
 
}

/******************************************************************
 * print the time to the lcd
 ******************************************************************
 */
void lcdClockDisplay(){

  int temp;
  temp = hour();  
  lcd.setCursor(0, 0);   // set the cursor to column 0, line 0
   if (milTime == false) {
      if (temp == 0) 
         temp = 12; // 12 midnight
      else if( temp  > 12)
         temp = temp - 12 ;
   }
     
  if (temp < 10)
     lcd.print(' ');  // leading space
  lcd.print(temp);
  lcd.print(':');
  if (minute() < 10)
     lcd.print('0');  // leading zero
  lcd.print(minute());
  lcd.print(':');
  if (second() < 10)
     lcd.print('0');  // leading zero
  lcd.print(second());
  
  if (milTime == true) {
    lcd.print("   ");  // make sure display is clear incase of a flag change
  }
  else {
    if (isAM() == true)
       lcd.print(" AM ");
    else
       lcd.print(" PM ");
  }


  lcd.setCursor(0, 1);   // set the cursor to column 0, line 1
  lcd.print(month());
  lcd.print("/");
  lcd.print(day());
  lcd.print("/");
  lcd.print(year());
  lcd.print(" "); 
}

// digital clock display of the time
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

