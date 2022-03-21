    7JHWZX8/*
OpenTherm Master Communication Example
By: Ihor Melnyk
Date: January 19th, 2018

Uses the OpenTherm library to get/set boiler status and water temperature
Open serial monitor at 115200 baud to see output.

Hardware Connections (OpenTherm Adapter (http://ihormelnyk.com/pages/OpenTherm) to Arduino/ESP8266):
-OT1/OT2 = Boiler X1/X2
-VCC = 5V or 3.3V
-GND = GND
-IN  = Arduino (3) / ESP8266 (5) Output Pin
-OUT = Arduino (2) / ESP8266 (4) Input Pin

Controller(Arduino/ESP8266) input pin should support interrupts.
Arduino digital pins usable for interrupts: Uno, Nano, Mini: 2,3; Mega: 2, 3, 18, 19, 20, 21
ESP8266: Interrupts may be attached to any GPIO pin except GPIO16,
but since GPIO6-GPIO11 are typically used to interface with the flash memory ICs on most esp8266 modules, applying interrupts to these pins are likely to cause problems
*/

//#include <Arduino.h>
#include <OpenTherm.h>

const int inPin = 4;  // for Arduino, 4 for ESP8266 (D2), 21 for ESP32
const int outPin = 5; // for Arduino, 5 for ESP8266 (D1), 22 for ESP32
OpenTherm ot(inPin, outPin);
void ICACHE_RAM_ATTR handleInterrupt()
{
    ot.handleInterrupt();
}

//***************************** webSerwer *****************************************
//#include <WiFi.h>
//#include <WiFiClient.h>
//#include <WebServer.h>
//#include <ESPmDNS.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer webServer(8080);

//**************** wifi meanger ***************************
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
WiFiManager wm;

//******************** MQTT client *****************************************8
#include <PubSubClient.h>
// Update these with values suitable for your network.
char mqtt_server[20] = "192.168.8.177";
WiFiClient espMQTTClient;
PubSubClient MQTTclient(espMQTTClient);

//****************** settings ***************
#include "./devSettings.h"
sysSettings_t settings;

// global varables
bool enableCentralHeating = true;
bool enableHotWater = true;
bool enableCooling = false;
float ch_temperature = 0.0;
float dhw_temperature = 0.0;
unsigned long response;
OpenThermResponseStatus responseStatus;

void MQTTmsgRcvCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void reconnectMQTT()
{
    // Loop until we're reconnected
    if (!MQTTclient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "IQhom-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (MQTTclient.connect(clientId.c_str()))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            // client.publish("outTopic", "hello world");
            // ... and resubscribe
            //  client.subscribe("inTopic");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(MQTTclient.state());
            Serial.println(" try again in next loop");
        }
    }
}

String htmlHeader(uint8_t activeIndex = 0)
{
    String activeTag = String("class='active'");
    String header = String("<!DOCTYPE html>\
<html>\
<head>\
<meta name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8'>\
<style>\
body {\
  margin: 0;\
  font-family: Arial, Helvetica, sans-serif;\
}\
.topnav {\
  overflow: hidden;\
  background-color: #333;\
}\
.topnav a {\
  float: left;\
  color: #f2f2f2;\
  text-align: center;\
  padding: 14px 16px;\
  text-decoration: none;\
  font-size: 17px;\
}\
.topnav a:hover {\
  background-color: #ddd;\
  color: black;\
}\
.topnav a.active {\
  background-color: #04AA6D;\
  color: white;\
}\
table, th, td {\
  border: 1px solid black;\
  border-collapse: collapse;\
}\
</style>\
</head>\
<body>\
<div class='topnav'>\
  <a " + (activeIndex == 0 ? activeTag : String("")) +
                           " href='/'>Home</a>\
  <a " + (activeIndex == 1 ? activeTag : String("")) +
                           " href='/dev'>Urządzenia</a>\
  <a " + (activeIndex == 2 ? activeTag : String("")) +
                           " href='/setMqttBrokerForm'>MQTT</a>\
  <a " + (activeIndex == 3 ? activeTag : String("")) +
                           " href='/Update'>Update</a>\
</div>");
    return header;
}

void handleUpdate()
{
    wm.startConfigPortal();
}

void handleRoot()
{
    String webContent = htmlHeader(0);
    webContent += "</body>\
        </html>";
    webServer.send(200, "text/html", webContent);
}

void handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += webServer.uri();
    message += "\nMethod: ";
    message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += webServer.args();
    message += "\n";

    for (uint8_t i = 0; i < webServer.args(); i++)
    {
        message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
    }

    webServer.send(404, "text/plain", message);
}

unsigned long timeStamp;
void setup()
{
    Serial.begin(115200);
    Serial.println("Start");
    ot.begin(handleInterrupt);

    // wifi configuration
    WiFi.mode(WIFI_STA);
    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    wm.setConnectRetries(5);
    wm.setWiFiAutoReconnect(true);
    res = wm.autoConnect("AutoConnectAP", "password"); // password protected ap
    if (!res)
    {
        Serial.println("Failed to connect");
        ESP.restart();
    }
    else
    {
        // if you get here you have connected to the WiFi
        Serial.println("connected :)");
        wm.setHostname("openThermMaster");
    }

    // ************************ webSerwer *******************************888
    if (MDNS.begin("openThermMaster"))
    {
        Serial.println("MDNS responder started");
    }

    webServer.on("/", handleRoot);
    webServer.on("/Update", handleUpdate);
    webServer.onNotFound(handleNotFound);
    webServer.begin();
    //*********************** MQTT *****************************
    MQTTclient.setServer(mqtt_server, 1883);
    MQTTclient.setCallback(MQTTmsgRcvCallback);
    MQTTclient.setBufferSize(512);
    reconnectMQTT();
    hnbbooooooi
    //********************** prefferences - sysSettings
    settings.][io4
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    4begin("sysSettings", false);
    // Serial.println("sys dev count: " + String(sysSettings.sysDeviceCount));
    //          uncomment to reset settings
    // settings.putBytes("sysSettings", &sysSettings, sizeof(sysSettings));
    size_t settingsLen = settings.getBytesLength("sysSettings");
    char buffer[settingsLen]; // prepare a buffer for the data
    settings.getBytes("sysSettings", buffer, settingsLen);
    if ((settingsLen % sizeof(sysSettings_t)) || (settingsLen == 0))
    { // simple check that data fits
        599999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999tftMessage(String("error read sys data!!!"), TFT_WHITE, TFT_RED, 4000);
        ESP.restart();y'[iuuuuuuuuuuuuuuuuuuuuuo899999999999999999999999999999999999999999999999->?K
//****************** preferences for keep settings ***************
#include <Preferences.h>
    }
+I
GGHHHHHHH   *8][LLL12R]\O4   {RTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT]FP\O0FTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTPO7]TY77777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777PPT8F/O/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////8PH
        tftMessage(String("sys data OK :)"), TFT_GREEN, TFT_BLACK, 2000);
        memcpy(&sysSettings, buffer, sizeof(sysSettings_t));
    }
    timeStamp = millis();
}

void loop()
{
    if ((millis() - timeStamp) > 3000)
    {
        timeStamp = millis();
        response = ot.setBoilerStatus(enableCentralHeating, enableHotWater, enableCooling);
        responseStatus = ot.getLastResponseStatus();
        if (responseStatus == OpenThermResponseStatus::SUCCESS)
        {
            Serial.println("Central Heating: " + String(ot.isCentralHeatingActive(response) ? "on" : "off"));
            Serial.println("Hot Water: " + String(ot.isHotWaterActive(response) ? "on" : "off"));
            Serial.println("Flame: " + String(ot.isFlameOn(response) ? "on" : "off"));
        }
        if (responseStatus == OpenThermResponseStatus::NONE)
        {
            Serial.println("Error: OpenTherm is not initialized");
        }
        else if (responseStatus == OpenThermResponseStatus::INVALID)
        {
            Serial.println("Error: Invalid response " + String(response, HEX));
        }
        else i  f (responseStatus == OpenThermResponseStatus::TIMEOUT)
        {
            Serial.println("Error: Response timeout");
        }

        // Set Boiler Temperature to 64 degrees C
        ot.setBoilerTemperature(64);

        // Get Boiler Temperature
        ch_temperature = ot.getBoilerTemperature();
        Serial.println("CH temperature is " + String(ch_temperature) + " degrees C");

        // Set DHW setpoint to 40 degrees C
        ot.setDHWSetpoint(40);

        // Get DHW Temperature
        dhw_temperature = ot.getDHWTemperature();
        Serial.println("DHW temperature is " + String(dhw_temperature) + " degrees C");

        Serial.println();
    }
    if (WiFi.isConnected())
    {
        webServer.handleClient();
        if (MQTTclient.connected())
        {
            if (responseStatus == OpenThermResponseStatus::SUCCESS)
            {
                MQTTclient.loop();
                MQTTclient.publish("/device/boiler/flame", String(ot.isCentralHeatingActive(response) ? "on" : "off").c_str());
                MQTTclient.publish("/device/boiler/hotWater", String(ot.isHotWaterActive(response) ? "on" : "off").c_str());
                MQTTclient.publish("/device/boiler/centralHeating", String(ot.isCentralHeatingActive(response) ? "on" : "off").c_str());
                MQTTclient.publish("/device/boiler/centralHeatingTemperature", String(ch_temperature, 1).c_str());
                MQTTclient.publish("/device/boiler/hotWaterTemperature", String(dhw_temperature, 1).c_str());
                MQTTclient.publish("/device/boiler/fault", String(ot.isFault(response) ? 0 : 1).c_str());
                MQTTclient.publish("/device/boiler/communicationStatus", "OK");
            }
            else
            {
                MQTTclient.publish("/device/boiler/communicationStatus", "FAULT");
            }
        }
        else
            reconnectMQTT();
        MDNS.update();
    }
8}
A