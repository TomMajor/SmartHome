
//---------------------------------------------------------
// Leveljet.h
// Version 1.01
// 2019-04-06 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _LEVELJET_H_
#define _LEVELJET_H_

#include "Peiltabelle.h"

const uint16_t cLAST_TABLE_INDEX = (sizeof(PEILTABELLE) / sizeof(PEILTABELLE[0])) - 1;
const uint16_t cTANK_ANZAHL      = 4;

namespace as {

template <bool PEGEL_MM, bool USE_PEILTABELLE> class LEVELJET {

public:
    LEVELJET()
        : rxState(0)
        , rxTimeout(0)
        , ljLevel(0)
        , ljVolumeLiter(0)
        , ljVolumePercent(0)
        , ljCrcOk(0)
        , ljCrcErr(0)
        , timeStamp(0)
    {
    }

    void init()
    {
        // dieser Sketch setzt das AskSinPP Serial Port Init (DINIT) mit 19200 Baud voraus!
        DPRINTLN(F("LevelJet init"));
    }

    void loop()
    {
        if (Serial.available() > 0) {
            ProcessLevelJetData(Serial.read());
        }
        if (millis() - timeStamp > 100) {    // @100ms
            timeStamp = millis();
            rxTimeout++;
            if (rxTimeout >= 8) {
                rxTimeout = 0;
                if (rxState != 0) {    // reset RX StateMachine after 800ms
                    rxState = 0;
                    DPRINTLN(F("StateMachine Reset"));
                }
            }
        }
    }

    uint16_t level() { return ljLevel; }
    uint16_t volume() { return ljVolumeLiter; }
    uint8_t  percent() { return ljVolumePercent; }

private:
    uint8_t       rxState, rxTimeout;
    uint8_t       rxBuffer[16], *rxPtr, rxLength;
    uint8_t       ljVolumePercent;
    uint16_t      ljLevel, ljVolumeLiter, ljCrcOk, ljCrcErr;
    unsigned long timeStamp;
    char          dbgBuffer[80];

    void ProcessLevelJetData(uint8_t uc)
    {
        rxTimeout = 0;
        switch (rxState) {
        case 0:
            if (uc == 0x00) {    // header0
                rxState  = 1;
                rxPtr    = &rxBuffer[0];
                *rxPtr++ = uc;
            }
            break;
        case 1:
            if (uc == 0x10) {    // header1
                rxState  = 2;
                *rxPtr++ = uc;
                rxLength = 10;    // 10 bytes left
            } else {
                rxState = 0;
            }
            break;
        case 2:
            *rxPtr++ = uc;
            rxLength--;
            if (rxLength == 0) {    // log example: 00 10 DE 05 E4 00 5D 00 0F 00 49 67
                uint32_t crc = 0;
                for (uint8_t i = 0; i < 10; i++) {
                    crc = LevelJet_CRC(crc, rxBuffer[i]);
                }
                crc = LevelJet_CRC(crc, 0);
                crc = LevelJet_CRC(crc, 0);
                crc = (crc >> 8) & 0xFFFF;
                if (((crc >> 8) == rxBuffer[11]) &&                            // crc high byte (in memory big endian, in receive log little endian)
                    ((crc & 0xFF) == rxBuffer[10])) {                          // crc low byte ""
                    ljCrcOk++;                                                 // crc is correct
                    ljLevel = (((uint16_t)rxBuffer[5]) << 8) + rxBuffer[4];    // Pegel in mm
                    // einige neuere LevelJet geben den Pegel in cm aus, nicht mehr in mm! siehe HM thread, user nuiler, LevelJet Version 3.15
                    if (PEGEL_MM == false) {
                        ljLevel = 10 * ljLevel;
                    }
                    // Volumen- und Füllstandsberechnung mit Peiltabelle, für komplexe Tanks deren Form sich nicht im LevelJet-Geräte Setup abbilden
                    // lässt
                    if (USE_PEILTABELLE) {
                        calcVolume(ljLevel);
                    } else {    // Volumen- und Füllstandsberechnung original vom LevelJet-Gerät, für einfache Tanks
                        ljVolumeLiter   = (((uint16_t)rxBuffer[7]) << 8) + rxBuffer[6];    // Volumen in Liter
                        ljVolumePercent = rxBuffer[8];                                     // Füllstand in Prozent
                    }
                    sprintf(dbgBuffer, "CRC OK, Cfg %d %d, Level %d, Volume %d (%d%%)", (int)PEGEL_MM, (int)USE_PEILTABELLE, ljLevel, ljVolumeLiter,
                            ljVolumePercent);
                    DPRINTLN(dbgBuffer);
                } else {    // crc is invalid
                    ljCrcErr++;
                    DPRINTLN(F("CRC ERROR"));
                }
                rxState = 0;
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
        ljVolumeLiter    = 0;
        uint16_t levelCM = levelMM / 10;    // Abrunden auf volle cm
        if (levelCM <= (cLAST_TABLE_INDEX - 1)) {
            // Interpolation Volumen zwischen zwei Stützstellen (Stützstellen bei vollen cm)
            float x       = (float)levelMM / 10.0;
            float x1      = levelCM;
            float x2      = levelCM + 1;
            float y1      = pgm_read_word_near(PEILTABELLE + levelCM);
            float y2      = pgm_read_word_near(PEILTABELLE + levelCM + 1);
            float volume  = y1 + (x - x1) * (y2 - y1) / (x2 - x1);
            ljVolumeLiter = cTANK_ANZAHL * volume;
        } else if (levelCM >= cLAST_TABLE_INDEX) {
            ljVolumeLiter = cTANK_ANZAHL * pgm_read_word_near(PEILTABELLE + cLAST_TABLE_INDEX);
        }
        float percent   = 100.0 * (float)ljVolumeLiter / (float)pgm_read_word_near(PEILTABELLE + cLAST_TABLE_INDEX) / cTANK_ANZAHL;
        ljVolumePercent = floor(percent + 0.5);
    }
};

}

#endif
