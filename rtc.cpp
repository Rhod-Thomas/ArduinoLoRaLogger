//TODO
//two major functions?
//RtcSetTime (const char* time);
//RtcGetTime (with format option?);
//
//thoughts
//any kind of interrupt/wakeup interface? ISR?
//if yes, must cause micro low power wakeup.
//we could hold the micro awake until the interrupt flag is serviced
//
#include <Wire.h>
#include <DS3231.h>
#include <Arduino.h>

#define DAY_OFFSET 4
#define MONTH_OFFSET 2
#define YEAR_OFFSET 0
#define DOW_OFFSET 6
#define HOUR_OFFSET 7
#define MINUTE_OFFSET 9
#define SECOND_OFFSET 11

#define RTC_INT_PIN  3

RTClib myRtcLib;
DS3231 myRtc;

char TimeString[25];
bool AlarmFlag = false;

byte Year;
byte Month;
byte Date;
byte Dow;
byte Hour;
byte Minute;
byte Second;

void isr_rtcAlarm();

/*
 *
 *
 *
 */
bool RtcSetTime(const char* time, char* feedbackMsg)
{
  //expected format:
  //DDMMYYwHHmmSS
  //
  
  if(strlen(time) != 13)
  {
    strcpy(feedbackMsg, "string length incorrect");
    return false;
  }

  //TODO 
  //guard expressions - length correct?
  //Use strlen. 
  byte temp1, temp2;
  
  temp1 = (byte)time[DAY_OFFSET] - 48;
  temp2 = (byte)time[DAY_OFFSET+1] - 48;
  Date = (temp1 * 10) + temp2;

  temp1 = (byte)time[MONTH_OFFSET] - 48;
  temp2 = (byte)time[MONTH_OFFSET+1] - 48;
  Month = (temp1 * 10) + temp2;

  temp1 = (byte)time[YEAR_OFFSET] - 48;
  temp2 = (byte)time[YEAR_OFFSET+1] - 48;
  Year = (temp1 * 10) + temp2;

  Dow = (byte)time[DOW_OFFSET] - 48;

  temp1 = (byte)time[HOUR_OFFSET] - 48;
  temp2 = (byte)time[HOUR_OFFSET+1] - 48;
  Hour = (temp1 * 10) + temp2;

  temp1 = (byte)time[MINUTE_OFFSET] - 48;
  temp2 = (byte)time[MINUTE_OFFSET+1] - 48;
  Minute = (temp1 * 10) + temp2;

  temp1 = (byte)time[SECOND_OFFSET] - 48;
  temp2 = (byte)time[SECOND_OFFSET+1] - 48;
  Second = (temp1 * 10) + temp2;

  //TODO
  //checks 
  if(Date < 1 || Date > 31)
  {
    strcpy(feedbackMsg, "day value out of range");
    return false;
  }
  
  if(Month < 1 || Month > 12)
  {
    strcpy(feedbackMsg, "month value out of range");
    return false;
  }

  if(Year < 25 || Year > 50)
  {
    strcpy(feedbackMsg, "year value out of range (25-50)");
    return false;
  }

  if(Dow < 1 || Dow > 7)
  {
    strcpy(feedbackMsg, "Dow value out of range (1-7)");
    return false;
  }
  
  if(Hour > 23)
  {
    strcpy(feedbackMsg, "hours value out of range");
    return false;
  }
  if(Minute > 59)
  {
    strcpy(feedbackMsg, "minutes value out of range");
    return false;
  }

  if(Second > 59)
  {
    strcpy(feedbackMsg, "seconds value out of range");
    return false;
  }

  myRtc.setClockMode(false); //24hr
  myRtc.setYear(Year);
  myRtc.setMonth(Month);
  myRtc.setDate(Date);
  myRtc.setDoW(Dow);
  myRtc.setHour(Hour);
  myRtc.setMinute(Minute);
  myRtc.setSecond(Second);

  strcpy(feedbackMsg, "time value accepted");
  return true;

}
    
char* RtcGetTime()
{
    DateTime now = myRtcLib.now();

    char buffer[6];
		
    sprintf(buffer, "%.2d//", now.day());
    strcpy(TimeString, buffer);

    sprintf(buffer, "%.2d//", now.month());
    strcat(TimeString, buffer);

    sprintf(buffer, "%.4d ", now.year());
    strcat(TimeString, buffer);

    sprintf(buffer, "%.2d:", now.hour());
    strcat(TimeString, buffer);

    sprintf(buffer, "%.2d:", now.minute());
    strcat(TimeString, buffer);

    sprintf(buffer, "%.2d", now.second());
    strcat(TimeString, buffer);

    return TimeString;
}

void RtcSetUnixTime(uint32_t seconds)
{
   myRtc.setEpoch((time_t)seconds, false);
   myRtc.setClockMode(false); //24hr

}

uint32_t RtcGetUnixTime()
{
    DateTime now = myRtcLib.now();
    return now.unixtime();
}

void RtcSetAlarmTime(uint32_t unixTime)
{
  myRtc.turnOffAlarm(1);
  myRtc.checkIfAlarm(1);
  
  DateTime alarm = DateTime(unixTime);
  byte alarmBits = 0b00001000; //alarm when hr, min a seconds match.
  bool dayIsDay = false;
  bool h12 = false;
  bool alarmPm = false;

  myRtc.setA1Time(alarm.day(), alarm.hour(), alarm.minute(), alarm.second(),
		 alarmBits, dayIsDay, h12, alarmPm);

  myRtc.turnOnAlarm(1);
  myRtc.checkIfAlarm(1); 
}

bool RtcGetAlarmFlag()
{
  if(AlarmFlag)
  {
    AlarmFlag = false;
    return true;
  }

  return false;
}

void RtcInit()
{
  Wire.begin();
  pinMode(RTC_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RTC_INT_PIN), isr_rtcAlarm, FALLING);
}

bool RtcService()
{
  return AlarmFlag;
}

void isr_rtcAlarm()
{ 
  AlarmFlag = true;
  return;
}
