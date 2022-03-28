# 1 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino"
/*

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
# 21 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino"
# 22 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2

//#include <Arduino.h>
# 25 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2

const int inPin = 4; // for Arduino, 4 for ESP8266 (D2), 21 for ESP32
const int outPin = 5; // for Arduino, 5 for ESP8266 (D1), 22 for ESP32
OpenTherm ot(inPin, outPin);
void __attribute__((section("\".iram.text." "OpenThermMaster.ino" "." "29" "." "0" "\""))) handleInterrupt()
{
    ot.handleInterrupt();
}

//***************************** webSerwer *****************************************
//#include <WiFi.h>
//#include <WiFiClient.h>
//#include <WebServer.h>
//#include <ESPmDNS.h>

# 41 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2
# 42 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2
# 43 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2
# 44 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2

ESP8266WebServer webServer(8080);

//**************** wifi meanger ***************************
# 49 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2
WiFiManager wm;

//******************** oneWire *****************************

# 54 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2
# 55 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2
// Data wire is connected to 14 pin on the OpenTherm Shield

OneWire oneWire(14);
DallasTemperature temperatureSensors(&oneWire);

//******************** MQTT client *****************************************8
# 62 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2
// Update these with values suitable for your network.
char mqtt_server[20] = "192.168.8.177";
WiFiClient espMQTTClient;
PubSubClient MQTTclient(espMQTTClient);

//****************** settings ***************
# 69 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino" 2
device_t openThermDev;
// flag for saving data
WiFiManagerParameter paramEnableCH("enableCH", "Załącz ogrzewanie", String(openThermDev.settings.enableCentralHeating).c_str(), String(openThermDev.settings.enableCentralHeating).length());
WiFiManagerParameter paramEnableDHW("enableHWD", "Załącz ciepłą wodę", String(openThermDev.settings.enableHotWater).c_str(), String(openThermDev.settings.enableHotWater).length());
WiFiManagerParameter paramEnableCooling("enableCooling", "Załącz chłodzenie", String(openThermDev.settings.enableCooling).c_str(), String(openThermDev.settings.enableCooling).length());
WiFiManagerParameter paramSetpointCH("setpointCH", "Temperatura wody ogrzewanie", String(openThermDev.settings.ch_temperature).c_str(), String(openThermDev.settings.ch_temperature).length());
WiFiManagerParameter paramSetpointDHW("setopintDHW", "Temperatura ciepłej wody użytkowej", String(openThermDev.settings.dhw_temperature).c_str(), String(openThermDev.settings.dhw_temperature).length());

bool shouldSaveConfig = false;

// save config
void saveConfig()
{
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("failed to open config file for writing");
    }
    configFile.write((char *)&openThermDev.settings, sizeof(openThermDev.settings));
    configFile.close();
}

// callback notifying us of the need to save config
void saveConfigCallback()
{
    Serial.println("Should save config");
    shouldSaveConfig = true;

    // read updated parameters
    openThermDev.settings.enableCentralHeating = (uint8_t)String(paramEnableCH.getValue()).toInt();
    openThermDev.settings.enableHotWater = (uint8_t)String(paramEnableDHW.getValue()).toInt();
    openThermDev.settings.enableCooling = (uint8_t)String(paramEnableCooling.getValue()).toInt();
    openThermDev.settings.ch_temperature = String(paramSetpointCH.getValue()).toFloat();
    openThermDev.settings.dhw_temperature = String(paramSetpointDHW.getValue()).toFloat();

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("failed to open config file for writing");
    }
    configFile.write((char *)&openThermDev.settings, sizeof(openThermDev.settings));
    configFile.close();
}

// global varables
// bool enableCentralHeating = true;
// bool enableHotWater = true;
// bool enableCooling = false;
// float ch_temperature = 0.0;
// float dhw_temperature = 0.0;

unsigned long timeStamp, timeStampMQTT;
unsigned long response;
OpenThermResponseStatus responseStatus;
float roomTemperature;

void MQTTmsgRcvCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    char buff[length + 1];
    strncpy(buff, (char *)payload, length);
    buff[length] = '\0';

    String msgTopic = String(topic);
    if (msgTopic == "device/boiler/centralHeating/enable/remote")
    {
        uint8_t state = String(buff).toInt() == 1 ? 1 : 0;
        Serial.print(state);
        //   Serial.print(buff);
        Serial.println();
        if (openThermDev.settings.enableCentralHeating != state)
        {
            Serial.println("MQTT: nowe nastawy zal/wyl ogrzewania");
            openThermDev.settings.enableCentralHeating = state;
            paramEnableCH.setValue(String(state).c_str(), String(state).length());
            Serial.println("Nowa nastawa : " + String(state));
            saveConfig();
        }
    }
    if (msgTopic == "device/boiler/centralHeating/setpoint/remote")
    {
        float value = String(buff).toFloat();
        Serial.print(value);
        Serial.println();
        if (openThermDev.settings.ch_temperature != value)
        {
            Serial.println("MQTT: nowe nastawy temeratury ogrzewania");
            openThermDev.settings.ch_temperature = value;
            paramSetpointCH.setValue(String(value, 1).c_str(), String(value, 1).length());
            Serial.println("Nowa temperatura: " + String(value));
            saveConfig();
        }
    }
    if (msgTopic == "device/boiler/hotWater/enable/remote")
    {
        uint8_t state = String(buff).toInt() == 1 ? 1 : 0;
        ;
        Serial.print(state);
        Serial.println();
        if (openThermDev.settings.enableHotWater != state)
        {
            Serial.println("MQTT: nowe nastawy zal/wyl cieplej wody");
            openThermDev.settings.enableHotWater = state;
            paramEnableDHW.setValue(String(state).c_str(), String(state).length());
            Serial.println("Nowa nastawa : " + String(state));
            saveConfig();
        }
    }
    if (msgTopic == "device/boiler/hotWater/setpoint/remote")
    {
        float value = String(buff).toFloat();
        Serial.print(value);
        Serial.println();
        if (openThermDev.settings.dhw_temperature != value)
        {
            Serial.println("MQTT: nowe nastawy temeratury cieplej wody");
            openThermDev.settings.dhw_temperature = value;
            paramSetpointDHW.setValue(String(value, 1).c_str(), String(value, 1).length());
            Serial.println("Nowa temperatura: " + String(value, 1));
            saveConfig();
        }
    }
}

void reconnectMQTT()
{
    // Loop until we're reconnected
    if (!MQTTclient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "IQhom-";
        clientId += String(random(0xffff), 16);
        // Attempt to connect
        if (MQTTclient.connect(clientId.c_str()))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            // client.publish("outTopic", "hello world");
            // ... and resubscribe
            //  client.subscribe("inTopic");

            MQTTclient.subscribe("device/boiler/centralHeating/setpoint/remote");
            MQTTclient.subscribe("device/boiler/centralHeating/enable/remote");
            MQTTclient.subscribe("device/boiler/hotWater/setpoint/remote");
            MQTTclient.subscribe("device/boiler/hotWater/enable/remote");
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
    String header = String("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8'><style>body {  margin: 0;  font-family: Arial, Helvetica, sans-serif;}.topnav {  overflow: hidden;  background-color: #333;}.topnav a {  float: left;  color: #f2f2f2;  text-align: center;  padding: 14px 16px;  text-decoration: none;  font-size: 17px;}.topnav a:hover {  background-color: #ddd;  color: black;}.topnav a.active {  background-color: #04AA6D;  color: white;}table, th, td {  border: 1px solid black;  border-collapse: collapse;  margin: 20px;  padding: 10px;}</style></head><body><div class='topnav'>  <a "
# 269 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\OpenThermMaster.ino"
       + (activeIndex == 0 ? activeTag : String("")) +
                           " href='/'>Home</a>  <a "
       + (activeIndex == 2 ? activeTag : String("")) +
                           " href='/setMqttBrokerForm'>MQTT</a>  <a "
       + (activeIndex == 3 ? activeTag : String("")) +
                           " href='/Update'>Update</a></div>");

    return header;
}

void handleSetMqttBrokerForm()
{
    String webContent(htmlHeader(2));
    webContent += String("<h2> &nbsp; Ustawienia brokera MQTT </br></h2>    <h3>Aktualny adres brokera: ");

    webContent += mqtt_server;
    webContent += "</h3><form action='/setMqttBroker'><table style='text-align:right'><tr><td>  <label for='mqttAddr'>Adres brokera MQTT:</label></td><td>  <input type='text' id='mqttAddr' name='mqttAddr'></td></tr><tr><td></td><td><input type='submit' value='Ustaw'></td></tr></table></form>";






    webContent += "</body></html>";

    webServer.send(200, "text/html", webContent);
}

void handleSetMqttBroker()
{
    String brokerAddr = String(webServer.arg(0).c_str());
    bool errorAddr = false;

    if (brokerAddr.length() > 28 || brokerAddr.length() < 8)
        errorAddr = true;

    String webContent(htmlHeader(1) + "<h2> Ustawianie adresu brokera MQTT </h2><br><br>");
    if (errorAddr)
        webContent += "<h3> Operacja zakonczona niepowodzeniem!</h3>";
    else
    {
        webContent += "<h3> Operacja zakonczona powodzeniem</h3>";
        strcpy(&openThermDev.settings.mqttAddress[0], brokerAddr.c_str());
        saveConfig();
    }
    webContent += "<br><br><a href='/dev'> Powrót </a>";
    webContent += "</body></html>";

    webServer.send(200, "text/html", webContent);
}

void handleUpdate()
{
    String webContent = htmlHeader(0);
    webContent += "<h3 style='margin: 20px'>Nastąpi uruchomienie configPortalu. Zaloguj się do domyślnego wifi. Adres urządzenia <a href='192.168.4.1'>192.168.4.1</a></h3>";
    webContent += "</body>        </html>";

    webServer.send(200, "text/html", webContent);
    wm.startConfigPortal();
}

void handleResetErrors()
{
    openThermDev.status.wifiFault = 0;
    openThermDev.status.mqttFault = 0;
    openThermDev.status.otFault = 0;
    handleRoot();
}

void handleRoot()
{
    String webContent = htmlHeader(0);
    webContent += "<h2 style='margin: 20px'>Nastawy:</h2>        <table>            <tr>                <th>Parametr</th>                <th>Wartość</th>            </tr>";





    webContent += "<tr>                <td>Ogrzewanie</td>                <td>"

                      +
                  String(openThermDev.settings.enableCentralHeating ? "Załączone" : "Wyłączone") + "</td>            </tr>";

    webContent += "<tr>                <td>Ogrzewanie Temperatura Zadana</td>                <td>"

                      +
                  String(openThermDev.settings.ch_temperature, 1) + "  °C</td>            </tr>";

    webContent += "<tr>                <td>Ciepła woda</td>                <td>"

                      +
                  String(openThermDev.settings.enableHotWater ? "Załączona" : "Wyłączona") + "</td>            </tr>";

    webContent += "<tr>                <td>Ciepła Woda Temperatura Zadana</td>                <td>"

                      +
                  String(openThermDev.settings.dhw_temperature, 1) + "  °C</td>            </tr></table>";


    // status urzadzenia
    webContent += "<h2 style='margin: 20px'>Status:</h2>        <table>            <tr>                <th>Parametr</th>                <th>Wartość</th>            </tr>";





    webContent += "<tr>                <td>Ogrzewanie</td>                <td>"

                      +
                  String(openThermDev.status.CentralHeating ? "Aktywne" : "Nieaktywne") + "</td>            </tr>";

    webContent += "<tr>                <td>Ogrzewanie Aktualna Temperatura</td>                <td>"

                      +
                  String(openThermDev.status.ch_temperature, 1) + " °C</td>            </tr>";

    webContent += "<tr>                <td>Ciepła woda</td>                <td>"

                      +
                  String(openThermDev.status.HotWater ? "Aktywna" : "Nieaktywna") + "</td>            </tr>";

    webContent += "<tr>                <td>Ciepła Woda Aktualna Temperatura</td>                <td>"

                      +
                  String(openThermDev.status.dhw_temperature, 1) + "  °C</td>            </tr>";

    webContent += "<tr>                <td>Aktualne ciśnienie</td>                <td>"

                      +
                  String(openThermDev.status.pressure, 1) + "  bar</td>            </tr>";

    webContent += "<tr>                <td>Aktualna modulacja</td>                <td>"

                      +
                  String(openThermDev.status.modulation, 1) + "  %</td>            </tr>";

    webContent += "<tr>                <td>Płomień</td>                <td>"

                      +
                  String(openThermDev.status.Flame ? "Załączony" : "Wyłączony") + "</td>            </tr>";

    webContent += "<tr>                <td>Status Komunikacji Piec</td>                <td>"

                      +
                  String(openThermDev.status.communicationStatus) + "</td>            </tr>";

    webContent += "<tr>                <td>Błąd Komunikacji Wifi</td>                <td>"

                      +
                  String(openThermDev.status.wifiFault ? "TAK" : "NIE") + "</td>            </tr>";

    webContent += "<tr>                <td>Błąd Komunikacji MQTT</td>                <td>"

                      +
                  String(openThermDev.status.mqttFault ? "TAK" : "NIE") + "</td>            </tr>";

    webContent += "<tr>                <td>Błąd Pieca</td>                <td>"

                      +
                  String(openThermDev.status.otFault ? "TAK" : "NIE") + "</td>            </tr>";

    if (openThermDev.status.otFault != 0)
    {
        webContent += "<tr>                <td>Kod Błędu Pieca</td>                <td>"

                      +
                      String(openThermDev.status.otFaultCode) + "</td>            </tr>";

    }
    webContent += "</table>";
    if (openThermDev.status.mqttFault || openThermDev.status.wifiFault)
        webContent += "<form action='/resetErrors'><table style='border: 0px;text-align:right'><tr><td>  <label>Aktywne błędny na urządzeniu</label></td><td><input type='submit' value='Resetuj'></td></tr></table></form>";





    webContent += "</body>        </html>";

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

void setup()
{
    delay(10000);
    Serial.begin(115200);
    Serial.println("Start");
    pinMode(BUILTIN_LED, 0x01);
    ot.begin(handleInterrupt);

    ////////////////////// config modyfication //////////////////////////////

    // read configuration from FS json
    Serial.println("mounting FS...");

    if (SPIFFS.begin())
    {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            // file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile)
            {
                Serial.println("opened config file");
                size_t size = configFile.size();
                if ((size == sizeof(sysSettingsRetein_t)) && (size > 0))
                { // simple check that data fits

                    char buffer[size];
                    configFile.readBytes(buffer, size);
                    memcpy(&openThermDev.settings, buffer, sizeof(sysSettingsRetein_t));
                    Serial.println("Zaladowano nastawy z pamieci");

                    // flag for saving data
                    paramEnableCH.setValue(String(openThermDev.settings.enableCentralHeating).c_str(), String(openThermDev.settings.enableCentralHeating).length());
                    paramEnableDHW.setValue(String(openThermDev.settings.enableHotWater).c_str(), String(openThermDev.settings.enableHotWater).length());
                    paramSetpointCH.setValue(String(openThermDev.settings.ch_temperature).c_str(), String(openThermDev.settings.ch_temperature).length());
                    paramSetpointDHW.setValue(String(openThermDev.settings.dhw_temperature).c_str(), String(openThermDev.settings.dhw_temperature).length());
                    configFile.close();
                }
                else
                {
                    Serial.println("Brak danych w pamięci lub dane nieprawidlowe");
                    configFile.close();
                    saveConfig();
                }
            }
        }
        else
        {
            File configFile = SPIFFS.open("/config.json", "w");
            configFile.write("first open");
            configFile.close();
        }
    }
    else
    {
        Serial.println("failed to mount FS");
    }
    // end read

    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length

    // set config save notify callback
    wm.setBreakAfterConfig(true);
    wm.setSaveConfigCallback(saveConfigCallback);

    // add all your parameters here
    wm.addParameter(&paramEnableCH);
    wm.addParameter(&paramEnableDHW);
    wm.addParameter(&paramEnableCooling);
    wm.addParameter(&paramSetpointCH);
    wm.addParameter(&paramSetpointDHW);
    // wifi configuration
    WiFi.mode(WIFI_STA);
    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    wm.setConnectRetries(5);
    wm.setWiFiAutoReconnect(true);
    res = wm.autoConnect("otInterface"); // password protected ap
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
    webServer.on("/setMqttBrokerForm", handleSetMqttBrokerForm);
    webServer.on("/setMqttBroker", handleSetMqttBroker);
    webServer.on("/resetErrors", handleResetErrors);
    webServer.onNotFound(handleNotFound);
    webServer.begin();

    //******************* oneWire *****************************
    // Init DS18B20 sensor
    temperatureSensors.begin();
    temperatureSensors.requestTemperatures();
    temperatureSensors.setWaitForConversion(false); // switch to async mode
    roomTemperature = temperatureSensors.getTempCByIndex(0);

    //*********************** MQTT *****************************
    strcpy(mqtt_server, openThermDev.settings.mqttAddress);
    MQTTclient.setServer(mqtt_server, 1883);
    MQTTclient.setCallback(MQTTmsgRcvCallback);
    MQTTclient.setBufferSize(512);
    reconnectMQTT();

    if (openThermDev.settings.ch_temperature > 80.0 || openThermDev.settings.ch_temperature < 20.0)
        openThermDev.settings.ch_temperature = 45.0;
    if (openThermDev.settings.dhw_temperature > 65.0 || openThermDev.settings.dhw_temperature < 20.0)
        openThermDev.settings.dhw_temperature = 45.0;

    timeStamp = millis();
    timeStampMQTT = millis();
}

void loop()
{
    if ((millis() - timeStamp) > 1000)
    {
        roomTemperature = temperatureSensors.getTempCByIndex(0);
        timeStamp = millis();
        // if (!openThermDev.settings.enableCentralHeating && !openThermDev.settings.enableHotWater)
        //     openThermDev.settings.enableCentralHeating = 1;
        response = ot.setBoilerStatus((bool)openThermDev.settings.enableCentralHeating, (bool)openThermDev.settings.enableHotWater, false);
        responseStatus = ot.getLastResponseStatus();
        strcpy(openThermDev.status.communicationStatus, ot.statusToString(responseStatus));
        if (responseStatus == OpenThermResponseStatus::SUCCESS)
        {
            openThermDev.status.CentralHeating = ot.isCentralHeatingActive(response) ? 1 : 0;
            openThermDev.status.HotWater = ot.isHotWaterActive(response) ? 1 : 0;
            openThermDev.status.Flame = ot.isFlameOn(response) ? 1 : 0;
            openThermDev.status.otFault = ot.isFault(response) ? 1 : 0;
            if (openThermDev.status.otFault)
                openThermDev.status.otFaultCode = ot.getFault() ? 1 : 0;
            Serial.println("Central Heating: " + String(openThermDev.status.CentralHeating ? "on" : "off"));
            Serial.println("Hot Water: " + String(openThermDev.status.HotWater ? "on" : "off"));
            Serial.println("Flame: " + String(openThermDev.status.Flame ? "on" : "off"));
            Serial.println("Fault: " + String(openThermDev.status.otFault ? "on" : "off"));
            if (openThermDev.status.otFault)
                Serial.println("Fault code: " + String(openThermDev.status.otFaultCode));

            // Set Boiler Temperature to 64 degrees C
            ot.setBoilerTemperature(openThermDev.settings.ch_temperature);
            // Get Boiler Temperature
            openThermDev.status.ch_temperature = ot.getBoilerTemperature();
            Serial.println("CH temperature is " + String(openThermDev.status.ch_temperature) + " degrees C");
            // Set DHW setpoint to 40 degrees C
            ot.setDHWSetpoint(openThermDev.settings.dhw_temperature);
            // Get DHW Temperature
            openThermDev.status.dhw_temperature = ot.getDHWTemperature();
            Serial.println("DHW temperature is " + String(openThermDev.status.dhw_temperature) + " degrees C");
            // Get pressure
            openThermDev.status.pressure = ot.getPressure();
            Serial.println("Pressure: " + String(openThermDev.status.pressure));
            // Get modulation
            openThermDev.status.modulation = ot.getModulation();
            Serial.println("Actual modulation: " + String(openThermDev.status.modulation) + " %");

            if (MQTTclient.connected() && WiFi.isConnected())
                digitalRead(BUILTIN_LED) ? digitalWrite(BUILTIN_LED, 0x0) : digitalWrite(BUILTIN_LED, 0x1);
            Serial.println();
        }
        if (responseStatus == OpenThermResponseStatus::NONE)
        {
            Serial.println("Error: OpenTherm is not initialized");
        }
        else if (responseStatus == OpenThermResponseStatus::INVALID)
        {
            Serial.println("Error: Invalid response " + String(response, 16));
        }
        else if (responseStatus == OpenThermResponseStatus::TIMEOUT)
        {
            Serial.println("Error: Response timeout");
        }
        temperatureSensors.requestTemperatures();
    }

    if ((millis() - timeStampMQTT) > 10000)
    {
        timeStampMQTT = millis();
        if (WiFi.isConnected())
        {
            if (MQTTclient.connected())
            {
                if (responseStatus == OpenThermResponseStatus::SUCCESS)
                {
                    MQTTclient.publish("device/boiler/centralHeating/state", String(openThermDev.status.CentralHeating).c_str());
                    MQTTclient.publish("device/boiler/centralHeating/actual", String(openThermDev.status.ch_temperature, 1).c_str());

                    MQTTclient.publish("device/boiler/hotWater/state", String(openThermDev.status.HotWater).c_str());
                    MQTTclient.publish("device/boiler/hotWater/actual", String(openThermDev.status.dhw_temperature, 1).c_str());

                    MQTTclient.publish("device/boiler/pressure/actual", String(openThermDev.status.pressure, 1).c_str());
                    MQTTclient.publish("device/boiler/modulation/actual", String(openThermDev.status.modulation, 1).c_str());

                    MQTTclient.publish("device/boiler/flame/state", String(openThermDev.status.Flame).c_str());
                    MQTTclient.publish("device/boiler/fault", String(openThermDev.status.otFault).c_str());

                    if (openThermDev.status.otFault)
                        MQTTclient.publish("device/boiler/faultCode", String(openThermDev.status.otFaultCode).c_str());
                }

                MQTTclient.publish("device/boiler/communicationStatus", openThermDev.status.communicationStatus);
                MQTTclient.publish("device/boiler/centralHeating/enable", String(openThermDev.settings.enableCentralHeating).c_str());
                MQTTclient.publish("device/boiler/centralHeating/setpoint", String(openThermDev.settings.ch_temperature, 1).c_str());

                MQTTclient.publish("device/boiler/hotWater/enable", String(openThermDev.settings.enableHotWater).c_str());
                MQTTclient.publish("device/boiler/hotWater/setpoint", String(openThermDev.settings.dhw_temperature, 1).c_str());

                MQTTclient.publish("device/roomTemperatureSensor/0001/actual", String(roomTemperature, 1).c_str());
            }
            else
            {
                Serial.println("start reconect...");
                openThermDev.status.mqttFault = 1;
                reconnectMQTT();
            }
        }
    }
    if (WiFi.isConnected())
    {
        webServer.handleClient();
        if (MQTTclient.connected())
        {
            MQTTclient.loop();
        }
    }
    else
    {
        openThermDev.status.wifiFault = 1;
    }
    MDNS.update();
}
