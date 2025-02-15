#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <AltSoftSerial.h>
#include "lora.h"
#include "userComms.h"
#include "rtc.h"
#include "scheduler.h"

#define BUFF_MAX 16
#define TERMINATOR_LEN 1

#define PRINT_HELP_CMD "help;"

#define GET_TIME_CMD "rt;"
#define SET_TIME_CMD "st"
#define SET_TIME_CMD_LEN 2
#define SET_TIME_TOTAL_LEN 16
#define SET_TIME_ARG_LEN 13

#define GET_UNIX_TIME_CMD "rut;"
#define SET_UNIX_TIME_CMD "sut"
#define SET_UNIX_TIME_CMD_LEN 3
//10 digits for UTC
#define SET_UNIX_TIME_TOTAL_LEN 14

#define LORA_TEST_TX_CMD "tx;"
#define SCHEDULER_TEST_CMD "scht;"

char buffer[BUFF_MAX];
int bufferCount = 0;

char* helpString =
"\r\n*******************\r\nLORA COMMANDS\r\n********************\r\ntx; LoRa transmitter sequence test\r\n********************\r\nRTC COMMAMDS\r\n*******************\r\nstYYMMDDwHHMMSS; set RTC datetime in specified format\r\nrt; get the RTC datetime\r\nsutXXXXXXXXXX; set unix time\r\nrut; get the unix time\r\n";


/*************************
 * LOCAL FUNCTIONS
 **************************/

void addToBuffer(char c)
{
  if(bufferCount < BUFF_MAX)
  {
    buffer[bufferCount++] = c;
  }
  else
  {
    bufferCount = 0;
  }
}

/*
 *compares buffer input to list of accepted commands.
 *format and function can be printed with helP command
 */
void processBuffer()
{
  if(strncmp(buffer, PRINT_HELP_CMD, bufferCount) == 0)
  {
    Serial.println(helpString);
  }
  else if(strncmp(buffer, LORA_TEST_TX_CMD, bufferCount) == 0)
  { 
    LoRaSendPacket("Test packet", true);
    Serial.println("LoRa tx test started...");
  }
  else if((strncmp(buffer, SET_TIME_CMD, SET_TIME_CMD_LEN) == 0) 
	&& (bufferCount == SET_TIME_TOTAL_LEN))
  {
    //take the time in the following format:
    //YYMMDDwHHMMSS

    char respBuff[40];
    char arg[SET_TIME_ARG_LEN + 1];

    strlcpy(arg, buffer + SET_TIME_CMD_LEN, SET_TIME_ARG_LEN + 1);

    RtcSetTime(arg, respBuff);

    Serial.println(respBuff);
  }
  else if (strncmp(buffer, GET_TIME_CMD, bufferCount) == 0)
  {
    Serial.println(RtcGetTime());
  }
  else if((strncmp(buffer, SET_UNIX_TIME_CMD, SET_UNIX_TIME_CMD_LEN) == 0) && (bufferCount == SET_UNIX_TIME_TOTAL_LEN))
  {
    uint32_t unixTime = 0;
    byte tempDigit = 0;

    for(int n = SET_UNIX_TIME_CMD_LEN ; n < (bufferCount - 1); n++)
    {
      tempDigit = (byte)buffer[n] - 48;
      
      //last digit?
      if(n == (bufferCount - 2))
      {
        unixTime += tempDigit;
	break;
      }

      unixTime += tempDigit;
      unixTime *= 10;
    }
    Serial.println(unixTime, DEC);
    RtcSetUnixTime(unixTime);
  }
  else if(strncmp(buffer, GET_UNIX_TIME_CMD, bufferCount) == 0)
  {
    Serial.println(RtcGetUnixTime(), DEC);
  }
  else if(strncmp(buffer,SCHEDULER_TEST_CMD, bufferCount) == 0)
  {
    Serial.println("testing..");
    SchedulerTest();
  }

  Serial.println("buffer count is...");
  Serial.println(bufferCount, DEC);
}

void clearBuffer()
{
  bufferCount = 0;
}

/*
* accepted command format: command;
*/
void userSerialIn(char c)
{
  addToBuffer(c);

  if(c == ';')
  {
    // command start/end character
    processBuffer();
    clearBuffer();
  }

}

/*************************
 * MODULE API
 **************************/

/*
 *Call thus function regularly to service 
 *incoming user input commands
 */
bool UserCommsService()
{
  char c;

  if (Serial.available())
  {
    c = Serial.read();
    userSerialIn(c);
  }
  return false;
}  

/*
 *
 */
void UserCommsInit()
{
  clearBuffer();
} 
