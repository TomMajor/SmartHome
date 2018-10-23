
//---------------------------------------------------------
// Sens_DIGINPUT
// 2018-10-16 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
// HB-UNI-Sen-PRESS-SC 2018-05-14 jp112sdl Creative Commons
//---------------------------------------------------------

#ifndef _SENS_1PINSTATE_H_
#define _SENS_1PINSTATE_H_

#include <Sensors.h>

namespace as {

class Sens_DIGINPUT : public Sensor {

    static uint8_t _pin;
    static bool    _notify;

    static void isr()
    {
        disableInterrupt(_pin);
        _notify = true;
    }

public:
    Sens_DIGINPUT() {}

    void init(uint8_t inputPin)
    {
        _pin = inputPin;
        pinMode(_pin, INPUT_PULLUP);    // pull-up enabled
        enableINT();
    }

    void enableINT()
    {
        if (digitalPinToInterrupt(_pin) == NOT_AN_INTERRUPT) {
            enableInterrupt(_pin, isr, CHANGE);
            DPRINTLN(F("Sens_DIGINPUT enable INT"));
        } /*else {
            attachInterrupt(digitalPinToInterrupt(_pin), isr, CHANGE);
            DPRINTLN(F("Sens_DIGINPUT attach INT"));
        }*/
    }

    uint8_t pinState() { return digitalRead(_pin); }
    bool    notifyEvent() { return _notify; }
    void    resetEvent() { _notify = false; }
};

uint8_t Sens_DIGINPUT::_pin    = 0;
bool    Sens_DIGINPUT::_notify = false;

}

#endif
