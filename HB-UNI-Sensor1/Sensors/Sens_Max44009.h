
#ifndef _SENS_MAX44009_H_
#define _SENS_MAX44009_H_

#include <Wire.h>
#include <Sensors.h>

#define MAX44009_ADDR	0x4A

namespace as {

class Sens_Max44009 : public Sensor {

  uint32_t  _brightnessLux;

public:

  Sens_Max44009() : _brightnessLux(0) { }

  bool init() {

    Wire.begin(); //ToDo sync with further I2C sensor classes if needed

    uint8_t i = 10;
    while (i > 0) {
      Wire.beginTransmission(MAX44009_ADDR);
      if (Wire.endTransmission() == 0) {
        _present = true;
	Wire.beginTransmission(MAX44009_ADDR);
        // set continuous mode 800ms (lowest possible supply current), automatic mode (autorange for timing and gain)
        Wire.write(0x02);
        Wire.write(0x03);
        Wire.endTransmission();
        DPRINTLN("MAX44009 found");
        return true;
      }
      delay(100);
      i--;
    }
    DPRINTLN("Error: MAX44009 not found");
    return false;
  }

  bool measure() {
    if (_present == true) {
      // If user wants to read both the Lux High-Byte register 0x03 and Lux Low-Byte register 0x04, then the master should not
      // send a STOP command between the reads of the two registers. Instead a Repeated START command should be used.
      // This ensures accurate data is obtained from the I 2 C registers (by disabling internal updates during the read process).
      Wire.beginTransmission(MAX44009_ADDR); Wire.write(0x03); Wire.endTransmission();
      if (Wire.requestFrom(MAX44009_ADDR, 1, false) == 1) {	// Repeated START
        uint8_t data[2];
	data[0] = Wire.read();
        Wire.beginTransmission(MAX44009_ADDR); Wire.write(0x04); Wire.endTransmission();
        if (Wire.requestFrom(MAX44009_ADDR, 1, true) == 1) {
          data[1] = Wire.read();
          //DHEX(data[0]); DPRINT(" "); DHEXLN(data[1]);
          int expo = (data[0] >> 4) & 0x0F;
          int mant = ((data[0] & 0x0F) << 4) | (data[1] & 0x0F);
          float lux = pow(2, expo) * mant * 0.045;
          _brightnessLux = (uint32_t)lux;
          return true;
        }
      }
    }
    return false;
  }

  uint32_t  brightnessLux()   { return _brightnessLux; }
};

}

#endif
