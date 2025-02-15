
#include "sensors.h"
#include <Arduino.h>
#include <DHT11.h>

#define SENSOR_ERROR_VALUE -32768

//enum of explicit sensors

typedef enum
{

  dht11_hum = 0,
  dht11_temp = 1,
  thermistor = 2,
  ldr = 3,

} SENSOR_ID;

//array of sensors in order
uint8_t sensors[MAX_SENSORS];
int16_t SensorReading[MAX_SENSORS];

bool dht11StartFlag = false;
int dht11Success = -1;
uint8_t dht11SensorNo;
DHT11 dht11(2);
int Temperature;
int Humidity;

int thermistorPin = 0;
int ldrPin = 1;

void SensorsInit()
{
  //TODO 
  //these could be loaded from memory. 
  for (int n=0; n < MAX_SENSORS; n++)
  {
    sensors[n] = n;
  }

  dht11StartFlag = false;
}

bool SensorsService()
{
}

//TODO 
//implement sensor read time funcrions if we ever use a sensor that needs a warmup.//RTC alarm would then need to trigger before the log time and the time reading mechanisms be appropriately adjusted.
//


void SensorStart(uint8_t sensorNumber)
{
  uint8_t sensId = sensors[sensorNumber];

  switch(sensId)
  {
	  case dht11_hum:

		  if(!dht11StartFlag)
		  {
	            //TODO
		    //READ SENSOR
                    dht11StartFlag = true;
                    dht11SensorNo = sensorNumber;
		    dht11Success = dht11.readTemperatureHumidity(Temperature, Humidity);
		  }

		  if(dht11Success == 0)
		  {
		    //copy humidity result
                    SensorReading[sensorNumber] = Humidity;
		  }
		  else
		  {
                    SensorReading[sensorNumber] = SENSOR_ERROR_VALUE;;
		  }
		  break;

	  case dht11_temp:
		  //TODO
		  //set flag for dht11 done on first pass
		  if(!dht11StartFlag)
		  {
	            //TODO
		    //READ SENSOR
                    dht11StartFlag = true;
                    dht11SensorNo = sensorNumber;
		    dht11Success = dht11.readTemperatureHumidity(Temperature, Humidity);
		  }

		  if(dht11Success == 0)
		  {
		    //copy temp result
                    SensorReading[sensorNumber] = Temperature;
		  }
		  else
		  {
                    SensorReading[sensorNumber] = SENSOR_ERROR_VALUE;;
		  }
		  break;

	  case thermistor:
		  {
                  float a = analogRead(thermistorPin);
                  //the calculating formula of temperature
                  float resistor = (1023.0*10000)/a-10000;
                  float tempC = (3435.0/(log(resistor/10000)+(3435.0/(273.15+25)))) - 273.15;
                  SensorReading[sensorNumber] = (int)tempC;
		  }
		  break;
	  case ldr:
		  {
                  float a = analogRead(ldrPin);
                  SensorReading[sensorNumber] = (int)a;
		  }
		  break;
	  default:
		  break;
  
  }
}

int16_t SensorRead(uint8_t sensorNumber)
{
  if(dht11StartFlag)
  { 
    dht11StartFlag = false;
    dht11Success = -1; 
  }

  return SensorReading[sensorNumber];
}
