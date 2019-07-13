
//---------------------------------------------------------
// Sens_BH1750
// 2019-06-30 Wolfram Winter (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _SENS_BH1750_H_
#define _SENS_BH1750_H_

#include <Wire.h>
#include <Sensors.h>
#include <AS_BH1750.h>    // https://github.com/hexenmeister/AS_BH1750

namespace as {

template <uint8_t I2C_ADDR> class Sens_BH1750 : public Sensor {

    uint32_t    _brightnessLux100;
    ::AS_BH1750 _bh1750;

public:
    // constructor with parameter in header file -> Initalize with 'member initialiser' syntax in constructor
    Sens_BH1750()
        : _bh1750(I2C_ADDR)
        , _brightnessLux100(0)
    {
    }

    bool init()
    {
        Wire.begin();

        uint8_t i = 10;
        while (i > 0) {
            Wire.beginTransmission(I2C_ADDR);
            if (Wire.endTransmission() == 0) {

                _present = _bh1750.begin();    // Default: Modus=RESOLUTION_AUTO_HIGH, AutoPowerDown=true

                // https://github.com/hexenmeister/AS_BH1750/blob/master/AS_BH1750.cpp
                //
                // - Modus für die Sensorauflösung:
                //   -- RESOLUTION_LOW:         Physische Sensormodus mit 4 lx Auflösung. Messzeit ca. 16ms. Bereich 0-54612.
                //   -- RESOLUTION_NORMAL:      Physische Sensormodus mit 1 lx Auflösung. Messzeit ca. 120ms. Bereich 0-54612.
                //   -- RESOLUTION_HIGH:        Physische Sensormodus mit 0,5 lx Auflösung. Messzeit ca. 120ms. Bereich 0-54612.
                //                              (Die Messbereiche können durch Änderung des MTreg verschoben werden.)
                //   -- RESOLUTION_AUTO_HIGH:   Die Werte im MTreg werden je nach Helligkeit automatisch so angepasst,
                //                              dass eine maximalmögliche Auflösung und Messbereich erziehlt werden.
                //                              Die messbaren Werte fangen von 0,11 lx und gehen bis über 100000 lx.
                //                              (ich weis nicht, wie genau die Werte in Grenzbereichen sind,
                //                              besonders bei hohen Werte habe ich da meine Zweifel.
                //                              Die Werte scheinen jedoch weitgehend linear mit der steigenden Helligkeit zu wachsen.)
                //                              Auflösung im Unteren Bereich ca. 0,13 lx, im mittleren 0,5 lx, im oberen etwa 1-2 lx.
                //                              Die Messzeiten verlängern sich durch mehrfache Messungen und
                //                              die Änderungen von Measurement Time (MTreg) bis max. ca. 500 ms.
                // - AutoPowerDown: true = Der Sensor wird nach der Messung in den Stromsparmodus versetzt.
                //   Das spätere Aufwecken wird ggf. automatisch vorgenommen, braucht jedoch geringfügig mehr Zeit.

                if (_present) {
                    DPRINTLN(F("BH1750 found"));
                    return true;
                }
            }
            delay(100);
            i--;
        }
        DPRINTLN(F("Error: BH1750 not found"));
        return false;
    }

    bool measure()
    {
        _brightnessLux100 = 0;
        if (_present == true) {

            float lux = _bh1750.readLightLevel();

            _brightnessLux100 = (uint32_t)floor(100.0 * lux + 0.5);

            DPRINT(F("BH1750 Brightness Lux x100: "));
            DDECLN(_brightnessLux100);
            return true;
        }
        return false;
    }

    uint32_t brightnessLux() { return _brightnessLux100; }
};

}

#endif
