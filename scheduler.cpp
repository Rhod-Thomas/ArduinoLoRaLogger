#include <Arduino.h>
#include "rtc.h"
#include "lora.h"
#include "sensors.h"

#define SENSOR_SELECT_MASK 0x0f; //use the first 4 sensors.

//all actions have the same period. 
#define LOG_PERIOD 60
#define LOG_BACKUP_OFFSET 3
#define MILLIS_PER_SECOND 1000

unsigned long CurrentRtcTime = 0;
unsigned long CurrentCount = 0; 

bool AlarmConfigured = false;
unsigned long LogPeriod = LOG_PERIOD;

uint32_t UnixTime, UnixAlarmTime;

//TODO
//implement Trigger and actions

typedef enum  
{
  configureAlarm,
  waitForAlarm,
  startSensors,
  getSensorResult,
  checkTriggers,
  startActions,
  waitForActions,
  startTx,
  waitForTx,

}schedulerState;

schedulerState CurrentState = configureAlarm;

uint8_t SensorSelect = SENSOR_SELECT_MASK;
int16_t SensorResults[MAX_SENSORS];


void configAlarm()
{
  uint32_t remainder, beforeAlarm;
  	
  //get the current unix time from RTC.
  UnixTime = RtcGetUnixTime();
  Serial.println(UnixTime, DEC);

  //figure out the next alarm time.
  remainder = UnixTime % LOG_PERIOD;
  beforeAlarm = LOG_PERIOD - remainder;
  UnixAlarmTime = UnixTime + beforeAlarm;

  //set the RTC alarm time
  RtcSetAlarmTime(UnixAlarmTime);
  Serial.println(UnixAlarmTime);
}

bool alarmTriggered()
{
  return RtcGetAlarmFlag();
}


/****************************
 * SCHEDULER API
 ****************************/

void SchedulerInit()
{
  //testPeriodReset();
  CurrentState = configureAlarm;
}

void SchedulerService()
{
  //TODO
  //Scheduler works by setting alarms for events in the future.
  //if the alarm has not been set the Scheduler does that first.
  //if the alarm is set and the alarm flag is not, the scheduler returns not busy.
  //if the alarm flag is true the scheduler performs the appropriate action. 
  //then sets the next alarm and enters wait mode again. 	
  switch(CurrentState)
  {
    case waitForAlarm:

      if(alarmTriggered())
      {
	Serial.println("alarm processed by scheduler!");
	CurrentState = startSensors;
      }
      else if(millis() > (CurrentCount + ((unsigned long)(LOG_PERIOD + LOG_BACKUP_OFFSET) * MILLIS_PER_SECOND)))
      {
	Serial.println("WARNING!! Backup alarm triggered because RTC alarm did not occur when expected."); 
	CurrentState = configureAlarm; 
      }

    break;
    case startSensors:
    {
    //TODO
    //for each sensor, start
    
    uint8_t sensorSel = SensorSelect;
    uint8_t sensorNumber = 0;

    for(int n = 0; n < MAX_SENSORS; n++)
    {
      if(sensorSel & 0x01)
      {
        SensorStart(sensorNumber);
      }

      sensorSel = sensorSel >> 1;
      sensorNumber++;
    }
    
    Serial.println("Sensors started"); 
    CurrentState = getSensorResult;

    break;
    }
    case getSensorResult:
    {
    //TODO
    //for each sensor, get value.   
    
    uint8_t sensorSel = SensorSelect;
    uint8_t sensorNumber = 0;

    for(int n = 0; n < MAX_SENSORS; n++)
    {
      if(sensorSel & 0x01)
      {
        SensorResults[sensorNumber] =  SensorRead(sensorNumber);
      }

      sensorSel = sensorSel >> 1;
      sensorNumber++;
    }

    Serial.println("Sensors read"); 
    CurrentState = startTx;

    break;
    }
    case startTx:
    {
    //Build packet from SensorResults array and UnixAlarmTime.
     char packet[PACKET_DATA_MAX_LENGTH];

     sprintf(packet, "%lu", UnixAlarmTime);

    uint8_t sensorSel = SensorSelect;
    uint8_t sensorNumber = 0;

    for(int n = 0; n < MAX_SENSORS; n++)
    {
      if(sensorSel & 0x01)
      {
        sprintf(packet + strlen(packet), ",%d", SensorResults[sensorNumber]); 
      }

      sensorSel = sensorSel >> 1;
      sensorNumber++;
    }
     
    // Serial.println(packet);
     LoRaSendPacket(packet, true); 
     CurrentState = waitForTx;

    break;
    }
    case waitForTx:
      if(!LoRaService())
      {
        CurrentState = configureAlarm;
      } 
    break;
    case configureAlarm:
      configAlarm();
      CurrentCount = millis(); 
      CurrentState = waitForAlarm;
    break;
  }

  return (CurrentState != waitForAlarm);
}

void SchedulerTest()
{
  CurrentState = configureAlarm;
}

