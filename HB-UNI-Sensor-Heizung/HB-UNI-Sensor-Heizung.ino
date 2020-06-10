//---------------------------------------------------------
// HB-UNI-Sensor-Heizung (ein modifizierter HB-UNI-Sensor1)
// Version 2.01
// (C) 2019-2020 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

//---------------------------------------------------------
// !! NDEBUG sollte aktiviert werden wenn die Sensorentwicklung und die Tests abgeschlossen sind und das Gerät in den 'Produktionsmodus' geht.
// Zum Beispiel bei aktiviertem BME280 und MAX44009 werden damit ca. 2,6 KBytes Flash und 100 Bytes RAM eingespart.
// Insbesondere die RAM-Einsparungen sind wichtig für die Stabilität / dynamische Speicherzuweisungen etc.
// Dies beseitigt dann auch die mögliche Arduino-Warnung 'Low memory available, stability problems may occur'.
//
#define NDEBUG

//---------------------------------------------------------
// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <MultiChannelDevice.h>
#include <Register.h>
#include "Sensors/tmBattery.h"

//---------------------------------------------------------
// clang-format off
#define cDEVICE_ID          { 0xA5, 0xA5, 0xEE }
#define cDEVICE_SERIAL      "UNISENSHZG"

#define CONFIG_BUTTON_PIN   4
#define LED_PIN             5
#define ONEWIRE_PIN         A0
#define MAX44009_ADDR       0x4A
#define CLOCK_SYSCLOCK
#define BAT_SENSOR          tmBatteryLoad<A3, 3, 4000, 200>
#define BAT_VOLT_LOW        11      // 1.1V
#define BAT_VOLT_CRITICAL   9       // 0.9V

#define MEASURE_INTERVAL    54      // 60sec (Korrekturfaktor: 0,8928, siehe WDT_Frequenz.ino)
#define CHK_ERROR_CYCLE     10      // jedes 10 Messung auf Fehler (Blinken) prüfen, hier aller 600sec
#define MAX_SEND_INTERVAL   25712   // 8h (28800sec, Korrekturfaktor: 0,8928), auch ohne Schwellwertänderung mindestens in diesem Intervall senden
#define LIGHT_THRESHOLD     75      // Schwellwert Lux
// clang-format on

enum eHgzState { hzgOff = 0, hzgOn = 1, hzgStoer = 2 };

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

#include "Sensors/Sens_DS18X20.h"     // HB-UNI-Sensor1 custom sensor class
#include "Sensors/Sens_MAX44009.h"    // HB-UNI-Sensor1 custom sensor class

#ifdef CLOCK_SYSCLOCK
#define CLOCK sysclock
#define SAVEPWR_MODE Sleep<>
#elif defined CLOCK_RTC
#define CLOCK rtc
#define SAVEPWR_MODE SleepRTC
#undef seconds2ticks
#define seconds2ticks(tm) (tm)
#else
#error INVALID CLOCK OPTION
#endif

// define all device properties
// Bei mehreren Geräten des gleichen Typs (HB-UNI-Sensor1) muss Device ID und Device Serial unterschiedlich sein!
const struct DeviceInfo PROGMEM devinfo = {
    cDEVICE_ID,        // Device ID
    cDEVICE_SERIAL,    // Device Serial
    { 0xF1, 0x05 },    // Device Model
    // Firmware Version
    // die CCU Addon xml Datei ist mit der Zeile <parameter index="9.0" size="1.0" cond_op="E" const_value="0x10" />
    // fest an diese Firmware Version gebunden! cond_op: E Equal, GE Greater or Equal
    // bei Änderungen von Payload, message layout, Datenpunkt-Typen usw. muss die Version an beiden Stellen hochgezogen werden!
    0x10,
    as::DeviceType::THSensor,    // Device Type
    { 0x01, 0x01 }               // Info Bytes
};

// Configure the used hardware
typedef AvrSPI<10, 11, 12, 13>                 SPIType;
typedef Radio<SPIType, 2>                      RadioType;
typedef StatusLed<LED_PIN>                     LedType;
typedef AskSin<LedType, BAT_SENSOR, RadioType> BaseHal;

class Hal : public BaseHal {
public:
    void init(const HMID& id)
    {
        BaseHal::init(id);
#ifdef CLOCK_RTC
        rtc.init();    // init real time clock - 1 tick per second
#endif
        // measure battery every 12h
        battery.init(seconds2ticks(12UL * 60 * 60), CLOCK);
        battery.low(BAT_VOLT_LOW);
        battery.critical(BAT_VOLT_CRITICAL);
    }

    bool runready() { return CLOCK.runready() || BaseHal::runready(); }
} hal;

class WeatherEventMsg : public Message {
public:
    void init(uint8_t msgcnt, int16_t temp, uint32_t brightness, uint8_t state, uint16_t batteryVoltage, bool batLow)
    {

        uint8_t t1 = (temp >> 8) & 0x7f;
        uint8_t t2 = temp & 0xff;
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
        Message::init(19, msgcnt, 0x70, flags, t1, t2);

        // Message Length (first byte param.): 11 + payload
        //  1 Byte payload -> length 12
        // 12 Byte payload -> length 23
        // max. payload: 17 Bytes (https://www.youtube.com/watch?v=uAyzimU60jw)

        // BIDI|WKMEUP: erwartet ACK vom Empfänger, ohne ACK wird das Senden wiederholt
        // LazyConfig funktioniert, d.h. eine anstehende Conf.Änderung von der CCU wird nach dem nächsten Senden übernommen. Aber erhöhter
        // Funkverkehr wegen ACK
        //
        // BCAST: ohne ACK zu Erwarten, Standard für HM Sensoren.
        // LazyConfig funktioniert nicht, d.h. eine anstehende Conf.Änderung von der CCU muss durch den Config Button am Sensor übernommen
        // werden!!

        // papa:
        // BIDI - fordert den Empfänger auf ein Ack zu schicken. Das wird auch zwingend für AES-Handling gebraucht. BCAST - signalisiert
        // eine Broadcast-Message. Das wird z.B. verwendet, wenn mehrere Peers vor einen Sensor existieren. Es wird dann an einen Peer
        // gesndet und zusätzlich das BCAST-Flag gesetzt. So dass sich alle die Nachrricht ansehen. Ein Ack macht dann natürlich keinen Sinn
        // - es ist ja nicht klar, wer das senden soll.
        //
        // WKMEUP - wird für LazyConfig verwendet. Ist es in einer Message gesetzt, so weiss
        // die Zentrale, dass das Geräte noch kurz auf weitere Nachrichten wartet. Die Lib setzt diese Flag für die StatusInfo-Message
        // automatisch. Außerdem bleibt nach einer Kommunikation der Empfang grundsätzlich für 500ms angeschalten.

        // brightness (Lux)
        pload[0] = (brightness >> 24) & 0xff;
        pload[1] = (brightness >> 16) & 0xff;
        pload[2] = (brightness >> 8) & 0xff;
        pload[3] = (brightness >> 0) & 0xff;

        // state
        pload[4] = state;

        // batteryVoltage
        pload[5] = (batteryVoltage >> 8) & 0xff;
        pload[6] = batteryVoltage & 0xff;
    }
};

DEFREGISTER(Reg0, MASTERID_REGS, DREG_LEDMODE, DREG_LOWBATLIMIT, DREG_TRANSMITTRYMAX)
class SensorList0 : public RegList0<Reg0> {
public:
    SensorList0(uint16_t addr)
        : RegList0<Reg0>(addr)
    {
    }

    void defaults()
    {
        clear();
        ledMode(1);
        lowBatLimit(BAT_VOLT_LOW);
        transmitDevTryMax(6);
    }
};

class WeatherChannel : public Channel<Hal, List1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {

    WeatherEventMsg msg;

    int16_t                      temperature10;
    uint32_t                     brightness100;
    uint8_t                      hzgState;
    uint16_t                     batteryVoltage;
    uint8_t                      measureTemp;
    Sens_DS18X20                 ds18x20;
    Sens_MAX44009<MAX44009_ADDR> max44009;

public:
    WeatherChannel()
        : Channel()
        , Alarm(seconds2ticks(60))
        , temperature10(0)
        , brightness100(0)
        , hzgState(hzgOff)
        , batteryVoltage(0)
        , measureTemp(0)
    {
    }
    virtual ~WeatherChannel() {}

    virtual void trigger(AlarmClock& clock)
    {
        measure();
        // ========================================
        // HB-UNI-Sensor-Heizung Sondercode
        // der Datenpunkt hzgState zum Senden an die Zentrale wird nur in shouldSend() geändert!
        bool bSend = shouldSend(brightness100 / 100);
        if (bSend) {
            uint8_t msgcnt = device().nextcount();
            msg.init(msgcnt, temperature10, brightness100, hzgState, batteryVoltage, device().battery().low());
            if (msg.flags() & Message::BCAST) {
                device().broadcastEvent(msg, *this);
            } else {
                device().sendPeerEvent(msg, *this);
            }
        }
        // ========================================
        // reactivate for next measure
        set(seconds2ticks(MEASURE_INTERVAL));
        clock.add(*this);
    }

    void measure()
    {
        // HB-UNI-Sensor-Heizung Sondercode
        if (!(measureTemp % 16)) {    // nur alle 16min die Temp messen (Strom sparen)
            ds18x20.measure();
            temperature10 = ds18x20.temperature();
        }
        measureTemp++;

        max44009.measure();
        brightness100 = max44009.brightnessLux();

        batteryVoltage = device().battery().current();    // BatteryTM class, mV resolution
    }

    void initSensors()
    {
        ds18x20.init(ONEWIRE_PIN);
        // HB-UNI-Sensor-Heizung Sondercode, max44009.init() parameter
        // wegen Erkennung Blinken s.u.: MAX44009 Continuous mode, Manual mode, Integration Time 100msec
        max44009.init(0xC3);

        DPRINTLN(F("Sensor setup done"));
        DPRINT(F("Serial: "));
        DPRINTLN(cDEVICE_SERIAL);
#ifdef CLOCK_SYSCLOCK
        DPRINTLN(F("Clock SYSCLOCK"));
#elif defined CLOCK_RTC
        DPRINTLN(F("Clock RTC"));
#endif
    }

    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr)
    {
        Channel::setup(dev, number, addr);
        initSensors();
        set(seconds2ticks(5));    // first message in 5 sec.
        CLOCK.add(*this);
    }

    void configChanged()
    {
        // DPRINTLN(F("Config changed: List1"));
    }

    // HB-UNI-Sensor-Heizung Sondercode
    bool shouldSend(uint32_t lux)
    {
        static uint16_t silentCounter     = MAX_SEND_INTERVAL / MEASURE_INTERVAL;    // nach Power-On sofort ein Telegramm senden
        static uint8_t  checkErrorCounter = 0;
        bool            changeError       = false;
        bool            changeOnOff       = false;
        bool            changeMaxTime     = false;

        DPRINT(F("Brightness: "));
        DDECLN(lux);

        // Erkennung Blinken 10sec
        if (checkErrorCounter >= CHK_ERROR_CYCLE) {
            checkErrorCounter = 0;
            uint8_t lightLast = 0, light = 0, blinkCounter = 0;
            for (int i = 0; i < 100; i++) {    // 10sec
                max44009.measure();
                uint32_t luxNow = max44009.brightnessLux() / 100;
                if (luxNow >= LIGHT_THRESHOLD) {
                    light = 1;
                } else {
                    light = 0;
                }
                if (light != lightLast) {
                    lightLast = light;
                    blinkCounter++;
                }
                delay(100);
            }
            DPRINT(F("BLINK COUNT "));
            DDECLN(blinkCounter);
            // Buderus Blinken on/off 0,63sec/0,57sec = 16..17 Flanken in 10sec
            // klappt nur wenn der MAX44009 eine kleinere Integration Time als seine default 800msec hat! siehe max44009.init()
            if ((hzgState != hzgStoer) && (blinkCounter >= 8)) {
                hzgState = hzgStoer;
                DPRINTLN(F("ERROR ON"));
                changeError = true;
            } else if ((hzgState == hzgStoer) && (blinkCounter < 8)) {
                hzgState = hzgOff;
                DPRINTLN(F("ERROR OFF"));
                changeError = true;
            }
        }

        // im Fehlerfall macht der on/off State 0/1 keinen Sinn
        if (hzgState != hzgStoer) {
            if ((hzgState == hzgOff) && (lux >= LIGHT_THRESHOLD)) {
                hzgState = hzgOn;
                DPRINTLN(F("STATE ON"));
                changeOnOff = true;
            } else if ((hzgState == hzgOn) && (lux < LIGHT_THRESHOLD)) {
                hzgState = hzgOff;
                DPRINTLN(F("STATE OFF"));
                changeOnOff = true;
            }
        }

        // Erkennung max. Zeit ohne Sende-Telegramm erreicht, die CCU soll nicht unruhig werden
        if (silentCounter >= (MAX_SEND_INTERVAL / MEASURE_INTERVAL)) {
            DPRINTLN(F("MAX INTERVAL EXPIRED"));
            changeMaxTime = true;
        }

        silentCounter++;
        checkErrorCounter++;

        // clang-format off
        DPRINT(F("shouldSend: ")); DDEC(changeError); DPRINT(F(" ")); DDEC(changeOnOff); DPRINT(F(" ")); DDECLN(changeMaxTime);
        // clang-format on

        if (changeError || changeOnOff || changeMaxTime) {
            silentCounter = 0;
            return true;
        }
        return false;
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
        hal.activity.savePower<SAVEPWR_MODE>(hal);
    }
}
