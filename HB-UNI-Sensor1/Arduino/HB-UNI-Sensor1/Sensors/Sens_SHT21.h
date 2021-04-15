
//---------------------------------------------------------
// Sens_SHT21
// 2019-08-08 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
// For SHT21 / Si7020 / Si7021
//---------------------------------------------------------

#ifndef _SENS_SHT21_H_
#define _SENS_SHT21_H_

#include <Wire.h>
#include <Sensors.h>
#include <Si7021.h>    // https://github.com/jayjayuk/Si7021-Humidity-And-Temperature-Sensor-Library

namespace as {

class Sens_SHT21 : public Sensor {
    SI7021   _si7021;
    int16_t  _temperature;
    uint16_t _humidity;

public:
    Sens_SHT21()
        : _temperature(0)
        , _humidity(0)
    {
    }

    bool init()
    {
        Wire.begin();
        uint8_t i = 10;
        while (i > 0) {
            _si7021.begin();
            uint16_t id = _si7021.getDeviceID();
            if ((id == 7013) || (id == 7020) || (id == 7021)) {
                _present = true;
                DPRINTLN(F("SHT21/Si7021 found"));
                _si7021.setHumidityRes(12);    // Humidity = 12-bit / Temperature = 14-bit
                _si7021.setHeater(0);          // Turn heater off
                return true;
            }
            delay(100);
            i--;
        }
        DPRINTLN(F("Error: SHT21/Si7021 not found"));
        return false;
    }

    bool measure()
    {
        _temperature = _humidity = 0;
        bool bRet                = false;
        if (_present == true) {
            _temperature = (int16_t)(_si7021.readTemp() * 10.0);         // HB-UNI-Sensor1: Temp *10
            _humidity    = (uint16_t)(_si7021.readHumidity() * 10.0);    // HB-UNI-Sensor1: Humi *10
            bRet         = true;
            DPRINT(F("SHT21 Temperature x10   : "));
            DDECLN(_temperature);
            DPRINT(F("SHT21 Humidity x10      : "));
            DDECLN(_humidity);
        }
        return bRet;
    }

    int16_t  temperature() { return _temperature; }
    uint16_t humidity() { return _humidity; }
};

}

#endif
