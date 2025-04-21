/*
This sketxh serves as the basis for a LoRa logger intenzed for use in a home smart sensor system

Design for test methodology will be followed. With the folloqing test milestones.

2) Establish  Serial test commands. All Grove commands to be used. At, Test mod3, test transmit, low power mod3, wakeup PASS

3) 5 minute LoRa test data send to raspi (hard-coded)PASS

4) arduino battery powered..  Awaiting parts. TODO
 - build regulator
 - fitbatteries
 - test self-enable and load
 - de-solder arduino 5V regulator and Fet

5) range increase. 1hr worth of data at 5 mins. TODO
 - move to shed for range test

6) permanent physical build on Nylon 6 plate. TODO
 
7) modularisation of Arduino code. DONE:q

8) RTC integration. DONE
 - i2c hardwae interface
 - basic hard coded function test
 - comms interface; set and read.
 - RTC stamp for logging.

9) Sensor integration. TODO

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

  //TODO
  //figure out if waiting for serial begin is a good idea
  //disable when making hardware portable.
 
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
