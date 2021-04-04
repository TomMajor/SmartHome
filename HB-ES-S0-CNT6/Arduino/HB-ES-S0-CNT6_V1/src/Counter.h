
//---------------------------------------------------------
// Counter.h
// 2019-02-28 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
//---------------------------------------------------------

#ifndef _COUNTER_H_
#define _COUNTER_H_

#include <EEPROM.h>
#include <Ethernet.h>
#include "CRC32.h"

const uint16_t cCOUNTER_SIZE     = 6;    // S0 Input pins: Arduino pins 2..7
const uint16_t cEE_BEGIN         = 512;
const uint16_t cEE_STORAGE_COUNT = 12;    // 12 verschiedene COUNTER_STORAGE structs a 8 DWORDs (32 Byte) werden im EE ab Addresse cEE_BEGIN abgelegt
const unsigned long cSaveIntervallMSec = 3600000ul;    // Wie oft sollen die Counter in den EEPROM backuped werden? - 1mal pro Stunde

template <uint8_t LED_PIN> class COUNTER {

private:
    typedef struct {
        uint32_t Counter[cCOUNTER_SIZE];
        uint32_t WriteCounter;
        uint32_t CRC;
    } COUNTER_STORAGE;

    bool            _initialized;
    uint8_t         _currentEEStorageIndex;
    uint8_t         _pinState;
    uint8_t         _pulseLengthLo[cCOUNTER_SIZE];
    uint8_t         _pulseLengthHi[cCOUNTER_SIZE];
    unsigned long   _lastTimeEESaved;
    COUNTER_STORAGE _counterStorage;
    COUNTER_STORAGE _tmpStorage;
    CRC32           _crc32;
    char            _buffer[150];    // max. 75 bytes in debugOutput() required
    uint8_t         _ledCounter;
    uint8_t*        _ledPort;
    uint8_t         _ledBit;
    HardwareSerial* _pSerial;

    uint32_t calculateCRC(COUNTER_STORAGE& storage)
    {
        _crc32.reset();
        for (uint8_t i = 0; i < cCOUNTER_SIZE; i++) {
            _crc32.update(storage.Counter[i]);
        }
        _crc32.update(storage.WriteCounter);
        return _crc32.finalize();
    }

    void incrementEEStorageIndex()
    {
        _currentEEStorageIndex++;
        if (_currentEEStorageIndex >= cEE_STORAGE_COUNT) {    // wrap around
            _currentEEStorageIndex = 0;
        }
    }

public:
    COUNTER()
        : _initialized(false)
        , _currentEEStorageIndex(0)
        , _pinState(0)
        , _lastTimeEESaved(0)
        , _ledCounter(0)
        , _pSerial(0)
    {
    }

    void init(HardwareSerial* pSerial)
    {
        _pSerial = pSerial;

        if (_pSerial) {
            _pSerial->println(F("COUNTER init"));
        }

        pinMode(2, INPUT_PULLUP);
        pinMode(3, INPUT_PULLUP);
        pinMode(4, INPUT_PULLUP);
        pinMode(5, INPUT_PULLUP);
        pinMode(6, INPUT_PULLUP);
        pinMode(7, INPUT_PULLUP);
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, LOW);

        // prepare _ledBit/_ledPort for fast write in ISR, don't use digitalWrite there!
        _ledBit      = digitalPinToBitMask(LED_PIN);
        uint8_t port = digitalPinToPort(LED_PIN);
        _ledPort     = portOutputRegister(port);

        memset(_pulseLengthLo, 0, sizeof(_pulseLengthLo));
        memset(_pulseLengthHi, 0, sizeof(_pulseLengthHi));
        int      lastStorageIndex = -1;
        uint32_t maxWriteCounter  = 0;

        // search last used storage index, that is the entry with the highest write counter value
        for (uint8_t i = 0; i < cEE_STORAGE_COUNT; i++) {
            memset(&_tmpStorage, 0, sizeof(COUNTER_STORAGE));
            uint16_t eeAddr = cEE_BEGIN + (i * sizeof(COUNTER_STORAGE));
            EEPROM.get(eeAddr, _tmpStorage);
            if (_tmpStorage.WriteCounter == 0xFFFFFFFFul) {    // fresh/unused EE location
                continue;
            }
            if (_tmpStorage.CRC == calculateCRC(_tmpStorage)) {      // CRC valid
                if (_tmpStorage.WriteCounter > maxWriteCounter) {    // maximum write counter?
                    maxWriteCounter  = _tmpStorage.WriteCounter;
                    lastStorageIndex = i;
                }
            }
        }
        if (lastStorageIndex >= 0) {    // valid COUNTER_STORAGE in EE found
            uint16_t eeAddr = cEE_BEGIN + (lastStorageIndex * sizeof(COUNTER_STORAGE));
            EEPROM.get(eeAddr, _counterStorage);
            _currentEEStorageIndex = lastStorageIndex;
            incrementEEStorageIndex();
            sprintf(_buffer, "Last valid storage data found, index: %d, write counter %ld", lastStorageIndex, _counterStorage.WriteCounter);
            if (_pSerial) {
                _pSerial->println(_buffer);
            }
        } else {
            if (_pSerial) {
                _pSerial->println(F("No valid storage data found (fresh device?)"));
            }
            memset(&_counterStorage, 0, sizeof(COUNTER_STORAGE));
            _currentEEStorageIndex = 0;
        }

        _initialized = true;
    }

    void loop()
    {
        if (!_initialized) {
            return;
        }
        if ((millis() - _lastTimeEESaved) >= cSaveIntervallMSec) {
            _lastTimeEESaved = millis();
            _counterStorage.WriteCounter++;
            // get a consistent storage copy, atomic operation needed!
            uint8_t sreg = SREG;
            cli();
            memcpy(&_tmpStorage, &_counterStorage, sizeof(COUNTER_STORAGE));
            SREG = sreg;
            // from here, the copy is consistent and saved to EE
            _tmpStorage.CRC = calculateCRC(_tmpStorage);
            uint16_t eeAddr = cEE_BEGIN + (_currentEEStorageIndex * sizeof(COUNTER_STORAGE));
            EEPROM.put(eeAddr, _tmpStorage);
            sprintf(_buffer, "Storage saved to EE Addr 0x%.3X, index: %d, seconds: %lu", eeAddr, _currentEEStorageIndex, _lastTimeEESaved / 1000);
            if (_pSerial) {
                _pSerial->println(_buffer);
            }
            incrementEEStorageIndex();
        }
    }

    uint32_t getCounter(uint8_t i)
    {
        if (i < cCOUNTER_SIZE) {
            return _counterStorage.Counter[i];
        }
        return 0;
    }

    void timer2ms()
    {
        if (!_initialized) {
            return;
        }
        bool    bLedOn   = false;
        uint8_t pinLevel = (PIND & 0xFC) >> 2;    // sample pins only once, inputs here are Arduino pin 2..7 -> PD2..PD7
        for (uint8_t i = 0; i < cCOUNTER_SIZE; i++) {
            if (_pinState & (1u << i)) {       // last time level was high
                if (pinLevel & (1u << i)) {    // still high
                    if (_pulseLengthHi[i] < 255) {
                        _pulseLengthHi[i]++;    // count pulse length up to 1sec
                    }
                } else {                                                            // H->L pulse
                    if ((_pulseLengthHi[i] >= 7) && (_pulseLengthHi[i] <= 75)) {    // count only high pulses between 14..150msec
                        ;                                                           // if needed: count high pulse here
                    }
                    _pulseLengthLo[i] = 0;
                    _pinState &= ~(1u << i);    // clear pin state: low
                }
            } else {                              // last time level was low
                if (!(pinLevel & (1u << i))) {    // still low
                    if (_pulseLengthLo[i] < 255) {
                        _pulseLengthLo[i]++;    // count pulse length up to 1sec
                    }
                } else {                                                            // L->H pulse
                    if ((_pulseLengthLo[i] >= 7) && (_pulseLengthLo[i] <= 75)) {    // count only low pulses between 14..150msec
                        _counterStorage.Counter[i]++;                               // count low pulse
                        bLedOn = true;
                    }
                    _pulseLengthHi[i] = 0;
                    _pinState |= (1u << i);    // set pin state: high
                }
            }
        }
        if (bLedOn) {
            _ledCounter = 75;    // 150ms LED on
            *_ledPort |= _ledBit;
        }
        if (_ledCounter > 0) {
            _ledCounter--;
            if (_ledCounter == 0) {    // LED off
                *_ledPort &= ~_ledBit;
            }
        }
    }

    void clearAll()
    {
        for (uint8_t i = 0; i < cEE_STORAGE_COUNT; i++) {
            memset(&_tmpStorage, 0xFF, sizeof(COUNTER_STORAGE));
            uint16_t eeAddr = cEE_BEGIN + (i * sizeof(COUNTER_STORAGE));
            EEPROM.put(eeAddr, _tmpStorage);
        }
    }

    void debugOutput(EthernetClient client)
    {
        char b1[16];

        client.println(F("<body><div style=\"font-family:monospace\">"));

        client.println(F("<br>Counter Debug: RAM Data<br>"));
        sprintf(_buffer, "_initialized: %d", _initialized);
        client.println(_buffer);
        client.println("<br>");
        sprintf(_buffer, "_currentEEStorageIndex: %d", _currentEEStorageIndex);
        client.println(_buffer);
        client.println("<br>");
        sprintf(_buffer, "_counterStorage.WriteCounter: %.8lX", _counterStorage.WriteCounter);
        client.println(_buffer);
        client.println("<br>");

        memset(_buffer, 0, sizeof(_buffer));
        for (uint8_t i = 0; i < cCOUNTER_SIZE; i++) {
            sprintf(_buffer, "Cnt%02d: ", i);
            sprintf(b1, "%.8lX ", _counterStorage.Counter[i]);
            strcat(_buffer, b1);
            sprintf(b1, "L %.3d ", _pulseLengthLo[i]);
            strcat(_buffer, b1);
            sprintf(b1, "H %.3d", _pulseLengthHi[i]);
            strcat(_buffer, b1);
            client.println(_buffer);
            client.println("<br>");
        }

        client.println(F("<br>Counter Debug: EEPROM Data<br>"));
        COUNTER_STORAGE tmpSt;
        for (uint8_t n = 0; n < cEE_STORAGE_COUNT; n++) {
            memset(_buffer, 0, sizeof(_buffer));
            sprintf(b1, "%02d: ", n);
            strcat(_buffer, b1);
            memset(&tmpSt, 0, sizeof(COUNTER_STORAGE));
            uint16_t eeAddr = cEE_BEGIN + (n * sizeof(COUNTER_STORAGE));
            EEPROM.get(eeAddr, tmpSt);
            for (uint8_t i = 0; i < cCOUNTER_SIZE; i++) {
                sprintf(b1, "%.8lX ", tmpSt.Counter[i]);
                strcat(_buffer, b1);
            }
            sprintf(b1, "%.8lX ", tmpSt.WriteCounter);
            strcat(_buffer, b1);
            sprintf(b1, "%.8lX", tmpSt.CRC);
            strcat(_buffer, b1);
            client.println(_buffer);
            client.println("<br>");
        }
        client.println(F("</div></body>"));
    }
};

#endif
