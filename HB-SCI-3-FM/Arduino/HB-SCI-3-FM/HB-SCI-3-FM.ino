//---------------------------------------------------------
// HB-SCI-3-FM
// Version 1.01
// (C) 2018-2020 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <Register.h>
#include <ThreeState.h>

//---------------------------------------------------------
// sample time / wakeup interval
#define SAMPLE_TIME_SEC 1    // 1 Sekunde ist default in AskSinPP class Position, file Sensors.h

//---------------------------------------------------------
// Battery definitions
#define BAT_VOLT_LOW 22         // 2.2V
#define BAT_VOLT_CRITICAL 19    // 1.9V

//---------------------------------------------------------
// Pin definitions
#define CHANNEL_COUNT 3
#define INPUT1_PIN A0
#define INPUT2_PIN A1
#define INPUT3_PIN A2
#define CONFIG_BUTTON_PIN A3
#define LED_PIN 7

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    { 0xA5, 0xA5, 0x20 },    // Device ID
    "SCI3FM_001",            // Device Serial
    { 0x00, 0x5F },          // Device Model
    0x10,                    // Firmware Version
    as::DeviceType::Swi,     // Device Type
    { 0x00, 0x00 }           // Info Bytes
};

// Configure the used hardware
typedef AvrSPI<10, 11, 12, 13>                    SPIType;
typedef Radio<SPIType, 2>                         RadioType;
typedef StatusLed<LED_PIN>                        LedType;
typedef AskSin<LedType, BatterySensor, RadioType> BaseHal;
class Hal : public BaseHal {
public:
    void init(const HMID& id)
    {
        BaseHal::init(id);
        // measure battery every 12h
        battery.init(seconds2ticks(12UL * 60 * 60), sysclock);
        battery.low(BAT_VOLT_LOW);
        battery.critical(BAT_VOLT_CRITICAL);
    }
} hal;

DEFREGISTER(Reg0, DREG_INTKEY, DREG_CYCLICINFOMSG, MASTERID_REGS, DREG_TRANSMITTRYMAX)
class RHSList0 : public RegList0<Reg0> {
public:
    RHSList0(uint16_t addr)
        : RegList0<Reg0>(addr)
    {
    }
    void defaults()
    {
        clear();
        cycleInfoMsg(true);
        transmitDevTryMax(6);
    }
};

DEFREGISTER(Reg1, CREG_AES_ACTIVE, CREG_MSGFORPOS, CREG_EVENTDELAYTIME, CREG_LEDONTIME, CREG_TRANSMITTRYMAX)
class RHSList1 : public RegList1<Reg1> {
public:
    RHSList1(uint16_t addr)
        : RegList1<Reg1>(addr)
    {
    }
    void defaults()
    {
        clear();
        msgForPosA(1);    // CLOSED
        msgForPosB(2);    // OPEN
        // aesActive(false);
        eventDelaytime(0);
        ledOntime(100);
        transmitTryMax(6);
    }
};

// --------------------------------------------------------
class ONEPinPosition : public Position {
    uint8_t m_SensePin;

public:
    ONEPinPosition()
        : m_SensePin(0)
    {
        _present = true;
    }

    void init(uint8_t input_pin)
    {
        m_SensePin = input_pin;
        pinMode(m_SensePin, INPUT);
        digitalWrite(m_SensePin, HIGH);    // pull-up enabled
    }

    void measure(__attribute__((unused)) bool async = false)
    {
        if (digitalRead(m_SensePin)) {    // high, offen
            _position = State::PosB;
        } else {    // low, geschlossen
            _position = State::PosA;
        }
    }

    uint32_t interval() { return seconds2ticks(SAMPLE_TIME_SEC); }    // alle xx Sekunden aufwachen/messen
};

// --------------------------------------------------------
template <class HALTYPE, class List0Type, class List1Type, class List4Type, int PEERCOUNT>
class OnePinChannel : public ThreeStateGenericChannel<ONEPinPosition, HALTYPE, List0Type, List1Type, List4Type, PEERCOUNT> {
public:
    typedef ThreeStateGenericChannel<ONEPinPosition, HALTYPE, List0Type, List1Type, List4Type, PEERCOUNT> BaseChannel;

    OnePinChannel()
        : BaseChannel() {};
    ~OnePinChannel() {}

    void init(uint8_t input_pin)
    {
        BaseChannel::init();
        BaseChannel::possens.init(input_pin);
    }
};

typedef OnePinChannel<Hal, RHSList0, RHSList1, DefList4, PEERS_PER_CHANNEL> ChannelType;
typedef ThreeStateDevice<Hal, ChannelType, CHANNEL_COUNT, RHSList0>         RHSType;

RHSType               sdev(devinfo, 0x20);
ConfigButton<RHSType> cfgBtn(sdev);

void setup()
{
    DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
    sdev.init(hal);
    buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
    sdev.channel(1).init(INPUT1_PIN);
    sdev.channel(2).init(INPUT2_PIN);
    sdev.channel(3).init(INPUT3_PIN);
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
