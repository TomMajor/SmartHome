
//---------------------------------------------------------
// tmBattery
// 2019-01-04 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _TM_BATTERY_H_
#define _TM_BATTERY_H_

#include <Debug.h>
#include <AlarmClock.h>

#ifdef ARDUINO_ARCH_AVR

#define ADMUX_ADCMASK ((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0))
#define ADMUX_REFMASK ((1 << REFS1) | (1 << REFS0))

#define ADMUX_REF_AREF ((0 << REFS1) | (0 << REFS0))
#define ADMUX_REF_AVCC ((0 << REFS1) | (1 << REFS0))
#define ADMUX_REF_RESV ((1 << REFS1) | (0 << REFS0))
#define ADMUX_REF_VBG ((1 << REFS1) | (1 << REFS0))

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega644P__)               \
    || defined(__AVR_ATmega1284P__)
#define ADMUX_ADC_VBG ((1 << MUX4) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0))
#else
#define ADMUX_ADC_VBG ((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0))
#endif
#else
#error INVALID ARCHITEKTURE
#endif

namespace as {

//---------------------------------------------------------
class tmBattery : public Alarm {
    uint8_t  m_Low, m_Critical;
    uint16_t m_LastValue;
    uint32_t m_Period;

public:
    tmBattery()
        : Alarm(0)
        , m_Low(0)
        , m_Critical(0)
        , m_LastValue(0)
        , m_Period(0)
    {
    }
    virtual ~tmBattery() {}

    virtual void trigger(AlarmClock& clock)
    {
        m_LastValue = voltage();
        tick        = m_Period;
        clock.add(*this);
    }

    uint16_t current() const { return m_LastValue; }

    bool critical() const
    {
        return ((m_LastValue / 100) < m_Critical);    // low/critical values have to be in default AskSinPP 100mV resolution
    }

    void critical(uint8_t value)
    {
        m_Critical = value;
        DPRINT(F("Battery set crit: "));
        DDECLN(m_Critical);
    }

    bool low() const { return ((m_LastValue / 100) < m_Low); }    // low/critical values have to be in default AskSinPP 100mV resolution

    void low(uint8_t value)
    {
        m_Low = value;
        DPRINT(F("Battery set low:  "));
        DDECLN(m_Low);
    }

    void init(uint32_t period, AlarmClock& clock)
    {
        // enable ADC, prescaler 128 = 62.5kHz ADC clock @8MHz (range 50..1000 kHz)
        ADCSRA      = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
        ADCSRB      = 0;
        m_LastValue = voltage();
        m_Period    = period;
        tick        = m_Period;
        clock.add(*this);
    }

protected:
    uint16_t measure()
    {
        delay(50);    // load CVref 100nF, 5*Tau = 25ms
        // messure 4 times
        uint16_t adc = 0;
        for (int i = 0; i < 4; i++) {
            ADCSRA |= (1 << ADSC);    // start conversion
            uint8_t timeout = 50;
            while (ADCSRA & (1 << ADSC)) {
                delayMicroseconds(10);
                timeout--;
                if (timeout == 0) {
                    return 0;
                }
            }
            adc += (ADC & 0x3FF);
            // DDECLN(ADC);
        }
        if ((adc & 0x03) >= 2) {    // rounding
            return (adc >> 2) + 1;
        } else {
            return (adc >> 2);
        }
    }

private:
    virtual uint16_t voltage()
    {
        // setup reference and input each time, just in case other SW modules have changed this
        ADMUX &= ~(ADMUX_REFMASK | ADMUX_ADCMASK);
        ADMUX |= ADMUX_REF_AVCC;    // select AVCC as reference
        ADMUX |= ADMUX_ADC_VBG;     // select bandgap as input
        uint16_t adc = measure();
        if (adc) {
            // read Bandgap against AVCC:
            // ADC = Vin * 1024 / Vref -> Vref = Vin * 1024 / ADC
            // -> AVcc = Vbg * 1024 / ADC
            uint32_t avcc = 1100UL * 1024 / adc;
            DPRINT(F("tmBattery Voltage: "));
            DDECLN(avcc);
            return (uint16_t)avcc;
        } else {
            DPRINT(F("tmBattery Error"));
            return 0;
        }
    }
};

//---------------------------------------------------------
template <uint8_t SENSPIN, uint8_t ACTIVATIONPIN, uint16_t FACTOR> class tmBatteryResDiv : public tmBattery {
    uint8_t  m_SensePin;         // A0
    uint8_t  m_ActivationPin;    // D9
    uint16_t m_Factor;           // Faktor = Rges/Rlow*1000, z.B. Teiler 470k/100k, Faktor 570k/100k*1000 = 5700

public:
    tmBatteryResDiv()
        : tmBattery()
        , m_SensePin(SENSPIN)
        , m_ActivationPin(ACTIVATIONPIN)
        , m_Factor(FACTOR)
    {
    }
    virtual ~tmBatteryResDiv() {}

    void init(uint32_t period, AlarmClock& clock)
    {
        pinMode(m_SensePin, INPUT);         // pull-up off
        pinMode(m_ActivationPin, INPUT);    // disable resistor divider
        tmBattery::init(period, clock);
    }

private:
    virtual uint16_t voltage()
    {
        pinMode(m_ActivationPin, OUTPUT);
        digitalWrite(m_ActivationPin, LOW);    // enable resistor divider
        digitalWrite(m_SensePin, LOW);         // pull-up off

        // setup reference and input each time, just in case other SW modules have changed this
        ADMUX &= ~(ADMUX_REFMASK | ADMUX_ADCMASK);
        ADMUX |= ADMUX_REF_VBG;                     // select bandgap as reference
        ADMUX |= ((m_SensePin - PIN_A0) & 0x0F);    // select SensePin as input
        uint16_t adc = tmBattery::measure();

        digitalWrite(m_SensePin, HIGH);     // pull-up on
        pinMode(m_ActivationPin, INPUT);    // disable resistor divider

        if (adc) {
            // read SensePin against Bandgap:
            // ADC = Vin * 1024 / Vref -> Vin = Vref * ADC / 1024 (* resistor factor)
            uint32_t vbat = 11UL * adc * m_Factor / 1024 / 10;
            DPRINT(F("tmBatteryResDiv Voltage: "));
            DDECLN(vbat);
            return (uint16_t)vbat;
        } else {
            DPRINT(F("tmBatteryResDiv Error"));
            return 0;
        }
    }
};

}

#endif
