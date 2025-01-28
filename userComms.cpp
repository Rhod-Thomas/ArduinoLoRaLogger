#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <AltSoftSerial.h>
#include <DS3231-RTC.h>
#include "lora.h"
#include "userComms.h"
#include "rtc.h"

#define BUFF_MAX 6

#define PRINT_HELP_CMD "help;"
#define READ_TIME_CMD "rt;"
#define SET_TIME_CMD "st"
#define LORA_TEST_TX_CMD "tx;"


char buffer[BUFF_MAX];
int bufferCount = 0;

char* helpString =
"\r\n*******************\r\nLORA COMMANDS\r\n********************\r\ntx; LoRa transmitter sequence test\r\n********************\r\nRTC COMMAMDS\r\n*******************\r\nstYYMMDDwHHMMSS; upload RTC datetime in specified format\r\nrt; read the RTC datetime\r\n ";


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
  else if((strncmp(buffer, SET_TIME_CMD, SET_TIME_CMD_LENGTH) == 0) 
	&& (bufferCount == SET_TIME_TOTAL_LENGTH) 
	&& (strncmp(buffer + SET_TIME_TOTAL_LENGTH - 1, ";", TERMINATOR_LENGTH) == 0))
  {
    //take the time in the following format:
    //YYMMDDwHHMMSS
    //set myRtc object with these arguments.
  }
  else if (strncmp(buffer, READ_TIME_CMD, bufferCount) == 0)
  {

  }
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
void UserCommsService()
{
  char c;

  if (Serial.available())
  {
    c = Serial.read();
    userSerialIn(c);
  }
}  

/*
 *
 */
void UserCommsInit()
{
  clearBuffer();
} 
