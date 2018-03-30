
#ifndef _SENS_TSL2561_H_
#define _SENS_TSL2561_H_

#include <Wire.h>
#include <Sensors.h>
#include <TSL2561.h>

namespace as {

class Sens_Tsl2561 : public Sensor {

  uint16_t  _brightnessFull, _brightnessIR, _brightnessVis;
  uint32_t  _brightnessLux;
  bool      _x16;
  ::TSL2561 _tsl2561;

public:

  Sens_Tsl2561 () : _tsl2561(TSL2561_ADDR_FLOAT), _x16(true) { }  // !! constructor with parameter in header file -> Initalize with 'member initialiser' syntax in constructor

  void init () {

    Wire.begin(); //ToDo sync with further I2C sensor classes if needed

    uint8_t i = 10;
    while( (!_tsl2561.begin()) && (i > 0) ) {
      delay(100);
      i--;
    }
    if (i > 0) {
      _present = true;
      _tsl2561.setGain(TSL2561_GAIN_16X);
      _tsl2561.setTiming(TSL2561_INTEGRATIONTIME_402MS);
      DPRINTLN("Success: found TSL2561 sensor");
    }
    else {
      DPRINTLN("Error: no TSL2561 sensor found");
    }
  }

  void measure () {
    if (_present == true) {
      uint32_t lum = _tsl2561.getFullLuminosity();
      uint16_t vis = ((lum & 0xFFFF) - (lum >> 16));
      if( vis > 63000 && _x16 == true ) {
        _x16 = false;
        _tsl2561.setGain(TSL2561_GAIN_0X);
        lum = _tsl2561.getFullLuminosity();
      }
      else if ( vis < 500 && _x16 == false ) {
        _x16 = true;
        _tsl2561.setGain(TSL2561_GAIN_16X);
        lum = _tsl2561.getFullLuminosity();
      }
      _brightnessFull = lum & 0xFFFF;
      _brightnessIR   = lum >> 16;
      _brightnessVis  = _brightnessFull - _brightnessIR;
      _brightnessLux  = _tsl2561.calculateLux(_brightnessFull, _brightnessIR);
      
      DPRINT("TSL2561 Brightness Full: "); DDECLN(_brightnessFull);
      DPRINT("TSL2561 Brightness IR  : "); DDECLN(_brightnessIR);
      DPRINT("TSL2561 Brightness Vis : "); DDECLN(_brightnessVis);
      DPRINT("TSL2561 Brightness Lux : "); DDECLN(_brightnessLux);
    }
  }
  
  uint16_t  brightnessFull () { return _brightnessFull; }
  uint16_t  brightnessIR ()   { return _brightnessIR; }
  uint16_t  brightnessVis ()  { return _brightnessVis; }
  uint32_t  brightnessLux ()  { return _brightnessLux; }
};

}

#endif
