
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

    void measure(uint16_t altitude)
    {
        _temperature = _pressure = _pressureNN = 0;
        if (_present == true) {
            float temp = _bmp180.getTemperature();    // C
            if (temp != BMP180_ERROR) {
                _temperature = (int16_t)(temp * 10.0);    // HB-UNI-Sensor1: C*10
                DPRINT(F("BMP180   Temperature   : "));
                DDECLN(_temperature);

                float pres = _bmp180.getPressure();    // Pa
                if (pres != BMP180_ERROR) {
                    _pressure    = (uint16_t)(pres / 10.0);    // HB-UNI-Sensor1: hPa*10
                    _pressureNN  = (uint16_t)(EquivalentSeaLevelPressure(float(altitude), temp, pres) / 10.0);
                    DPRINT(F("BMP180   Pressure      : "));
                    DDECLN(_pressure);
                    DPRINT(F("BMP180   PressureNN    : "));
                    DDECLN(_pressureNN);
                }
            }
        }
    }

    int16_t  temperature() { return _temperature; }
    uint16_t pressure() { return _pressure; }
    uint16_t pressureNN() { return _pressureNN; }

private:
    float EquivalentSeaLevelPressure(float altitude, float temp, float pres)
    {
        return (pres / pow(1 - ((0.0065 * altitude) / (temp + (0.0065 * altitude) + 273.15)), 5.257));
    }
};

}

#endif
