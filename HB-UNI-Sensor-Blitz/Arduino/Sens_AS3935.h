//---------------------------------------------------------
// AS3935.h
// 2020-10-01 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++             2016 papa (Creative Commons)
// HB-UNI-Sen-WEA       2018 jp112sdl (Creative Commons)
// PWFusion_AS3935_I2C  2015 Playing With Fusion, J. Steinlage
// AS3935_Wire (C)      2012 Raivis Rengelis
//---------------------------------------------------------
//
// AS3935 Mode: Voltage Regulator OFF, I2C Active:
// Pin          Connection
//             Sensor   MCU
//-------------------------
// 1   VCC              VCC
// 2   GND              GND
// 3   SCL              SCL
// 4   MOSI             SDA
// 5   MISO    GND
// 6   CS      GND
// 7   SI      VCC
// 8   IRQ              3
// 9   EN_V    GND
// 10  A0      VCC
// 11  A1      GND
//---------------------------------------------------------

#ifndef _SENS_AS3935_H_
#define _SENS_AS3935_H_

#include <limits.h>
#include <Wire.h>

#define AS3935_ADDR 1
#define AS3935_INT_PIN 3
#define CLK_TEST_FREQ_HZ 2026 // 2000 - see README, section calibration

// clang-format off

// register access macros - register address, bitmask
#define AS3935_AFE_GB           0x00, 0x3E
#define AS3935_PWD              0x00, 0x01
#define AS3935_NF_LEV           0x01, 0x70
#define AS3935_WDTH             0x01, 0x0F
#define AS3935_CL_STAT          0x02, 0x40
#define AS3935_MIN_NUM_LIGH     0x02, 0x30
#define AS3935_SREJ             0x02, 0x0F
#define AS3935_LCO_FDIV         0x03, 0xC0
#define AS3935_MASK_DIST        0x03, 0x20
#define AS3935_INT              0x03, 0x0F
#define AS3935_DISTANCE         0x07, 0x1F
#define AS3935_DISP_LCO         0x08, 0x80
#define AS3935_DISP_SRCO        0x08, 0x40
#define AS3935_DISP_TRCO        0x08, 0x20
#define AS3935_TUN_CAP          0x08, 0x0F

#define AS3935_AFE_INDOOR	    0x12
#define AS3935_AFE_OUTDOOR	    0x0E

// clang-format on

namespace as {

class Sens_AS3935 {

public:
    Sens_AS3935()
        : _present(false)
    {
        pinMode(AS3935_INT_PIN, INPUT);
    }

    bool init(uint8_t capacitorIndex, uint8_t indoor, uint8_t showDisturbers, uint8_t noiseFloor, uint8_t spikeRejection, uint8_t watchdogThreshold,
              uint8_t minNumberOfLightnings)
    {
        Wire.begin();
        uint8_t i = 10;
        while (i > 0) {
            Wire.beginTransmission(AS3935_ADDR);
            if (Wire.endTransmission() == 0) {
                _present = true;
                DPRINTLN(F("AS3935 found"));
                reset();
                powerUp();    // does also calibrateRCO()
                setIndoor(indoor);
                setShowDisturbers(showDisturbers);
                writeReg(0x08, 0xE0, 0);    // clear IRQ pin display bits
                delay(200);
                setCapacitorIndex(capacitorIndex);
                delay(200);
                setNoiseFloor(noiseFloor);
                setSpikeRejection(spikeRejection);
                setWatchdogThreshold(watchdogThreshold);
                setMinNumberOfLightnings(minNumberOfLightnings);
                delay(200);
                enableINT();
                return true;
            }
            delay(100);
            i--;
        }
        DPRINTLN(F("ERROR: AS3935 not found"));
        return false;
    }

    void enableINT()
    {
        _notify = false;
        attachInterrupt(digitalPinToInterrupt(AS3935_INT_PIN), isr, RISING);
    }

    void disableINT() { detachInterrupt(digitalPinToInterrupt(AS3935_INT_PIN)); }

    bool notifyEvent() { return (_present == true) ? _notify : false; }

    void resetEvent() { _notify = false; }

    uint8_t interruptSource() { return (_present == true) ? readReg(AS3935_INT) : 255; }

    uint8_t lightningDistanceKm() { return (_present == true) ? readReg(AS3935_DISTANCE) : 255; }

    uint32_t strikeEnergy()
    {
        uint32_t energy = readReg(0x06, 0x1F);
        energy <<= 8;
        energy |= readReg(0x05, 0xFF);
        energy <<= 8;
        energy |= readReg(0x04, 0xFF);
        return energy;
    }

    uint8_t getCapacitorIndex() { return readReg(AS3935_TUN_CAP); }

    uint8_t setCapacitorIndex(uint8_t capIndex)
    {
        writeReg(AS3935_TUN_CAP, capIndex);
        return getCapacitorIndex();
    }

    uint8_t getIndoor()
    {
        uint8_t gain = readReg(AS3935_AFE_GB);
        return (gain == AS3935_AFE_INDOOR);
    }

    uint8_t setIndoor(uint8_t enable)
    {
        if (enable) {
            writeReg(AS3935_AFE_GB, AS3935_AFE_INDOOR);
        } else {
            writeReg(AS3935_AFE_GB, AS3935_AFE_OUTDOOR);
        }
        return getIndoor();
    }

    uint8_t getShowDisturbers()
    {
        uint8_t maskDist = readReg(AS3935_MASK_DIST);
        return (maskDist == 0);
    }

    uint8_t setShowDisturbers(uint8_t show)
    {
        if (show) {
            writeReg(AS3935_MASK_DIST, 0);
        } else {
            writeReg(AS3935_MASK_DIST, 1);
        }
        return getShowDisturbers();
    }

    uint8_t getNoiseFloor() { return readReg(AS3935_NF_LEV); }

    uint8_t setNoiseFloor(uint8_t noisefloor)
    {
        writeReg(AS3935_NF_LEV, noisefloor);
        return getNoiseFloor();
    }

    uint8_t getSpikeRejection() { return readReg(AS3935_SREJ); }

    uint8_t setSpikeRejection(uint8_t srej)
    {
        writeReg(AS3935_SREJ, srej);
        return getSpikeRejection();
    }

    uint8_t getWatchdogThreshold() { return readReg(AS3935_WDTH); }

    uint8_t setWatchdogThreshold(uint8_t watchdThr)
    {
        writeReg(AS3935_WDTH, watchdThr);
        return getWatchdogThreshold();
    }

    uint8_t getMinNumberOfLightnings() { return readReg(AS3935_MIN_NUM_LIGH); }

    uint8_t setMinNumberOfLightnings(uint8_t number)
    {
        writeReg(AS3935_MIN_NUM_LIGH, number);
        return getMinNumberOfLightnings();
    }

    void calibrateFreq()
    {
        if (!_present) {
            return;
        }

        uint16_t measureTime = uint16_t(((float)CLK_TEST_FREQ_HZ / 2000.0 * 250.0) + 0.5);
        DPRINT(F("Corrected measure time: "));
        DDECLN(measureTime);

        char buffer[32];
        // set lco_fdiv divider to 1, which translates to 32, so we are looking for 15625Hz on irq pin
        // and since we are counting for 250ms that translates to number 3906
        writeReg(AS3935_LCO_FDIV, 1);    // Freq. divider 32
        writeReg(AS3935_DISP_LCO, 1);

        // going over all built-in cap values and finding the best
        uint8_t  bestTune = 0;
        uint16_t bestDiff = 0xFFFF;
        for (uint8_t currTune = 0; currTune <= 15; currTune++) {
            writeReg(AS3935_TUN_CAP, currTune);
            delay(500);    // let it stabilize
            uint16_t      currentCount = 0;
            uint8_t       prevIrq      = digitalRead(AS3935_INT_PIN);
            unsigned long startTime    = millis();
            while ((millis() - startTime) < measureTime) {
                uint8_t currIrq = digitalRead(AS3935_INT_PIN);
                if (currIrq != prevIrq) {
                    currentCount++;
                    prevIrq = currIrq;
                }
            }
            currentCount            = currentCount / 2u;                      // slope count to frequency
            unsigned long frequency = (unsigned long)currentCount * 128ul;    // Freq. divider 32, measure time *4 (250ms)
            uint16_t      currDiff  = abs(3906u - currentCount);
            if (currDiff < bestDiff) {
                bestDiff = currDiff;
                bestTune = currTune;
            }
            sprintf(buffer, "Cap %2u %5u %3u %6lu", currTune, currentCount, currDiff, frequency);
            DPRINTLN(buffer);
        }
        delay(10);
        writeReg(AS3935_DISP_LCO, 0);

        DPRINT(F("Best Capacitor Index: "));
        DDECLN(bestTune);

        writeReg(AS3935_TUN_CAP, bestTune);

        // and now do RCO calibration
        calibrateRCO();
    }

    void dumpRegs()
    {
        if (!_present) {
            return;
        }
        char b1[8];
        DPRINTLN(F("Addr 00 01 02 03 04 05 06 07 08 3A 3B"));
        DPRINT(F("Data "));
        Wire.beginTransmission(AS3935_ADDR);
        Wire.write(0);
        Wire.endTransmission(false);
        Wire.requestFrom(AS3935_ADDR, 9u);
        for (uint8_t i = 0; i <= 8; i++) {
            uint8_t val = Wire.read();
            sprintf(b1, "%.2X ", val);
            DPRINT(b1);
        }
        Wire.beginTransmission(AS3935_ADDR);
        Wire.write(0x3A);
        Wire.endTransmission(false);
        Wire.requestFrom(AS3935_ADDR, 2u);
        for (uint8_t i = 0; i < 2; i++) {
            uint8_t val = Wire.read();
            sprintf(b1, "%.2X ", val);
            DPRINT(b1);
        }
        DPRINTLN("");
    }

    void printParams()
    {
        if (!_present) {
            return;
        }
        DPRINT(F("Capacitor Index:           "));
        DDECLN(getCapacitorIndex());
        DPRINT(F("Indoor:                    "));
        DDECLN(getIndoor());
        DPRINT(F("Show Disturbers:           "));
        DDECLN(getShowDisturbers());
        DPRINT(F("Noise Floor:               "));
        DDECLN(getNoiseFloor());
        DPRINT(F("Spike Rejection:           "));
        DDECLN(getSpikeRejection());
        DPRINT(F("Watchdog Threshold:        "));
        DDECLN(getWatchdogThreshold());
        DPRINT(F("Min. Number Of Lightnings: "));
        DDECLN(decodeMinNumberOfLightnings(getMinNumberOfLightnings()));
    }

    uint8_t decodeMinNumberOfLightnings(uint8_t minLightnings)
    {
        switch (minLightnings) {
        case 0:
            return 1;
        case 1:
            return 5;
        case 2:
            return 9;
        case 3:
            return 16;
        default:
            return 0;
        }
    }

private:
    static bool _notify;
    bool        _present;

    static void isr() { _notify = true; }

    void reset()
    {
        Wire.beginTransmission(AS3935_ADDR);
        Wire.write(0x3C);    // PRESET_DEFAULT
        Wire.write(0x96);
        Wire.endTransmission(true);
        delay(10);
    }

    void calibrateRCO()
    {
        Wire.beginTransmission(AS3935_ADDR);
        Wire.write(0x3D);    // CALIB_RCO
        Wire.write(0x96);
        Wire.endTransmission(true);
        delay(10);
    }

    void powerDown() { writeReg(AS3935_PWD, 1); }

    void powerUp()
    {
        // datasheet p.36
        writeReg(AS3935_PWD, 0);
        calibrateRCO();
        writeReg(AS3935_DISP_SRCO, 1);
        delay(2);
        writeReg(AS3935_DISP_SRCO, 0);
    }

    void writeReg(uint8_t reg, uint8_t mask, uint8_t data)
    {
        if (!_present) {
            return;
        }
        // write addr
        Wire.beginTransmission(AS3935_ADDR);
        Wire.write(reg);
        Wire.endTransmission(false);
        // read reg
        Wire.requestFrom(AS3935_ADDR, 1u);
        uint8_t regval = Wire.read();
        // do masking
        regval &= ~(mask);
        if (mask) {
            regval |= (data << (_ffsz(mask) - 1));
        } else {
            regval |= data;
        }
        // write the register back
        Wire.beginTransmission(AS3935_ADDR);
        Wire.write(reg);
        Wire.write(regval);
        Wire.endTransmission(true);
    }

    uint8_t readReg(uint8_t reg, uint8_t mask)
    {
        if (!_present) {
            return 0;
        }
        // write addr
        Wire.beginTransmission(AS3935_ADDR);
        Wire.write(reg);
        Wire.endTransmission(false);
        // read reg
        Wire.requestFrom(AS3935_ADDR, 1u);
        uint8_t regval = Wire.read();
        // mask
        regval = regval & mask;
        if (mask) {
            regval >>= (_ffsz(mask) - 1);
        }
        return regval;
    }

    uint8_t _ffsz(uint8_t mask)
    {
        uint8_t i = 0;
        if (mask) {
            for (i = 1; ~mask & 1; i++) {
                mask >>= 1;
            }
        }
        return i;
    }
};

bool Sens_AS3935::_notify = false;

}

#endif
