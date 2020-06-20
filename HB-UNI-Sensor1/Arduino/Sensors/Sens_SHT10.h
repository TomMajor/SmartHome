
//---------------------------------------------------------
// Sens_SHT10
// 2018-08-12 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _SENS_SHT10_H_
#define _SENS_SHT10_H_

#include <Wire.h>
#include <Sensors.h>
#include <Sensirion.h>    // https://github.com/spease/Sensirion

namespace as {

template <uint8_t DATAPIN, uint8_t CLOCKPIN> class Sens_SHT10 : public Sensor {
    Sensirion _sht10;
    bool      _i2cSharedAccess;
    int16_t   _temperature;
    uint8_t   _humidity;

    void i2cDisable()
    {
        if (_i2cSharedAccess && DATAPIN == A4 && CLOCKPIN == A5) {
            Wire.end();
        }
    }

    void i2cEnable()
    {
        if (_i2cSharedAccess && DATAPIN == A4 && CLOCKPIN == A5) {
            Wire.begin();
        }
    }

public:
    Sens_SHT10()
        : _sht10(DATAPIN, CLOCKPIN)
        , _i2cSharedAccess(false)
        , _temperature(0)
        , _humidity(0)
    {
    }

    void i2cEnableSharedAccess()
    {
        _i2cSharedAccess = true;
        DPRINTLN(F("SHT10 Ena I2C shared access"));
    }

    bool init()
    {
        i2cDisable();
        uint8_t stat, i = 10;
        while (i > 0) {
            if (_sht10.readSR(&stat) == 0) {
                _present = true;
                DPRINTLN(F("SHT10 found"));
                i2cEnable();
                return true;
            }
            delay(100);
            i--;
        }
        DPRINTLN(F("Error: SHT10 not found"));
        i2cEnable();
        return false;
    }

    bool measure()
    {
        _temperature = _humidity = 0;
        bool bRet    = false;
        if (_present == true) {
            i2cDisable();
            uint16_t rawData;
            if (_sht10.measTemp(&rawData) == 0) {
                float t      = _sht10.calcTemp(rawData);
                _temperature = (int16_t)(t * 10);
                if (_sht10.measHumi(&rawData) == 0) {
                    float h   = _sht10.calcHumi(rawData, t);
                    _humidity = (uint8_t)h;
                    bRet      = true;
                }
            }
            DPRINT("SHT10    Temperature   : ");
            DDECLN(_temperature);
            DPRINT("SHT10    Humidity      : ");
            DDECLN(_humidity);
            i2cEnable();
        }
        return bRet;
    }

    int16_t temperature() { return _temperature; }
    uint8_t humidity() { return _humidity; }
};

}

#endif
