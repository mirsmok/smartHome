#include "./devSettings.h"
void devError::checkLoraPing(const sysSettings_t &sysSettings, unsigned long actualTime)
{
    for (int i = 0; i < sysSettings.SystemMaxDevCount; i++)
    {
        // id < 1000 - special device
        if ((sysSettings.device[i].id > 1000) and ((actualTime - loraLastPing[i]) > this->loraPingTimeout))
        {
            this->loraDevErrorArr[i] = true;
            this->loraDevError = true;
        }
    }
}

bool devError::checkError(void)
{
    this->errorActive = this->wifiError or this->mqttError or this->loraDevError or this->localSensorError or this->extIoError or this->blynkError;
    return this->errorActive;
}

void devError::clearErrors(void)
{
    this->wifiError = false;
    this->mqttError = false;
    this->loraDevError = false;
    this->localSensorError = false;
    this->extIoError = false;
    this->blynkError = false;
}
