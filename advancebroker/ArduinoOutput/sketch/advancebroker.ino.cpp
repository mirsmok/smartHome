#line 1 "c:\\Users\\mirsmok\\work\\smartHome\\advancebroker\\advancebroker.ino"
#define BLYNK_TEMPLATE_ID "TMPLOPmBUAZg"
#define BLYNK_DEVICE_NAME "MQTTBroker"
#define BLYNK_AUTH_TOKEN "Ao-FOFILurEg9WuBaLirpOQG5xLB-eHN"
// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>
BlynkTimer timer;
int vPinStateFromBlink[128];
bool BlynkConected = false;

#include <sMQTTBroker.h>
#include "SSD1306.h"
#include <Arduino.h>
#include "WiFi.h"
#include <WiFiUdp.h>
#include "SSD1306.h"
#include "NTP.h"
#include <ESPmDNS.h>

SSD1306 display(0x3c, 5, 4);
WiFiUDP wifiUdp;
NTP ntp(wifiUdp);
uint8_t clientCount = 0;

const char *ssid = "HUAWEI_B818_18EE"; // The SSID (name) of the Wi-Fi network you want to connect to
const char *password = "QG20M7RY90F";  // The password of the Wi-Fi network

#line 28 "c:\\Users\\mirsmok\\work\\smartHome\\advancebroker\\advancebroker.ino"
void display_text(String text);
#line 67 "c:\\Users\\mirsmok\\work\\smartHome\\advancebroker\\advancebroker.ino"
void wifiConnect();
#line 87 "c:\\Users\\mirsmok\\work\\smartHome\\advancebroker\\advancebroker.ino"
void BlynkConnect(void);
#line 105 "c:\\Users\\mirsmok\\work\\smartHome\\advancebroker\\advancebroker.ino"
void setup();
#line 135 "c:\\Users\\mirsmok\\work\\smartHome\\advancebroker\\advancebroker.ino"
void loop();
#line 174 "c:\\Users\\mirsmok\\work\\smartHome\\advancebroker\\advancebroker.ino"
void myTimerEvent();
#line 28 "c:\\Users\\mirsmok\\work\\smartHome\\advancebroker\\advancebroker.ino"
void display_text(String text)
{
    display.clear();
    display.setColor(WHITE);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, text);
    display.display();
}

class MyBroker : public sMQTTBroker
{
public:
    bool onConnect(sMQTTClient *client, const std::string &username, const std::string &password)
    {
        // check username and password, if ok return true
        Serial.println("new client!!!!!!!!!!!!!!!!!!!!!!!!");
        clientCount++;
        return true;
    };
    void onRemove(sMQTTClient *)
    {
        clientCount--;
    };
    void onPublish(sMQTTClient *client, const std::string &topic, const std::string &payload)
    {
        // client publish a message to the topic
        // you can parse the message, save it to a database, or whatever, based on your goals
        Serial.println("nowa wiadomosc!!!!!!!!!!!!!!!!!!!");
        Serial.println("topic");
        Serial.println(topic.c_str());
        Serial.println(payload.c_str());
    }
};

MyBroker broker;
unsigned long Time;
unsigned long freeRam;
bool OTAconfigured = false;

void wifiConnect()
{
    static int i = 0;
    if (WiFi.status() != WL_CONNECTED)
    { // Wait for the Wi-Fi to connect

        WiFi.begin(ssid, password);
        display_text("Connecting " + String(i));
        delay(500);
        i++;
        if (i > 30)
        {
            ESP.restart();
        }
    }
    else
        i = 0;
}

////********************************* BLYNK **********************************************
void BlynkConnect(void)
{
    Blynk.config(BLYNK_AUTH_TOKEN);
    long time = millis();
    while ((Blynk.connect() == false) && ((millis() - time) < 10000))
    {
        /* code */
    }
    if ((millis() - time) > 10000)
    {
        Serial.println("Nie udalo sie połączyć z blynkiem");
        display_text("Brak odpowiedzi z Blynk: Reset");
        delay(4000);
        ESP.restart();
    }
    timer.setInterval(1000L, myTimerEvent);
}

void setup()
{
    Serial.begin(115200);
    display.init();
    display.flipScreenVertically();
    WiFi.begin(ssid, password);
    wifiConnect();
    Serial.println("Connection established!");
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    display.clear();
    display_text("Connected");
    delay(500);
    ntp.ruleDST("CEST", Last, Sun, Mar, 2, 120); // last sunday in march 2:00, timetone +120min (+1 GMT + 1h summertime offset)
    ntp.ruleSTD("CET", Last, Sun, Oct, 3, 60);   // last sunday in october 3:00, timezone +60min (+1 GMT)
    // ntp.isDST(false);
    // ntp.timeZone(1);
    // ntp.offset(0, 0, 0, 0);
    ntp.begin();
    display_text("start NTP");
    delay(500);

    const unsigned short mqttPort = 1883;
    broker.init(mqttPort);
    // all done
    // your magic code
    Time = millis();
    freeRam = ESP.getFreeHeap();
};

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        broker.update();
        if (!Blynk.connected())
            BlynkConnect();
        else
        {
            Blynk.run();
            timer.run();
        }
    }
    // your magic code
    if (millis() - Time > 5000)
    {
        Time = millis();
        if (WiFi.status() == WL_CONNECTED)
            ntp.update();
        if (ESP.getFreeHeap() != freeRam)
        {
            freeRam = ESP.getFreeHeap();
            Serial.print("RAM:");
            Serial.println(freeRam);
        }

        if (WiFi.status() != WL_CONNECTED)
            wifiConnect();
    }

    display.clear();
    display.fillRect(1, 0, 126 * ntp.seconds() / 59, 2);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 5, ntp.formattedTime("%d. %B %Y"));
    display.drawString(64, 15, ntp.formattedTime("%A %T"));
    display.drawString(64, 25, "RAM: " + String((float)freeRam / 5200.0, 1) + "%");
    display.drawString(64, 35, "client count: " + String(clientCount));
    display.display();
}
void myTimerEvent()
{
    ///  Blynk.virtualWrite(V0, 0.2);
}

