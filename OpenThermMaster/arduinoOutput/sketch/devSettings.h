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
} sysSettingsRetein_t;

typedef struct
{
    sysSettingsRetein_t settings;

} device_t;

#endif