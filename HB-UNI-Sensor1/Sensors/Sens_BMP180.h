
//---------------------------------------------------------
// Sens_BMP180
// 2018-12-14 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _SENS_BMP180_H_
#define _SENS_BMP180_H_

#include <Wire.h>
#include <Sensors.h>
#include <BMP180.h>    // https://github.com/enjoyneering/BMP180

namespace as {

class Sens_BMP180 : public Sensor {

    int16_t  _temperature;
    uint16_t _pressure;
    uint16_t _pressureNN;
    BMP180   _bmp180;

public:
    Sens_BMP180()
        : _temperature(0)
        , _pressure(0)
        , _pressureNN(0)
        , _bmp180(BMP180_HIGHRES)
    {
    }

    bool init()
    {
        Wire.begin();    // ToDo sync with further I2C sensor classes

        uint8_t i = 10;
        while (i > 0) {
            if (_bmp180.begin()) {
                _present = true;
                DPRINTLN(F("BMP180 found"));
                return true;
            }
            delay(100);
            i--;
        }
        DPRINTLN(F("Error: BMP180 not found"));
        return false;
    }

    void measure(uint16_t height)
    {
        _temperature = _pressure = _pressureNN = 0;
        if (_present == true) {
            float T = _bmp180.getTemperature();
            if (T != BMP180_ERROR) {
                _temperature = (int16_t)(T * 10);    // C*10
                DPRINT(F("BMP180   Temperature   : "));
                DDECLN(_temperature);
            }
            int32_t P = _bmp180.getPressure();
            if (P != BMP180_ERROR) {
                _pressure = (uint16_t)(P / 10);    // hPa*10
                DPRINT(F("BMP180   Pressure      : "));
                DDECLN(_pressure);
            }
        }
    }

    int16_t  temperature() { return _temperature; }
    uint16_t pressure() { return _pressure; }
    uint16_t pressureNN() { return _pressureNN; }
};

}

#endif
