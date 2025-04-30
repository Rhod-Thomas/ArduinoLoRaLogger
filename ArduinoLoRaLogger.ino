/*
TODO
alarm redundancy implementation and test - use millis().
*/

#include <AltSoftSerial.h>
#include <Wire.h>
#include <string.h>
#include <stdio.h>
#include "lora.h"
#include "userComms.h"
#include "rtc.h"
#include "scheduler.h"
#include "sensors.h"

/**********************                         
SETUP   
***********************/

void setup() {   

  Serial.begin(9600);                    
  while (!Serial) ; // wait for serial monitor to open

  UserCommsInit();  
  RtcInit();
  LoRaInit();
  SchedulerInit();
  SensorsInit();
  //PwrMngmtInit();
}

/**********************  
INFINITE LOOP                                
***********************/

void loop() {
  
  bool busy = false;

  busy |= UserCommsService();
  busy |= SchedulerService();
  busy |= RtcService();
  busy |= LoRaService();
  //SensorsService();

  //micro deep sleep if nothing busy
  if(!busy)
  {
    //TODO
    //Use deepSleepwakeup timer as backup to RTC
    //b3vause Rtc or micro has missed a wakeup alarm
    //before which has rendered thw logger silent.

    //PwrMngmntServ();
   // LowPower.deepSleep();
  }
}
