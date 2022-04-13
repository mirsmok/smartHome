#line 1 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\devSettings.h"

#ifndef DEVSETTINGS_H
#define DEVSETTINGS_H

#include <stdint.h>
#define DefinedSystemMaxDevCount 10
#define maxUnassignedDevCount 3
#define maxOneWireChannelsInDevice 9
#define maxPinCountInPort 8
#define DI_localCount 8
#define DO_localCount 8
#define AI_localCount 3
#define AO_localCount 2

enum devTypes
{
    noneDev = 0,
    remIOv01 = 1,
    remTempSensor = 2,
    localPorts = 3
};

typedef struct
{
    uint32_t id;
    uint8_t vPinAdrr;
} oneWireChannel_t;

typedef struct
{
    uint8_t count;
    uint8_t vPinAdrr[maxPinCountInPort];
} ioPortAdrr_t;

typedef struct
{
    enum devTypes type;
    uint32_t id;
    uint8_t virtualPinStartAddr;
    char description[20];
    oneWireChannel_t oneWireChannel[maxOneWireChannelsInDevice];
    ioPortAdrr_t DI_PortCfg;
    ioPortAdrr_t DO_PortCfg;
    ioPortAdrr_t AI_PortCfg;
    ioPortAdrr_t AO_PortCfg;
    ;
} device_t;
typedef struct
{
    const uint8_t SystemMaxDevCount = DefinedSystemMaxDevCount;
    char MqttBrokerAddress[30];
    device_t device[DefinedSystemMaxDevCount];
} sysSettings_t;

typedef struct
{
    uint32_t devID;
    enum devTypes devType;
    oneWireChannel_t oneWireChannel[maxOneWireChannelsInDevice];
    ioPortAdrr_t DI_PortCfg;
    ioPortAdrr_t DO_PortCfg;
    ioPortAdrr_t AI_PortCfg;
    ioPortAdrr_t AO_PortCfg;
} unassigendDevice_t;

class devError
{
public:
    devError()
    {
        this->errorActive = false;
        this->blynkError = 0;
        this->wifiError = 0;
        this->mqttError = 0;
        this->localSensorError = 0;
        this->extIoError = 0;
        this->loraDevError = 0;
        this->loraDevErrorArr[DefinedSystemMaxDevCount] = {0};
        this->loraLastPing[DefinedSystemMaxDevCount] = {0};
        this->loraPingTimeout = 60000;
    }
    bool errorActive;
    uint8_t blynkError;
    uint8_t wifiError;
    uint8_t mqttError;
    uint8_t localSensorError;
    uint8_t extIoError;
    uint8_t loraDevError;
    uint8_t loraDevErrorArr[DefinedSystemMaxDevCount];
    unsigned long loraLastPing[DefinedSystemMaxDevCount];
    int loraPingTimeout;
    void checkLoraPing(const sysSettings_t &, unsigned long);
    bool checkError(void);
    void clearErrors(void);
};

typedef struct
{
    char topic[60] = {0};
    char payload[10] = {0};
    uint8_t len = 0;
} mqttMessage_t;

#endif