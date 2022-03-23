#line 1 "c:\\Users\\mirsmok\\work\\smartHome\\OpenThermMaster\\devSettings.h"

#ifndef DEVSETTINGS_H
#define DEVSETTINGS_H

#include <stdint.h>

typedef struct
{
    uint8_t enableCentralHeating = 1;
    uint8_t enableHotWater = 1;
    uint8_t enableCooling = 0;
    float ch_temperature = 0.0;
    float dhw_temperature = 0.0;
    char mqttAddress[20];
} sysSettingsRetein_t;
typedef struct
{
    uint8_t CentralHeating = 0;
    uint8_t HotWater = 0;
    uint8_t Cooling = 0;
    uint8_t Flame = 0;
    uint8_t wifiFault = 0;
    uint8_t mqttFault = 0;
    uint8_t otFault = 0;
    uint8_t otFaultCode = 0;
    float ch_temperature = 0.0;
    float dhw_temperature = 0.0;
    float modulation = 0.0;
    float pressure = 0.0;
    char communicationStatus[20];
} sysStatus_t;

typedef struct
{
    sysSettingsRetein_t settings;
    sysStatus_t status;

} device_t;

#endif