//---------------------------------------------------------
// HB-SEC-WDS-2
// Version 1.05
// (C) 2018-2021 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

// clang-format off
//---------------------------------------------------------
// User definitions
// General
#define WDS2_STANDARD                   // mögliche Varianten: WDS2_STANDARD / WDS2_CUSTOM
#define TWO_STATE                       // TWO_STATE: nur Status Trocken/Wasser (4 Elektroden), THREE_STATE: Status Trocken/Feucht/Wasser (5 Elektroden)
#define MEASUREMENT_INTERVAL    60      // jede Minute messen
// Pins
#define CONFIG_BUTTON_PIN       3
#define LED_PIN1                4
#define LED_PIN2                5
#define SENS_PIN_WET            A3      // ADC sensor pin (TWO_STATE: Wasser / THREE_STATE: Feucht)
#define SENS_PIN_WATER          A2      // ADC sensor pin (TWO_STATE: nicht benutzt / THREE_STATE: Wasser)
// Parameters
#define BAT_VOLT_LOW            24      // 2.4V
#define BAT_VOLT_CRITICAL       21      // 2.1V
#define DETECTION_THRESHOLD     800     // Wasser-Erkennung, Vergleichswert ADC
#define PEERS_PER_CHANNEL       6       // number of available peers per channel
//---------------------------------------------------------
// clang-format on

//---------------------------------------------------------
// Schaltungsvariante und Pins für Batteriespannungsmessung, siehe README HB-UNI-Sensor1
// Standard: tmBattery, UBatt = Betriebsspannung AVR
//#define BAT_SENSOR tmBattery
// Echte Batteriespannungsmessung unter Last, siehe HB-UNI-Sensor1 und Thema "Babbling Idiot Protection"
// tmBatteryLoad: sense pin A0, activation pin D6, Faktor = Rges/Rlow*1000, z.B. 10/30 Ohm, Faktor 40/10*1000 = 4000, 200ms Belastung vor Messung
#define BAT_SENSOR tmBatteryLoad<A0, 6, 4000, 200>

//---------------------------------------------------------
// NDEBUG sollte aktiviert werden wenn die Entwicklung und die Tests abgeschlossen sind und das Gerät in den 'Produktionsmodus' geht.
//#define NDEBUG

//---------------------------------------------------------
// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <Register.h>
#include <ContactState.h>
#include "tmBattery.h"

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    { 0x49, 0x29, 0xd3 },    // Device ID
    "WATERCHK_1",            // Device Serial
#ifdef WDS2_STANDARD
    { 0x00, 0xb2 },    // Device Model Standard HM-SEC-WDS-2
#elif defined WDS2_CUSTOM
    { 0xF1, 0x21 },    // Device Model Custom HB-SEC-WDS-2
#else
#error "Invalid device define"
#endif
    0x13,                                // Firmware Version
    as::DeviceType::ThreeStateSensor,    // Device Type
    { 0x01, 0x00 }                       // Info Bytes
};

// --------------------------------------------------------
// Configure the used hardware
typedef AvrSPI<10, 11, 12, 13>                 SPIType;
typedef Radio<SPIType, 2>                      RadioType;
typedef DualStatusLed<LED_PIN1, LED_PIN2>      LedType;
typedef AskSin<LedType, BAT_SENSOR, RadioType> HalType;
class WdsHal : public HalType {
public:
#ifdef WDS2_CUSTOM
    void prepareSend(Message& msg)
    {
        if (msg.isSensorEvent() || msg.isInfoActuatorStatusMsg()) {
            uint16_t batteryVoltage = battery.current();    // BatteryTM class, mV resolution
            msg.append(batteryVoltage);
        }
    }
#endif
};

DEFREGISTER(Reg0, MASTERID_REGS, DREG_CYCLICINFOMSG, DREG_LOWBATLIMIT, DREG_TRANSMITTRYMAX)
class WDSList0 : public RegList0<Reg0> {
public:
    WDSList0(uint16_t addr)
        : RegList0<Reg0>(addr)
    {
    }
    void defaults()
    {
        clear();
        cycleInfoMsg(true);
        lowBatLimit(BAT_VOLT_LOW);
        transmitDevTryMax(6);
    }
};

DEFREGISTER(Reg1, CREG_AES_ACTIVE, CREG_MSGFORPOS, CREG_EVENTFILTERTIME, CREG_TRANSMITTRYMAX)
class WDSList1 : public RegList1<Reg1> {
public:
    WDSList1(uint16_t addr)
        : RegList1<Reg1>(addr)
    {
    }
    void defaults()
    {
        clear();
        msgForPosA(1);    // DRY
        msgForPosB(3);    // WET
        msgForPosC(2);    // WATER
        aesActive(false);
        eventFilterTime(5);
        transmitTryMax(6);
    }
};

//---------------------------------------------------------
// Wassermelder ADC/Position extension
//
// Sensor pin Masse: 4.7k nach Masse
// Sensor pin Wet:   4.7k zum ADC Eingang SENS_PIN_WET (A3), 100k vom ADC Eingang nach +3V
// Sensor pin Water: 4.7k zum ADC Eingang SENS_PIN_WATER (A2), 100k vom ADC Eingang nach +3V
//
// ADC Werte mit 10mm Abstand zwischen den Sensorpins, Batteriespannung 3V:
// offen          1023
// gebrueckt      88
// Wasser         550-650
// Mineralwasser  ca. 550
// Sekt           ca. 500

//---------------------------------------------------------
class ADCPosition : public Position {
    uint8_t m_SensePinWet, m_SensePinWater;

public:
    ADCPosition()
        : m_SensePinWet(0)
        , m_SensePinWater(0)
    {
        _present = true;
    }

    void init(uint8_t adcPinWet, uint8_t adcPinWater)
    {
        m_SensePinWet   = adcPinWet;
        m_SensePinWater = adcPinWater;
        pinMode(m_SensePinWet, INPUT);
        digitalWrite(m_SensePinWet, LOW);    // kein pull-up
        pinMode(m_SensePinWater, INPUT);
        digitalWrite(m_SensePinWater, LOW);    // kein pull-up
    }

    void measure(__attribute__((unused)) bool async = false)
    {
        // after device reset, make sure to always trigger a first message if not in dry state
        static bool onlyOnce = false;
        if (!onlyOnce) {
            onlyOnce  = true;
            _position = State::PosA;
            DPRINTLN(F("Init Pos"));
            return;
        }

        uint16_t adcWet = measureChannel(m_SensePinWet);
        DPRINT(F("ADC Wet:   "));
        DDECLN(adcWet);
#ifdef THREE_STATE
        uint16_t adcWater = measureChannel(m_SensePinWater);
        DPRINT(F("ADC Water: "));
        DDECLN(adcWater);
#endif

#ifdef TWO_STATE
        // SENS_PIN_WATER wird nicht benutzt
        if (adcWet < DETECTION_THRESHOLD) {
            _position = State::PosC;
            DPRINTLN(F("Status:    WATER"));
        } else {
            _position = State::PosA;
            DPRINTLN(F("Status:    DRY"));
        }
#elif defined THREE_STATE
        if (adcWater < DETECTION_THRESHOLD) {
            _position = State::PosC;
            DPRINTLN(F("Status:    WATER"));
        } else if (adcWet < DETECTION_THRESHOLD) {
            _position = State::PosB;
            DPRINTLN(F("Status:    WET"));
        } else {
            _position = State::PosA;
            DPRINTLN(F("Status:    DRY"));
        }
#else
#error "Invalid state define"
#endif
    }

    uint16_t measureChannel(uint8_t pin)
    {
        // setup ADC: complete ADC init in case other modules have chamged this
        ADCSRA = 1 << ADEN | 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0;    // enable ADC, prescaler 128 = 62.5kHz ADC clock @8MHz (range 50..1000 kHz)
        ADMUX  = 1 << REFS0;                                          // AREF: AVCC with external capacitor at AREF pin
        ADCSRB = 0;
        uint8_t channel = pin - PIN_A0;
        ADMUX |= (channel & 0x0F);    // select channel
        delay(30);                    // load CVref 100nF, 5*Tau = 25ms

        // 1x dummy read, dann Mittelwert aus 4 samples
        uint16_t adc = 0;
        for (uint8_t i = 0; i < 5; i++) {
            ADCSRA |= 1 << ADSC;
            uint8_t timeout = 50;    // start ADC
            while (ADCSRA & (1 << ADSC)) {
                delayMicroseconds(10);
                timeout--;
                if (timeout == 0)
                    break;
            }
            if (i > 0) {
                adc += (ADC & 0x3FF);
            }
        }
        return (adc >> 2);
    }

    uint32_t interval() { return seconds2ticks(MEASUREMENT_INTERVAL); }
};

// --------------------------------------------------------
template <class HALTYPE, class List0Type, class List1Type, class List4Type, int PEERCOUNT>
class ADCThreeStateChannel : public StateGenericChannel<ADCPosition, HALTYPE, List0Type, List1Type, List4Type, PEERCOUNT> {
public:
    typedef StateGenericChannel<ADCPosition, HALTYPE, List0Type, List1Type, List4Type, PEERCOUNT> BaseChannel;

    ADCThreeStateChannel()
        : BaseChannel() {};
    ~ADCThreeStateChannel() {}

    void init(uint8_t adcpin1, uint8_t adcpin2)
    {
        BaseChannel::init();
        BaseChannel::possens.init(adcpin1, adcpin2);
#ifdef WDS2_STANDARD
        DPRINTLN(F("Configuration: WDS2_STANDARD"));
#elif defined WDS2_CUSTOM
        DPRINTLN(F("Configuration: WDS2_CUSTOM"));
#endif
    }
};

typedef ADCThreeStateChannel<WdsHal, WDSList0, WDSList1, DefList4, PEERS_PER_CHANNEL> ChannelType;

#define CYCLETIME seconds2ticks(60ul * 60 * 12)    // 2 Status/Alive Meldungen pro Tag
class DevType : public StateDevice<WdsHal, ChannelType, 1, WDSList0, CYCLETIME> {
public:
    typedef StateDevice<WdsHal, ChannelType, 1, WDSList0, CYCLETIME> TSDevice;
    DevType(const DeviceInfo& info, uint16_t addr)
        : TSDevice(info, addr)
    {
    }
    virtual ~DevType() {}

    virtual void configChanged()
    {
        TSDevice::configChanged();
        DPRINTLN(F("Config Changed: List0"));

        uint8_t cycInfo = this->getList0().cycleInfoMsg();
        DPRINT(F("cycleInfoMsg: "));
        DDECLN(cycInfo);

        uint8_t lowBatLimit = this->getList0().lowBatLimit();
        DPRINT(F("lowBatLimit: "));
        DDECLN(lowBatLimit);
        battery().low(lowBatLimit);

        uint8_t txDevTryMax = this->getList0().transmitDevTryMax();
        DPRINT(F("transmitDevTryMax: "));
        DDECLN(txDevTryMax);
    }
};

WdsHal                hal;
DevType               sdev(devinfo, 0x20);
ConfigButton<DevType> cfgBtn(sdev);

void setup()
{
    DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
    sdev.init(hal);
    sdev.channel(1).init(SENS_PIN_WET, SENS_PIN_WATER);
    buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
    hal.battery.low(BAT_VOLT_LOW);
    hal.battery.critical(BAT_VOLT_CRITICAL);
    hal.battery.init(seconds2ticks(60ul * 60 * 24), sysclock);    // 1x Batt.messung täglich
    sdev.initDone();
}

void loop()
{
    bool worked = hal.runready();
    bool poll   = sdev.pollRadio();
    if (worked == false && poll == false) {
        // deep discharge protection
        // if we drop below critical battery level - switch off all and sleep forever
        if (hal.battery.critical()) {
            // this call will never return
            hal.activity.sleepForever(hal);
        }
        // if nothing to do - go sleep
        hal.activity.savePower<Sleep<>>(hal);
    }
}

/*
SensorEventMsg 0x41
mit msg.append(0x1234):
<- 0E 01 A2 41 4929D3 435F5B 01 00 C8 12 34  - 243
-> 0A 01 80 02 435F5B 4929D3 00  - 368
waitAck: 01

InfoActuatorStatusMsg 0x10 ("CYCLETIME")
mit msg.append(0x1234):
<- 10 02 A2 10 4929D3 435F5B 06 01 C8 00 20 12 34  - 913
-> 0A 02 80 02 435F5B 4929D3 00  - 1038
waitAck: 01
*/
