#line 1 "c:\\Users\\mirsmok\\work\\smartHome\\KoncentratorIOV3\\KoncentratorIOV3\\devSettings.cpp"
#include "./devSettings.h"
void devError::checkLoraPing(const sysSettings_t &sysSettings, unsigned long actualTime)
{
    bool errorPrasent = false;
    for (int i = 0; i < sysSettings.SystemMaxDevCount; i++)
    {
        // id < 1000 - special device
        if ((sysSettings.device[i].id > 1000) and ((actualTime - loraLastPing[i]) > this->loraPingTimeout))
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
    this->errorActive = this->wifiError or this->mqttError or this->loraDevError or this->localSensorError or this->extIoError or this->blynkError;
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
}
