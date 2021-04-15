
//---------------------------------------------------------
// Sens_SHT31
// 2019-12-01 Wolfram Winter (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _SENS_SHT31_H_
#define _SENS_SHT31_H_

#include <Wire.h>
#include <Sensors.h>
#include <Adafruit_SHT31.h>    // https://github.com/adafruit/Adafruit_SHT31
                               // Aduino Ide - Adafruit SHT31 library

namespace as {

template <uint8_t I2C_ADDR> class Sens_SHT31 : public Sensor {

    int16_t          _temperature;
    uint16_t         _humidity;
    ::Adafruit_SHT31 _sht31;

public:
    // constructor with parameter in header file -> Initalize with 'member initialiser' syntax in constructor
    Sens_SHT31()
        : _temperature(0)
        , _humidity(0)
    {
    }

    bool init()
    {
        Wire.begin();
        uint8_t i = 10;
        while (i > 0) {
            if (_sht31.begin(I2C_ADDR) == true) {
                _present = true;
                DPRINTLN(F("SHT31 found"));
                _sht31.heater(false);    // Turn heater off
                return true;
            }
            delay(100);
            i--;
        }
        DPRINTLN(F("Error: SHT31 not found"));
        return false;
    }

    bool measure()
    {
        _temperature = _humidity = 0;
        bool bRet                = false;
        if (_present == true) {
            bool  rRet = true;
            float t    = _sht31.readTemperature();
            float h    = _sht31.readHumidity();
            if (!isnan(t)) {                           // check if 'is not a number'
                _temperature = (int16_t)(t * 10.0);    // HB-UNI-Sensor1: Temp *10
                DPRINT(F("SHT31 Temperature x10   : "));
                DDECLN(_temperature);
            } else {
                rRet = false;
                DPRINT(F("SHT31 failed to read temperature"));
            }
            if (!isnan(h)) {                         // check if 'is not a number'
                _humidity = (uint16_t)(h * 10.0);    // HB-UNI-Sensor1: Humi *10
                DPRINT(F("SHT31 Humidity x10      : "));
                DDECLN(_humidity);
            } else {
                rRet = false;
                DPRINT(F("SHT31 failed to read humidity"));
            }
            bRet = rRet;
        }
        return bRet;
    }

    int16_t  temperature() { return _temperature; }
    uint16_t humidity() { return _humidity; }
};

}

#endif
