
#ifndef _SENSOR_BATTERY_H_
#define _SENSOR_BATTERY_H_

#include <Debug.h>
#include <AlarmClock.h>

namespace as {

template <uint8_t SENSPIN, uint8_t ACTIVATIONPIN>
class BatterySensorLoad : public Alarm {

  uint8_t  m_SensePin, m_ActivationPin;
  uint16_t m_LastValue, m_Low, m_Critical, m_Factor;
  uint32_t m_Period;

public:
  BatterySensorLoad () : Alarm(0), m_LastValue(0), m_Period(0), m_Low(0), m_Critical(0), m_Factor(2000), m_SensePin(SENSPIN), m_ActivationPin(ACTIVATIONPIN) {}
  virtual ~BatterySensorLoad() {}

  virtual void trigger (AlarmClock& clock) {
    DPRINTLN(F("BattLoad trigger"));
    tick = m_Period;
    clock.add(*this);
    m_LastValue = voltage();
  }

  uint16_t current () const {
    return m_LastValue;
  }

  bool critical () const {
    return m_LastValue < m_Critical;
  }

  void critical (uint16_t value) {
    m_Critical = value;
    DPRINT(F("BattLoad set crit: ")); DDECLN(m_Critical);
  }

  bool low () const {
    return m_LastValue < m_Low;
  }

  void low (uint16_t value) {
    m_Low = value;
    DPRINT(F("BattLoad set low:  ")); DDECLN(m_Low);
  }

  void init(uint32_t period, AlarmClock& clock, uint16_t factor = 2000) {
    m_Factor = factor;
    pinMode(m_ActivationPin, OUTPUT);
    digitalWrite(m_ActivationPin, LOW);     		// N-Channel Mosfet off
    pinMode(m_SensePin, INPUT);
    digitalWrite(m_SensePin, LOW);          		// pull-up off
    m_LastValue = voltage();
    m_Period = period;
    tick = m_Period;
    clock.add(*this);
  }

  virtual uint16_t voltage() {
    uint8_t  timeout;
    uint32_t adc;
    
    // setup ADC: complete ADC init in case other modules have chamged this
    ADCSRA = 1<<ADEN | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0;	// enable ADC, prescaler 128 = 62.5kHz ADC clock @8MHz (range 50..1000 kHz)
    ADMUX  = 1<<REFS1 | 1<<REFS0;			// AREF: Internal 1.1V Voltage Reference with external capacitor at AREF pin
    ADCSRB = 0;
    uint8_t channel = m_SensePin - PIN_A0;
    ADMUX  |= (channel & 0x0F);          		// select channel
    //delay(30);					// load CVref 100nF, 5*Tau = 25ms -> not needed here because of delay(300) below

    digitalWrite(m_ActivationPin, HIGH);     		// N-Channel Mosfet on
    delay(200);						// Batterie vor der Messung für 200ms belasten

    // 1x dummy read
    ADCSRA |= 1<<ADSC; timeout = 50;
    while (ADCSRA & (1<<ADSC)) { delayMicroseconds(10); timeout--; if (timeout==0) break; }

    ADCSRA |= 1<<ADSC; timeout = 50;
    while (ADCSRA & (1<<ADSC)) { delayMicroseconds(10); timeout--; if (timeout==0) break; }

    digitalWrite(m_ActivationPin, LOW);     		// N-Channel Mosfet off

    adc = 11UL * m_Factor * (ADC & 0x3FF) / 1024 / 10;
    DPRINT(F("BattLoad voltage:  ")); DDECLN(adc);
    return (uint16_t)adc;
  }
};

}

#endif
