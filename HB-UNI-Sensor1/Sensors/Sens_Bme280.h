
#ifndef _SENS_BME280_H_
#define _SENS_BME280_H_

#include <Wire.h>
#include <Sensors.h>
#include <BME280I2C.h>
#include <EnvironmentCalculations.h>

namespace as {

class Sens_Bme280 : public Sensor {

    int16_t  _temperature;
    uint16_t _pressure;
    uint16_t _pressureNN;
    uint8_t  _humidity;
    BME280I2C
    _bme280;    // Default : forced mode, standby time = 1000 ms, Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off

public:
    Sens_Bme280() {}

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
            DPRINTLN("Success: found BME280 sensor");
            break;
        case BME280::ChipModel_BMP280:
            _present = true;
            DPRINTLN("Success: found BMP280 sensor, (No Humidity available)");
            break;
        default:
            DPRINTLN("Error: no BME280 / BMP280 sensor found");
        }
    }

    void measure(uint16_t height)
    {
        if (_present == true) {
            float temp(NAN), hum(NAN), pres(NAN), presNN(NAN);
            _bme280.read(pres, temp, hum, BME280::TempUnit_Celsius, BME280::PresUnit_hPa);

            _temperature = (int16_t)(temp * 10);
            _pressure    = (uint16_t)(pres * 10);
            _pressureNN  = (uint16_t)(EnvironmentCalculations::EquivalentSeaLevelPressure(float(height), temp, pres) * 10);
            _humidity    = (uint8_t)hum;

            DPRINT("BME280   Temperature   : ");
            DDECLN(_temperature);
            DPRINT("BME280   Pressure      : ");
            DDECLN(_pressure);
            DPRINT("BME280   PressureNN    : ");
            DDECLN(_pressureNN);
            DPRINT("BME280   Humidity      : ");
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
