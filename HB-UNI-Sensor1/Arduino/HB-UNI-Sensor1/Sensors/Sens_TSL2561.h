
//---------------------------------------------------------
// Sens_TSL2561
// 2019-04-08 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _SENS_TSL2561_H_
#define _SENS_TSL2561_H_

#include <Wire.h>
#include <Sensors.h>
#include <TSL2561.h>

namespace as {

template <uint8_t I2C_ADDR> class Sens_TSL2561 : public Sensor {

    uint16_t  _brightnessRawFull, _brightnessRawIR;
    uint32_t  _brightnessLux100;
    uint8_t   _sensitivity;
    ::TSL2561 _tsl2561;

    uint8_t getID()
    {
        Wire.beginTransmission(I2C_ADDR);
        Wire.write(TSL2561_REGISTER_ID);
        Wire.endTransmission();
        if (Wire.requestFrom(I2C_ADDR, (uint8_t)1) == 1) {
            return Wire.read();
        }
        return 0x00;
    }

    void measureRaw()
    {
        switch (_sensitivity) {
        case 0:
            _tsl2561.setTiming(TSL2561_INTEGRATIONTIME_13MS);
            break;
        case 1:
            _tsl2561.setTiming(TSL2561_INTEGRATIONTIME_101MS);
            break;
        case 2:
        default:
            _tsl2561.setTiming(TSL2561_INTEGRATIONTIME_402MS);
            break;
        }
        uint32_t lum       = _tsl2561.getFullLuminosity();
        _brightnessRawFull = lum & 0xFFFF;
        _brightnessRawIR   = lum >> 16;
    }

public:
    // constructor with parameter in header file -> Initalize with 'member initialiser' syntax in constructor
    Sens_TSL2561()
        : _tsl2561(I2C_ADDR)
        , _sensitivity(2)
        , _brightnessRawFull(0)
        , _brightnessRawIR(0)
        , _brightnessLux100(0)
    {
    }

    bool init()
    {
        Wire.begin();

        uint8_t i = 10;
        while (i > 0) {
            if ((_tsl2561.begin()) && (getID() == 0x0A)) {    // Adafruit Lib begin() return code funktioniert nicht bei nicht angeschl.
                                                              // Sensor, deshalb eigener check hier
                _present = true;
                _tsl2561.setGain(TSL2561_GAIN_0X);
                _tsl2561.setTiming(TSL2561_INTEGRATIONTIME_402MS);
                DPRINTLN(F("TSL2561 found"));
                return true;
            }
            delay(100);
            i--;
        }
        DPRINTLN(F("Error: TSL2561 not found"));
        return false;
    }

    bool measure()
    {
        _brightnessLux100 = 0;
        if (_present == true) {
            _sensitivity = 2;
            do {
                measureRaw();
                // default wird mit Gain 0 und 402ms Messzeit gemessen
                // bei zu viel Licht die Empfindlichkeit (exposure time) solange verringern bis sinnvolle Werte kommen
                // momentan wird hier nur mit Gain 0 gearbeitet, Gain 16 ist fuer Aussenhelligkeiten problematisch (siehe Messwerte) und
                // wuerde nur etwas bringen falls man in sehr dunklen Umgebungen Lux-Werte mit hoher Aufloesung braucht
                if ((_brightnessRawFull == _brightnessRawIR) || (_brightnessRawFull > 60000)) {    // ungueltiger Wert
                    // DPRINT("#INVALID# "); DDEC(_brightnessRawFull); DPRINT(" "); DDEC(_brightnessRawIR); DPRINT(" "); DDEC(_sensitivity);
                    // DPRINT(F("\r\n"));
                    if (_sensitivity > 0) {    // ungueltiger Wert, Empfindlichkeit verringern
                        _sensitivity--;
                        delay(100);                     // Photonen overflow Erholung
                    } else {                            // ungueltiger Wert, keine weitere Verringerung der Empfindlichkeit moeglich
                        _brightnessLux100 = 99999ul;    // overflow, zu viel Licht, besser dem SmartHome System einen hohen Helligkeitswert
                                                        // anstatt 0 Lux melden!
                        break;
                    }
                } else {    // // gueltiger Wert
                    // x100 scaling um zum MAX44009 kompatibel zu sein
                    _brightnessLux100 = 100 * _tsl2561.calculateLux(_brightnessRawFull, _brightnessRawIR);
                    break;
                }
            } while (true);

            DPRINT(F("TSL2561 Sensitivity        : "));
            DDECLN(_sensitivity);
            DPRINT(F("TSL2561 Brightness Raw Full: "));
            DDECLN(_brightnessRawFull);
            DPRINT(F("TSL2561 Brightness Raw IR  : "));
            DDECLN(_brightnessRawIR);
            DPRINT(F("TSL2561 Brightness Lux x100: "));
            DDECLN(_brightnessLux100);
            return true;
        }
        return false;
    }

    uint8_t  sensitivity() { return _sensitivity; }
    uint16_t brightnessRawFull() { return _brightnessRawFull; }
    uint16_t brightnessRawIR() { return _brightnessRawIR; }
    uint32_t brightnessLux() { return _brightnessLux100; }
};

}

#endif

/*

Innenraum
-------------------------------------------------------------------------------
                                        Faktor	sensitivity
Time  13  Gain  0  Lum 00060015  Full    21  IR     6  Vis    15  Lux   192	1	0
Time 101  Gain  0  Lum 002F009C  Full   156  IR    47  Vis   109  Lux   186	7.8	1
Time 402  Gain  0  Lum 00BC026F  Full   623  IR   188  Vis   435  Lux   186	31	2
Time  13  Gain 16  Lum 0064014B  Full   331  IR   100  Vis   231  Lux   181	16	-
Time 101  Gain 16  Lum 02EF09BA  Full  2490  IR   751  Vis  1739  Lux   185	125	-
Time 402  Gain 16  Lum 0BB826D0  Full  9936  IR  3000  Vis  6936  Lux   186	500	-

Sonne, Juni, 18 Uhr, Sensor im Halbschatten
-------------------------------------------------------------------------------
Time  13  Gain  0  Lum 025305EF  Full  1519  IR   595  Vis   924  Lux  9713
Time 101  Gain  0  Lum 114D2C25  Full 11301  IR  4429  Vis  6872  Lux  9805
Time 402  Gain  0  Lum 4451AE93  Full 44691  IR 17489  Vis 27202  Lux  9779
Time  13  Gain 16  Lum 13441344  Full  4932  IR  4932  Vis     0  Lux    53	invalid
Time 101  Gain 16  Lum 90C690C6  Full 37062  IR 37062  Vis     0  Lux    54	invalid
Time 402  Gain 16  Lum FFFFFFFF  Full 65535  IR 65535  Vis     0  Lux    24	invalid

Sonne, Juni, 18 Uhr, Sensor direkt in die Sonne ausgerichtet
-------------------------------------------------------------------------------
Time  13  Gain  0  Lum 13441344  Full  4932  IR  4932  Vis     0  Lux   846	invalid
Time 101  Gain  0  Lum 90C690C6  Full 37062  IR 37062  Vis     0  Lux   863	invalid
Time 402  Gain  0  Lum FFFFFFFF  Full 65535  IR 65535  Vis     0  Lux   384	invalid
Time  13  Gain 16  Lum 13441344  Full  4932  IR  4932  Vis     0  Lux    53	invalid
Time 101  Gain 16  Lum 90C690C6  Full 37062  IR 37062  Vis     0  Lux    54	invalid
Time 402  Gain 16  Lum FFFFFFFF  Full 65535  IR 65535  Vis     0  Lux    24	invalid

*/
