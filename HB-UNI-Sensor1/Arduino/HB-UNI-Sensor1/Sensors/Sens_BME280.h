
//---------------------------------------------------------
// Sens_BME280
// 2019-07-14 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
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

BME280I2C::Settings settings(BME280::OSR_X1,                // Temperature Oversampling Rate (tempOSR): OSR Enum, default = OSR_X1
                             BME280::OSR_X1,                // Humidity Oversampling Rate (humOSR): OSR Enum, default = OSR_X1
                             BME280::OSR_X1,                // Pressure Oversampling Rate (presOSR): OSR Enum, default = OSR_X1
                             BME280::Mode_Forced,           // Mode (mode): Mode Enum, default = Mode_Forced
                             BME280::StandbyTime_1000ms,    // Standby Time (standbyTime): StandbyTime Enum, default = StandbyTime_1000ms
                             BME280::Filter_Off,            // Filter (filter): Filter Enum, default = Filter_16
                             BME280::SpiEnable_False        // SPI Enable: SpiEnable Enum, default = false
);

class Sens_BME280 : public Sensor {

    int16_t  _temperature;
    uint16_t _pressure;
    uint16_t _pressureNN;
    uint8_t  _humidity;
    BME280I2C
    _bme280;

    void measureRaw(uint16_t altitude)
    {
        float temp(NAN), hum(NAN), pres(NAN);
        _bme280.read(pres, temp, hum, BME280::TempUnit_Celsius, BME280::PresUnit_hPa);    // hPa
        _temperature = (int16_t)(temp * 10);                                              // HB-UNI-Sensor1: C*10
        _pressure    = (uint16_t)(pres * 10);                                             // HB-UNI-Sensor1: hPa*10
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
        Wire.begin();

        uint8_t i = 10;
        while ((!_bme280.begin()) && (i > 0)) {
            delay(100);
            i--;
        }

        switch (_bme280.chipModel()) {
        case BME280::ChipModel_BME280:
            _present = true;
            _bme280.setSettings(settings);
            DPRINTLN(F("BME280 found"));
            measureRaw(0);    // dummy read da die Messwerte der ersten Messung nach power-on weit daneben liegen
            break;
        case BME280::ChipModel_BMP280:
            _present = true;
            _bme280.setSettings(settings);
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
            DPRINT(F("BME280 Temperature x10  : "));
            DDECLN(_temperature);
            DPRINT(F("BME280 Pressure x10     : "));
            DDECLN(_pressure);
            DPRINT(F("BME280 PressureNN x10   : "));
            DDECLN(_pressureNN);
            DPRINT(F("BME280 Humidity         : "));
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
