//---------------------------------------------------------
// HB-UNI-Sensor1
// Version 1.18
// (C) 2018-2020 Tom Major (Creative Commons)
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
#include "Sensors/tmBattery.h"

//---------------------------------------------------------
// Alle Device Parameter werden aus einer .h Datei (hier im Beispiel Cfg/Device_Example.h) geholt um mehrere Geräte ohne weitere Änderungen des
// Sketches flashen zu können. Für mehrere Geräte einfach mehrere .h Dateien anlegen und dort die Unterschiede zwischen den Geräten definieren. Die
// konfigurierbaren Device Parameter in der .h Datei sind im Einzelnen:
// - Device ID und Device Serial
// - Aktivierung der verwendeten Sensoren
// - Pin Definitionen Allgemein
// - Pin und Address Definitionen der Sensoren
// - Clock Definition
// - Schaltungsvariante und Pins für Batteriespannungsmessung
// - Schwellwerte für Batteriespannungsmessung
#include "Cfg/Device_Example.h"


// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

#ifdef SENSOR_DS18X20
#include "Sensors/Sens_DS18X20.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_BME280
#include "Sensors/Sens_BME280.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_BMP180
#include "Sensors/Sens_BMP180.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_MAX44009
#include "Sensors/Sens_MAX44009.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_TSL2561
#include "Sensors/Sens_TSL2561.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_BH1750
#include "Sensors/Sens_BH1750.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_SHT31
#include "Sensors/Sens_SHT31.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_SHT21
#include "Sensors/Sens_SHT21.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_SHT10
#include "Sensors/Sens_SHT10.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_DIGINPUT
#include "Sensors/Sens_DIGINPUT.h"    // HB-UNI-Sensor1 custom sensor class
Sens_DIGINPUT digitalInput;           // muss wegen Verwendung in loop() global sein (Interrupt event)
#endif

#ifdef SENSOR_VEML6070
#include "Sensors/Sens_VEML6070.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_VEML6075
#include "Sensors/Sens_VEML6075.h"    // HB-UNI-Sensor1 custom sensor class
#endif

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
// Device ID und Device Serial werden aus einer .h Datei (hier im Beispiel Cfg/Device_Example.h) geholt um mehrere Geräte ohne weitere Änderungen des
// Sketches flashen zu können.
const struct DeviceInfo PROGMEM devinfo = {
    cDEVICE_ID,        // Device ID
    cDEVICE_SERIAL,    // Device Serial
    { 0xF1, 0x03 },    // Device Model
    // Firmware Version
    // die CCU Addon xml Datei ist mit der Zeile <parameter index="9.0" size="1.0" cond_op="E" const_value="0x13" />
    // fest an diese Firmware Version gebunden! cond_op: E Equal, GE Greater or Equal
    // bei Änderungen von Payload, message layout, Datenpunkt-Typen usw. muss die Version an beiden Stellen hochgezogen werden!
    0x13,
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
    void init(uint8_t msgcnt, int16_t temp, uint16_t airPressure, uint8_t humidity, uint32_t brightness, uint8_t digInputState,
              uint16_t batteryVoltage, bool batLow, uint16_t customData)
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
        Message::init(23, msgcnt, 0x70, flags, t1, t2);

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

        // airPressure
        pload[0] = (airPressure >> 8) & 0xff;
        pload[1] = airPressure & 0xff;

        // humidity
        pload[2] = humidity;

        // brightness (Lux)
        pload[3] = (brightness >> 24) & 0xff;
        pload[4] = (brightness >> 16) & 0xff;
        pload[5] = (brightness >> 8) & 0xff;
        pload[6] = (brightness >> 0) & 0xff;

        // digInputState
        pload[7] = digInputState;

        // batteryVoltage
        pload[8] = (batteryVoltage >> 8) & 0xff;
        pload[9] = batteryVoltage & 0xff;

        // user custom data
        pload[10] = (customData >> 8) & 0xff;
        pload[11] = customData & 0xff;
    }
};

// die "freien" Register 0x20/21 werden hier als 16bit memory für das Update
// Intervall in Sek. benutzt siehe auch hb-uni-sensor1.xml, <parameter
// id="Sendeintervall"> .. ausserdem werden die Register 0x22/0x23 für den
// konf. Parameter Höhe benutzt
DEFREGISTER(Reg0, MASTERID_REGS, DREG_LEDMODE, DREG_LOWBATLIMIT, DREG_TRANSMITTRYMAX, 0x20, 0x21, 0x22, 0x23)
class SensorList0 : public RegList0<Reg0> {
public:
    SensorList0(uint16_t addr)
        : RegList0<Reg0>(addr)
    {
    }

    bool     updIntervall(uint16_t value) const { return this->writeRegister(0x20, (value >> 8) & 0xff) && this->writeRegister(0x21, value & 0xff); }
    uint16_t updIntervall() const { return (this->readRegister(0x20, 0) << 8) + this->readRegister(0x21, 0); }

    bool     altitude(uint16_t value) const { return this->writeRegister(0x22, (value >> 8) & 0xff) && this->writeRegister(0x23, value & 0xff); }
    uint16_t altitude() const { return (this->readRegister(0x22, 0) << 8) + this->readRegister(0x23, 0); }

    void defaults()
    {
        clear();
        ledMode(1);
        lowBatLimit(BAT_VOLT_LOW);
        transmitDevTryMax(6);
        updIntervall(600);
        altitude(0);
    }
};

class WeatherChannel : public Channel<Hal, List1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {

    WeatherEventMsg msg;

    int16_t  temperature10;
    uint16_t airPressure10;
    uint8_t  humidity;
    uint32_t brightness100;
    uint8_t  digInputState;
    uint16_t customData;
    uint16_t batteryVoltage;
    bool     regularWakeUp;

#ifdef SENSOR_DS18X20
    Sens_DS18X20 ds18x20;
#endif
#ifdef SENSOR_BME280
    Sens_BME280 bme280;
#endif
#ifdef SENSOR_BMP180
    Sens_BMP180 bmp180;
#endif
#ifdef SENSOR_MAX44009
    Sens_MAX44009<MAX44009_ADDR> max44009;
#endif
#ifdef SENSOR_TSL2561
    Sens_TSL2561<TSL2561_ADDR> tsl2561;
#endif
#ifdef SENSOR_BH1750
    Sens_BH1750<BH1750_ADDR> bh1750;
#endif
#ifdef SENSOR_SHT31
    Sens_SHT31<SHT31_ADDR> sht31;
#endif
#ifdef SENSOR_SHT21
    Sens_SHT21 sht21;
#endif
#ifdef SENSOR_SHT10
    Sens_SHT10<SHT10_DATAPIN, SHT10_CLKPIN> sht10;
#endif
#ifdef SENSOR_VEML6070
    Sens_VEML6070<> veml6070;
#endif
#ifdef SENSOR_VEML6075
    Sens_VEML6075 veml6075;
#endif

public:
    WeatherChannel()
        : Channel()
        , Alarm(seconds2ticks(60))
        , temperature10(0)
        , airPressure10(0)
        , humidity(0)
        , brightness100(0)
        , digInputState(0)
        , customData(0)
        , batteryVoltage(0)
        , regularWakeUp(true)
    {
    }
    virtual ~WeatherChannel() {}

    virtual void trigger(AlarmClock& clock)
    {
#ifdef SENSOR_DIGINPUT
        digitalInput.disableINT();    // digitalInput Interrupt abschalten, dieser könnte beim Senden ausgelöst werden (bei PIR aufgetreten)
#endif
        measure();
        uint8_t msgcnt = device().nextcount();
        msg.init(msgcnt, temperature10, airPressure10, humidity, brightness100, digInputState, batteryVoltage, device().battery().low(), customData);
        if (msg.flags() & Message::BCAST) {
            device().broadcastEvent(msg, *this);
        } else {
            device().sendPeerEvent(msg, *this);
        }
        // reactivate for next measure
        uint16_t updCycle = this->device().getList0().updIntervall();
        set(seconds2ticks(updCycle));
        clock.add(*this);
#ifdef SENSOR_DIGINPUT
        if (regularWakeUp) {             // bei Senden nach regulären WakeUp den digitalInput Interrupt sofort wieder einschalten
            digitalInput.enableINT();    // andernfalls erst nach Entprellen in forceSend()
        }
#endif
        regularWakeUp = true;
    }

    void forceSend()
    {
        CLOCK.cancel(*this);
        regularWakeUp = false;    // Verhindert enableINT in trigger()
        trigger(CLOCK);           // Messen/Senden
        delay(250);               // Verzögerung für wiederholtes Senden bzw. digitalInput Entprellen
#ifdef SENSOR_DIGINPUT
        digitalInput.enableINT();
#endif
    }

    void measure()
    {
        // Messwerte mit Dummy-Werten vorbelegen falls kein realer Sensor für die Messgröße vorhanden ist
        // zum Testen der Anbindung an HomeMatic/RaspberryMatic/FHEM
#if !defined(SENSOR_DS18X20) && !defined(SENSOR_BME280) && !defined(SENSOR_BMP180) && !defined(SENSOR_SHT31) && !defined(SENSOR_SHT21)               \
    && !defined(SENSOR_SHT10)
        temperature10 = 188;    // 18.8C (scaling 10)
#endif
#if !defined(SENSOR_BME280) && !defined(SENSOR_SHT31) && !defined(SENSOR_SHT21) && !defined(SENSOR_SHT10)
        humidity = 88;    // 88%
#endif
#if !defined(SENSOR_BME280) && !defined(SENSOR_BMP180)
        airPressure10 = 10880;    // 1088 hPa (scaling 10)
#endif
#if !defined(SENSOR_MAX44009) && !defined(SENSOR_TSL2561) && !defined(SENSOR_BH1750)
        brightness100 = 8800000;    // 88000 Lux (scaling 100)
#endif

// Entweder BME280 oder BMP180 für Luftdruck/Temp, ggf. für anderen Bedarf anpassen
#ifdef SENSOR_BME280
        uint16_t altitude = this->device().getList0().altitude();
        bme280.measure(altitude);
        temperature10 = bme280.temperature();
        airPressure10 = bme280.pressureNN();
        humidity      = bme280.humidity();
#elif defined SENSOR_BMP180
        uint16_t altitude = this->device().getList0().altitude();
        bmp180.measure(altitude);
        temperature10 = bmp180.temperature();
        airPressure10 = bmp180.pressureNN();
#endif

// Falls DS18X20 vorhanden, dessen Temp der BME280/BMP180 Temp vorziehen
#ifdef SENSOR_DS18X20
        ds18x20.measure();
        temperature10 = ds18x20.temperature();
#endif

// Feuchte/Temp vom SHT31/21/10 falls kein BME280 vorhanden
#ifndef SENSOR_BME280
#ifdef SENSOR_SHT31
        sht31.measure();
        temperature10 = sht31.temperature();
        humidity      = sht31.humidity();
#elif defined SENSOR_SHT21
        sht21.measure();
        temperature10 = sht21.temperature();
        humidity      = sht21.humidity();
#elif defined SENSOR_SHT10
        sht10.measure();
        temperature10 = sht10.temperature();
        humidity      = sht10.humidity();
#endif
#endif

// Entweder MAX44009 oder TSL2561 oder BH1750 für Helligkeit, ggf. für anderen Bedarf anpassen
#ifdef SENSOR_MAX44009
        max44009.measure();
        brightness100 = max44009.brightnessLux();
#elif defined SENSOR_TSL2561
        tsl2561.measure();
        brightness100 = tsl2561.brightnessLux();
#elif defined SENSOR_BH1750
        bh1750.measure();
        brightness100 = bh1750.brightnessLux();
#endif

#ifdef SENSOR_DIGINPUT
        digInputState = digitalInput.pinState();
#endif

#ifdef SENSOR_VEML6070
        veml6070.measure();
        // Beispiel custom payload, 4bit für UV-Index (integer 0..11)
        uint8_t uvi = veml6070.uvIndex();
        customData &= 0xFFF0;
        customData |= (uvi & 0x0F);
#endif

#ifdef SENSOR_VEML6075
        veml6075.measure();
        // Beispiel custom payload, 8bit für UV-Index * 10 (integer 0..110, 1 Kommastelle)
        uint8_t uvi10 = veml6075.uvIndex10();
        customData &= 0xFF00;
        customData |= uvi10;
#endif

        batteryVoltage = device().battery().current();    // BatteryTM class, mV resolution
    }

    void initSensors()
    {
#ifdef SENSOR_DS18X20
        ds18x20.init(ONEWIRE_PIN);
#endif
#ifdef SENSOR_BME280
        bme280.init();
#endif
#ifdef SENSOR_BMP180
        bmp180.init();
#endif
#ifdef SENSOR_MAX44009
        max44009.init();
#endif
#ifdef SENSOR_TSL2561
        tsl2561.init();
#endif
#ifdef SENSOR_BH1750
        bh1750.init();
#endif
#ifdef SENSOR_SHT31
        sht31.init();
#endif
#ifdef SENSOR_SHT21
        sht21.init();
#endif
#ifdef SENSOR_SHT10
#if defined SENSOR_BME280 || defined SENSOR_BMP180 || defined SENSOR_MAX44009 || defined SENSOR_TSL2561 || defined SENSOR_BH1750                     \
    || defined SENSOR_VEML6070 || defined SENSOR_VEML6075
        sht10.i2cEnableSharedAccess();    // falls I2C Sensoren vorhanden dies dem SHT10 mitteilen
#endif
        sht10.init();
#endif
#ifdef SENSOR_DIGINPUT
        digitalInput.init(DIGINPUT_PIN);
#endif
#ifdef SENSOR_VEML6070
        veml6070.init();
#endif
#ifdef SENSOR_VEML6075
        veml6075.init();
#endif
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

        uint16_t updCycle = this->getList0().updIntervall();
        DPRINT(F("updCycle: "));
        DDECLN(updCycle);

        uint16_t altitude = this->getList0().altitude();
        DPRINT(F("altitude: "));
        DDECLN(altitude);
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
#ifdef SENSOR_DIGINPUT
        if (digitalInput.notifyEvent()) {
            digitalInput.resetEvent();
            DPRINTLN(F("DIGINPUT change"));
            sdev.channel(1).forceSend();
        }
#endif
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
