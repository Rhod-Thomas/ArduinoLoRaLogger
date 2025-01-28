

//log period processing
#define LOG_PERIOD_MS 10000

unsigned long startPeriodTime = 0;
unsigned long currentTime = 0;
/**********************
LOG PERIOD HANDLING
***********************/

bool logPeriodElapsed()
{
  currentTime = millis();
  return ((currentTime - startPeriodTime) > LOG_PERIOD_MS); 
}

void logData()
{
  //TODO          
  //build data packet

  //TODO
  //call RTC module and sensor drivers to build packet.
  char packet [20];
  int temp = 23;
  int humid = 75;
  unsigned int count = 2;

<BS> sprintf(packet, "%d, %d, %u", temp, humid, count);

  //transmit sequence
  if(!transmitSequence(packet))
  {
    //log error
    Serial.println("transmit sequence failed");
  }
  else 
  {
    Serial.println("transmit sequence success!");
  }

  //Sd card log
}                                     

void logPeriodReset()
{
  startPeriodTime = millis();
}



void SchedulerService()
{
  if(logPeriodElapsed())
  { 
    // logData();
    logPeriodReset();
  }
}

void SchedulerInit()
{
  logPeriodReset();
}
