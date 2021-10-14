//---------------------------------------------------------
// Sens_AHTxx
// 2021-10 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
// enjoyneering https://github.com/enjoyneering/AHTxx GPLv3
// For Aosong ASAIR AHT1x/AHT2x, Temperature & Humidity Sensors
//---------------------------------------------------------

#ifndef _SENS_AHTXX_H_
#define _SENS_AHTXX_H_

#include <Wire.h>
#include <Sensors.h>

// clang-format off
/* list of I2C addresses */
#define AHTXX_ADDRESS_X38                 0x38  //AHT15/AHT20/AHT21/AHT25 I2C address, AHT10 I2C address if address pin to GND
#define AHT10_ADDRESS_X39                 0x39  //AHT10 I2C address, if address pin to Vcc

/* list of command registers */
#define AHT1X_INIT_REG                    0xE1  //initialization register, for AHT1x only
#define AHT2X_INIT_REG                    0xBE  //initialization register, for AHT2x only
#define AHTXX_STATUS_REG                  0x71  //read status byte register
#define AHTXX_START_MEASUREMENT_REG       0xAC  //start measurement register
#define AHTXX_SOFT_RESET_REG              0xBA  //soft reset register

/* calibration register controls */
#define AHT1X_INIT_CTRL_NORMAL_MODE       0x00  //normal mode on/off       bit[6:5], for AHT1x only
#define AHT1X_INIT_CTRL_CYCLE_MODE        0x20  //cycle mode on/off        bit[6:5], for AHT1x only
#define AHT1X_INIT_CTRL_CMD_MODE          0x40  //command mode  on/off     bit[6:5], for AHT1x only
#define AHTXX_INIT_CTRL_CAL_ON            0x08  //calibration coeff on/off bit[3]
#define AHTXX_INIT_CTRL_NOP               0x00  //NOP control, send after any "AHT1X_INIT_CTRL..."

/* status byte register controls */
#define AHTXX_STATUS_CTRL_BUSY            0x80  //busy                      bit[7]
#define AHT1X_STATUS_CTRL_NORMAL_MODE     0x00  //normal mode status        bit[6:5], for AHT1x only
#define AHT1X_STATUS_CTRL_CYCLE_MODE      0x20  //cycle mode status         bit[6:5], for AHT1x only
#define AHT1X_STATUS_CTRL_CMD_MODE        0x40  //command mode status       bit[6:5], for AHT1x only
#define AHTXX_STATUS_CTRL_CRC             0x10  //CRC8 status               bit[4], no info in datasheet
#define AHTXX_STATUS_CTRL_CAL_ON          0x08  //calibration coeff status  bit[3]
#define AHTXX_STATUS_CTRL_FIFO_ON         0x04  //FIFO on status            bit[2], no info in datasheet
#define AHTXX_STATUS_CTRL_FIFO_FULL       0x02  //FIFO full status          bit[1], no info in datasheet
#define AHTXX_STATUS_CTRL_FIFO_EMPTY      0x02  //FIFO empty status         bit[1], no info in datasheet

/* measurement register controls */
#define AHTXX_START_MEASUREMENT_CTRL      0x33  //measurement controls, suspect this is temperature & humidity DAC resolution
#define AHTXX_START_MEASUREMENT_CTRL_NOP  0x00  //NOP control, send after any "AHTXX_START_MEASUREMENT_CTRL..."

/* sensor delays */
#define AHTXX_CMD_DELAY                   10      //delay between commands, in milliseconds
#define AHTXX_MEASUREMENT_DELAY           80      //wait for measurement to complete, in milliseconds
#define AHT1X_POWER_ON_DELAY              40      //wait for AHT1x to initialize after power-on, in milliseconds
#define AHT2X_POWER_ON_DELAY              100     //wait for AHT2x to initialize after power-on, in milliseconds
#define AHTXX_SOFT_RESET_DELAY            20      //less than 20 milliseconds

/* misc */
#define AHTXX_FORCE_READ_DATA             true    //force to read data via I2C
#define AHTXX_USE_READ_DATA               false   //force to use data from previous read

#define AHTXX_NO_ERROR                    0x00    //success, no errors
#define AHTXX_BUSY_ERROR                  0x01    //sensor is busy
#define AHTXX_ACK_ERROR                   0x02    //sensor didn't return ACK (not connected, broken, long wires (reduce speed), bus locked by slave (increase stretch limit))
#define AHTXX_DATA_ERROR                  0x03    //received data smaller than expected
#define AHTXX_CRC8_ERROR                  0x04    //computed CRC8 not match received CRC8, for AHT2x only
#define AHTXX_ERROR                       0xFF    //other errors
// clang-format on

typedef enum : uint8_t {
    AHT1x_SENSOR = 0x00,
    AHT2x_SENSOR = 0x01,
} AHTXX_I2C_SENSOR;

namespace as {

//-----------------------------------------------------------------------------
class Sens_AHTxx : public Sensor {

private:
    AHTXX_I2C_SENSOR _sensorType;
    uint8_t          _address;
    uint8_t          _status;
    uint8_t          _rawData[7];    // {status, RH, RH, RH+T, T, T, CRC}, CRC for AHT2x only
    int16_t          _temperature;
    uint16_t         _humidity;

    //---------------------------------------------------------
    bool begin()
    {
        Wire.begin();
        // Wire.setClock(100000);
        Wire.setWireTimeout(25000, true);    // TM: this seems to avoid the lock problem when a AHT15 and a BME280 are together on the same I2C bus
        delay(AHT2X_POWER_ON_DELAY);         // wait for sensor to initialize
        return softReset();    // soft reset is recommended at start (reset, set normal mode, set calibration bit & check calibration bit)
    }

    //---------------------------------------------------------
    bool softReset()
    {
        Wire.beginTransmission(_address);
        Wire.write(AHTXX_SOFT_RESET_REG);
        if (Wire.endTransmission(true) != 0) {
            return false;
        }    // collision on I2C bus, sensor didn't return ACK
        delay(AHTXX_SOFT_RESET_DELAY);
        return ((setNormalMode() == true) && (getCalibration() == AHTXX_STATUS_CTRL_CAL_ON));    // set mode & check calibration bit
    }

    //---------------------------------------------------------
    void readMeasurement()
    {
        /* send measurement command */
        Wire.beginTransmission(_address);

        Wire.write(AHTXX_START_MEASUREMENT_REG);         // send measurement command, strat measurement
        Wire.write(AHTXX_START_MEASUREMENT_CTRL);        // send measurement control
        Wire.write(AHTXX_START_MEASUREMENT_CTRL_NOP);    // send measurement NOP control

        if (Wire.endTransmission(true) != 0)    // collision on I2C bus
        {
            _status = AHTXX_ACK_ERROR;    // update status byte, sensor didn't return ACK
            return;                       // no reason to continue
        }

        /* check busy bit */
        _status = getBusy(AHTXX_FORCE_READ_DATA);    // update status byte, read status byte & check busy bit

        if (_status == AHTXX_BUSY_ERROR) {
            delay(AHTXX_MEASUREMENT_DELAY - AHTXX_CMD_DELAY);
        } else if (_status != AHTXX_NO_ERROR) {
            return;
        }    // no reason to continue, received data smaller than expected

        /* read data from sensor */
        uint8_t dataSize;
        if (_sensorType == AHT1x_SENSOR) {
            dataSize = 6;
        }    //{status, RH, RH, RH+T, T, T, CRC*}, *CRC for AHT2x only
        else {
            dataSize = 7;
        }

        Wire.requestFrom(_address, dataSize, (uint8_t) true);    // read n-byte to "wire.h" rxBuffer, true-send stop after transmission
        if (Wire.available() != dataSize) {
            _status = AHTXX_DATA_ERROR;    // update status byte, received data smaller than expected
            return;                        // no reason to continue
        }

        /* read n-bytes from "wire.h" rxBuffer */
        for (uint8_t i = 0; i < dataSize; i++) {
            _rawData[i] = Wire.read();
        }

        /* check busy bit after measurement dalay */
        _status = getBusy(AHTXX_USE_READ_DATA);    // update status byte, read status byte & check busy bit

        if (_status != AHTXX_NO_ERROR) {
            return;    // no reason to continue, sensor is busy
        }

        /* check CRC8, for AHT2x only */
        if ((_sensorType == AHT2x_SENSOR) && (checkCRC8() != true)) {
            _status = AHTXX_CRC8_ERROR;
        }    // update status byte
    }

    //---------------------------------------------------------
    bool setInitializationRegister(uint8_t value)
    {
        delay(AHTXX_CMD_DELAY);
        Wire.beginTransmission(_address);
        if (_sensorType == AHT1x_SENSOR) {
            Wire.write(AHT1X_INIT_REG);
        }    // send initialization command, for AHT1x only
        else {
            Wire.write(AHT2X_INIT_REG);
        }                                            // send initialization command, for AHT2x only
        Wire.write(value);                           // send initialization register controls
        Wire.write(AHTXX_INIT_CTRL_NOP);             // send initialization register NOP control
        return (Wire.endTransmission(true) == 0);    // true=success, false=I2C error
    }

    //---------------------------------------------------------
    uint8_t readStatusRegister()
    {
        delay(AHTXX_CMD_DELAY);
        Wire.beginTransmission(_address);
        Wire.write(AHTXX_STATUS_REG);
        if (Wire.endTransmission(true) != 0) {
            return AHTXX_ERROR;
        }                                                          // collision on I2C bus, sensor didn't return ACK
        Wire.requestFrom(_address, (uint8_t)1, (uint8_t) true);    // read 1-byte to "wire.h" rxBuffer, true-send stop after transmission
        if (Wire.available() == 1) {
            return Wire.read();
        }                      // read 1-byte from "wire.h" rxBuffer
        return AHTXX_ERROR;    // collision on I2C bus, "wire.h" rxBuffer is empty
    }

    //---------------------------------------------------------
    uint8_t getCalibration()
    {
        uint8_t value = readStatusRegister();
        if (value != AHTXX_ERROR) {
            return (value & AHTXX_STATUS_CTRL_CAL_ON);
        }                      // 0x08=loaded, 0x00=not loaded
        return AHTXX_ERROR;    // collision on I2C bus, sensor didn't return ACK
    }

    //---------------------------------------------------------
    uint8_t getBusy(bool readAHT)
    {
        if (readAHT == AHTXX_FORCE_READ_DATA) {    // force to read data via I2C & update "_rawData[]" buffer
            delay(AHTXX_CMD_DELAY);
            Wire.requestFrom(_address, (uint8_t)1, (uint8_t) true);    // read 1-byte to "wire.h" rxBuffer, true-send stop after transmission
            if (Wire.available() != 1) {
                return AHTXX_DATA_ERROR;
            }                             // no reason to continue, "return" terminates the entire function & "break" just exits the loop
            _rawData[0] = Wire.read();    // read 1-byte from "wire.h" rxBuffer
        }
        if ((_rawData[0] & AHTXX_STATUS_CTRL_BUSY) == AHTXX_STATUS_CTRL_BUSY) {
            _status = AHTXX_BUSY_ERROR;
        }    // 0x80=busy, 0x00=measurement completed
        else {
            _status = AHTXX_NO_ERROR;
        }
        return _status;
    }

    //---------------------------------------------------------
    bool readHumidity(bool readI2C, float& value)
    {
        value = 0.0;
        if (readI2C == AHTXX_FORCE_READ_DATA) {    // force to read data via I2C & update "_rawData[]" buffer
            readMeasurement();
        }
        if (_status != AHTXX_NO_ERROR) {
            return false;    // no reason to continue, call "getStatus()" for error description
        }
        uint32_t humidity = _rawData[1];    // 20-bit raw humidity data
        humidity <<= 8;
        humidity |= _rawData[2];
        humidity <<= 4;
        humidity |= _rawData[3] >> 4;
        if (humidity > 0x100000) {
            humidity = 0x100000;
        }    // check if RH>100, no need to check for RH<0 since "humidity" is "uint"
        value = ((float)humidity / 0x100000) * 100;
        return true;
    }

    //---------------------------------------------------------
    bool readTemperature(bool readAHT, float& value)
    {
        value = 0.0;
        if (readAHT == AHTXX_FORCE_READ_DATA) {    // force to read data via I2C & update "_rawData[]" buffer
            readMeasurement();
        }
        if (_status != AHTXX_NO_ERROR) {
            return false;    // no reason to continue, call "getStatus()" for error description
        }
        uint32_t temperature = _rawData[3] & 0x0F;    // 20-bit raw temperature data
        temperature <<= 8;
        temperature |= _rawData[4];
        temperature <<= 8;
        temperature |= _rawData[5];
        value = ((float)temperature / 0x100000) * 200 - 50;
        return true;
    }

    //---------------------------------------------------------
    bool checkCRC8()
    {
        if (_sensorType == AHT2x_SENSOR) {
            uint8_t crc = 0xFF;                                          // initial value
            for (uint8_t byteIndex = 0; byteIndex < 6; byteIndex++) {    // 6-bytes in data, {status, RH, RH, RH+T, T, T, CRC}
                crc ^= _rawData[byteIndex];
                for (uint8_t bitIndex = 8; bitIndex > 0; --bitIndex) {    // 8-bits in byte
                    if (crc & 0x80) {
                        crc = (crc << 1) ^ 0x31;    // 0x31=CRC seed/polynomial
                    } else {
                        crc = (crc << 1);
                    }
                }
            }
            return (crc == _rawData[6]);
        }
        return true;
    }

    //---------------------------------------------------------
    bool setNormalMode() { return setInitializationRegister(AHTXX_INIT_CTRL_CAL_ON | AHT1X_INIT_CTRL_NORMAL_MODE); }

    //---------------------------------------------------------
    bool measureRaw()
    {
        float f;
        if (readTemperature(AHTXX_FORCE_READ_DATA, f)) {    // read 6-bytes via I2C, takes 80 milliseconds
            _temperature = (int16_t)(f * 10.0);             // HB-UNI-Sensor1: Temp *10
            if (readHumidity(AHTXX_USE_READ_DATA, f)) {     // use 6-bytes from temperature reading, takes zero milliseconds
                _humidity = (uint16_t)(f * 10.0);           // HB-UNI-Sensor1: Humi *10
                DPRINT(F("AHTxx Temperature x10   : "));
                DDECLN(_temperature);
                DPRINT(F("AHTxx Humidity x10      : "));
                DDECLN(_humidity);
                return true;
            }
        }
        return false;
    }

public:
    Sens_AHTxx()
        : _sensorType(AHT1x_SENSOR)
        , _address(AHTXX_ADDRESS_X38)
        , _status(AHTXX_NO_ERROR)
        , _rawData { 0 }
        , _temperature(0)
        , _humidity(0)
    {
    }

    //---------------------------------------------------------
    bool init()
    {
        uint8_t i = 10;
        while (i > 0) {
            if (begin()) {
                _present = true;
                DPRINTLN(F("AHT1x found"));
                return true;
            }
            delay(100);
            i--;
        }
        DPRINTLN(F("Error: AHT1x not connected or failed to load calib. coeff."));
        return false;
    }

    //---------------------------------------------------------
    bool measure()
    {
        _temperature = _humidity = 0;
        if (_present == true) {
            if (measureRaw()) {
                return true;
            } else {
                DPRINT(F("AHTxx read error: "));
                DDECLN(_status);
                DPRINTLN(F("Soft reset"));
                softReset();
                delay(250);
                return measureRaw();
            }
        }
        return false;
    }

    //---------------------------------------------------------
    int16_t  temperature() { return _temperature; }
    uint16_t humidity() { return _humidity; }
};

}

#endif
