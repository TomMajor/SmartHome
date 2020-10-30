//---------------------------------------------------------
// HB-UNI-Sensor-Blitz
// Version 1.00
// (C) 2020 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++             2016 papa (Creative Commons)
// HB-UNI-Sen-WEA       2018 jp112sdl (Creative Commons)
// PWFusion_AS3935_I2C  2015 Playing With Fusion, J. Steinlage
// AS3935_Wire (C)      2012 Raivis Rengelis
//---------------------------------------------------------

//---------------------------------------------------------
// !! NDEBUG sollte aktiviert werden wenn die Sensorentwicklung und die Tests abgeschlossen sind und das Gerät in den 'Produktionsmodus' geht.
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
#include "tmBattery.h"
#include "Sens_AS3935.h"
#include "Sens_DS18X20.h"

// clang-format off

#define LED_PIN             4
#define CONFIG_BUTTON_PIN   8
#define ONEWIRE_PIN         5
#define CALIBRATION_PIN     A0
#define BAT_VOLT_LOW        23              // 2.3V
#define BAT_VOLT_CRITICAL   20              // 2.0V
#define UPDATE_INTERVAL_BAT (12ul * 3600)   // alle 12h Batterie messen
#define UPDATE_INTERVAL_DEF (1ul * 3600)    // ohne Blitzevent alle 6h bei der Zentrale melden
#define PEERS_PER_CHANNEL   6               // number of available peers per channel

// clang-format on

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
// Bei mehreren Geräten des gleichen Typs muss Device ID und Device Serial unterschiedlich sein!
const struct DeviceInfo PROGMEM devinfo = {
    { 0xA5, 0xA5, 0xF0 },    // Device ID
    "UNI-BLITZ1",            // Device Serial
    { 0xF1, 0x20 },          // Device Model
    // Firmware Version
    // die CCU Addon xml Datei ist mit der Zeile <parameter index="9.0" size="1.0" cond_op="E" const_value="0x13" />
    // fest an diese Firmware Version gebunden! cond_op: E Equal, GE Greater or Equal
    // bei Änderungen von Payload, message layout, Datenpunkt-Typen usw. muss die Version an beiden Stellen hochgezogen werden!
    0x10,
    as::DeviceType::THSensor,    // Device Type
    { 0x01, 0x01 }               // Info Bytes
};

// Configure the used hardware
typedef AvrSPI<10, 11, 12, 13>                SPIType;
typedef Radio<SPIType, 2>                     RadioType;
typedef StatusLed<LED_PIN>                    LedType;
typedef AskSin<LedType, tmBattery, RadioType> BaseHal;

class Hal : public BaseHal {
public:
    void init(const HMID& id)
    {
        BaseHal::init(id);
        battery.init(seconds2ticks(UPDATE_INTERVAL_BAT), sysclock);
        battery.low(BAT_VOLT_LOW);
        battery.critical(BAT_VOLT_CRITICAL);
    }

    bool runready() { return sysclock.runready() || BaseHal::runready(); }
} hal;

class WeatherEventMsg : public Message {
public:
    void init(uint8_t msgcnt, int16_t temperature, uint16_t lightningCounter, uint8_t lightningDistance, uint16_t batteryVoltage, bool batLow)
    {
        uint8_t t1 = (temperature >> 8) & 0x7f;
        uint8_t t2 = temperature & 0xff;
        if (batLow == true) {
            t1 |= 0x80;    // set bat low bit
        }

        // als Standard wird BCAST gesendet um Energie zu sparen, siehe Beschreibung unten.
        // Bei jeder 20. Nachricht senden wir stattdessen BIDI|WKMEUP, um eventuell anstehende Konfigurationsänderungen auch
        // ohne Betätigung des Anlerntaster übernehmen zu können (mit Verzögerung, worst-case 20x Sendeintervall).
        uint8_t flags = BCAST;
        if ((msgcnt % 20) == 2) {
            flags = BIDI | WKMEUP;
        }
        Message::init(16, msgcnt, 0x70, flags, t1, t2);

        pload[0] = (lightningCounter >> 8) & 0xff;
        pload[1] = lightningCounter & 0xff;
        pload[2] = lightningDistance;
        pload[3] = (batteryVoltage >> 8) & 0xff;
        pload[4] = batteryVoltage & 0xff;
    }
};

DEFREGISTER(Reg0, MASTERID_REGS, DREG_LEDMODE, DREG_LOWBATLIMIT, DREG_TRANSMITTRYMAX, 0x20, 0x21)
class SensorList0 : public RegList0<Reg0> {
public:
    SensorList0(uint16_t addr)
        : RegList0<Reg0>(addr)
    {
    }

    bool     updInterval(uint16_t value) const { return this->writeRegister(0x20, (value >> 8) & 0xff) && this->writeRegister(0x21, value & 0xff); }
    uint16_t updInterval() const { return (this->readRegister(0x20, 0) << 8) + this->readRegister(0x21, 0); }

    void defaults()
    {
        clear();
        ledMode(1);
        lowBatLimit(BAT_VOLT_LOW);
        transmitDevTryMax(6);
        updInterval(UPDATE_INTERVAL_DEF);
    }
};

DEFREGISTER(Reg1, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07)
class SensorList1 : public RegList1<Reg1> {
public:
    SensorList1(uint16_t addr)
        : RegList1<Reg1>(addr)
    {
    }

    bool    capacitorIndex(uint8_t value) const { return this->writeRegister(0x01, value); }
    uint8_t capacitorIndex() const { return this->readRegister(0x01, 0); }

    bool    indoor(uint8_t value) const { return this->writeRegister(0x02, value); }
    uint8_t indoor() const { return this->readRegister(0x02, 0); }

    bool    showDisturbers(uint8_t value) const { return this->writeRegister(0x03, value); }
    uint8_t showDisturbers() const { return this->readRegister(0x03, 0); }

    bool    noiseFloor(uint8_t value) const { return this->writeRegister(0x04, value); }
    uint8_t noiseFloor() const { return this->readRegister(0x04, 0); }

    bool    spikeRejection(uint8_t value) const { return this->writeRegister(0x05, value); }
    uint8_t spikeRejection() const { return this->readRegister(0x05, 0); }

    bool    watchdogThreshold(uint8_t value) const { return this->writeRegister(0x06, value); }
    uint8_t watchdogThreshold() const { return this->readRegister(0x06, 0); }

    bool    minNumberOfLightnings(uint8_t value) const { return this->writeRegister(0x07, value); }
    uint8_t minNumberOfLightnings() const { return this->readRegister(0x07, 0); }

    void defaults()
    {
        clear();
        capacitorIndex(7);           // 4bit
        indoor(1);                   // 1bit: mode indoor/outdoor
        showDisturbers(0);           // 1bit: show/hide disturbers
        noiseFloor(2);               // 3bit
        spikeRejection(2);           // 4bit
        watchdogThreshold(2);        // 4bit
        minNumberOfLightnings(0);    // 2bit
    }
};

class WeatherChannel : public Channel<Hal, SensorList1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {

    WeatherEventMsg msg;
    int16_t         temperature10;
    uint16_t        lightningCounter;
    uint8_t         lightningDistance;
    uint16_t        batteryVoltage;
    bool            bAS3935Initialized;
    bool            bPeriodicWakeUp;
    Sens_DS18X20    ds18x20;

public:
    Sens_AS3935 as3935;
    uint16_t    sleepCounter;

    WeatherChannel()
        : Channel()
        , Alarm(seconds2ticks(60))
        , temperature10(0)
        , lightningCounter(0)
        , lightningDistance(255)
        , batteryVoltage(0)
        , bAS3935Initialized(false)
        , bPeriodicWakeUp(true)
        , sleepCounter(0)
    {
    }
    virtual ~WeatherChannel() {}

    virtual void trigger(AlarmClock& clock)
    {
        // as3935.disableINT();    // as3935 Interrupt abschalten, dieser könnte beim Senden ausgelöst werden (bei PIR aufgetreten)
        bool bValidLightningData = measureLightning();
        if (bValidLightningData || bPeriodicWakeUp) {
            measureSensors();
            if (bPeriodicWakeUp) {
                DPRINTLN(F("<- Periodic Message"));
            } else {
                DPRINTLN(F("<- Event Message"));
            }
            uint8_t msgcnt = device().nextcount();
            msg.init(msgcnt, temperature10, lightningCounter, lightningDistance, batteryVoltage, device().battery().low());
            if (msg.flags() & Message::BCAST) {
                device().broadcastEvent(msg, *this);
            } else {
                device().sendPeerEvent(msg, *this);
            }
        }
        // reactivate for next measure, but only for periodic wake up, not for interrupt
        if (bPeriodicWakeUp) {
            uint16_t updCycle = this->device().getList0().updInterval();
            set(seconds2ticks(updCycle));
            clock.add(*this);
        }
        // if (bPeriodicWakeUp) {      // bei Senden nach periodischem WakeUp den digitalInput Interrupt sofort wieder einschalten
        //     as3935.enableINT();    // andernfalls erst nach Entprellen in lightningEvent()
        // }
        bPeriodicWakeUp = true;
    }

    void lightningEvent()
    {
        // hier kein sysclock.cancel() da der ursprüngliche, periodische WakeUp Zyklus beibehalten werden soll
        bPeriodicWakeUp = false;
        trigger(sysclock);    // Messen/Senden
        delay(250);           // Verzögerung für wiederholtes Senden bzw. digitalInput Entprellen
        // as3935.enableINT();
    }

    bool measureLightning()
    {
        bool bRet = false;
        if (!bAS3935Initialized) {
            initAS3935();
            bAS3935Initialized = true;
        } else if (as3935.notifyEvent()) {
            uint8_t intSource = as3935.interruptSource();
            as3935.resetEvent();
            DPRINT(F("INT Source: "));
            DDEC(intSource);
            switch (intSource) {
            case 0:
                // If the AS3935 issues an interrupt and the Interrupt register is 0 the distance estimation has changed
                // due to purging of old events in the statistics, based on the lightning distance estimation algorithm.
                DPRINTLN(F(", Update Distance"));
                bRet = updateLightningDistance();    // only send message to CCU if lightning distance changed
                break;
            case 1:
                DPRINTLN(F(", Noise level too high"));
                break;
            case 4:
                DPRINTLN(F(", Disturber detected"));
                break;
            case 8:
                DPRINTLN(F(", Lightning"));
                lightningCounter++;
                if (lightningCounter == 0) {    // preset lightningCounter to 1 on overflow
                    lightningCounter = 1;
                }
                updateLightningDistance();
                bRet = true;
                break;
            default:
                DPRINTLN(F(", Unknown"));
                break;
            }
        }
        return bRet;
    }

    void measureSensors()
    {
        ds18x20.measure();
        temperature10  = ds18x20.temperature();
        batteryVoltage = device().battery().current();    // BatteryTM class, mV resolution
    }

    bool updateLightningDistance()
    {
        bool    bRet       = false;
        uint8_t lightnDist = as3935.lightningDistanceKm();
        if (lightnDist != lightningDistance) {
            lightningDistance = lightnDist;
            bRet              = true;
            DPRINT(F("Distance (Change): "));
        } else {
            DPRINT(F("Distance (No Change): "));
        }
        DDEC(lightningDistance);
        DPRINTLN(F(" km"));
        return bRet;
    }

    bool initAS3935()
    {
        uint8_t capacitorIndex        = this->getList1().capacitorIndex();
        uint8_t indoor                = this->getList1().indoor();
        uint8_t showDisturbers        = this->getList1().showDisturbers();
        uint8_t noiseFloor            = this->getList1().noiseFloor();
        uint8_t spikeRejection        = this->getList1().spikeRejection();
        uint8_t watchdogThreshold     = this->getList1().watchdogThreshold();
        uint8_t minNumberOfLightnings = this->getList1().minNumberOfLightnings();
        if (as3935.init(capacitorIndex, indoor, showDisturbers, noiseFloor, spikeRejection, watchdogThreshold, minNumberOfLightnings)) {
            DPRINTLN(F("Sensor init done"));
            updateLightningDistance();    // 1x lightningDistance vom Chip nach Init einlesen, für das erste HM Telegramm nach Reset
            DPRINTLN(F("-------------------------------------"));
            DPRINTLN(F("AS3935 Chip Data:"));
            as3935.dumpRegs();
            as3935.printParams();
            DPRINTLN(F("-------------------------------------"));
            return true;
        } else {
            DPRINTLN(F("ERROR: AS3935 Init"));
            return false;
        }
    }

    void initSensors() { ds18x20.init(ONEWIRE_PIN); }

    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr)
    {
        Channel::setup(dev, number, addr);
        initSensors();
        set(seconds2ticks(5));    // first message in 5 sec.
        sysclock.add(*this);
    }

    void configChanged()
    {
        DPRINTLN(F("Config changed: List1"));
        bool bChange = false;

        if (this->getList1().capacitorIndex() != as3935.getCapacitorIndex()) {
            DPRINT(F("Changed capacitorIndex: "));
            DDECLN(this->getList1().capacitorIndex());
            bChange = true;
        }

        if (this->getList1().indoor() != as3935.getIndoor()) {
            DPRINT(F("Changed indoor: "));
            DDECLN(this->getList1().indoor());
            bChange = true;
        }

        if (this->getList1().showDisturbers() != as3935.getShowDisturbers()) {
            DPRINT(F("Changed showDisturbers: "));
            DDECLN(this->getList1().showDisturbers());
            bChange = true;
        }

        if (this->getList1().noiseFloor() != as3935.getNoiseFloor()) {
            DPRINT(F("Changed noiseFloor: "));
            DDECLN(this->getList1().noiseFloor());
            bChange = true;
        }

        if (this->getList1().spikeRejection() != as3935.getSpikeRejection()) {
            DPRINT(F("Changed spikeRejection: "));
            DDECLN(this->getList1().spikeRejection());
            bChange = true;
        }

        if (this->getList1().watchdogThreshold() != as3935.getWatchdogThreshold()) {
            DPRINT(F("Changed watchdogThreshold: "));
            DDECLN(this->getList1().watchdogThreshold());
            bChange = true;
        }

        if (this->getList1().minNumberOfLightnings() != as3935.getMinNumberOfLightnings()) {
            DPRINT(F("Changed minNumberOfLightnings: "));
            uint8_t minLightnings = this->getList1().minNumberOfLightnings();
            DDECLN(as3935.decodeMinNumberOfLightnings(minLightnings));
            bChange = true;
        }

        if (!bChange) {
            DPRINTLN(F("No Change"));
        }

        // sometimes the event configChanged List1 occures twice, therefore we wait a bit before the actual chip reinit
        if (bAS3935Initialized && bChange) {    // Chip already initialized AND something changed?
            as3935.disableINT();
            bAS3935Initialized = false;    // flag for new chip init in measure()
            DPRINTLN(F("Reinit AS3935 in 5sec"));
            sysclock.cancel(*this);
            set(seconds2ticks(5));    // do the reinit in 5sec
            sysclock.add(*this);
        }
    }

    uint8_t status() const { return 0; }

    uint8_t flags() const { return 0; }
};

class SensChannelDevice : public MultiChannelDevice<Hal, WeatherChannel, 1, SensorList0> {
public:
    typedef MultiChannelDevice<Hal, WeatherChannel, 1, SensorList0> TSDevice;
    SensChannelDevice(const DeviceInfo& info, uint16_t addr)
        : TSDevice(info, addr)
    {
    }
    virtual ~SensChannelDevice() {}

    virtual void configChanged()
    {
        TSDevice::configChanged();
        DPRINTLN(F("Config Changed: List0"));

        uint8_t ledMode = this->getList0().ledMode();
        DPRINT(F("ledMode: "));
        DDECLN(ledMode);

        uint8_t lowBatLimit = this->getList0().lowBatLimit();
        DPRINT(F("lowBatLimit: "));
        DDECLN(lowBatLimit);
        battery().low(lowBatLimit);

        uint8_t txDevTryMax = this->getList0().transmitDevTryMax();
        DPRINT(F("transmitDevTryMax: "));
        DDECLN(txDevTryMax);

        uint16_t updCycle = this->getList0().updInterval();
        DPRINT(F("updCycle: "));
        DDECLN(updCycle);
    }
};

SensChannelDevice               sdev(devinfo, 0x20);
ConfigButton<SensChannelDevice> cfgBtn(sdev);

void setup()
{
    DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
    sdev.init(hal);
    buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
    sdev.initDone();
    //
    pinMode(CALIBRATION_PIN, INPUT_PULLUP);
    delay(250);
    if (digitalRead(CALIBRATION_PIN) == LOW) {
        DPRINTLN(F("AS3935 CALIBRATION MODE"));
        if (sdev.channel(1).as3935.init(0, 1, 0, 2, 2, 2, 0)) {
            sdev.channel(1).as3935.disableINT();
            sdev.channel(1).as3935.calibrateFreq();
        }
        DPRINTLN(F("CALIBRATION DONE"));
        while (1) {
            ;
        }
    }
}

void loop()
{
    bool worked = hal.runready();
    bool poll   = sdev.pollRadio();
    if (worked == false && poll == false) {
        if (sdev.channel(1).as3935.notifyEvent()) {
            delay(5);    // "After the signal IRQ goes high the external unit should wait 2ms before reading the interrupt register."
            DPRINT(F("AS3935 INT ("));
            DDEC(sdev.channel(1).sleepCounter);
            DPRINTLN(F(")"));
            sdev.channel(1).lightningEvent();
        }
        // deep discharge protection - if we drop below critical battery level - switch off all and sleep forever
        if (hal.battery.critical()) {
            // this call will never return
            hal.activity.sleepForever(hal);
        }
        // if nothing to do - go sleep
        hal.activity.savePower<Sleep<>>(hal);
        sdev.channel(1).sleepCounter++;
    }
}
