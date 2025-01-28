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
#include <DS3231-RTC.h>

#define DAY_OFFSET 0
#define MONTH_OFFSET 2
#define YEAR_OFFSET 4
#define DOW_OFFSET 6
#define HOUR_OFFSET 7
#define MINUTE_OFFSET 9
#define SECOND_OFFSET 11

DS3231 myRtc;

char TimeString[25];

byte Year;
byte Month;
byte Day;
byte Dow;
byte Hour;
byte Minute;
byte Second;

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
    feedbackMsg = "string length incorrect";
    return false;
  }

  //TODO 
  //guard expressions - length correct?
  //Use strlen. 
  byte temp1, temp2;
  
  temp1 = (byte)time[DAY_OFFSET] - 48;
  temp2 = (byte)time[DAY_OFFSET+1] - 48;
  Day = (temp1 × 10) + temp2;

  temp1 = (byte)time[MONTH_OFFSET] - 48;
  temp2 = (byte)time[MONTH_OFFSET+1] - 48;
  Month = (temp1 × 10) + temp2;

  temp1 = (byte)time[YEAR_OFFSET] - 48;
  temp2 = (byte)time[YEAR_OFFSET+1] - 48;
  Year = (temp1 × 10) + temp2;

  Dow = (byte)time[DOW_OFFSET] - 48;

  temp1 = (byte)time[HOUR_OFFSET] - 48;
  temp2 = (byte)time[HOUR_OFFSET+1] - 48;
  Hour = (temp1 × 10) + temp2;

  temp1 = (byte)time[MINUTE_OFFSET] - 48;
  temp2 = (byte)time[MINUTE_OFFSET+1] - 48;
  Minute = (temp1 × 10) + temp2;

  temp1 = (byte)time[SECOND_OFFSET] - 48;
  temp2 = (byte)time[SECOND_OFFSET+1] - 48;
  Second = (temp1 × 10) + temp2;

  //TODO
  //checks 
  if(Day < 1 || Day > 31)
  {
    feedbackMsg = "day value out of range";
    return false;
  }
  
  if(Month < 1 || Month > 12)
  {
    feedbackMsg = "month value out of range";
    return false;
  }

  if(Year < 25 || Year > 50)
  {
    feedbackMsg = "year value out of range (25-50)";
    return false;
  }

  if(Dow < 1 || Dow > 7)
  {
    feedbackMsg = "Dow value out of range (1-7)";
    return false;
  }
  
  if(Hours > 23)
  {
    feedbackMsg = "hours value out of range";
    return false;
  }
  if(Minute > 59)
  {
    feedbackMsg = "minutes value out of range";
    return false;
  }

  if(Second > 59)
  {
    feedbackMsg = "seconds value out of range";
    return false;
  }

  myRtc.setClockMode(false); //24hr
  myRtc.setYear(Year);
  myRtc.setMonth(Month);
  myRtc.setDay(Day);
  myRtc.setDoW(Dow);
  myRtc.setHour(Hour);
  myRtc.setMinute(Minute);
  myRtc.setSecond(Second);

  feedbackMsg = "time value accepted";
  return true;

}
    
char* RtcGetTime()
{
    DateTime now = myRTC.now();

    char buffer[6];
		
    sprintf(buffer, "%.2d//", now.getDay());
    strcpy(TimeString, buffer);

    sprintf(buffer, "%.2d//", now.getMonth());
    strcat(TimeString, buffer);

    sprintf(buffer, "%.4d ", now.getYear());
    strcat(TimeString, buffer);

    sprintf(buffer, "%.2d:", now.getHour());
    strcat(TimeString, buffer);

    sprintf(buffer, "%.2d:", now.getMinute());
    strcat(TimeString, buffer);

    sprintf(buffer, "%.2d", now.getSecond());
    strcat(TimeString, buffer);

    return TimeString;
}


