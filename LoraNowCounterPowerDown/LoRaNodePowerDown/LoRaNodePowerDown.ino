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

#define sendPeriod 1 // in 8 seconds units
int sleepCounter;
unsigned long lastTime = 0;

#define ledPin 9
String jsonData;

void setup()
{
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
    jsonData = "{\n\"dev\":\"remCounterv01\"";
    jsonData += ",\n\"id\":\"" + String(LoRaNow.id()) + "\"";
    // reading temperature
    lastTime = millis();
    jsonData += "\n}";

    Serial.println(jsonData);
    Serial.println(jsonData.length());
    LoRaNow.print(jsonData.c_str());
    LoRaNow.print(millis());
    LoRaNow.send();
    sleepCounter = 0;
  }
}
void pinSetup()
{
  // configure pins for DS18B20
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}
void pinSetupForSleep()
{
  pinMode(ledPin, INPUT);
}
