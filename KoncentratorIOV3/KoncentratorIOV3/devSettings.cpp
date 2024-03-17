#include "./devSettings.h"
void devError::checkLoraPing(const sysSettings_t &sysSettings, unsigned long actualTime)
{
    bool errorPrasent = false;
    for (int i = 0; i < sysSettings.SystemMaxDevCount; i++)
    {
        // id < 1000 - special device
        if ((sysSettings.device[i].id > 1000) && ((actualTime - loraLastPing[i]) > this->loraPingTimeout))
        {
            this->loraDevErrorArr[i] = 1;
            errorPrasent = true;
        }
        else
        {
            if (this->loraDevErrorArr[i] == 1)
                this->loraDevErrorArr[i] = 2;
        }
    }
    if (errorPrasent)
        this->loraDevError = 1;
    else if (this->loraDevError == 1)
        this->loraDevError = 2;
}

bool devError::checkError(void)
{
    this->errorActive = this->wifiError || this->mqttError || this->loraDevError || this->localSensorError || this->extIoError || this->blynkError;
    return this->errorActive;
}

void devError::clearErrors(void)
{
    this->wifiError = 0;
    this->mqttError = 0;
    this->loraDevError = 0;
    this->localSensorError = 0;
    this->extIoError = 0;
    this->blynkError = 0;
    for (uint8_t i = 0; i < DefinedSystemMaxDevCount; i++)
    {
        this->loraDevErrorArr[i] = 0;
    }
}
