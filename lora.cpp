
#include <AltSoftSerial.h>
#include <string.h>
#include <stdio.h>

#define AT_TIMEOUT_MS 2000
#define TX_TIMEOUT_MS 5000
#define AT_RETRIES 3
#define RESPONSE_MAX_LENGTH 30

//LoRa grove E5 AT COMMANDS
#define AT "AT\r\n"
#define AT_RESPONSE "+AT: OK\r\n"
#define TEST_MODE "AT+MODE=TEST\r\n"
#define TEST_MODE_RESPONSE "+MODE: TEST\r\n" 
#define TEST_CONFIG "AT+TEST=RFCFG,868,SF7,125,8,8,14,OFF,OFF,OFF\r\n"
#define TEST_CONFIG_RESPONSE "+TEST: RFCFG"
#define TEST_TRANSMIT_START "AT+TEST=TXLRSTR"
#define TEST_TRANSMIT "AT+TEST=TXLRSTR,\"Testing, Testing ....\"\r\n"
#define TEST_TRANSMIT_RESPONSE_START "+TEST: TXLRSTR"
#define TEST_TRANSMIT_DONE "+TEST: TX DONE\r\n"
#define TEST_DELAY_TRANSMIT "AT+TEST=TXLRSTR,\"Delay test\"\r\n"
#define LOW_POWER "AT+LOWPOWER\r\n"
#define LOW_POWER_RESPONSE "+LOWPOWER: SLEEP\r\n"
#define LOW_POWER_WAKEUP_RESPONSE "+LOWPOWER: WAKEUP\r\n"

//state machine to handle the transmit sequence.
enum class txSeqStg
{
  idle,
  atSend,
  atSendRsp,
  testModeEnable,
  testModeEnableRsp,
  testModeConfig,
  testModeConfigRsp,
  testTransmit,
  testTransmitRsp,
  initTestTransmitSuccess,
  waitTestTransmitSuccess,
  lowPowerMode,
  lowPowerModeRsp,
  cleanUp,
};

//state machine to handle the comms response.
enum class rspStat
{
  wait,
  done,
  timeout,
  error,

};

txSeqStg CurrentTxStage = idle;

unsigned long StartTime;
unsigned long Timeout;

char RespCopy[RESPONSE_MAX_LENGTH];
int ResponseLength;
int Retries;

bool DebugFlag = false;

void atSendCommand(const char* command, const char* resp)
{

  //get string lengths
  int sendLength = 0;
  sendLength = strlen(command);
  ResponseLength = strlen(resp);
  strcpy(RespCopy, resp);

  //clear out rx the buffer.
  while(altSerial.available()) altSerial.read();
 
  char* add = command;

  for(int n = 0; n < sendLength; n++)
  {
    altSerial.print(*add);

    if (DebugFlag == true)
    { 
      Serial.print(*add);
    }
    add++;
  }

  StartTime = millis();
  Retries = AT_RETRIES;
  Timeout = AT_TIMEOUT_MS;
}


rspStat atRespServ()
{
  if(ResponseLength == 0)
  {
    return error;
  } 

  //no timeout and response not complete, so wait. 
  if((altSerial.available() < ResponseLength) &&
    ((millis() - StartTime) < Timeout))
  {
    return wait;
  }

  if(altSerial.available() < ResponseLength)
  {
    return timeout;
  }
  
  char buffer[100];
  int bufferPointer = 0;

  while(altSerial.available() && (bufferPointer < ResponseLength))
  {
    buffer[bufferPointer] = altSerial.read();
    
    if(DebugFlag == true)
    {
      Serial.print(buffer[bufferPointer]);
    }
    bufferPointer++;
  }
 
  if(strncmp(RespCopy, buffer, ResponseLength) != 0)
  {
     return error;
  }

  return done;
}

void ProcessCmdRsp(rspStat rsp, txSeqStg next, txSeqStg prev)
{
  if(rsp == wait)
  {
    //do nothing
  }
  else if(rsp == done)
  {
    //next stage and refresh retries.
    CurrentTxStage = next;
  }
  else if (((rsp == timeout) || (rsp == error)) && (Retries > 0)) 
  {
    CurrentTxStage = prev;
    Retries--;
  }
  else
  {
    //ran out of retries.
    CurrentTxStage = cleanUp;
  }	      
}

void LoRaInit()
{
  currentTxStage = idle;
}

bool LoRaService()
{
  rspStat rspStatus; 

  switch (CurrenTxStage)
  {
    case idle:
      //do nothing
    break;
    case atSend:
      
      atSendCommand(AT, AT_RESPONSE);
      CurrentTxStage = atSendRsp;

    break;
    case atSendRsp:

      rspStatus = atRespServ();
      processCmdRsp(rspStatus, testModeEnable, atSend);

    break;
    case testModeEnable:

      atSendCommand(TEST_MODE, TEST_MODE_RESPONSE);
      CurrentTxStage = testModeEnableRsp;

    break;
    case testModeEnableRsp: 

      rspStatus= atRespServ();
      processCmdRsp(rspStatus, testModeConfig, testModeEnable);

    break;
    case testModeConfig:

      atSendCommand(TEST_CONFIG, TEST_CONFIG_RESPONSE);
      CurrentTxStage = testModeConfigRsp;

    break;
    case testModeConfigRsp:

      rspStatus= atRespServ();
      processCmdRsp(rspStatus, testTransmit, testModeConfig);

    break;
    case testTransmit:

      //TODO 
      //build transmit packet here
      char message [100];
      strcpy(message, TEST_TRANSMIT_START);
      strcat(message, ",\"");
      strcat(message, Packet);
      strcat(message, "\"\r\n");
    
      //response is the message with the first two characters removed.
      char response[100];
      strcpy(response, TEST_TRANSMIT_RESPONSE_START);  
      strcat(response, " \"");
      strcat(response, Packet); 
      strcat(response, "\"\r\n");

      atSendCommand(message, response);
      CurrentTxStage = testTransmitRsp;

    break;
    case testTransmitRsp:

      rspStatus= atRespServ();
      processCmdRsp(rspStatus, initTestTransmitSuccess, testTransmit);

    break;
    case initTestTransmitSuccess:
      //initialise for next response.
      
      ResponseLength = strlen(TEST_TRANSMIT_DONE);
      strcpy(RespCopy, TEST_TRANSMIT_DONE);
      StartTime = millis();
      Retries = AT_RETRIES;
      Timeout = TX_TIMEOUT_MS;
      CurrentTxStage = waitTestTransmitSuccess;

    break;
    case waitTestTransmitSuccess:

      rspStatus= atRespServ();
      processCmdRsp(rspStatus, lowPowerMode, testTransmit);

    break;
    case lowPowerMode:

      atSendCommand(LOW_POWER, LOW_POWER_RESPONSE);
      CurrentTxStage = lowPowerModeRsp;

    break;
    case lowPowerModeRsp:

      rspStatus= atRespServ();
      processCmdRsp(rspStatus, cleanUp, lowPowerMode);

    break;
    case cleanUp:

      CurrentTxStage = idle;

    break;
  }

  return (CurrentTxStage != idle); 
}

void LoRaSendPacket(const char* packet, bool debugFlag)
{
  //TODO 
  //load packet and initialise state machine.
  strcpy(Packet, packet);
  DebugFlag = debugFlag;
  CurrentTxStage = atSend;

}


