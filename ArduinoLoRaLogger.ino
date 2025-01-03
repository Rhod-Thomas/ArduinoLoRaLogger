/*
This sketxh serves as the basis for a LoRa logger intenzed for use in a home smart sensor system

Design for test methodology will be followed. With the folloqing test milestones.

1) virtual and regular serial comms. transparent uart to grove lora wi5. Test with verified commands.PASS

2) Establish  Serial test commands. All Grove commands to be used. At, Test mod3, test transmit, low power mod3, wakeup PASS

3) 5 minute LoRa test data send to raspi (hard-coded)

4) arduino battery powered.

5) range increase. 1hr worth of data at 5 mins.

*/


#include <AltSoftSerial.h>

// AltSoftSerial always uses these pins:
//
// Board          Transmit  Receive   PWM Unusable
// -----          --------  -------   ------------
// Teensy 3.0 & 3.1  21        20         22
// Teensy 2.0         9        10       (none)
// Teensy++ 2.0      25         4       26, 27
// Arduino Uno        9         8         10
// Arduino Leonardo   5        13       (none)
// Arduino Mega      46        48       44, 45
// Wiring-S           5         6          4
// Sanguino          13        14         12

// This example code is in the public domain.

/**************************
GLOBAL VARIABLES
***************************/

//software uart
AltSoftSerial altSerial;

//user input processing
#define BUFF_MAX 6
char buffer[BUFF_MAX];
int bufferCount = 0;
char* helpString =
"at;    send AT\r\ntm;    send AT+MODE=TEST\r\ntc;    send AT+TEST=RFCFG\r\nt1;    send AT+TEST=TXLRSTR\r\nt2;    send AT+TEST=TXLRSTR in 20s\r\ntxRx;  send AT and validate response\r\nlp;    send AT+LOWPOWER\r\n";

//log period processing
#define LOG_PERIOD_MS 60000 //5 mins in ms
unsigned long startPeriodTime = 0;
unsigned long currentTime = 0;

//At comms handling
#define TIMEOUT_MS 2000
bool sendAndWaitResponse(const char* send, const char* response);
bool transmitSequence(char* packet);

//AT COMMANDS
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
#define LOW_POWER_WAKEUP "+LOWPOWER: WAKEUP\r\n"

/***************************
USER INPUT HANDLING
****************************/
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

void processBuffer()
{
  if(strncmp(buffer, "at;", bufferCount) == 0)
  {
    Serial.println("at received");
    altSerial.println(AT);
  }
  else if(strncmp(buffer, "tm;", bufferCount) == 0)
  {
    Serial.println("test mode req received");
    altSerial.println(TEST_MODE);
  }
  else if(strncmp(buffer, "tc;", bufferCount) == 0)
  {
    Serial.println("test config uploaded");
    altSerial.println(TEST_CONFIG);
  }
  else if(strncmp(buffer, "t1;", bufferCount) == 0)
  {
    Serial.println("test transmit...");
    altSerial.println(TEST_TRANSMIT);
  }
  else if(strncmp(buffer, "t2;", bufferCount) == 0)
  {
    Serial.println("test transmit in 20 seconds...");
    delay(20000);
    altSerial.println(TEST_DELAY_TRANSMIT);
  }
  else if(strncmp(buffer, "lp;", bufferCount) == 0)
  {
    Serial.println("entering low power mode");
    altSerial.println(LOW_POWER);
  }
  else if(strncmp(buffer, "help;", bufferCount) == 0)
  {
    Serial.println(helpString);
  }
  else if(strncmp(buffer, "tx;", bufferCount) == 0)
  {
    transmitSequence("test packet");
  }
}

void clearBuffer()
{
  bufferCount = 0;
}

// recomended command format: command;
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

/**********************                                     TRANSCEIVER COMMS                                           ***********************/
bool sendAndWaitResponse(const char* send, const char* response)
{ 
  //TODO                    
  //local timestamp
  unsigned long startTime;

  //get string lengths
  int sendLength = 0;
  int responseLength = 0;
  sendLength = strlen(send);
  responseLength = strlen(response);
  
  //clear out the buffer.
  while(altSerial.available()) altSerial.read();
 
  //send data
  char* add = send;

  for(int n = 0; n < sendLength; n++)
  {
    altSerial.print(*add);
    Serial.print(*add);
    add++;
  }

  startTime = millis();

  //wait for response or timeout elapse
  while((altSerial.available() < responseLength) &&
        ((millis() - startTime) < TIMEOUT_MS));

  if(altSerial.available() < responseLength) return false;

  char buffer[100];
  int bufferPointer = 0;

  while(altSerial.available() && (bufferPointer < responseLength))
  {
    buffer[bufferPointer] = altSerial.read();
    Serial.print(buffer[bufferPointer]);
    bufferPointer++;
  }
 
  if(strncmp(response, buffer, responseLength) != 0) return false;

  return true;
}


bool waitResponse(char* response, unsigned long waitms)
{
  int responseLength = 0;
  responseLength = strlen(response);

  unsigned long startTime = millis();

  while((altSerial.available() < responseLength) && ((millis() - startTime) < waitms));

  if(altSerial.available() < responseLength)
  {
    Serial.println("wait response timeout");
    return false;
  }

  char buffer [100];
  unsigned int bufferCount = 0;

  while (altSerial.available())
  { 
    buffer[bufferCount] = altSerial.read();
    Serial.print(buffer[bufferCount]);
    bufferCount++;
  }

  if(strncmp(buffer, response, responseLength) != 0)
  { 
    Serial.println("response doean't match");
    return false;
  }

  return true;
}


bool transmitSequence(char* packet)
{
  //device should be in low power, try first to wakeup
  if(!sendAndWaitResponse(AT, LOW_POWER_WAKEUP))
  {
    
    //retry with simple AT send/receive
    if(!sendAndWaitResponse(AT, AT_RESPONSE))
    {
      Serial.println("At failed!");
      return false;
    }
  }

  if(!sendAndWaitResponse(TEST_MODE, TEST_MODE_RESPONSE))
  {
    Serial.println("test mode failed!"); 
    return false;
  }

  if(!sendAndWaitResponse(TEST_CONFIG, TEST_CONFIG_RESPONSE)) 
  {
    Serial.println("test config failed!");
    return false;
  }
    
  delay(500);
  while(altSerial.available()) Serial.print((char)altSerial.read());  
  
  Serial.println("");

  char message [100];
  strcpy(message, TEST_TRANSMIT_START);
  strcat(message, ",\"");
  strcat(message, packet);
  strcat(message, "\"\r\n");

  //response is the message with the first two characters removed.
  char response[100];
  strcpy(response, TEST_TRANSMIT_RESPONSE_START);  
  strcat(response, " \"");                                    strcat(response, packet);                                   strcat(response, "\"\r\n");

  if(!sendAndWaitResponse(message,response))
  {
    //log error.
    Serial.println("test transmit message upload failed!");
    return false;
  }
 
  //check for successful transmit
  if(!waitResponse(TEST_TRANSMIT_DONE, 5000))
  {
    Serial.println("no transmit done confirmation");
    return false;
  }

  return true;
  
}

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
  char packet [20];
  int temp = 23;
  int humid = 75;
  unsigned int count = 2;

  sprintf(packet, "%d, %d, %u", temp, humid, count);

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

/**********************                                     SETUP                                                       ***********************/

void setup() {                                                Serial.begin(9600);                                         while (!Serial) ; // wait for Arduino Serial Monitor to open
  Serial.println("AltSoftSerial Test Begin");

  altSerial.begin(9600);
  altSerial.println(AT);
  altSerial.println(LOW_POWER);    

  logPeriodReset();
}

/**********************  
INFINITE LOOP                                
***********************/

void loop() {
  char c;

  if (Serial.available()) {
    c = Serial.read();
    userSerialIn(c);
    //altSerial.print(c);
  }

  /*
  if (altSerial.available()) {
    c = altSerial.read();
    Serial.print(c);
  }
  */ 

  if(logPeriodElapsed())
  {
    logData();
    logPeriodReset();
  }
}
