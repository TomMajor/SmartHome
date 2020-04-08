//---------------------------------------------------------
// HB-SEN-LevelJet
// Version 1.04
// (C) 2019-2020 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
// HB-UNI-Sen-LEV-US 2018-04-16 jp112sdl Creative Commons
//---------------------------------------------------------

//---------------------------------------------------------
// !! NDEBUG beim LevelJet Sketch unbedingt ausgeschaltet lassen
// Serial Port Init (DINIT) via AskSinPP/Debug.h wird 2fach benutzt: Tx AskSinPP Debug out, Rx LevelJet (19200 Baud erforderlich)
//#define NDEBUG

//---------------------------------------------------------
// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <MultiChannelDevice.h>
#include <Register.h>
#include "src/Leveljet.h"

//---------------------------------------------------------
// Pin definitions
#define CONFIG_BUTTON_PIN 5
#define LED_PIN 7

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

LEVELJET<true, true> leveljet;    // Pegel in mm, benutze Peiltabelle

// define all device properties
// Bei mehreren Geräten des gleichen Typs muss Device ID und Device Serial unterschiedlich sein!
const struct DeviceInfo PROGMEM devinfo = {
    { 0xA5, 0xA6, 0x00 },    // Device ID
    "LEVELJET01",            // Device Serial
    { 0xF1, 0x04 },          // Device Model
    // Firmware Version
    // die CCU Addon xml Datei ist mit der Zeile <parameter index="9.0" size="1.0" cond_op="E" const_value="0x10" />
    // fest an diese Firmware Version gebunden! cond_op: E Equal, GE Greater or Equal
    // bei Änderungen von Payload, message layout, Datenpunkt-Typen usw. muss die Version an beiden Stellen hochgezogen werden!
    0x10,
    0x53,             // Device Type
    { 0x01, 0x01 }    // Info Bytes
};

// Configure the used hardware
typedef AvrSPI<10, 11, 12, 13>                SPIType;
typedef Radio<SPIType, 2>                     RadioType;
typedef StatusLed<LED_PIN>                    LedType;
typedef AskSin<LedType, NoBattery, RadioType> Hal;
Hal                                           hal;

// die "freien" Register 0x20/21 werden hier als 16bit memory für das Update
// Intervall in Sek. benutzt siehe auch hb-sen-leveljet.xml, <parameter id="Sendeintervall">
DEFREGISTER(Reg0, MASTERID_REGS, DREG_TRANSMITTRYMAX, 0x20, 0x21)
class SensorList0 : public RegList0<Reg0> {
public:
    SensorList0(uint16_t addr)
        : RegList0<Reg0>(addr)
    {
    }

    bool     updIntervall(uint16_t value) const { return this->writeRegister(0x20, (value >> 8) & 0xff) && this->writeRegister(0x21, value & 0xff); }
    uint16_t updIntervall() const { return (this->readRegister(0x20, 0) << 8) + this->readRegister(0x21, 0); }

    void defaults()
    {
        clear();
        transmitDevTryMax(6);
        updIntervall(21600);    // 6h
    }
};

class MeasureEventMsg : public Message {
public:
    void init(uint8_t msgcnt, uint8_t percent, uint16_t levelMM, uint16_t volumeLiter)
    {
        // als Standard wird BCAST gesendet um Energie zu sparen, siehe Beschreibung HB-UNI-Sensor1.
        // Bei jeder 10. Nachricht senden wir stattdessen BIDI|WKMEUP, um eventuell anstehende Konfigurationsänderungen auch
        // ohne Betätigung des Anlerntaster übernehmen zu können (mit Verzögerung, worst-case 10x Sendeintervall).
        uint8_t flags = BCAST;
        if ((msgcnt % 10) == 2) {
            flags = BIDI | WKMEUP;
        }
        Message::init(15, msgcnt, 0x53, flags, percent, 0);
        pload[0] = volumeLiter >> 8;
        pload[1] = volumeLiter & 0xff;
        pload[2] = levelMM >> 8;
        pload[3] = levelMM & 0xff;
    }
};

class MeasureChannel : public Channel<Hal, List1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {
    MeasureEventMsg msg;
    uint8_t         percent;
    uint16_t        levelMM;
    uint16_t        volumeLiter;

public:
    MeasureChannel()
        : Channel()
        , Alarm(seconds2ticks(60))
        , percent(0)
        , levelMM(0)
        , volumeLiter(0)
    {
    }
    virtual ~MeasureChannel() {}

    virtual void trigger(AlarmClock& clock)
    {
        measure();
        uint8_t msgcnt = device().nextcount();
        msg.init(msgcnt, percent, levelMM, volumeLiter);
        if (msg.flags() & Message::BCAST) {
            device().broadcastEvent(msg, *this);
        } else {
            device().sendPeerEvent(msg, *this);
        }
        // reactivate for next measure
        uint16_t updCycle = this->device().getList0().updIntervall();
        set(seconds2ticks(updCycle));
        clock.add(*this);
    }

    void measure()
    {
        percent     = leveljet.percent();
        levelMM     = leveljet.level();
        volumeLiter = leveljet.volume();
    }

    void setupSensor()
    {
        leveljet.init();
        DPRINTLN("Sensor setup done");
    }

    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr)
    {
        Channel::setup(dev, number, addr);
        setupSensor();
        set(seconds2ticks(20));    // first message in 20 sec.
        sysclock.add(*this);
    }

    void configChanged()
    {
        // DPRINTLN("Config changed: List1");
    }

    uint8_t status() const { return 0; }

    uint8_t flags() const { return 0; }
};

class SensChannelDevice : public MultiChannelDevice<Hal, MeasureChannel, 1, SensorList0> {
public:
    typedef MultiChannelDevice<Hal, MeasureChannel, 1, SensorList0> TSDevice;
    SensChannelDevice(const DeviceInfo& info, uint16_t addr)
        : TSDevice(info, addr)
    {
    }
    virtual ~SensChannelDevice() {}

    virtual void configChanged()
    {
        TSDevice::configChanged();
        DPRINTLN("Config Changed: List0");

        uint8_t txDevTryMax = this->getList0().transmitDevTryMax();
        DPRINT("transmitDevTryMax: ");
        DDECLN(txDevTryMax);

        uint16_t updCycle = this->getList0().updIntervall();
        DPRINT("updCycle: ");
        DDECLN(updCycle);
    }
};

SensChannelDevice               sdev(devinfo, 0x20);
ConfigButton<SensChannelDevice> cfgBtn(sdev);

void setup()
{
    // !! Serial Port Init (DINIT) via AskSinPP/Debug.h wird 2fach benutzt: Tx AskSinPP Debug out, Rx LevelJet (19200 Baud erforderlich)
    DINIT(19200, ASKSIN_PLUS_PLUS_IDENTIFIER);
    sdev.init(hal);
    buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
    sdev.initDone();
}

void loop()
{
    bool worked = hal.runready();
    bool poll   = sdev.pollRadio();
    if (worked == false && poll == false) {
        // Device ist Netzteil betrieben und sollte wach bleiben um die seriellen Telegramme vom Leveljet zu empfangen
        // if nothing to do - go sleep
        // hal.activity.savePower<Sleep<>>(hal);
        leveljet.loop();
        delay(1);
    }
}
