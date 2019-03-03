
//---------------------------------------------------------
// HB-UNI-Sensor1
// Version 1.11
// 2019-03-03 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

//---------------------------------------------------------
// !! NDEBUG should be defined when the sensor development and testing ist done
// and the device moves to serious operation mode With BME280 and TSL2561
// activated, this saves 2k Flash and 560 Bytes RAM (especially the RAM savings
// are important for stability / dynamic memory allocation etc.) This will get
// rid of the Arduino warning "Low memory available, stability problems may
// occur."
//
//#define NDEBUG

//---------------------------------------------------------
// define this to read the device id, serial and device type from bootloader section
//
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <MultiChannelDevice.h>
#include <Register.h>
#include "Sensors/tmBattery.h"
#include "DeviceID.h"

//---------------------------------------------------------
// Über diese defines werden die real angeschlossenen Sensoren aktiviert.
// Andernfalls verwendet der Sketch Dummy-Werte als Messwerte (zum Testen der Anbindung an HomeMatic/RaspberryMatic/FHEM)
//
//#define SENSOR_DS18X20  // Achtung, ONEWIRE_PIN define weiter unten muss zur HW passen!
#define SENSOR_BME280    // Achtung, finitespace BME280 Library verwendet I2C Addr. 0x76, für 0x77 die Library anpassen!
//#define SENSOR_BMP180
//#define SENSOR_TSL2561  // Achtung, TSL2561_ADDR define weiter unten muss zur HW passen!
#define SENSOR_MAX44009    // Achtung, MAX44009_ADDR define weiter unten muss zur HW passen!
//#define SENSOR_SHT10  // Achtung, SHT10_DATAPIN/SHT10_CLKPIN define weiter unten muss zur HW passen!
//#define SENSOR_DIGINPUT   // Achtung, DIGINPUT_PIN define weiter unten muss zur HW passen!

//---------------------------------------------------------
// Über diese defines wird die Clock festgelegt
// CLOCK_SYSCLOCK: 8MHz Quarz an XTAL oder 8MHz int. RC-Oszillator, Sleep Strom ca. 4uA
// CLOCK_RTC:      8MHz int. RC-Oszillator, 32.768kHz Quarz an XTAL, Sleep Strom ca. 1uA
#define CLOCK_SYSCLOCK
//#define CLOCK_RTC

//---------------------------------------------------------
// Schwellwerte für Batteriespannungsmessung
#define BAT_VOLT_LOW 21         // 2.1V
#define BAT_VOLT_CRITICAL 19    // 1.9V

//---------------------------------------------------------
// Optionen für Batteriespannungsmessung, siehe README
//------------
// 1) Standard: tmBattery, UBatt = Betriebsspannung AVR
#define BAT_SENSOR tmBattery
//------------
// 2) für StepUp/StepDown: tmBatteryResDiv, sense pin A0, activation pin D9, Faktor = Rges/Rlow*1000, z.B. 470k/100k, Faktor 570k/100k*1000 = 5700
//#define BAT_SENSOR tmBatteryResDiv<A0, 9, 5700>
//------------
// 3) Echte Batteriespannungsmessung unter Last, siehe README und Thema "Babbling Idiot Protection"
// tmBatteryLoad: sense pin A0, activation pin D9, Faktor = Rges/Rlow*1000, z.B. 10/30 Ohm, Faktor 40/10*1000 = 4000, 200ms Belastung vor Messung
//#define BAT_SENSOR tmBatteryLoad<A0, 9, 4000, 200>

//---------------------------------------------------------
// Pin definitions
#define CONFIG_BUTTON_PIN 8
#define LED_PIN 4

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

#ifdef SENSOR_DS18X20
#include "Sensors/Sens_DS18X20.h"    // HB-UNI-Sensor1 custom sensor class
#define ONEWIRE_PIN 3
#endif

#ifdef SENSOR_BME280
#include "Sensors/Sens_BME280.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_BMP180
#include "Sensors/Sens_BMP180.h"    // HB-UNI-Sensor1 custom sensor class
#endif

#ifdef SENSOR_TSL2561
#include "Sensors/Sens_TSL2561.h"    // HB-UNI-Sensor1 custom sensor class
#define TSL2561_ADDR TSL2561_ADDR_FLOAT
#endif

#ifdef SENSOR_MAX44009
#include "Sensors/Sens_MAX44009.h"    // HB-UNI-Sensor1 custom sensor class
#define MAX44009_ADDR 0x4A
#endif

#ifdef SENSOR_SHT10
#include "Sensors/Sens_SHT10.h"    // HB-UNI-Sensor1 custom sensor class
#define SHT10_DATAPIN A4
#define SHT10_CLKPIN A5
#endif

#ifdef SENSOR_DIGINPUT
#include "Sensors/Sens_DIGINPUT.h"    // HB-UNI-Sensor1 custom sensor class
#define DIGINPUT_PIN A0
Sens_DIGINPUT digitalInput;    // muss wegen Verwendung in loop() global sein (Interrupt event)
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
// Bei mehreren Geräten des gleichen Typs muss Device ID und Device Serial unterschiedlich sein!
// Device ID und Device Serial werden aus der Datei "DeviceID.h" geholt um mehrere Geräte ohne Änderung des Sketches flashen zu können
const struct DeviceInfo PROGMEM devinfo = {
    cDEVICE_ID,        // Device ID
    cDEVICE_SERIAL,    // Device Serial
    { 0xF1, 0x03 },    // Device Model
    // Firmware Version
    // die CCU Addon xml Datei ist mit der Zeile <parameter index="9.0" size="1.0" cond_op="E" const_value="0x12" />
    // fest an diese Firmware Version gebunden! cond_op: E Equal, GE Greater or Equal
    // bei Änderungen von Payload, message layout, Datenpunkt-Typen usw. muss die Version an beiden Stellen hochgezogen werden!
    0x12,
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
              uint16_t batteryVoltage, bool batLow)
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
        Message::init(21, msgcnt, 0x70, flags, t1, t2);

        // Message Length (first byte param.): 11 + payload
        //  1 Byte payload -> length 12
        // 10 Byte payload -> length 21
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
    uint32_t brightness;
    uint8_t  digInputState;
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
#ifdef SENSOR_TSL2561
    Sens_TSL2561<TSL2561_ADDR> tsl2561;
#endif
#ifdef SENSOR_MAX44009
    Sens_MAX44009<MAX44009_ADDR> max44009;
#endif
#ifdef SENSOR_SHT10
    Sens_SHT10<SHT10_DATAPIN, SHT10_CLKPIN> sht10;
#endif

public:
    WeatherChannel()
        : Channel()
        , Alarm(seconds2ticks(60))
        , temperature10(0)
        , airPressure10(0)
        , humidity(0)
        , brightness(0)
        , digInputState(0)
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
        uint8_t msgcnt = device().nextcount();
        measure();
        msg.init(msgcnt, temperature10, airPressure10, humidity, brightness, digInputState, batteryVoltage, device().battery().low());
        device().sendPeerEvent(msg, *this);
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
#if !defined(SENSOR_DS18X20) && !defined(SENSOR_BME280) && !defined(SENSOR_BMP180) && !defined(SENSOR_SHT10)
        temperature10 = 188;    // 18.8C
#endif
#if !defined(SENSOR_BME280) && !defined(SENSOR_SHT10)
        humidity = 88;    // 88%
#endif
#if !defined(SENSOR_BME280) && !defined(SENSOR_BMP180)
        airPressure10 = 10880;    // 1088 hPa
#endif
#if !defined(SENSOR_TSL2561) && !defined(SENSOR_MAX44009)
        brightness = 88000;    // 88000 Lux
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

// Feuchte/Temp vom SHT10 falls kein BME280 vorhanden
#if defined(SENSOR_SHT10) && !defined(SENSOR_BME280)
        sht10.measure();
        temperature10 = sht10.temperature();
        humidity      = sht10.humidity();
#endif

// Entweder TSL2561 oder MAX44009 für Helligkeit, ggf. für anderen Bedarf anpassen
#ifdef SENSOR_TSL2561
        tsl2561.measure();
        brightness = tsl2561.brightnessLux();    // also available: brightnessVis(),
                                                 // brightnessIR(), brightnessFull(), but these
                                                 // are dependent on integration time setting
#elif defined SENSOR_MAX44009
        max44009.measure();
        brightness = max44009.brightnessLux();
#endif

#ifdef SENSOR_DIGINPUT
        digInputState = digitalInput.pinState();
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
#ifdef SENSOR_TSL2561
        tsl2561.init();
#endif
#ifdef SENSOR_MAX44009
        max44009.init();
#endif
#ifdef SENSOR_SHT10
#if defined SENSOR_BME280 || defined SENSOR_BMP180 || defined SENSOR_TSL2561 || defined SENSOR_MAX44009
        sht10.i2cEnableSharedAccess();    // falls I2C Sensoren vorhanden dies dem SHT10 mitteilen
#endif
        sht10.init();
#endif
#ifdef SENSOR_DIGINPUT
        digitalInput.init(DIGINPUT_PIN);
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
        // DPRINTLN("Config changed: List1");
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
        DPRINTLN("Config Changed: List0");

        uint8_t ledMode = this->getList0().ledMode();
        DPRINT("ledMode: ");
        DDECLN(ledMode);

        uint8_t lowBatLimit = this->getList0().lowBatLimit();
        DPRINT("lowBatLimit: ");
        DDECLN(lowBatLimit);
        battery().low(lowBatLimit);

        uint8_t txDevTryMax = this->getList0().transmitDevTryMax();
        DPRINT("transmitDevTryMax: ");
        DDECLN(txDevTryMax);

        uint16_t updCycle = this->getList0().updIntervall();
        DPRINT("updCycle: ");
        DDECLN(updCycle);

        uint16_t altitude = this->getList0().altitude();
        DPRINT("altitude: ");
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
