#include <Arduino.h>
#line 1 "c:\\Users\\mirsmok\\Desktop\\stacjaMeteo\\stacjaMeteo.ino"
/*
  LoRaNow Simple Node
  This code sends message and listen expecting some valid message from the gateway
  created 01 04 2019
  by Luiz H. Cassettari
*/
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>

#include <stdio.h>

Adafruit_AHTX0 aht;

Adafruit_BMP280 bmp = Adafruit_BMP280(); // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

sensors_event_t pressure_event; // temp_event;

sensors_event_t humidityATH, tempATH;

volatile float temp, pressure, humidity;
volatile bool readValues = true;

#include <LoRaNow.h>
#include <LowPower.h>

#define sendPeriod 37 // in 8 seconds units
// float actualTemperature, supplayVoltage;
int sleepCounter;
// unsigned long lastTime = 0;

#define ds18b20GND 8
#define ds18b20VCC 6
#define ONE_WIRE_BUS 7       // DS18B20 pin
#define batVoltageInput 17   // DS18B20 pin
#define batVoltageInputCH A3 // DS18B20 pin
#define batVoltageDivGND 16  // DS18B20 pin
#define ledPin 13
#define DI00 3
#define batGNDPin 4
#define AI00 A0
#define AI01 A1

#line 45 "c:\\Users\\mirsmok\\Desktop\\stacjaMeteo\\stacjaMeteo.ino"
void setup();
#line 118 "c:\\Users\\mirsmok\\Desktop\\stacjaMeteo\\stacjaMeteo.ino"
void loop();
#line 128 "c:\\Users\\mirsmok\\Desktop\\stacjaMeteo\\stacjaMeteo.ino"
void onMessage(uint8_t *buffer, size_t size);
#line 136 "c:\\Users\\mirsmok\\Desktop\\stacjaMeteo\\stacjaMeteo.ino"
void readSensorData(void);
#line 145 "c:\\Users\\mirsmok\\Desktop\\stacjaMeteo\\stacjaMeteo.ino"
void sendSensorData(void);
#line 159 "c:\\Users\\mirsmok\\Desktop\\stacjaMeteo\\stacjaMeteo.ino"
void onSleep();
#line 45 "c:\\Users\\mirsmok\\Desktop\\stacjaMeteo\\stacjaMeteo.ino"
void setup()
{
  // DS18B20.begin();
  Serial.begin(115200);
  Serial.println("LoRaNow Simple Node");

  // LoRaNow.setFrequencyCN(); // Select the frequency 486.5 MHz - Used in China
  // LoRaNow.setFrequencyEU(); // Select the frequency 868.3 MHz - Used in Europe
  // LoRaNow.setFrequencyUS(); // Select the frequency 904.1 MHz - Used in USA, Canada and South America
  // LoRaNow.setFrequencyAU(); // Select the frequency 917.0 MHz - Used in Australia, Brazil and Chile

  LoRaNow.setFrequency(433E6);
  LoRaNow.setSpreadingFactor(7);
  // LoRaNow.setSpreadingFactor(sf);
  // LoRaNow.setPins(ss, dio0);

  // LoRaNow.setPinsSPI(sck, miso, mosi, ss, dio0); // Only works with ESP32

  if (!LoRaNow.begin())
  {
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ;
  }

  LoRaNow.onMessage(onMessage);
  LoRaNow.onSleep(onSleep);
  LoRaNow.showStatus(Serial);
  // pinSetup();
  // self diagnostic
  /*delay(1000);
  int i = -1;
  do
  {
    i++;
  } while (DS18B20.getAddress(sensorsAddr[i], i));
  i--;
  for (; i >= 0; i--)
  {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(400);
  }*/

  if (!aht.begin())
  {
    //   Serial.println("Could not find AHT? Check wiring");
    while (1)
      delay(10);
  }

  unsigned status;
  // status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  status = bmp.begin();
  if (!status)
  {
    //    Serial.println(F("Cnot find  BMP280 sensor"));
    //    Serial.print("SensorID was: 0x");
    //    Serial.println(bmp.sensorID(), 16);
  }
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  // bmp_temp->printSensorDetails();
  pinMode(batGNDPin,OUTPUT);
  digitalWrite(batGNDPin,LOW);
}

void loop()
{
  LoRaNow.loop();
  if(readValues){
    readSensorData();
    sendSensorData();
    readValues = false;
  }
}

void onMessage(uint8_t *buffer, size_t size)
{
  Serial.print("Receive Message: ");
  Serial.write(buffer, size);
  Serial.println();
  Serial.println();
  delay(10);
}
void readSensorData(void)
{
  bmp_pressure->getEvent(&pressure_event);
  aht.getEvent(&humidityATH, &tempATH); // populate temp and humidity objects with fresh data
  pressure = pressure_event.pressure;
  humidity = humidityATH.relative_humidity;
  temp = tempATH.temperature;  
}

void sendSensorData(void)
{
  char buffer [120];
  char str1_temp[6];
  char str2_temp[6];
  char str3_temp[8];
  sprintf(buffer,"{\n\"dev\":\"remIOv01\",\n\"CHs\": 3,\n\"ChIds\": [\"0001\",\"0002\",\"0003\"],\n\"ChValues\": [%s, %s, %s],\n\"DI\":[%d],\n\"AI\":[%d, %d]\n}"
  ,dtostrf(temp,4,2,str1_temp),dtostrf(humidity,4,1,str2_temp),dtostrf(pressure,5,1,str3_temp),digitalRead(DI00),analogRead(AI00),analogRead(AI01));

  Serial.println(buffer);
  delay(100);
  LoRaNow.print(buffer);
  LoRaNow.send();
}
void onSleep()
{
  Serial.println("Sleep");
  pinMode(batGNDPin,INPUT);
  delay(10);
  //  pinSetupForSleep();
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  sleepCounter++;
  Serial.println(sleepCounter);
  if (sleepCounter >= sendPeriod)
  {
    readValues = true;
    sleepCounter = 0;
    
  pinMode(batGNDPin,OUTPUT);
  digitalWrite(batGNDPin,LOW);
  }
}

