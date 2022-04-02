
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
    uint8_t manualCentralHeatinActive = 0;
    uint8_t manualHotWaterActive = 0;
    // falult status: 0 - inactivce acknowleged, 1 - active 2 - inactive not acknowleged
    uint8_t wifiFault = 0;
    uint8_t mqttFault = 0;
    uint8_t otCommunicationFault = 0;
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
    const uint8_t ledCentralHeatingStatusPin = 2;
    const uint8_t ledOnlineStatusPin = 12;
    const uint8_t ledOKStatusPin = 13;
    const uint8_t ledErrorStatusPin = 15;
    const uint8_t ledHotWaterStatusPin = 16;
    const uint16_t buttonCentralHeatingManualLevel = 835;
    const uint16_t buttonHotWaterManualLevel = 550;

} device_t;

#endif