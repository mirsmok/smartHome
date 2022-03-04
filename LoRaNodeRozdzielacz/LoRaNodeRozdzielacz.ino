/*
  LoRaNow Simple Node
  This code sends message and listen expecting some valid message from the gateway
  created 01 04 2019
  by Luiz H. Cassettari
*/

#include <LoRaNow.h>
#include <LowPower.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

#define sendPeriod 1 //in 8 seconds units
float actualTemperature, supplayVoltage;
int sleepCounter;
unsigned long lastTime = 0;

#define ds18b20GND 8
#define ds18b20VCC 6
#define ONE_WIRE_BUS 7       // DS18B20 pin
#define batVoltageInput 17   // DS18B20 pin
#define batVoltageInputCH A3 // DS18B20 pin
#define batVoltageDivGND 16  // DS18B20 pin
#define ledPin 9
#define DI00 3
#define DI01 4
#define DI02 5
#define DI03 A0
#define DI04 A1
#define DI05 A2
#define DI06 A3
#define DI07 A4
#define AI00 A5
#define AI01 A6
#define AI02 A7
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
String jsonData;
// arrays to hold device addresses
DeviceAddress sensorsAddr[10];

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16)
      Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

String ds18b20AddressToStr(DeviceAddress deviceAddress)
{
  char buf[4];
  String tmpStr = "";
  for (uint8_t i = 4; i < 8; i++)
  {
    sprintf(buf, "%02X", deviceAddress[i]);
    tmpStr += buf;
  }
  return tmpStr;
}

void setup()
{
  DS18B20.begin();
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
  pinSetup();
  // self diagnostic
  delay(1000);
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
  }
}

void loop()
{
  LoRaNow.loop();
}

void onMessage(uint8_t *buffer, size_t size)
{
  Serial.print("Receive Message: ");
  Serial.write(buffer, size);
  Serial.println();
  Serial.println();
  delay(10);
}

void onSleep()
{
  Serial.println("Sleep");
  delay(10);
  pinSetupForSleep();
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  sleepCounter++;
  Serial.println(sleepCounter);
  if (sleepCounter >= sendPeriod)
  {
    pinSetup();
    jsonData = "{\n\"dev\":\"remIOv01\"";
    //jsonData += ",\n\"id\":\"" + String(LoRaNow.id()) + "\"";
    //reading temperature
    lastTime = millis();
    /* do
    {
      DS18B20.requestTemperatures();
      actualTemperature = DS18B20.getTempCByIndex(0);
    } while ((actualTemperature == 85.0 || actualTemperature == (-127.0)) && ((millis() - lastTime) <= 1000));
    if ((millis()) - lastTime >= 1000)
      actualTemperature = 88888.0;*/

    // method 1: by index
    digitalWrite(ledPin, HIGH);
    DS18B20.requestTemperatures();
    //delay(200);
    int i = -1;
    do
    {
      i++;
    } while (DS18B20.getAddress(sensorsAddr[i], i));
    i--;

    jsonData += ",\n\"CHs\":";
    jsonData += String(i + 1);

    String sensorsIDs = "[";
    String sensorsValues = "[";
    for (; i >= 0; i--)
    {
      actualTemperature = DS18B20.getTempC(sensorsAddr[i]);

      if (sensorsIDs == "[")
      {
        sensorsIDs += "\"" + ds18b20AddressToStr(sensorsAddr[i]) + "\"";
        sensorsValues += String(actualTemperature, 1);
      }
      else
      {
        sensorsIDs += ",\"" + ds18b20AddressToStr(sensorsAddr[i]) + "\"";
        sensorsValues += "," + String(actualTemperature, 1);
      }
    }
    sensorsIDs += "]";
    sensorsValues += "]";
    jsonData += ",\n\"ChIds\":";
    jsonData += sensorsIDs;
    jsonData += ",\n\"ChValues\":";
    jsonData += sensorsValues;
    jsonData += ",\n\"DI\":[" + String(digitalRead(DI00));
    jsonData += "," + String(digitalRead(DI01));
    jsonData += "," + String(digitalRead(DI02));
    jsonData += "," + String(digitalRead(DI03));
    jsonData += "," + String(digitalRead(DI04));
    jsonData += "," + String(digitalRead(DI05));
    jsonData += "," + String(digitalRead(DI06));
    jsonData += "," + String(digitalRead(DI07)) + "]";
    jsonData += ",\n\"AI\":[" + String(analogRead(AI00)) + "," + String(analogRead(AI01)) + "," + String(analogRead(AI02)) + "]";
    jsonData += "\n}";
    Serial.println("liczba bajtow:" + String(jsonData.length()));
    Serial.println(jsonData);

    LoRaNow.print(jsonData.c_str());
    LoRaNow.send();
    sleepCounter = 0;
    digitalWrite(ledPin, LOW);
  }
}
void pinSetup()
{
  //configure pins for DS18B20
  pinMode(ds18b20GND, OUTPUT);
  digitalWrite(ds18b20GND, LOW);
  pinMode(ds18b20VCC, OUTPUT);
  digitalWrite(ds18b20VCC, HIGH);
  pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  //digital inputs
  pinMode(DI00, INPUT_PULLUP);
  pinMode(DI01, INPUT_PULLUP);
  pinMode(DI02, INPUT_PULLUP);
  pinMode(DI03, INPUT_PULLUP);
  pinMode(DI04, INPUT_PULLUP);
  pinMode(DI05, INPUT_PULLUP);
  pinMode(DI06, INPUT_PULLUP);
  pinMode(DI07, INPUT_PULLUP);
  //configure pins for baterry volage reading
  //pinMode(batVoltageDivGND, OUTPUT);
  //digitalWrite(batVoltageDivGND, LOW);
  //pinMode(batVoltageInput, INPUT);
}
void pinSetupForSleep()
{
  //configure pins for DS18B20
  pinMode(ds18b20GND, INPUT);
  pinMode(ds18b20VCC, INPUT);
  //configure pins for baterry volage reading
  //pinMode(batVoltageDivGND, INPUT);
  //pinMode(batVoltageInput, INPUT);
  pinMode(ledPin, INPUT);
}
