//---------------------------------------------------------
// Leveljet.h
// Version 1.1
// (C) 2019-2026 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------
// Ein Datenblock umfasst 12 Bytes, davon 10 Datenbytes und zwei CRC-Bytes.
// 0   Gerätekennung Low-Byte
// 1   Gerätekennung High-Byte
// 2   Distanz Low-Byte
// 3   Distanz High-Byte
// 4   Füllhöhe Low-Byte
// 5   Füllhöhe High-Byte
// 6   Liter Low-Byte
// 7   Liter High-Byte
// 8   Inhalt in Prozent
// 9   Zustand derAusgänge
// 10  CRC Low-Byte
// 11  CRC High-Byte
//---------------------------------------------------------

// dieser Sketch setzt das AskSinPP Serial Port Init (DINIT) mit 19200 Baud voraus!

#ifndef _LEVELJET_H_
#define _LEVELJET_H_

#include "Peiltabelle.h"

const uint16_t cLAST_TABLE_INDEX = (sizeof(PEILTABELLE) / sizeof(PEILTABELLE[0])) - 1;
const uint16_t cTANK_ANZAHL      = 4;

namespace as {

template <bool PEGEL_MM, bool USE_PEILTABELLE> class LEVELJET {

public:
    LEVELJET()
        : _RxState(0)
        , _RxTimeout(0)
        , _Distance(0)
        , _Level(0)
        , _Volume(0)
        , _CrcOk(0)
        , _CrcErr(0)
        , _Percent(0)
        , _TimeStamp(0)
    {
    }

    void init()
    {
        DPRINTLN(F("LevelJet init"));
    }

    void loop()
    {
        if (Serial.available() > 0) {
            ProcessLevelJetData(Serial.read());
        }
        if (millis() - _TimeStamp > 100) {    // @100ms
            _TimeStamp = millis();
            _RxTimeout++;
            if (_RxTimeout >= 8) {
                _RxTimeout = 0;
                if (_RxState != 0) {    // reset RX StateMachine after 800ms
                    _RxState = 0;
                    DPRINTLN(F("StateMachine Reset"));
                }
            }
        }
    }

    uint16_t Distance() { return _Distance; }
    uint16_t Level() { return _Level; }
    uint16_t Volume() { return _Volume; }
    uint8_t  Percent() { return _Percent; }

private:
    uint8_t       _RxState, _RxTimeout;
    uint8_t       _RxBuffer[16], *_RxPtr, _RxLength;
    uint16_t      _Distance, _Level, _Volume, _CrcOk, _CrcErr;
    uint8_t       _Percent;
    unsigned long _TimeStamp;
    char          _DbgBuffer[80];

    void ProcessLevelJetData(uint8_t uc)
    {
        _RxTimeout = 0;
        switch (_RxState) {
        case 0:
            if (uc == 0x00) {    // header0
                _RxState  = 1;
                _RxPtr    = &_RxBuffer[0];
                *_RxPtr++ = uc;
            }
            break;
        case 1:
            if (uc == 0x10) {    // header1
                _RxState  = 2;
                *_RxPtr++ = uc;
                _RxLength = 10;    // 10 bytes left
            } else {
                _RxState = 0;
            }
            break;
        case 2:
            *_RxPtr++ = uc;
            _RxLength--;
            if (_RxLength == 0) {    // log example: 00 10 DE 05 E4 00 5D 00 0F 00 49 67
                uint32_t crc = 0;
                for (uint8_t i = 0; i < 10; i++) {
                    crc = LevelJet_CRC(crc, _RxBuffer[i]);
                }
                crc = LevelJet_CRC(crc, 0);
                crc = LevelJet_CRC(crc, 0);
                crc = (crc >> 8) & 0xFFFF;
                if (((crc >> 8) == _RxBuffer[11]) &&                              // crc high byte (in memory big endian, in receive log little endian)
                    ((crc & 0xFF) == _RxBuffer[10])) {                            // crc low byte ""
                    _CrcOk++;                                                    // crc is correct
                    _Distance = (((uint16_t)_RxBuffer[3]) << 8) + _RxBuffer[2];    // Distanz in mm
                    _Level    = (((uint16_t)_RxBuffer[5]) << 8) + _RxBuffer[4];    // Pegel in mm
                    // einige neuere LevelJet geben den Pegel in cm aus, nicht mehr in mm! siehe HM thread, user nuiler, LevelJet Version 3.15
                    if (PEGEL_MM == false) {
                        _Level = 10 * _Level;
                    }
                    // Volumen- und Füllstandsberechnung mit Peiltabelle, für komplexe Tanks deren Form sich nicht im LevelJet-Geräte Setup abbilden
                    // lässt
                    if (USE_PEILTABELLE) {
                        calcVolume(_Level);
                    } else {    // Volumen- und Füllstandsberechnung original vom LevelJet-Gerät, für einfache Tanks
                        _Volume  = 10 * ((((uint16_t)_RxBuffer[7]) << 8) + _RxBuffer[6]);    // Volumen in Liter
                        _Percent = _RxBuffer[8];                                            // Füllstand in Prozent
                    }
                    sprintf(_DbgBuffer, "CRC OK, Cfg %d %d, Distance %d, Level %d, Volume %d (%d%%)", (int)PEGEL_MM, (int)USE_PEILTABELLE, _Distance,
                            _Level, _Volume, _Percent);
                    DPRINTLN(_DbgBuffer);
                } else {    // crc is invalid
                    _CrcErr++;
                    DPRINTLN(F("CRC ERROR"));
                }
                _RxState = 0;
            }
            break;
        default:
            break;
        }
    }

    uint32_t LevelJet_CRC(uint32_t crc, uint8_t data)
    {
        crc &= 0xFFFFFF00;
        crc |= data;
        for (uint8_t i = 0; i < 8; i++) {
            crc = crc * 2;
            if (crc & 0x01000000) {
                crc = crc ^ 0x00800500;    // LevelJet Polynom;
            }
        }
        return crc;
    }

    void calcVolume(uint16_t levelMM)
    {
        _Volume          = 0;
        uint16_t levelCM = levelMM / 10;    // Abrunden auf volle cm
        if (levelCM <= (cLAST_TABLE_INDEX - 1)) {
            // Interpolation Volumen zwischen zwei Stützstellen (Stützstellen bei vollen cm)
            float x      = (float)levelMM / 10.0;
            float x1     = levelCM;
            float x2     = levelCM + 1;
            float y1     = pgm_read_word_near(PEILTABELLE + levelCM);
            float y2     = pgm_read_word_near(PEILTABELLE + levelCM + 1);
            float volume = y1 + (x - x1) * (y2 - y1) / (x2 - x1);
            _Volume      = cTANK_ANZAHL * volume;
        } else if (levelCM >= cLAST_TABLE_INDEX) {
            _Volume = cTANK_ANZAHL * pgm_read_word_near(PEILTABELLE + cLAST_TABLE_INDEX);
        }
        float percent = 100.0 * (float)_Volume / (float)pgm_read_word_near(PEILTABELLE + cLAST_TABLE_INDEX) / cTANK_ANZAHL;
        _Percent      = floor(percent + 0.5);
    }
};

}

#endif
