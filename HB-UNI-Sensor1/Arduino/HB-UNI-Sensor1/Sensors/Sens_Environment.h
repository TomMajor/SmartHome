
//---------------------------------------------------------
// Sens_Environment
// (C) 2022 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
// needs Arduino Lib https://github.com/finitespace/BME280
//---------------------------------------------------------

#ifndef _SENS_ENVIRONMENT_H_
#define _SENS_ENVIRONMENT_H_

#include <EnvironmentCalculations.h>

namespace as {

class Sens_Environment {

public:
    Sens_Environment() {}

    uint16_t absHumidity(int16_t temp10, uint16_t humi10)
    {
        float    temp  = (float)(temp10) / 10.0;
        float    hum   = (float)(humi10) / 10.0;
        uint16_t aH100 = (uint16_t)(EnvironmentCalculations::AbsoluteHumidity(temp, hum, EnvironmentCalculations::TempUnit_Celsius) * 100.0);
        DPRINT(F("BME280 AbsHumidity x100 : "));
        DDECLN(aH100);
        return aH100;
    }

    int16_t dewPoint(int16_t temp10, uint16_t humi10)
    {
        float   temp = (float)(temp10) / 10.0;
        float   hum  = (float)(humi10) / 10.0;
        int16_t dP10 = (int16_t)(EnvironmentCalculations::DewPoint(temp, hum, EnvironmentCalculations::TempUnit_Celsius) * 10.0);
        DPRINT(F("BME280 DewPoint x10     : "));
        DDECLN(dP10);
        return dP10;
    }
};

}

#endif
