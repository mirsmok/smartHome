/*
  LoRaNow Simple Node
  This code sends message and listen expecting some valid message from the gateway
  created 01 04 2019
  by Luiz H. Cassettari
*/
#include <Arduino.h>
#include <LoRaNow.h>
#include <LowPower.h>
#include <EEPROM.h>

#define ledPin 9
#define photoPin 3
#define sendPeriod 25

unsigned int eepromBaseIndex = 0;
unsigned long eepromWriteCycles = 0;
unsigned long eepromLastCouterValue = 0;

volatile unsigned long lastTime = 0;
volatile unsigned long sendCount = 0;
volatile unsigned long rawPowerValue = 0;
volatile bool sendData = false;

String jsonData;

void counterInterrupt()
{
  // static unsigned long lastTime;
  unsigned long timeNow = millis();
  if (timeNow - lastTime < 5)
  {
    Serial.println("returning IRQ" + String(timeNow) + ":" + String(lastTime));
    return;
  }
  lastTime = timeNow;
  // rawPowerValue++;
  if ((++rawPowerValue % sendPeriod) == 0)
  {
    sendData = true;
  }
  Serial.println("rawCounter: " + String(rawPowerValue));
}
/*
ISR(ANALOG_COMP_vect)
{
  static unsigned long lastTime;
  unsigned long timeNow = millis();
  if (timeNow - lastTime < 50)
    return;
  rawPowerValue++;
  lastTime = timeNow;
  Serial.println("przerwanie");
}
*/
void setup()
{
  Serial.begin(115200);
  Serial.println("LoRaNow Simple Node");

  // analogComparator.setOn(INTERNAL_REFERENCE, AIN1);           // we instruct the lib to use voltages on the pins
  // analogComparator.enableInterrupt(counterInterrupt, RISING); // we set the interrupt and when it has to be raised
  /* ACSR |= B00010000;

   ACSR =
       (0 << ACD) |                 // Analog Comparator: Enabled
       (0 << ACBG) |                // Analog Comparator Bandgap Select: AIN0 is applied to the positive input
       (1 << ACO) |                 // Analog Comparator Output: Off
       (1 << ACI) |                 // Analog Comparator Interrupt Flag: Clear Pending Interrupt
       (1 << ACIE) |                // Analog Comparator Interrupt: Enabled
       (0 << ACIC) |                // Analog Comparator Input Capture: Disabled
       (1 << ACIS1) | (1 << ACIS0); // Analog Comparator Interrupt Mode: Comparator Interrupt on Rising Output Edge*/

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

  pinMode(photoPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(photoPin), counterInterrupt, FALLING);
  delay(5000);
  restoreCounter();
  pinSetup();
  interrupts();
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
  Serial.flush();
  delay(20);
}

void onSleep()
{
  Serial.println("Sleep");
  delay(10);
  pinSetupForSleep();
  // ADCSRA &= ~(1 << 7); // ADC off

  // BOD DISABLE - this must be called right before the __asm__ sleep instruction
  // MCUCR |= (3 << 5);                      // set both BODS and BODSE at the same time
  // = (MCUCR & ~(1 << 5)) | (1 << 6); // then set the BODS bit and clear the BODSE bit at the same time
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  Serial.println("Bateria: " + String(analogRead(A0)));

  if (sendData)
  {
    pinSetup();
    jsonData = "{\n\"dev\":\"remCounterV01\"";
    jsonData += ",\n\"id\":\"" + String(LoRaNow.id(), HEX) + "\"";
    jsonData += ",\n\"CHs\":2";
    jsonData += ",\n\"ChIds\":";
    jsonData += "[\"1\",\"2\"]";
    jsonData += ",\n\"ChValues\":";
    jsonData += "[" + String(sendCount++) + "," + String(rawPowerValue) + "]";
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
void pinSetup()
{
  // configure pins for DS18B20
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}
void pinSetupForSleep()
{
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void restoreCounter(void)
{
  EEPROM.get(1000, eepromBaseIndex);
  EEPROM.get(eepromBaseIndex, eepromWriteCycles);
  EEPROM.get(eepromBaseIndex + 4, eepromLastCouterValue);
  Serial.println("EEPROM base: " + String(eepromBaseIndex) + " writeCycles: " + String(eepromWriteCycles) + " lastValue: " + String(eepromLastCouterValue));
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
  Serial.println("EEPROM write base: " + String(eepromBaseIndex) + " writeCycles: " + String(eepromWriteCycles) + " lastValue: " + String(eepromLastCouterValue));
}