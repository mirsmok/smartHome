/*
  LoRaNow Simple Node
  This code sends message and listen expecting some valid message from the gateway
  created 01 04 2019
  by Luiz H. Cassettari
*/

#include <LoRaNow.h>
#include <LowPower.h>
#include <EEPROM.h>

#define ledPin 9
#define photoPin 3
#define baterryMessurePin 6
#define sendPeriod 25

unsigned int eepromBaseIndex = 0;
unsigned long eepromWriteCycles = 0;
unsigned long eepromLastCouterValue = 0;

volatile unsigned long lastTime = 0;
volatile unsigned long sendCount = 0;
volatile unsigned long rawPowerValue = 0;
volatile bool sendData = false;
float batteryVoltage = 0.0;

String jsonData;

void counterInterrupt()
{
  // static unsigned long lastTime;
  unsigned long timeNow = millis();
  if (timeNow - lastTime < 5)
    return;
  if (timeNow < 200000UL)
  {
    digitalWrite(ledPin, HIGH);
  }
  lastTime = timeNow;
  // rawPowerValue++;
  if ((++rawPowerValue % sendPeriod) == 0)
  {
    sendData = true;
  }
  // Serial.println("rawCounter: " + String(rawPowerValue));
}

void setup()
{
  Serial.begin(115200);
  Serial.println("LoRaNow Simple Node");

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

  pinMode(photoPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(photoPin), counterInterrupt, FALLING);
  restoreCounter();
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  interrupts();
}

void loop()
{
  LoRaNow.loop();
}

void onMessage(uint8_t *buffer, size_t size)
{
  // Serial.print("Receive Message: ");
  // Serial.write(buffer, size);
  // Serial.println();
  // Serial.println();
  // Serial.flush();
  delay(20);
}

void onSleep()
{
  // Serial.println("Sleep");
  delay(10);

  digitalWrite(ledPin, LOW);
  pinMode(baterryMessurePin, INPUT_PULLUP);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

  if (sendData)
  {
    if ((sendCount % 100) == 0)
    {
      pinMode(baterryMessurePin, OUTPUT);
      digitalWrite(baterryMessurePin, LOW);
      delay(1);
      batteryVoltage = ((float)analogRead(A0)) * 0.006654740608229;
      pinMode(baterryMessurePin, INPUT_PULLUP);
    }
    //  Serial.println("Bateria: " + String(analogRead(A0)));
    jsonData = "{\n\"dev\":\"remCounterV01\"";
    jsonData += ",\n\"id\":\"" + String(LoRaNow.id(), HEX) + "\"";
    jsonData += ",\n\"CHs\":3";
    jsonData += ",\n\"ChIds\":";
    jsonData += "[\"1\",\"2\",\"3\"]";
    jsonData += ",\n\"ChValues\":";
    jsonData += "[" + String(sendCount++) + "," + String(rawPowerValue) + "," + String(batteryVoltage, 2) + "]";
    // reading temperature
    // lastTime = millis();
    jsonData += "\n}";

    Serial.println(jsonData);
    LoRaNow.print(jsonData.c_str());
    LoRaNow.send();
    storeCounter();
    sendData = false;
  }
}

void restoreCounter(void)
{
  EEPROM.get(1000, eepromBaseIndex);
  EEPROM.get(eepromBaseIndex, eepromWriteCycles);
  EEPROM.get(eepromBaseIndex + 4, eepromLastCouterValue);
  // Serial.println("EEPROM base: " + String(eepromBaseIndex) + " writeCycles: " + String(eepromWriteCycles) + " lastValue: " + String(eepromLastCouterValue));
  rawPowerValue = eepromLastCouterValue;
}
void storeCounter(void)
{
  eepromWriteCycles++;
  if (eepromWriteCycles > ((unsigned long)eepromBaseIndex * (unsigned long)50000))
  {
    eepromBaseIndex = ((unsigned int)(eepromWriteCycles / ((unsigned long)50000))) * 8;
    EEPROM.put(1000, eepromBaseIndex);
  }
  EEPROM.put(eepromBaseIndex, eepromWriteCycles);
  eepromLastCouterValue = rawPowerValue;
  EEPROM.put(eepromBaseIndex + 4, eepromLastCouterValue);
  // Serial.println("EEPROM write base: " + String(eepromBaseIndex) + " writeCycles: " + String(eepromWriteCycles) + " lastValue: " + String(eepromLastCouterValue));
}
