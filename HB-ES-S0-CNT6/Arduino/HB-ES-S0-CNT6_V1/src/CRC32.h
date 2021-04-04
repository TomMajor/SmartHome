
//---------------------------------------------------------
// CRC32.h
// 2019-01-02 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
//---------------------------------------------------------

#ifndef _CRC32_H_
#define _CRC32_H_

class CRC32 {

private:
    uint32_t _crc;

public:
    CRC32() { reset(); }

    void reset()
    {
        _crc = 0xFFFFFFFF;    // CRC-32 ISO 3309
    }

    void update(uint8_t byte)
    {
        _crc = _crc ^ byte;
        for (int i = 0; i < 8; i++) {
            uint32_t mask = -(_crc & 1);
            _crc          = (_crc >> 1) ^ (0xEDB88320 & mask);
        }
    }

    void update(uint32_t dword)
    {
        for (int i = 3; i >= 0; i--) {
            uint8_t byte = (dword >> (i * 8)) & 0xFF;    // MSByte first
            update(byte);
        }
    }

    uint32_t finalize()
    {
        return ~_crc;    // CRC-32 ISO 3309
    }
};

#endif
