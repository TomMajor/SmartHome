
//---------------------------------------------------------
// Sens_BME280
// 2018-08-12 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _SENS_BME280_H_
#define _SENS_BME280_H_

#include <Wire.h>
#include <Sensors.h>
#include <BME280I2C.h>
#include <EnvironmentCalculations.h>

namespace as {

class Sens_BME280 : public Sensor {

    int16_t  _temperature;
    uint16_t _pressure;
    uint16_t _pressureNN;
    uint8_t  _humidity;
    BME280I2C
    _bme280;    // Default : forced mode, standby time = 1000 ms, Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off

    void measureRaw(uint16_t altitude)
    {
        float temp(NAN), hum(NAN), pres(NAN), presNN(NAN);
        _bme280.read(pres, temp, hum, BME280::TempUnit_Celsius, BME280::PresUnit_hPa); // hPa
        _temperature = (int16_t)(temp * 10); // HB-UNI-Sensor1: C*10
        _pressure    = (uint16_t)(pres * 10); // HB-UNI-Sensor1: hPa*10
        _pressureNN  = (uint16_t)(EnvironmentCalculations::EquivalentSeaLevelPressure(float(altitude), temp, pres) * 10);
        _humidity    = (uint8_t)hum;
    }

public:
    Sens_BME280()
        : _temperature(0)
        , _pressure(0)
        , _pressureNN(0)
        , _humidity(0)
    {
    }

    void init()
    {
        Wire.begin();    // ToDo sync with further I2C sensor classes

        uint8_t i = 10;
        while ((!_bme280.begin()) && (i > 0)) {
            delay(100);
            i--;
        }

        switch (_bme280.chipModel()) {
        case BME280::ChipModel_BME280:
            _present = true;
            DPRINTLN(F("BME280 found"));
            measureRaw(0);    // dummy read da die Messwerte der ersten Messung nach power-on weit daneben liegen
            break;
        case BME280::ChipModel_BMP280:
            _present = true;
            DPRINTLN(F("BMP280 found (No Humidity available)"));
            measureRaw(0);
            break;
        default:
            DPRINTLN(F("Error: no BME280/BMP280 sensor found"));
        }
    }

    void measure(uint16_t altitude)
    {
        _temperature = _pressure = _pressureNN = _humidity = 0;
        if (_present == true) {
            measureRaw(altitude);
            DPRINT(F("BME280   Temperature   : "));
            DDECLN(_temperature);
            DPRINT(F("BME280   Pressure      : "));
            DDECLN(_pressure);
            DPRINT(F("BME280   PressureNN    : "));
            DDECLN(_pressureNN);
            DPRINT(F("BME280   Humidity      : "));
            DDECLN(_humidity);
        }
    }

    int16_t  temperature() { return _temperature; }
    uint16_t pressure() { return _pressure; }
    uint16_t pressureNN() { return _pressureNN; }
    uint8_t  humidity() { return _humidity; }
};

}

#endif
