#ifndef __SENSORS_DS18B20_h__
#define __SENSORS_DS18B20_h__

#include <Sensors.h>
#include <OneWire.h>
//https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <DallasTemperature.h>

namespace as {

template <int DATAPIN>
class Ds18b20 : public Temperature {
  OneWire oneWire = OneWire(DATAPIN);
  DallasTemperature _ds18b20 = DallasTemperature(&oneWire);
  int _i;

public:
  Ds18b20 () : _ds18b20(&oneWire) {}

  void init () {
    _ds18b20.begin();
    _present = true;
  }
  void measure (uint8_t ds_num, __attribute__((unused)) bool async=false) {
    if( present() == true ) {
      _i = 0;
      do {
        if( _i != 0 ) {
          delay(200);
          DPRINT("DS18B20 Try # ");DDECLN(_i);
        }
        _ds18b20.requestTemperatures();
        _temperature = _ds18b20.getTempCByIndex(ds_num) * 10;
        _i = _i + 1;
      } while ((_temperature == 0) && _i <= 10);
    }
  }
};

}

#endif
