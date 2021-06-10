//---------------------------------------------------------
// Sens_BME280
// (C) 2018-2021 Tom Major (Creative Commons)
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

    int16_t   _temperature10;
    uint16_t  _pressure10;
    uint16_t  _pressureNN10;
    uint16_t  _humidity10;
    uint16_t  _altitude;
    BME280I2C _bme280;

    void measureRaw()
    {
        float temp(NAN), hum(NAN), pres(NAN);
        _bme280.read(pres, temp, hum, BME280::TempUnit_Celsius, BME280::PresUnit_hPa);    // hPa
        _temperature10 = (int16_t)(temp * 10.0);                                          // HB-UNI-Sensor1: Temp *10
        _pressure10    = (uint16_t)(pres * 10.0);                                         // HB-UNI-Sensor1: Press *10
        _pressureNN10  = (uint16_t)(EnvironmentCalculations::EquivalentSeaLevelPressure(float(_altitude), temp, pres) * 10.0);
        _humidity10    = (uint16_t)(hum * 10.0);    // HB-UNI-Sensor1: Humi *10
    }

public:
    Sens_BME280()
        : _temperature10(0)
        , _pressure10(0)
        , _pressureNN10(0)
        , _humidity10(0)
        , _altitude(0)
    {
    }

    void init(uint16_t altitude)
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
            break;
        case BME280::ChipModel_BMP280:
            _present = true;
            _bme280.setSettings(settings);
            DPRINTLN(F("BMP280 found (No Humidity available)"));
            break;
        default:
            DPRINTLN(F("Error: no BME280/BMP280 sensor found"));
        }

        if (_present == true) {
            _altitude = altitude;
            measureRaw();    // dummy read da die Messwerte der ersten Messung nach power-on weit daneben liegen
        }
    }

    void measure()
    {
        _temperature10 = _pressure10 = _pressureNN10 = _humidity10 = 0;
        if (_present == true) {
            measureRaw();
            DPRINT(F("BME280 Temperature x10  : "));
            DDECLN(_temperature10);
            DPRINT(F("BME280 Pressure x10     : "));
            DDECLN(_pressure10);
            DPRINT(F("BME280 PressureNN x10   : "));
            DDECLN(_pressureNN10);
            DPRINT(F("BME280 Humidity x10     : "));
            DDECLN(_humidity10);
        }
    }

    int16_t  temperature() { return _temperature10; }
    uint16_t pressure() { return _pressure10; }
    uint16_t pressureNN() { return _pressureNN10; }
    uint16_t humidity() { return _humidity10; }

    // absHumidity kann optional verwendet werden nur für Geräte die dies benötigen
    // die Offset-Korrekturen werden zentral im HB-UNI-SensorX Sketch am Ende von measure() berechnet
    // und können so bei der Berechnung von absHumidity einfliessen wenn absHumidity() erst nach den Offset-Korrekturen aufgerufen wird!
    // so werden die Sensorklassen nicht mit extra Code für die Offset-Korrekturen belastet
    // siehe auch HB-UNI-Sensor2.ino
    uint16_t absHumidity(int16_t temp10, uint16_t humi10)
    {
        float    temp  = (float)(temp10) / 10.0;
        float    hum   = (float)(humi10) / 10.0;
        uint16_t aH100 = (uint16_t)(EnvironmentCalculations::AbsoluteHumidity(temp, hum, EnvironmentCalculations::TempUnit_Celsius) * 100.0);
        DPRINT(F("BME280 AbsHumidity x100 : "));
        DDECLN(aH100);
        return aH100;
    }

    // dewPoint kann optional verwendet werden nur für Geräte die dies benötigen
    int16_t dewPoint(int16_t temp10, uint16_t humi10)
    {
        float   temp = (float)(temp10) / 10.0;
        float   hum  = (float)(humi10) / 10.0;
        int16_t dP10 = (int16_t)(EnvironmentCalculations::DewPoint(temp, hum, EnvironmentCalculations::TempUnit_Celsius) * 10.0);
        DPRINT(F("BME280 DewPoint x10     : "));
        DDECLN(dP10);
        return dP10;
    }
};

}

#endif
