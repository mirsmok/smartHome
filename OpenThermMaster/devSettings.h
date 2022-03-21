
#ifndef DEVSETTINGS_H
#define DEVSETTINGS_H

#include <stdint.h>

typedef struct
{
    bool enableCentralHeating = true;
    bool enableHotWater = true;
    bool enableCooling = false;
    float ch_temperature = 0.0;
    float dhw_temperature = 0.0;
} sysSettings_t;

#endif