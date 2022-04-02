# 1 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
//******************** blynk settings ***********************




// Comment this out to disable prints and save space

# 9 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
BlynkTimer timer;
int vPinStateFromBlink[128];
int vPinStateToBlink[128];

//****************** arduinoJSON **************************
# 15 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
StaticJsonDocument<800> loraMessege;

//**************** wifi meanger ***************************
# 19 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
WiFiManager wm;
//**************** LoRaNow ********************
# 22 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
byte messageCounter;
uint32_t LoRaId;
int8_t LoRaRSSI;

//**************** NTP **************************
# 28 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
# 29 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
WiFiUDP wifiUdp;
NTP ntp(wifiUdp);

//**************** TFT ***************
# 34 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
//#include <SPI.h>
TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

//******************** OneWire **************************************************
# 39 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
# 40 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2

OneWire oneWire(32);
DallasTemperature DS18B20(&oneWire);
DeviceAddress sensorsAddr[10];

//*********************** MCP23017 IO expander **************************************8
# 47 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
Adafruit_MCP23X17 extIO;


/// ****************************** varables ***********************************************8
unsigned long currentTime = millis();
float sensorsTemp[10];
uint8_t localSensorsCount = 0;

unsigned long Time;
unsigned long freeRam;

//****************** preferences for keep settings ***************
# 60 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
Preferences settings;
//******************* system settings *********************
# 63 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2

devError devErrors;
sysSettings_t sysSettings;
device_t unassigendDeviceArr[3];
String devTypeNames[4] = {"null", "remIOv01", "remTempSensor", "localPorts"};

//***************************** webSerwer *****************************************
# 71 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
# 72 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
# 73 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
# 74 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2

WebServer webServer(80);
const char *serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
    "</form>"
    "<div id='prg'>progress: 0%</div>"
    "<script>"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    " $.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!')"
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>";

//******************** MQTT client *****************************************8
# 115 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
# 116 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2

// Update these with values suitable for your network.
char mqtt_server[20] = "192.168.8.177";
WiFiClient espMQTTClient;
PubSubClient MQTTclient(espMQTTClient);
bool MQTTenabled = false;

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

    char buff[length + 1];
    strncpy(buff, (char *)payload, length);
    buff[length] = '\0';

    String msgTopic = String(topic);
    if (msgTopic == "device/boiler/centralHeating/state")
        Blynk.virtualWrite(100, String(buff).toInt());
    if (msgTopic == "device/boiler/centralHeating/enable")
        Blynk.virtualWrite(101, String(buff).toInt());
    if (msgTopic == "device/boiler/centralHeating/actual")
        Blynk.virtualWrite(102, String(buff).toFloat());
    if (msgTopic == "device/boiler/centralHeating/setpoint")
        Blynk.virtualWrite(103, String(buff).toFloat());

    if (msgTopic == "device/boiler/hotWater/state")
        Blynk.virtualWrite(106, String(buff).toInt());
    if (msgTopic == "device/boiler/hotWater/enable")
        Blynk.virtualWrite(107, String(buff).toInt());
    if (msgTopic == "device/boiler/hotWater/actual")
        Blynk.virtualWrite(108, String(buff).toFloat());
    if (msgTopic == "device/boiler/hotWater/setpoint")
        Blynk.virtualWrite(109, String(buff).toFloat());
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

            //        MQTTclient.subscribe("device/boiler/centralHeating/state");
            //        MQTTclient.subscribe("device/boiler/centralHeating/enable");
            //        MQTTclient.subscribe("device/boiler/centralHeating/actual");
            //        MQTTclient.subscribe("device/boiler/centralHeating/setpoint");

            //       MQTTclient.subscribe("device/boiler/hotWater/state");
            //       MQTTclient.subscribe("device/boiler/hotWater/enable");
            //       MQTTclient.subscribe("device/boiler/hotWater/actual");
            //       MQTTclient.subscribe("device/boiler/hotWater/setpoint");
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
# 237 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
       + (activeIndex == 0 ? activeTag : String("")) +
                           " href='/'>Home</a>  <a "
       + (activeIndex == 1 ? activeTag : String("")) +
                           " href='/dev'>Urządzenia</a>  <a "
       + (activeIndex == 2 ? activeTag : String("")) +
                           " href='/setMqttBrokerForm'>MQTT</a>  <a "
       + (activeIndex == 3 ? activeTag : String("")) +
                           " href='/devUpdate'>Update</a>  <a href='#about'>About</a></div>");


    return header;
}

void handleDevUpdate()
{
    const char *content =
        "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
        "<div style='margin: 30px'>"
        "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
        "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
        "</form>"
        "<div id='prg'>progress: 0%</div>"
        "</div>"
        "<script>"
        "$('form').submit(function(e){"
        "e.preventDefault();"
        "var form = $('#upload_form')[0];"
        "var data = new FormData(form);"
        " $.ajax({"
        "url: '/update',"
        "type: 'POST',"
        "data: data,"
        "contentType: false,"
        "processData:false,"
        "xhr: function() {"
        "var xhr = new window.XMLHttpRequest();"
        "xhr.upload.addEventListener('progress', function(evt) {"
        "if (evt.lengthComputable) {"
        "var per = evt.loaded / evt.total;"
        "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
        "}"
        "}, false);"
        "return xhr;"
        "},"
        "success:function(d, s) {"
        "console.log('success!')"
        "},"
        "error: function (a, b, c) {"
        "}"
        "});"
        "});"
        "</script>";

    String webContent(htmlHeader(3));
    webContent += content;
    webContent += "</body></html>";

    webServer.sendHeader("Connection", "close");
    webServer.send(200, "text/html", webContent);
}

void handleShowDev()
{
    int devIndex = atoi(webServer.arg(0).c_str());
    device_t *selDev = &sysSettings.device[devIndex];
    String webContent = htmlHeader(1) + "<h2><br>Konfiguracja urządzenia: ";
    webContent += String(selDev->description) + "<br></h2>";
    webContent += "<br><br><h3>1-Wire:<br></h3>";
    for (size_t i = 0; (i < 9) && (selDev->oneWireChannel[i].id != 0); i++)
    {
        webContent += "oneWire CH " + String(i) + " Id: " + String(selDev->oneWireChannel[i].id, 16) + " vPin adres: " + String(selDev->oneWireChannel[i].vPinAdrr) + "<br>";
    }
    webContent += "<h3>DI:<br></h3>";
    for (size_t i = 0; i < selDev->DI_PortCfg.count; i++)
    {
        webContent += "DI " + String(i) + " vPin adres: " + String(selDev->DI_PortCfg.vPinAdrr[i]) + "<br>";
    }
    webContent += "<h3>DO:<br></h3>";
    for (size_t i = 0; i < selDev->DO_PortCfg.count; i++)
    {
        webContent += "DO " + String(i) + " vPin adres: " + String(selDev->DO_PortCfg.vPinAdrr[i]) + "<br>";
    }
    webContent += "<h3>AI:<br></h3>";
    for (size_t i = 0; i < selDev->AI_PortCfg.count; i++)
    {
        webContent += "AI " + String(i) + " vPin adres: " + String(selDev->AI_PortCfg.vPinAdrr[i]) + "<br>";
    }
    webContent += "<h3>AO:<br></h3>";
    for (size_t i = 0; i < selDev->AO_PortCfg.count; i++)
    {
        webContent += "AO " + String(i) + " vPin adres: " + String(selDev->AO_PortCfg.vPinAdrr[i]) + "<br>";
    }

    webContent += "</body></html>";
    webServer.send(200, "text/html", webContent);
}

void handleDelDev()
{
    int devIndex = atoi(webServer.arg(0).c_str());
    sysSettings.device[devIndex].description[0] = '\0';
    sysSettings.device[devIndex].id = 0;
    sysSettings.device[devIndex].virtualPinStartAddr = 0;
    sysSettings.device[devIndex].type = (devTypes)0;

    String webContent(htmlHeader(1) + "<h2> Usunięto urządzenie </h2><br><br>   <a href='/dev'> Powrót </a>");

    webContent += "</body></html>";

    webServer.send(200, "text/html", webContent);
}

void handleAddDev()
{

    int devType = atoi(webServer.arg(0).c_str());
    String devDescription(webServer.arg(1));
    int devIndex = atoi(webServer.arg(2).c_str());
    uint32_t devId = strtoul(String(webServer.arg(3)).c_str(), 
# 356 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 3 4
                                                              __null
# 356 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
                                                                  , 16);
    int vPinStart = atoi(webServer.arg(4).c_str());

    clearDevice(devIndex);

    sysSettings.device[devIndex].type = (devTypes)devType;
    if (devDescription.length() > 18)
        devDescription = devDescription.substring(0, 18);
    strcpy(&sysSettings.device[devIndex].description[0], devDescription.c_str());
    sysSettings.device[devIndex].id = devId;
    sysSettings.device[devIndex].virtualPinStartAddr = vPinStart;

    uint8_t i = 0, j = 0, k = 0, l = 0, m = 0;
    for (i = 5, j = 0; i < webServer.args(); j++, i += 2)
    {
        if (webServer.argName(i).startsWith("ChId"))
        {
            sysSettings.device[devIndex].oneWireChannel[j].id = strtoul(webServer.arg(i).c_str(), 
# 373 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 3 4
                                                                                                 __null
# 373 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
                                                                                                     , 16);
            sysSettings.device[devIndex].oneWireChannel[j].vPinAdrr = atoi(webServer.arg(i + 1).c_str());
        }
    }
    for (i = 0, j = 0, k = 0, l = 0, m = 0; i < webServer.args(); i++)
    {
        if (webServer.argName(i).startsWith("DI_vPinAdrr"))
        {
            sysSettings.device[devIndex].DI_PortCfg.vPinAdrr[j] = atoi(webServer.arg(i).c_str());
            j++;
        }
        if (webServer.argName(i).startsWith("DO_vPinAdrr"))
        {
            sysSettings.device[devIndex].DO_PortCfg.vPinAdrr[k] = atoi(webServer.arg(i).c_str());
            k++;
        }
        if (webServer.argName(i).startsWith("AI_vPinAdrr"))
        {
            sysSettings.device[devIndex].AI_PortCfg.vPinAdrr[l] = atoi(webServer.arg(i).c_str());
            l++;
        }
        if (webServer.argName(i).startsWith("AO_vPinAdrr"))
        {
            sysSettings.device[devIndex].AO_PortCfg.vPinAdrr[m] = atoi(webServer.arg(i).c_str());
            m++;
        }
    }
    sysSettings.device[devIndex].DI_PortCfg.count = j;
    sysSettings.device[devIndex].DO_PortCfg.count = k;
    sysSettings.device[devIndex].AI_PortCfg.count = l;
    sysSettings.device[devIndex].AO_PortCfg.count = m;

    // usuniecie dodanego urzadzenia z listy nieprzypisanych urzadzen
    for (size_t i = 0; i < 3; i++)
    {
        if (unassigendDeviceArr[i].id == devId)
            unassigendDeviceArr[i].id = 0;
    }

    String webContent(htmlHeader(1) + " <h2> Dodano urządzenie </h2><br><br>   <a href='/dev'> Powrót </a>");

    webContent += "</body></html>";

    webServer.send(200, "text/html", webContent);
    settings.putBytes("sysSettings", &sysSettings, sizeof(sysSettings));
}

void handleSetMqttBrokerForm()
{
    int unassignedDevIndex = atoi(webServer.arg(0).c_str());
    size_t i;
    // device_t sourceDevice = unassignedDevIndex == -1 ? sysSettings.localPorts : unassigendDeviceArr[unassignedDevIndex];
    for (i = 0; i < 10; i++)
    {
        if (sysSettings.device[i].id == 0)
            break;
    }
    int firstFreeDevIndex = i;
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
    if (!errorAddr)
        strcpy(&sysSettings.MqttBrokerAddress[0], brokerAddr.c_str());

    String webContent(htmlHeader(1) + "<h2> Ustawianie adresu brokera MQTT </h2><br><br>");
    if (errorAddr)
        webContent += "<h3> Operacja zakonczona niepowodzeniem!</h3>";
    else
    {
        webContent += "<h3> Operacja zakonczona powodzeniem</h3>";
        settings.putBytes("sysSettings", &sysSettings, sizeof(sysSettings));
    }
    webContent += "<br><br><a href='/dev'> Powrót </a>";
    webContent += "</body></html>";

    webServer.send(200, "text/html", webContent);
}

void handleAddDevForm()
{
    int unassignedDevIndex = atoi(webServer.arg(0).c_str());
    size_t i;
    // device_t sourceDevice = unassignedDevIndex == -1 ? sysSettings.localPorts : unassigendDeviceArr[unassignedDevIndex];
    for (i = 0; i < 10; i++)
    {
        if (sysSettings.device[i].id == 0)
            break;
    }
    int firstFreeDevIndex = i;
    String webContent(htmlHeader(1) + "<h2> Dodawanie urządzenia </h2>");
    webContent += "<form action='/addDev'><table style='text-align:right'><tr><td>  <label for='devType'>Typ urządzenia:</label> </td><td>  <select name='devType' id='devType'>    <option value='1'>remIOv01</option>    <option value='2'>remTempSensor</option>    <option value='3'>localPorts</option>  </select></td></tr><tr><td>  <label for='Opis'>Opis urządzenia:</label></td><td>  <input type='text' id='Opis' name='Opis'></td></tr><tr><td>  <label for='No'>Index nowego urządzenia:</label></td><td>  <input type='text' id='No' name='No' value='";
# 496 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
    webContent += String(firstFreeDevIndex);
    webContent += "'></td></tr><tr><td>  <label for='Id'>Device Id:</label></td><td>  <input type='text' id='Id' name='Id' value='";


    webContent += String(unassigendDeviceArr[unassignedDevIndex].id, 16);
    webContent += "'></td></tr><tr><td>  <label for='vPin'>Wirtual pin start adres:</label></td><td>  <input type='text' id='vPin' name='vPin'></td></tr>";


    // onewire config part
    for (size_t i = 0; i < 9; i++)
    {
        Serial.println("DevIndex " + String(unassignedDevIndex) + " Channel " + String(i) + " Id :" + String(unassigendDeviceArr[unassignedDevIndex].oneWireChannel[i].id, 16));
        if (unassigendDeviceArr[unassignedDevIndex].oneWireChannel[i].id == 0)
            break;
        String htmlId = "ChId" + String(i);
        webContent += "<tr><td>  <label for='" + htmlId + "'>OneWire Chanell " + String(i) + " Id:</label></td>";
        webContent += "<td>  <input type='text' id='" + htmlId + "' name='" + htmlId + "' value='" + String(unassigendDeviceArr[unassignedDevIndex].oneWireChannel[i].id, 16) + "'></td>";
        htmlId = "vPinAddr" + String(i);
        webContent += "<td>  <label for='" + htmlId + "'> vPin:</label></td>";
        webContent += "<td>  <input type='text' id='" + htmlId + "' name='" + htmlId + "' value='" + String(i) + "'></td></tr>";
    }
    // digital input config part
    for (size_t i = 0; i < unassigendDeviceArr[unassignedDevIndex].DI_PortCfg.count; i++)
    {
        String htmlId = "DI_vPinAdrr" + String(i);
        webContent += "<td>  <label for='" + htmlId + "'> DI " + String(i) + " vPin offeset:</label></td>";
        webContent += "<td>  <input type='text' id='" + htmlId + "' name='" + htmlId + "' value='" + String(i + 10) + "'></td></tr>";
    }
    // digital output config part
    for (size_t i = 0; i < unassigendDeviceArr[unassignedDevIndex].DO_PortCfg.count; i++)
    {
        String htmlId = "DO_vPinAdrr" + String(i);
        webContent += "<td>  <label for='" + htmlId + "'> DO " + String(i) + " vPin offeset:</label></td>";
        webContent += "<td>  <input type='text' id='" + htmlId + "' name='" + htmlId + "' value='" + String(i + 20) + "'></td></tr>";
    }
    // analog input config part
    for (size_t i = 0; i < unassigendDeviceArr[unassignedDevIndex].AI_PortCfg.count; i++)
    {
        String htmlId = "AI_vPinAdrr" + String(i);
        webContent += "<td>  <label for='" + htmlId + "'> AI " + String(i) + " vPin offeset:</label></td>";
        webContent += "<td>  <input type='text' id='" + htmlId + "' name='" + htmlId + "' value='" + String(i + 30) + "'></td></tr>";
    }
    // analog input config part
    for (size_t i = 0; i < unassigendDeviceArr[unassignedDevIndex].AO_PortCfg.count; i++)
    {
        String htmlId = "AO_vPinAdrr" + String(i);
        webContent += "<td>  <label for='" + htmlId + "'> AO " + String(i) + " vPin offeset:</label></td>";
        webContent += "<td>  <input type='text' id='" + htmlId + "' name='" + htmlId + "' value='" + String(i + 40) + "'></td></tr>";
    }

    webContent += "<tr><td></td><td><input type='submit' value='Dodaj'></td></tr></table></form>";


    webContent += "</body></html>";

    webServer.send(200, "text/html", webContent);
}

void handleDevList()
{
    String webContent(htmlHeader(1) + " <h2> Aktualna konfiguracja systemu </h2>");
    // tablica urzadzen systemu
    webContent += "<br><table  style='width:600px'>    <caption>Lista zarejestrowanych urządzeń w sytemie</caption>  <tr>    <th>No</th>    <th>Typ</th>    <th>Id</th>    <th>Opis</th>    <th>vPin Start</th>    <th>Akcja</th>  </tr>";
# 568 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
    /*

    webContent += "<tr><td>0</td>";

    webContent += "<td>" + devTypeNames[sysSettings.localPorts.type] + "</td>";

    webContent += "<td>" + String(sysSettings.localPorts.id, HEX) + "</td>";

    webContent += "<td>" + String(&sysSettings.localPorts.description[0]) + "</td>";

    webContent += "<td>" + String(sysSettings.localPorts.virtualPinStartAddr) + "</td>";

    webContent += "<td>" + String("<a href='/addDevForm?No=-1'>edytuj</a>") + "</td></tr>";*/
# 575 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
    for (size_t i = 0; i < 10; i++)
    {
        if (sysSettings.device[i].id != 0)
        {
            webContent += "<tr><td>" + String(i + 1) + "</td>";
            webContent += "<td>" + devTypeNames[sysSettings.device[i].type] + "</td>";
            webContent += "<td>" + String(sysSettings.device[i].id, 16) + "</td>";
            webContent += "<td>" + String(&sysSettings.device[i].description[0]) + "</td>";
            webContent += "<td>" + String(sysSettings.device[i].virtualPinStartAddr) + "</td>";
            webContent += "<td>" + String("<a href='/showDev?No=" + String(i) + "'>pokaż</a>&nbsp;&nbsp;<a href='/delDev?No=" + String(i) + "'>usun</a>") + "</td></tr>";
        }
    }
    webContent += "</table>";
    // tablica nie zarejtorwanych urzadzen
    webContent += "<br><br><table style='width:600px'>    <caption>Lista niezarejestowanych urządzeń w sytemie</caption>  <tr>    <th>No</th>    <th>Id</th>    <th>Akcja</th>  </tr>";






    for (size_t i = 0; i < 3; i++)
    {
        if (unassigendDeviceArr[i].id != 0)
        {
            webContent += "<tr><td>" + String(i) + "</td>";
            webContent += "<td>" + String(unassigendDeviceArr[i].id, 16) + "</td>";
            webContent += "<td>" + String("<a href='/addDevForm?No=" + String(i) + "'>dodaj</a>") + "</td></tr>";
        }
    }
    webContent += "</table>";
    webContent += "</body>    </html>";

    webServer.send(200, "text/html", webContent);
}

void handleRoot()
{

    String webContent = htmlHeader(0);
    webContent += "<h2 style='margin: 20px'>Status:</h2>        <table>            <tr>                <th>Parametr</th>                <th>Wartość</th>            </tr>";





    /*    webContent += "<tr>                    <td>Ogrzewanie</td>                    <td>" +
d>" +
                      String(openThermDev.settings.enableCentralHeating ? "Załączone" : "Wyłączone") + "</td>                </tr>";
";
        webContent += "<tr>                    <td>Ogrzewanie Temperatura Zadana</td>                    <td>" +
d>" +
                      String(openThermDev.settings.ch_temperature, 1) + "  °C</td>                </tr>";
";
        webContent += "<tr>                    <td>Ciepła woda</td>                    <td>" +
d>" +
                      String(openThermDev.settings.enableHotWater ? "Załączona" : "Wyłączona") + "</td>                </tr>";
";
        webContent += "<tr>                    <td>Ciepła Woda Temperatura Zadana</td>                    <td>" +
d>" +
                      String(openThermDev.settings.dhw_temperature, 1) + "  °C</td>                </tr></table>";
";


        // status urzadzenia

        webContent += "<h2 style='margin: 20px'>Status:</h2>            <table>                <tr>                    <th>Parametr</th>                    <th>Wartość</th>                </tr>";
       </tr>";
        webContent += "<tr>                    <td>Ogrzewanie</td>                    <td>" +
d>" +
                      String(openThermDev.status.CentralHeating ? "Aktywne" : "Nieaktywne") + "</td>                </tr>";
";
        webContent += "<tr>                    <td>Ogrzewanie Aktualna Temperatura</td>                    <td>" +
d>" +
                      String(openThermDev.status.ch_temperature, 1) + " °C</td>                </tr>";
";
        webContent += "<tr>                    <td>Ciepła woda</td>                    <td>" +
d>" +
                      String(openThermDev.status.HotWater ? "Aktywna" : "Nieaktywna") + "</td>                </tr>";
";
        webContent += "<tr>                    <td>Ciepła Woda Aktualna Temperatura</td>                    <td>" +
d>" +
                      String(openThermDev.status.dhw_temperature, 1) + "  °C</td>                </tr>";
";
        webContent += "<tr>                    <td>Aktualne ciśnienie</td>                    <td>" +
d>" +
                      String(openThermDev.status.pressure, 1) + "  bar</td>                </tr>";
";
        webContent += "<tr>                    <td>Aktualna modulacja</td>                    <td>" +
d>" +
                      String(openThermDev.status.modulation, 1) + "  %</td>                </tr>";
";
        webContent += "<tr>                    <td>Płomień</td>                    <td>" +
d>" +
                      String(openThermDev.status.Flame ? "Załączony" : "Wyłączony") + "</td>                </tr>";
";
        webContent += "<tr>                    <td>Status Komunikacji Piec</td>                    <td>" +
d>" +
                      String(openThermDev.status.communicationStatus) + "</td>                </tr>";*/
# 689 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
    webContent += "<tr>                <td>Błąd Komunikacji Wifi</td>                <td>"

                      +
                  String(devErrors.wifiError ? (devErrors.wifiError == 1 ? "AKTYWNY" : "NIEAKTYWNY") : "BRAK") + "</td>            </tr>";

    webContent += "<tr>                <td>Błąd Komunikacji MQTT </td>                <td>"

                      +
                  String(devErrors.mqttError ? (devErrors.mqttError == 1 ? "AKTYWNY" : "NIEAKTYWNY") : "BRAK") + "</td>            </tr>";

    webContent += "<tr>                <td>Błąd Komunikacji LORA</td>                <td>"

                      +
                  String(devErrors.loraDevError ? (devErrors.loraDevError == 1 ? "AKTYWNY" : "NIEAKTYWNY") : "BRAK") + "</td>            </tr>";

    webContent += "<tr>                <td>Błąd lokalne IO</td>                <td>"

                      +
                  String(devErrors.extIoError ? (devErrors.extIoError == 1 ? "AKTYWNY" : "NIEAKTYWNY") : "BRAK") + "</td>            </tr>";

    webContent += "<tr>                <td>Błąd Komunikacji blynk</td>                <td>"

                      +
                  String(devErrors.blynkError ? (devErrors.blynkError == 1 ? "AKTYWNY" : "NIEAKTYWNY") : "BRAK") + "</td>            </tr>";

    webContent += "<tr>                <td>Błąd 1-wire </td>                <td>"

                      +
                  String(devErrors.localSensorError ? (devErrors.localSensorError == 1 ? "AKTYWNY" : "NIEAKTYWNY") : "BRAK") + "</td>            </tr>";

    webContent += "</table>";
    if (devErrors.errorActive)
        webContent += "<form action='/resetErrors'><table style='border: 0px;text-align:right'><tr><td>  <label>Aktywne błędny na urządzeniu</label></td><td><input type='submit' value='Resetuj'></td></tr></table></form>";





    webContent += "</body>        </html>";

    webServer.send(200, "text/html", webContent);
}

void handleResetErrors()
{
    devErrors.clearErrors();
    handleRoot();
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

void drawGraph()
{
    String out = "";
    char temp[100];
    out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
    out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
    out += "<g stroke=\"black\">\n";
    int y = rand() % 130;
    for (int x = 10; x < 390; x += 10)
    {
        int y2 = rand() % 130;
        sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
        out += temp;
        y = y2;
    }
    out += "</g>\n</svg>\n";

    webServer.send(200, "image/svg+xml", out);
}

uint8_t countTagElements(String input, String tag)
{
    String startTag = "\"" + tag + "\":[";
    String endTag = "]";
    String missing = "";
    int startIndex = input.indexOf(startTag);
    int endIndex = input.indexOf(endTag, startIndex + startTag.length());
    if (startIndex >= 0 && endIndex >= 0)
    {
        String value = input.substring(startIndex, endIndex);
        uint8_t valuesCount = 1;
        for (unsigned int i = 0; i < value.length(); i++)
        {
            if (value.charAt(i) == ',')
                valuesCount++;
        }
        return valuesCount;
    }
    else
    {
        return 0;
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(35, 0x05);
    //***************************** settings for TFT *********************************************
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(0x0000 /*   0,   0,   0 */);
    tft.setTextSize(4);
    tft.setTextColor(0x07E0 /*   0, 255,   0 */);
    tft.setCursor(0, 0);
    tft.setTextDatum(4 /* Middle centre*/);
    tft.setTextSize(2);
    tftMessage(String("TFT OK!"), 0x07E0 /*   0, 255,   0 */, 0x0000 /*   0,   0,   0 */, 2000);

    WiFi.mode(WIFI_MODE_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:

    // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    wm.setConnectRetries(5);
    wm.setWiFiAutoReconnect(true);
    res = wm.autoConnect("AutoConnectAP", "password"); // password protected ap
    if (!res)
    {
        Serial.println("Failed to connect");
        tftMessage(String("Wifi Error: Reset"), 0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */, 4000);
        ESP.restart();
    }
    else
    {
        // if you get here you have connected to the WiFi
        Serial.println("connected :)");
        tftMessage(String("Wifi OK!"), 0x07E0 /*   0, 255,   0 */, 0x0000 /*   0,   0,   0 */, 1000);
        tftMessage(String("IP: ") + String(WiFi.localIP()[0]) + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3], 0x07E0 /*   0, 255,   0 */, 0x0000 /*   0,   0,   0 */, 4000);
    }

    //*************************** setttings for lora *****************************************************
    Serial.println("LoRaNow Simple Gateway");

    // LoRaNow.setFrequencyCN(); // Select the frequency 486.5 MHz - Used in China
    //  LoRaNow.setFrequencyEU(); // Select the frequency 868.3 MHz - Used in Europe
    //  LoRaNow.setFrequencyUS(); // Select the frequency 904.1 MHz - Used in USA, Canada and South America
    //  LoRaNow.setFrequencyAU(); // Select the frequency 917.0 MHz - Used in Australia, Brazil and Chile

    LoRaNow.setFrequency(433E6);
    LoRaNow.setSpreadingFactor(7);
    // LoRaNow.setPins(10, 2);

    // LoRaNow.setPinsSPI(sck, miso, mosi, ss, dio0); // Only works with ESP32
    LoRaNow.setPinsSPI(12, 15, 13, 17, 2); // Only works with ESP32

    if (!LoRaNow.begin())
    {
        Serial.println("LoRa init failed. Check your connections.");
        tftMessage(String("Lora Error: Reset"), 0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */, 4000);
        ESP.restart();
    }

    LoRaNow.onMessage(onMessage);
    LoRaNow.gateway();

    // Send data to the node
    LoRaNow.clear();
    LoRaNow.print("Gateway started");
    LoRaNow.print(millis());
    LoRaNow.send();

    ////********************************* BLYNK **********************************************
    Blynk.config("YwTvXvw9IQdzqzZJ3HHvj3HkjvZpfHQd");
    long time = millis();
    while ((Blynk.connect() == false) && ((millis() - time) < 10000))
    {
        /* code */
    }
    if ((millis() - time) > 10000)
    {
        Serial.println("Nie udalo sie połączyć z blynkiem");
        tftMessage(String("Brak odpowiedzi z Blynk: Reset"), 0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */, 4000);
        ESP.restart();
    }
    timer.setInterval(1000L, myTimerEvent);
    Blynk.syncAll();

    //**************************  NTP ********************************

    ntp.ruleDST("CEST", Last, Sun, Mar, 2, 120); // last sunday in march 2:00, timetone +120min (+1 GMT + 1h summertime offset)
    ntp.ruleSTD("CET", Last, Sun, Oct, 3, 60); // last sunday in october 3:00, timezone +60min (+1 GMT)
    ntp.begin();

    //******************** info *******************************************************8
    int i = -1;
    do
    {
        i++;
    } while (DS18B20.getAddress(sensorsAddr[i], i));
    tftMessage(String("Lokalnie ") + String(i) + (i == 1 ? String(" czujnik") : String(" czujniki")), 0x07E0 /*   0, 255,   0 */, 0x0000 /*   0,   0,   0 */, 2000);

    //*********************** MCP23017 - extIO ******************************
    if (!extIO.begin_I2C())
    {
        tftMessage(String("extIO Error!!!"), 0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */, 4000);
        ESP.restart();
    }
    for (i = 0; i < 16; i++)
        i < 8 ? extIO.pinMode(i, 0x05) : extIO.pinMode(i, 0x02);

    // ************************ webSerwer *******************************888
    if (MDNS.begin("esp32"))
    {
        Serial.println("MDNS responder started");
    }

    webServer.on("/", handleRoot);
    webServer.on("/dev", handleDevList);
    webServer.on("/addDevForm", handleAddDevForm);
    webServer.on("/setMqttBrokerForm", handleSetMqttBrokerForm);
    webServer.on("/setMqttBroker", handleSetMqttBroker);
    webServer.on("/addDev", handleAddDev);
    webServer.on("/delDev", handleDelDev);
    webServer.on("/showDev", handleShowDev);
    webServer.on("/resetErrors", handleResetErrors);
    webServer.on("/test.svg", drawGraph);
    webServer.on("/inline", []()
                 { webServer.send(200, "text/plain", "this works as well"); });
    webServer.onNotFound(handleNotFound);

    webServer.on("/devUpdate", HTTP_GET, handleDevUpdate);
    webServer.on(
        "/update", HTTP_POST, []()
        {
    webServer.sendHeader("Connection", "close");
    webServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); },
        []()
        {
            HTTPUpload &upload = webServer.upload();
            LoRa.idle();
            if (upload.status == UPLOAD_FILE_START)
            {
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin(0xFFFFFFFF))
                { // start with max available size
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE)
            {
                /* flashing firmware to ESP*/
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_END)
            {
                if (Update.end(true))
                { // true to set the size to the current progress
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                }
                else
                {
                    Update.printError(Serial);
                }
            }
        });
    webServer.begin();

    //********************** prefferences - sysSettings
    settings.begin("sysSettings", false);
    // Serial.println("sys dev count: " + String(sysSettings.sysDeviceCount));
    //          uncomment to reset settings
    // settings.putBytes("sysSettings", &sysSettings, sizeof(sysSettings));
    size_t settingsLen = settings.getBytesLength("sysSettings");
    char buffer[settingsLen]; // prepare a buffer for the data
    settings.getBytes("sysSettings", buffer, settingsLen);
    if ((settingsLen % sizeof(sysSettings_t)) || (settingsLen == 0))
    { // simple check that data fits
        tftMessage(String("error read sys data!!!"), 0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */, 4000);
        ESP.restart();
    }
    else
    {
        tftMessage(String("sys data OK :)"), 0x07E0 /*   0, 255,   0 */, 0x0000 /*   0,   0,   0 */, 2000);
        memcpy(&sysSettings, buffer, sizeof(sysSettings_t));
    }
    //****************************** MQTT settings ***************************88
    if (String(&sysSettings.MqttBrokerAddress[0]).length() > 8)
    {
        MQTTenabled = true;
        strcpy(mqtt_server, &sysSettings.MqttBrokerAddress[0]);
        MQTTclient.setServer(mqtt_server, 1883);
        MQTTclient.setCallback(MQTTmsgRcvCallback);
        MQTTclient.setBufferSize(512);
        reconnectMQTT();
    }
    // check ram
    Time = millis();
    freeRam = ESP.getFreeHeap();
}

void loop()
{
    // put your main code here, to run repeatedly:

    LoRaNow.loop();
    if (Blynk.connected())
    {
        Blynk.run();
        if (devErrors.blynkError == 1)
            devErrors.blynkError = 2;
    }
    else
    {
        devErrors.blynkError = 1;
    }
    timer.run();
    if (WiFi.isConnected())
    {
        webServer.handleClient();
        if (devErrors.wifiError == 1)
            devErrors.wifiError = 2;
    }
    else
        devErrors.wifiError = 1;
    // local mesurement
    if ((millis() - currentTime) > 10000)
    {

        mesureTemperatures();
        sendLocalDataToBlynk();

        currentTime = millis();
        if (ESP.getFreeHeap() != freeRam)
        {
            freeRam = ESP.getFreeHeap();
            Serial.print("RAM:");
            Serial.println(freeRam);
        }
    }

    // check if some errors ocured
    if ((millis() - Time) > 5000)
    {
        Time = millis();
        if (MQTTenabled)
        {
            if (!MQTTclient.connected())
            {
                reconnectMQTT();
                devErrors.mqttError = 1;
            }
            else
            {
                MQTTclient.publish("device/boiler/centralHeating/enable/remote", vPinStateFromBlink[20] ? "1" : "0");
                MQTTclient.publish("device/boiler/hotWater/enable/remote", vPinStateFromBlink[19] ? "1" : "0");
                MQTTclient.loop();
                if (devErrors.mqttError == 1)
                    devErrors.mqttError = 2;
            }
        }

        if (!Blynk.connected())
            Blynk.connect();
        devErrors.checkLoraPing(sysSettings, millis());
        if (!extIO.begin_I2C())
            devErrors.extIoError = 1;
        else
        {
            if (devErrors.extIoError == 1)
                devErrors.extIoError = 2;
        }
        devErrors.checkError();
        if (WiFi.status() == WL_CONNECTED)
            ntp.update();
        parseFormula("1 50 or 82 and 18 20");
        parseFormula("2 40 42 21");
        parseFormula("3 20 11 20 13 6 22");
        parseFormula("4 58 59 20 21 4 8 8 23");
    }

    // reset bledow
    // if (touchRead(T7) < 30)
    //    devErrors.clearErrors();

    displayData("");
    delay(2);
}

void mesureTemperatures(void)
{
    // Pomiar temperatur
    DS18B20.requestTemperatures();
    // delay(200);
    int i = -1;
    do
    {
        i++;
    } while (DS18B20.getAddress(sensorsAddr[i], i));
    i--;
    int j = i;
    String message = "Temperatury:";
    for (j = 0; j <= i; j++)
    {
        sensorsTemp[j] = DS18B20.getTempC(sensorsAddr[j]);
        message += " " + String(sensorsTemp[j]);
    }
    localSensorsCount = i + 1;
    // check if local ports are in configuration if not adding local oneWire ids
    for (i = 0; i < 9; i++)
    {
        if (sysSettings.device[i].id == 1)
            break;
    }
    // jezeli nie ma na liscie lokalnych portow to dodanie do listy nie przypisanych urzadzen
    if (i == 9)
    {
        for (j = 0; j < 3; j++)
        {
            if (unassigendDeviceArr[j].id == 1)
                break;
            if (unassigendDeviceArr[j].id != 0)
                continue;
            else
            {
                unassigendDeviceArr[j].id = 1;
                if (localSensorsCount > 0)
                {
                    for (size_t k = 0; (k < localSensorsCount) && (k < 9); k++)
                        unassigendDeviceArr[j].oneWireChannel[k].id = strtoul(ds18b20AddressToStr(sensorsAddr[k]).c_str(), 
# 1144 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 3 4
                                                                                                                          __null
# 1144 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
                                                                                                                              , 16);
                }
                unassigendDeviceArr[j].DI_PortCfg.count = 8;
                unassigendDeviceArr[j].DO_PortCfg.count = 8;
                unassigendDeviceArr[j].AI_PortCfg.count = 3;
                unassigendDeviceArr[j].AO_PortCfg.count = 2;
            }

            if (unassigendDeviceArr[j].id == 1)
                break;
        }
    }

    Serial.println(message);

    // erorr handle
    uint8_t sysLocalOneWireSensors = 0;
    for (size_t i = 0; i < sysSettings.SystemMaxDevCount; i++)
    {
        if (sysSettings.device[i].type == localPorts)
        {
            for (size_t j = 0; j < 9; j++)
            {
                if (sysSettings.device[i].oneWireChannel[j].id != 0)
                    sysLocalOneWireSensors++;
            }
        }
    }
    if (sysLocalOneWireSensors != localSensorsCount)
        devErrors.localSensorError = 1;
    else if (devErrors.localSensorError == 1)
        devErrors.localSensorError = 2;
}

void clearDevice(int devIndex)
{
    // clearing dev
    sysSettings.device[devIndex].type = noneDev;
    sysSettings.device[devIndex].virtualPinStartAddr = 0;
    sysSettings.device[devIndex].id = 0;
    for (size_t i = 0; i < 9; i++)
    {
        sysSettings.device[devIndex].oneWireChannel[i].id = 0;
        sysSettings.device[devIndex].oneWireChannel[i].vPinAdrr = 0;
    }
    for (size_t i = 0; i < 8; i++)
    {
        sysSettings.device[devIndex].DI_PortCfg.vPinAdrr[i] = 0;
        sysSettings.device[devIndex].DO_PortCfg.vPinAdrr[i] = 0;
        sysSettings.device[devIndex].AI_PortCfg.vPinAdrr[i] = 0;
        sysSettings.device[devIndex].AO_PortCfg.vPinAdrr[i] = 0;
    }
    sysSettings.device[devIndex].DI_PortCfg.count = 0;
    sysSettings.device[devIndex].DO_PortCfg.count = 0;
    sysSettings.device[devIndex].AI_PortCfg.count = 0;
    sysSettings.device[devIndex].AO_PortCfg.count = 0;
}

void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        // zero pad the address if necessary
        if (deviceAddress[i] < 16)
            Serial.print("0");
        Serial.print(deviceAddress[i], 16);
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

void onMessage(uint8_t *buffer, size_t size)
{
    messageCounter = LoRaNow.count();
    LoRaRSSI = LoRaNow.getRSSI();
    LoRaId = LoRaNow.id();

    Serial.print("Node Id: ");
    Serial.println(LoRaId, 16);
    Serial.print("Count: ");
    Serial.println(messageCounter);
    Serial.print("RSSI: ");
    Serial.println(LoRaRSSI);
    Serial.print("Message: ");
    Serial.write(buffer, size);
    Serial.println();
    Serial.println();

    String data = String((char *)buffer);
    LoRaNow.clear();
    LoRaNow.print("Gateway: ");
    LoRaNow.print(millis());
    LoRaNow.send();
    ///************ parse recieved data ****************************
    DeserializationError error = deserializeJson(loraMessege, data);
    if (error)
    {
        Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("deserializeJson() failed: ")))));
        Serial.println(error.f_str());
        displayData(String("LoRa: error deserializacji"));
    }
    else
    {

        int dis = 0;
        if (loraMessege.containsKey("CHs"))
        {
            if (loraMessege["CHs"] == 0)
            {
                displayData(String("LoRa: brak czujnikow"));
            }
        }
        else
        {
            if (loraMessege.containsKey("ChValues"))
            {
                int CHs = countTagElements(data, "ChValues");
                loraMessege["CHs"] = CHs;
            }
        }
        if (loraMessege.containsKey("DI"))
        {
            int dis = countTagElements(data, "DI");
            loraMessege["DIs"] = dis;
        }
        if (loraMessege.containsKey("DO"))
        {
            loraMessege["DOs"] = countTagElements(data, "DO");
            int dos = loraMessege["DOs"];
        }
        if (loraMessege.containsKey("AI"))
        {
            loraMessege["AIs"] = countTagElements(data, "AI");
            int ais = loraMessege["AIs"];
        }
        if (loraMessege.containsKey("AO"))
        {
            loraMessege["AOs"] = countTagElements(data, "AO");
            int aos = loraMessege["AOs"];
        }

        displayData("Odebrano dane");
    }

    // sprawdzenie czy urzadzenie jest na liscie
    size_t i, j;
    for (i = 0; i < 10; i++)
    {
        if (LoRaNow.id() == sysSettings.device[i].id)
            break;
    }
    // jezeli urzadzenia nie ma na liscie
    if (i == 10)
    {
        for (j = 0; j < 3; j++)
        {
            bool _break = false;
            for (i = 0; i < 3; i++)
                if (LoRaNow.id() == unassigendDeviceArr[i].id)
                    _break = true;

            if (_break)
                break;
            if (unassigendDeviceArr[j].id != 0)
                continue;
            if (unassigendDeviceArr[j].id == 0)
            {
                Serial.println("dodaje urzadzenie indeks: " + String(j));
                unassigendDeviceArr[j].id = LoRaNow.id();
                if (loraMessege.containsKey("CHs"))
                {
                    int CHs = loraMessege["CHs"];
                    for (size_t k = 0; (k < CHs) && (k < 9); k++)
                        unassigendDeviceArr[j].oneWireChannel[k].id = strtoul(loraMessege["ChIds"][k], 
# 1327 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 3 4
                                                                                                      __null
# 1327 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
                                                                                                          , 16);
                }
                if (loraMessege.containsKey("DIs"))
                    unassigendDeviceArr[j].DI_PortCfg.count = loraMessege["DIs"];
                else
                    unassigendDeviceArr[j].DI_PortCfg.count = 0;
                if (loraMessege.containsKey("DOs"))
                    unassigendDeviceArr[j].DO_PortCfg.count = loraMessege["DOs"];
                else
                    unassigendDeviceArr[j].DO_PortCfg.count = 0;
                if (loraMessege.containsKey("AIs"))
                    unassigendDeviceArr[j].AI_PortCfg.count = loraMessege["AIs"];
                else
                    unassigendDeviceArr[j].AI_PortCfg.count = 0;
                if (loraMessege.containsKey("AOs"))
                    unassigendDeviceArr[j].AO_PortCfg.count = loraMessege["AOs"];
                else
                    unassigendDeviceArr[j].AO_PortCfg.count = 0;
            }

            if (LoRaNow.id() == unassigendDeviceArr[j].id)
                break;
        }
    }
    else // jezeli urzadzenie jest na liscie wyslij dane do blynk
    {
        sendDataToBlynk();
        sendDataToMQTT(i, data);
        devErrors.loraLastPing[i] = millis();
    }
}

void sendDataToMQTT(int devIndex, String &data)
{
    String topic = String("device/");
    topic += String(sysSettings.device[devIndex].description);
    MQTTclient.publish(topic.c_str(), data.c_str());
}

void sendLocalDataToBlynk(void)
{

    // wyszukanie numeru lokalnego portu
    int8_t i, devIndex = -1;
    for (i = 0; i < 10; i++)
    {
        if (sysSettings.device[i].type == localPorts)
        {
            devIndex = i;
            break;
        }
    }
    // jezeli znaleziono lokalne urzadzenie
    if (devIndex >= 0)
    {
        // send local data
        uint8_t i;
        if (localSensorsCount > 0)
        {
            for (i = 0; (i < 9) && (i < localSensorsCount) && (sysSettings.device[devIndex].oneWireChannel[i].id != 0); i++)
            {
                int8_t sensorIndex = -1;
                for (size_t j = 0; j < localSensorsCount; j++)
                {
                    if (sysSettings.device[devIndex].oneWireChannel[i].id == strtoul(ds18b20AddressToStr(sensorsAddr[j]).c_str(), 
# 1391 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 3 4
                                                                                                                                 __null
# 1391 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
                                                                                                                                     , 16))
                    {
                        sensorIndex = j;
                        break;
                    }
                }
                if (sensorIndex >= 0)
                {
                    Blynk.virtualWrite(sysSettings.device[devIndex].virtualPinStartAddr + sysSettings.device[devIndex].oneWireChannel[i].vPinAdrr, sensorsTemp[sensorIndex]);
                }
            }
        }
        for (i = 0; i < sysSettings.device[devIndex].DI_PortCfg.count; i++)
        {
            int vPinAdr = sysSettings.device[devIndex].virtualPinStartAddr + sysSettings.device[devIndex].DI_PortCfg.vPinAdrr[i];
            int vPinState = extIO.digitalRead(i) ? 0 : 1;
            Blynk.virtualWrite(vPinAdr, vPinState);
            vPinStateFromBlink[vPinAdr] = vPinState;
        }
        for (i = 0; i < sysSettings.device[devIndex].DO_PortCfg.count; i++)
        {
            extIO.digitalWrite(8 + i, vPinStateFromBlink[sysSettings.device[devIndex].virtualPinStartAddr + sysSettings.device[devIndex].DO_PortCfg.vPinAdrr[i]] == 0 ? 0 : 1);
            ;
        }
    }
}

void sendDataToBlynk(void)
{
    int8_t i, devIndex = -1;
    for (i = 0; i < 10; i++)
    {
        if (LoRaNow.id() == sysSettings.device[i].id)
        {
            devIndex = i;
            break;
        }
    }
    // jezeli znaleziono urzadzenie
    if (devIndex >= 0)
    { // jezeli posiada dane 1-wire
        if (loraMessege.containsKey("ChValues") && loraMessege.containsKey("CHs") && loraMessege.containsKey("ChIds"))
        {
            for (i = 0; (i < 9) && (sysSettings.device[devIndex].oneWireChannel[i].id != 0); i++)
            {
                int8_t loraIndex = -1;
                for (size_t j = 0; j < loraMessege["CHs"]; j++)
                {
                    if (sysSettings.device[devIndex].oneWireChannel[i].id == strtoul(loraMessege["ChIds"][j], 
# 1439 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 3 4
                                                                                                             __null
# 1439 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
                                                                                                                 , 16))
                    {
                        loraIndex = j;
                        break;
                    }
                }
                if (loraIndex >= 0)
                {
                    Blynk.virtualWrite(sysSettings.device[devIndex].virtualPinStartAddr + sysSettings.device[devIndex].oneWireChannel[i].vPinAdrr, float(loraMessege["ChValues"][loraIndex]));
                }
            }
        }
        // jezeli posiada dane wejsc cyforwych
        if (loraMessege.containsKey("DI"))
        {
            for (i = 0; i < sysSettings.device[devIndex].DI_PortCfg.count; i++)
            {

                int vPinAdr = sysSettings.device[devIndex].virtualPinStartAddr + sysSettings.device[devIndex].DI_PortCfg.vPinAdrr[i];
                int vPinState = int(loraMessege["DI"][i]) ? 0 : 1;
                Blynk.virtualWrite(vPinAdr, vPinState);
                vPinStateFromBlink[vPinAdr] = vPinState;
            }
        }
        // jezeli posiada dane wejsc analogowych
        if (loraMessege.containsKey("AI"))
        {
            for (i = 0; i < sysSettings.device[devIndex].AI_PortCfg.count; i++)
            {
                Blynk.virtualWrite(sysSettings.device[devIndex].virtualPinStartAddr + sysSettings.device[devIndex].AI_PortCfg.vPinAdrr[i], int(loraMessege["AI"][i]));
            }
        }
    }
}

void displayData(const String &text)
{
    static bool firstRun = true;
    static unsigned long time = millis();
    if ((millis() - time) > 1000 or (text.length() > 1))
    {
        if (firstRun)
        {
            tft.fillScreen(0x0000 /*   0,   0,   0 */);
            firstRun = false;
        }
        time = millis();
        tft.setTextDatum(6 /* Bottom left*/);
        /*    if (text.length() > 1)

            {

                tft.fillRect(30, 0, 185, 19, TFT_BLACK);

                tft.drawString(text, 40, 5);

            }*/
# 1493 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
        // tft.fillRect(30, 0, 185, 19, TFT_BLACK);
        tft.drawString("RAM:" + String((float)freeRam / 5200.0, 0) + "%", 30, 10);
        tft.drawString("Wifi:" + String(WiFi.RSSI()), 120, 10);

        // time
        tft.setTextSize(4);
        tft.drawString(ntp.formattedTime("%R"), 42, 80);
        tft.setTextSize(3);
        tft.drawString(ntp.formattedTime("%S"), 165, 80);
        tft.setTextSize(2);

        uint16_t onColor = 0xFDA0 /* 255, 180,   0 */, fillColor;
        uint16_t offColor = 0x8410; // gray
        // input state
        uint8_t statePortIn = extIO.readGPIOA();
        tft.setTextColor(0x0000 /*   0,   0,   0 */, 0xFDA0 /* 255, 180,   0 */);
        for (size_t i = 0; i < 8; i++)
        {
            if (statePortIn & (1 << i))
            {
                tft.setTextColor(0x0000 /*   0,   0,   0 */, offColor);
                fillColor = offColor;
            }
            else
            {
                tft.setTextColor(0x0000 /*   0,   0,   0 */, onColor);
                fillColor = onColor;
            }
            tft.fillRoundRect(1, 16 * i + 4, 24, 15, 4, fillColor);
            tft.drawString(String(i + 1), 8, 20 + i * 16);
        }
        // output state
        // tft.setTextColor(TFT_BLACK, TFT_ORANGE);
        uint8_t statePortOut = extIO.readGPIOB();
        for (size_t i = 0; i < 8; i++)
        {
            if (statePortOut & (1 << i))
            {
                tft.setTextColor(0x0000 /*   0,   0,   0 */, onColor);
                fillColor = onColor;
            }
            else
            {
                tft.setTextColor(0x0000 /*   0,   0,   0 */, offColor);
                fillColor = offColor;
            }
            tft.fillRoundRect(215, 16 * i + 4, 24, 15, 4, fillColor);
            tft.drawString(String(i + 1), 222, 20 + i * 16);
        }
        tft.setTextColor(0x07E0 /*   0, 255,   0 */, 0x0000 /*   0,   0,   0 */);

        /*

                tft.drawString("Id: " + String(LoRaId, HEX), 10, 30); // + " °C");

                tft.drawString("No: " + String(messageCounter), 10, 50);

                tft.drawString("RSSI: " + String(LoRaRSSI), 10, 70);

                if (loraMessege.containsKey("CHs"))

                {

                    int sensors = loraMessege["CHs"];

                    tft.drawString("Sensors: " + String(sensors), 10, 90);

                }

                else

                    tft.drawString("No sensors!", 10, 90);

        */
# 1556 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
        // botom information panell
        // wifi state
        if (devErrors.wifiError == 1)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */);
        else if (devErrors.wifiError == 2)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xFDA0 /* 255, 180,   0 */);
        else
            tft.setTextColor(0x001F /*   0,   0, 255 */, 0x07E0 /*   0, 255,   0 */);
        tft.drawString("WF", 30, 133);
        // lora state
        if (devErrors.loraDevError == 1)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */);
        else if (devErrors.loraDevError == 2)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xFDA0 /* 255, 180,   0 */);
        else
            tft.setTextColor(0x001F /*   0,   0, 255 */, 0x07E0 /*   0, 255,   0 */);
        tft.drawString("LR", 59, 133);
        // MQTT state
        if (devErrors.mqttError == 1)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */);
        else if (devErrors.mqttError == 2)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xFDA0 /* 255, 180,   0 */);
        else
            tft.setTextColor(0x001F /*   0,   0, 255 */, 0x07E0 /*   0, 255,   0 */);
        tft.drawString("MQ", 86, 133);
        // Blynk state
        if (devErrors.blynkError == 1)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */);
        else if (devErrors.blynkError == 2)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xFDA0 /* 255, 180,   0 */);
        else
            tft.setTextColor(0x001F /*   0,   0, 255 */, 0x07E0 /*   0, 255,   0 */);
        tft.drawString("BLK", 116, 133);
        // ext IO
        if (devErrors.extIoError == 1)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */);
        else if (devErrors.extIoError == 2)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xFDA0 /* 255, 180,   0 */);
        else
            tft.setTextColor(0x001F /*   0,   0, 255 */, 0x07E0 /*   0, 255,   0 */);
        tft.drawString("IO", 157, 133);
        // local DS18b20
        if (devErrors.localSensorError == 1)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xF800 /* 255,   0,   0 */);
        else if (devErrors.localSensorError == 2)
            tft.setTextColor(0xFFFF /* 255, 255, 255 */, 0xFDA0 /* 255, 180,   0 */);
        else
            tft.setTextColor(0x001F /*   0,   0, 255 */, 0x07E0 /*   0, 255,   0 */);
        tft.drawString("1w", 185, 133);

        tft.setTextColor(0x07E0 /*   0, 255,   0 */, 0x0000 /*   0,   0,   0 */);
    }
}

void myTimerEvent()
{
    // You can send any value at any time.
    // Please don't send more that 10 values per second.
    /*    if (loraMessege.containsKey("CHs"))

            for (size_t i = 0; i < int(loraMessege["CHs"]); i++)

            {

                if (float(loraMessege["ChValues"][i]) > 1.0 && float(loraMessege["ChValues"][i]) < 99.0)

                    Blynk.virtualWrite(i, float(loraMessege["ChValues"][i]));

            }

        // send status R1

        if (loraMessege.containsKey("DI"))

            Blynk.virtualWrite(V10, int(loraMessege["DI"][0]) ? 0 : 1);

        // send local data

        uint8_t i;

        if (localSensorsCount > 0)

        {

            for (i = 0; i < localSensorsCount; i++)

                Blynk.virtualWrite(90 + i, sensorsTemp[i]);

        }

        for (i = 0; i < 8; i++)

        {

            Blynk.virtualWrite(100 + i, extIO.digitalRead(i) ? 1 : 0);

        }*/
# 1634 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
}
void tftMessage(String Message, int txtColor, int bgColor, int showTime)
{
    tft.fillScreen(bgColor);
    tft.setTextColor(txtColor);
    tft.setCursor(0, 0);
    tft.setTextDatum(4 /* Middle centre*/);
    tft.setTextSize(2);
    tft.drawString(Message, tft.width() / 2, tft.height() / 2 - 16);
    delay(showTime);
}

/************************parseFormula******************************

 *  formula template:

 * type 1 logic : "1 in1 operator in2 operator in3 output"

 * in1,in2,in3 - virtual pin numbers 0 - 127

 * operator - or, and

 * output - virtual pin number

 * type 2 once per hour:  "2 minuteOn minuteOff output"

 * type 3 once a day:  "3 hourOn minuteOn HourOff minuteOff dayMask output"

 * type 4 once per hour in defined 2 ranges:  "4 minuteOn minuteOff hourStart1 hourStop1 hourStart2 hourStop2 dayMask output"

 * dayMask - 127 all days  62 work days 65 weekends

 *********************************************************************/
# 1658 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino"
bool parseFormula(String formula)
{
    uint8_t index0 = formula.indexOf(" ");
    uint8_t formulaType = formula.substring(0, index0).toInt();
    if (index0 < 0)
        return false;
    if (formulaType == 1)
    {
        uint8_t index1 = formula.indexOf(" ", index0 + 1);
        uint8_t index2 = formula.indexOf(" ", index1 + 1);
        uint8_t index3 = formula.indexOf(" ", index2 + 1);
        uint8_t index4 = formula.indexOf(" ", index3 + 1);
        uint8_t index5 = formula.indexOf(" ", index4 + 1);
        if (!(index0 and index1 and index2 and index3 and index4 and index5))
        {
            Serial.println("Parse exit 1 !!!!!!!!!!!!!!!!!!!");
            return false;
        }
        uint8_t in1 = formula.substring(index0 + 1, index1).toInt();
        uint8_t in2 = formula.substring(index2 + 1, index3).toInt();
        uint8_t in3 = formula.substring(index4 + 1, index5).toInt();
        uint8_t out = formula.substring(index5 + 1).toInt();
        String operator1 = formula.substring(index1 + 1, index2);
        String operator2 = formula.substring(index3 + 1, index4);
        if ((in1 < 0) or (in1 > 127) or (in2 < 0) or (in2 > 127) or (in3 < 0) or (in3 > 127) or (out < 0) or (out > 127) or
            ((operator1 != "or") and (operator1 != "and")) or ((operator2 != "or") and (operator2 != "and")))
        {
            Serial.println("Parse exit 2 !!!!!!!!!!!!!!!!!!!");
            Serial.println(String(in1) + " " + String(in2) + " " + String(in3) + " " + String(out) + " " + operator1 + " " + operator2);
            return false;
        }
        bool result = false;
        Serial.println(String(in1) + ": " + String(vPinStateFromBlink[in1]) + " " + String(in2) + ": " + String(vPinStateFromBlink[in2]) + " " + String(in3) + ": " + String(vPinStateFromBlink[in3]) + " " + String(out) + " " + operator1 + " " + operator2);
        if (operator1 == "or")
            result = vPinStateFromBlink[in1] or vPinStateFromBlink[in2];
        else
            result = vPinStateFromBlink[in1] and vPinStateFromBlink[in2];
        if (operator2 == "or")
            result = result or vPinStateFromBlink[in3];
        else
            result = result and vPinStateFromBlink[in3];
        vPinStateFromBlink[out] = result ? 1 : 0;
        Serial.println("Parse result2: " + String(result));
    }
    if (formulaType == 2)
    {
        uint8_t index1 = formula.indexOf(" ", index0 + 1);
        uint8_t index2 = formula.indexOf(" ", index1 + 1);
        if (!(index0 and index1 and index2))
        {
            Serial.println("Parse exit 1 !!!!!!!!!!!!!!!!!!!");
            return false;
        }
        uint8_t minuteOn = formula.substring(index0 + 1, index1).toInt();
        uint8_t minuteOff = formula.substring(index1 + 1, index2).toInt();
        uint8_t out = formula.substring(index2 + 1).toInt();
        if ((minuteOn < 0) or (minuteOn > 59) or (minuteOff < 0) or (minuteOff > 59) or (out < 0) or (out > 127))
        {
            Serial.println("Parse exit 2 !!!!!!!!!!!!!!!!!!!");
            return false;
        }
        uint8_t minutes = ntp.minutes();
        bool result = false;
        //   Serial.println(String(in1) + ": " + String(vPinStateFromBlink[in1]) + " " + String(in2) + ": " + String(vPinStateFromBlink[in2]) + " " + " " + String(in3) + " " + String(out) + " " + operator1 + " " + operator2);
        if (minuteOn < minuteOff)
            vPinStateFromBlink[out] = (minutes >= minuteOn) and (minutes < minuteOff) ? 1 : 0;
        else
            vPinStateFromBlink[out] = (minutes >= minuteOn) or (minutes < minuteOff) ? 1 : 0;

        //  Serial.println("Parse result2: " + String(result));
    }
    //* type 3 once a day:  "3 hourOn minuteOn HourOff minuteOff dayMask output"
    if (formulaType == 3)
    {
        uint8_t index1 = formula.indexOf(" ", index0 + 1);
        uint8_t index2 = formula.indexOf(" ", index1 + 1);
        uint8_t index3 = formula.indexOf(" ", index2 + 1);
        uint8_t index4 = formula.indexOf(" ", index3 + 1);
        uint8_t index5 = formula.indexOf(" ", index4 + 1);
        if (!(index0 and index1 and index2 and index3 and index4 and index5))
        {
            Serial.println("Parse exit 1 !!!!!!!!!!!!!!!!!!!");
            return false;
        }
        uint8_t hourOn = formula.substring(index0 + 1, index1).toInt();
        uint8_t minuteOn = formula.substring(index1 + 1, index2).toInt();
        uint8_t hourOff = formula.substring(index2 + 1, index3).toInt();
        uint8_t minuteOff = formula.substring(index3 + 1, index4).toInt();
        uint8_t dayMask = formula.substring(index4 + 1, index5).toInt();
        uint8_t out = formula.substring(index5 + 1).toInt();
        if ((hourOn < 0) or (hourOn > 23) or (minuteOn < 0) or (minuteOn > 59) or (hourOff < 0) or (hourOff > 23) or (minuteOff < 0) or (minuteOff > 59) or (out < 0) or (out > 127) or (dayMask < 0) or (dayMask > 127))
        {
            Serial.println("Parse exit 2 !!!!!!!!!!!!!!!!!!!");
            return false;
        }
        uint16_t actualTimeStamp = ((uint16_t)ntp.hours() * 60) + (uint16_t)ntp.minutes();
        uint16_t startTimeStamp = (((uint16_t)hourOn) * 60) + ((u16_t)minuteOn);
        uint16_t stopTimeStamp = (((uint16_t)hourOff) * 60) + ((u16_t)minuteOff);
        //  Serial.println("tryb3: " + String(hourOn) + " " + String(minuteOn) + " " + String(hourOff) + " " + String(minuteOff) + " " + String(dayMask) + " " + String(out));
        //  Serial.println(String(actualTimeStamp) + " " + String(startTimeStamp) + " " + String(stopTimeStamp));
        if (stopTimeStamp > startTimeStamp)
            vPinStateFromBlink[out] = ((actualTimeStamp >= startTimeStamp) and (actualTimeStamp < stopTimeStamp)) and (dayMask & (1 << ntp.weekDay())) ? 1 : 0;
        else
            vPinStateFromBlink[out] = (actualTimeStamp >= startTimeStamp) or (actualTimeStamp < stopTimeStamp) and (dayMask & (1 << ntp.weekDay())) ? 1 : 0;

        bool result = false;
    }

    // type 4 once per hour in defined 2 ranges:  "4 minuteOn minuteOff hourOn1 hourOff1 hourOn2 hourOff2 dayMask output"
    if (formulaType == 4)
    {
        uint8_t index1 = formula.indexOf(" ", index0 + 1);
        uint8_t index2 = formula.indexOf(" ", index1 + 1);
        uint8_t index3 = formula.indexOf(" ", index2 + 1);
        uint8_t index4 = formula.indexOf(" ", index3 + 1);
        uint8_t index5 = formula.indexOf(" ", index4 + 1);
        uint8_t index6 = formula.indexOf(" ", index5 + 1);
        uint8_t index7 = formula.indexOf(" ", index6 + 1);
        if (!(index0 and index1 and index2 and index3 and index4 and index5 and index6 and index7))
        {
            Serial.println("Parse exit 1 !!!!!!!!!!!!!!!!!!!");
            return false;
        }
        uint8_t minuteOn = formula.substring(index0 + 1, index1).toInt();
        uint8_t minuteOff = formula.substring(index1 + 1, index2).toInt();
        uint8_t hourOn1 = formula.substring(index2 + 1, index3).toInt();
        uint8_t hourOff1 = formula.substring(index3 + 1, index4).toInt();
        uint8_t hourOn2 = formula.substring(index4 + 1, index5).toInt();
        uint8_t hourOff2 = formula.substring(index5 + 1, index6).toInt();
        uint8_t dayMask = formula.substring(index6 + 1, index7).toInt();
        uint8_t out = formula.substring(index7 + 1).toInt();
        if ((minuteOn < 0) or (minuteOn > 59) or (minuteOff < 0) or (minuteOff > 59) or (hourOn1 < 0) or (hourOn1 > 23) or (hourOff1 < 0) or (hourOff1 > 23) or (hourOn2 < 0) or (hourOn2 > 23) or (hourOff2 < 0) or (hourOff2 > 23) or (out < 0) or (out > 127) or (dayMask < 0) or (dayMask > 127))
        {
            Serial.println("Parse exit 2 !!!!!!!!!!!!!!!!!!!");
            return false;
        }
        if ((dayMask & (1 << ntp.weekDay())) and (ntp.minutes() >= minuteOn) and (ntp.minutes() <= minuteOff))
        {
            if ((ntp.hours() >= hourOn1) and (ntp.hours() <= hourOff1))
            {
                vPinStateFromBlink[out] = 1;
            }
            else if ((ntp.hours() >= hourOn2) and (ntp.hours() <= hourOff2))
            {
                vPinStateFromBlink[out] = 1;
            }
            else
            {
                vPinStateFromBlink[out] = 0;
            }
        }
        else
        {
            vPinStateFromBlink[out] = 0;
        }
    }
    return true;
}
# 1817 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\KoncentratorIOV3.ino" 2
