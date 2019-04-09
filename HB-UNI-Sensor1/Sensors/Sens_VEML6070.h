
//---------------------------------------------------------
// Sens_VEML6070
// 2019-04-09 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _SENS_VEML6070_H_
#define _SENS_VEML6070_H_

#include <Wire.h>
#include <Sensors.h>

#define VEML6070_ADDR_H 0x39
#define VEML6070_ADDR_L 0x38

typedef enum {    // enum values are for IT1:IT0 register setting and must not be changed
    VEML6070_05T = 0,
    VEML6070_1T  = 1,
    VEML6070_2T  = 2,
    VEML6070_4T  = 3,
} veml6070_integrationtime_t;

namespace as {

template <uint8_t INTEGRATION_TIME = VEML6070_1T> class Sens_VEML6070 : public Sensor {

    uint8_t  _uvIndex;
    uint16_t _uvRawValue;
    uint16_t _uvIntTimeFactor;

    uint16_t measureRaw()
    {
        if (Wire.requestFrom(VEML6070_ADDR_H, 1) != 1) {
            return 0xFFFF;
        }
        uint16_t value = Wire.read();
        value <<= 8;
        if (Wire.requestFrom(VEML6070_ADDR_L, 1) != 1) {
            return 0xFFFF;
        }
        value |= Wire.read();
        return value;
    }

    void shutdown(bool state)    // enter or exit shutdown mode, while in this mode the chip draws ~1uA
    {
        if (_present == true) {
            Wire.beginTransmission(VEML6070_ADDR_L);
            Wire.write((INTEGRATION_TIME << 2) | 0x02 | (state ? 1 : 0));
            Wire.endTransmission();
        }
    }

public:
    Sens_VEML6070()
        : _uvIndex(0)
        , _uvRawValue(0)
        , _uvIntTimeFactor(187)
    {
    }

    bool init()
    {
        Wire.begin();

        uint8_t i = 10;
        while (i > 0) {
            Wire.beginTransmission(VEML6070_ADDR_L);
            if (Wire.endTransmission() == 0) {
                _present = true;
                Wire.beginTransmission(VEML6070_ADDR_L);
                Wire.write((INTEGRATION_TIME << 2) | 0x02);    // set integration time
                Wire.endTransmission();
                switch (INTEGRATION_TIME) {
                case VEML6070_05T:
                    _uvIntTimeFactor = 187 / 2;
                    break;
                case VEML6070_1T:
                    _uvIntTimeFactor = 187;
                    break;
                case VEML6070_2T:
                    _uvIntTimeFactor = 2 * 187;
                    break;
                case VEML6070_4T:
                    _uvIntTimeFactor = 4 * 187;
                    break;
                }
                DPRINTLN(F("VEML6070 found"));
                return true;
            }
            delay(100);
            i--;
        }
        DPRINTLN(F("Error: VEML6070 not found"));
        return false;
    }

    bool measure()
    {
        _uvIndex    = 0;
        _uvRawValue = 0;
        if (_present == true) {
            shutdown(false);
            delay(65 * (INTEGRATION_TIME + 1));    // wait one measurement (default 56ms for VEML6070_05T with 270k resistor)
            _uvRawValue = measureRaw();
            shutdown(true);
            if (_uvRawValue == 0xFFFF) {
                _uvIndex = 11;
                return false;
            }
            _uvIndex = (_uvRawValue < (12 * _uvIntTimeFactor)) ? (_uvRawValue + 1) / _uvIntTimeFactor : 11;
            DPRINT(F("VEML6070 UV raw value   : "));
            DDECLN(_uvRawValue);
            DPRINT(F("VEML6070 UV-Index       : "));
            DDECLN(_uvIndex);
            return true;
        }
        return false;
    }

    uint8_t  uvIndex() { return _uvIndex; }
    uint16_t uvRawValue() { return _uvRawValue; }
};

}

#endif
