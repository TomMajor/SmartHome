
//---------------------------------------------------------
// Sens_DS18X20
// 2018-08-12 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _SENS_DS18X20_H_
#define _SENS_DS18X20_H_

#include <Sensors.h>
#include <OneWire.h>

namespace as {

class Sens_DS18X20 : public Sensor {

public:
    Sens_DS18X20()
        : _oneWire(0)
        , _temperature(-990)
    {
    }

    void init(uint8_t pin)
    {
        _oneWire.begin(pin);
        if (_oneWire.search(_addr) == 1) {
            if (OneWire::crc8(_addr, 7) == _addr[7]) {
                if (valid(_addr) == true) {
                    _present = true;
                    DPRINT("DS18x20 found: ");
                    for (uint8_t i = 0; i < 8; i++) {
                        DHEX(_addr[i]);
                    }
                    DPRINTLN("");
                }
            }
        }
        _oneWire.reset_search();
        if (!_present) {
            DPRINTLN("ERROR: no DS18x20 found");
        }
    }

    void measure(__attribute__((unused)) bool async = false)
    {
        _temperature = -990;
        if (_present == true) {
            convert();
            wait();
            read();
        }
    }

    int16_t temperature() { return _temperature; }

private:
    ::OneWire _oneWire;
    int16_t   _temperature;
    uint8_t   _addr[8];

    bool valid(uint8_t* addr) { return *addr == 0x10 || *addr == 0x28 || *addr == 0x22; }

    void convert()
    {
        _oneWire.reset();
        _oneWire.select(_addr);
        _oneWire.write(0x44);    // start conversion, use ds.write(0x44,1) with parasite power on at the end
    }

    void wait() { delay(1000); }

    void read()
    {
        _oneWire.reset();
        _oneWire.select(_addr);
        _oneWire.write(0xBE);    // Read Scratchpad

        uint8_t data[9];
        for (uint8_t i = 0; i < 9; i++) {    // we need 9 bytes
            data[i] = _oneWire.read();
        }

        if (OneWire::crc8(data, 8) == data[8]) {
            int16_t raw = (data[1] << 8) | data[0];
            if (_addr[0] == 0x10) {
                raw = raw << 3;    // 9 bit resolution default
                if (data[7] == 0x10) {
                    // "count remain" gives full 12 bit resolution
                    raw = (raw & 0xFFF0) + 12 - data[6];
                }
            } else {
                byte cfg = (data[4] & 0x60);
                // at lower res, the low bits are undefined, so let's zero them
                if (cfg == 0x00)
                    raw = raw & ~7;    // 9 bit resolution, 93.75 ms
                else if (cfg == 0x20)
                    raw = raw & ~3;    // 10 bit res, 187.5 ms
                else if (cfg == 0x40)
                    raw = raw & ~1;    // 11 bit res, 375 ms
                                       //// default is 12 bit resolution, 750 ms conversion time
            }
            _temperature = (raw * 10) / 16;
            DPRINT("DS18x20  Temperature   : ");
            DDECLN(_temperature);
        } else {
            DPRINTLN("ERROR: DS18x20 crc");
        }
    }
};

}

#endif
