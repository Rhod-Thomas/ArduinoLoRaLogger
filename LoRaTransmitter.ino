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

AltSoftSerial altSerial;

#define BUFF_MAX 3
char buffer[BUFF_MAX];
int bufferCount = 0;
 
void setup() {
  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor to open
  Serial.println("AltSoftSerial Test Begin");
  altSerial.begin(9600);
  altSerial.println("Hello World");
}

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
    altSerial.println("AT\r\n");
  }
  if(strncmp(buffer, "tm;", bufferCount) == 0)
  {
    Serial.println("test mode req received");
    altSerial.println("AT+MODE=TEST\r\n");
  }
  if(strncmp(buffer, "tc;", bufferCount) == 0)
  {
    Serial.println("test config uploaded");
    altSerial.println("AT+TEST=RFCFG,868,SF7,125,8,8,14,OFF,OFF,OFF\r\n");
  }
  if(strncmp(buffer, "t1;", bufferCount) == 0)
  {
    Serial.println("test transmit...");
    altSerial.println("AT+TEST=TXLRSTR,\"Testing, Testing ....\"\r\n");
  }
  if(strncmp(buffer, "t2;", bufferCount) == 0)
  {
    Serial.println("test transmit in 20 seconds...");
    delay(20000);
    altSerial.println("AT+TEST=TXLRSTR,\"delayed testing!\"\r\n");
  }
  if(strncmp(buffer, "lp;", bufferCount) == 0)
  {
    Serial.println("entering low power mode");
    altSerial.println("AT+LOWPOWER\r\n");
  }
}

void clearBuffer()
{
  bufferCount = 0;
}

// recomended command format: ;command;
void userSerialIn(char c)
{
  addToBuffer(c);

  if(c == ';' && bufferCount > 0)
  {
    // command start/end character
    processBuffer();
    clearBuffer();
  }

}

void loop() {
  char c;

  if (Serial.available()) {
    c = Serial.read();
    userSerialIn(c);
    //altSerial.print(c);
  }
  if (altSerial.available()) {
    c = altSerial.read();
    Serial.print(c);
  }


}
