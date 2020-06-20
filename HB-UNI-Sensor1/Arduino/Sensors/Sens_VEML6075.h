
//---------------------------------------------------------
// Sens_VEML6075
// 2019-07-06 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _SENS_VEML6075_H_
#define _SENS_VEML6075_H_

#include <Wire.h>
#include <Sensors.h>
#include <VEML6075.h>

namespace as {

class Sens_VEML6075 : public Sensor {

    uint8_t  _uvIndex10;
    float    _uva, _uvb, _uvi;
    VEML6075 _veml6075;

    void writeConf(uint16_t data)
    {
        Wire.beginTransmission(VEML6075_ADDR);
        Wire.write(VEML6075_REG_CONF);
        Wire.write((uint8_t)(0xFF & (data >> 0)));    // LSB
        Wire.write((uint8_t)(0xFF & (data >> 8)));    // MSB
        Wire.endTransmission();
    }

public:
    Sens_VEML6075()
        : _uvIndex10(0)
        , _uva(0)
        , _uvb(0)
        , _uvi(0)
    {
    }

    bool init()
    {
        Wire.begin();

        uint8_t i = 10;
        while (i > 0) {
            if ((_veml6075.begin()) && (_veml6075.getDevID() == VEML6075_DEVID)) {
                _present = true;
                DPRINTLN(F("VEML6075 found"));
                return true;
            }
            delay(100);
            i--;
        }
        DPRINTLN(F("Error: VEML6075 not found"));
        return false;
    }

    bool measure()
    {
        _uvIndex10 = 0;
        _uva = _uvb = _uvi = 0;
        if (_present == true) {
            writeConf(0x10);     // integration time 100ms, device on
            delay(150);          // allow one measurement
            _veml6075.poll();    // poll sensor
            _uva       = _veml6075.getUVA();
            _uvb       = _veml6075.getUVB();
            _uvi       = _veml6075.getUVIndex();
            _uvIndex10 = uint8_t(_uvi * 10.0);
            writeConf(0x11);    // integration time 100ms, device shut down

#ifndef NDEBUG
            DPRINT(F("VEML6075 UVA            : "));
            DDECLN((int)_uva);
            DPRINT(F("VEML6075 UVB            : "));
            DDECLN((int)_uvb);
            DPRINT(F("VEML6075 UVI            : "));
            Serial.println(_uvi, 1);
#endif
            return true;
        }
        return false;
    }

    uint8_t uvIndex10() { return _uvIndex10; }
    float   uvA() { return _uva; }
    float   uvB() { return _uvb; }
};

}

#endif
