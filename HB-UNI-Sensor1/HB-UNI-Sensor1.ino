//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// HB-UNI-Sensor1
// 2018-03-30 Tom Major (Creative Commons)
//- -----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------
// !! NDEBUG should be defined when the sensor development and testing ist done and the device moves to serious operation mode
// With BME280 and TSL2561 activated, this saves 2k Flash and 560 Bytes RAM (especially the RAM savings are important for stability / dynamic memory allocation etc.)
//#define NDEBUG

//----------------------------------------------
// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define  EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <Register.h>
#include <MultiChannelDevice.h>

//----------------------------------------------
// SENSOR_ definitions for real sensors, if undefined dummy sensor values are used (for testing HM/RM BidCoS device communication without the real sensors)
//#define SENSOR_DS18X20
#define SENSOR_BME280
#define SENSOR_TSL2561

//----------------------------------------------
// Pin definitions
#define CONFIG_BUTTON_PIN   9
#define LED_PIN             6
#define ONEWIRE_PIN         3

// number of available peers per channel
#define PEERS_PER_CHANNEL   6

#ifdef SENSOR_DS18X20
  #include "Sensors/Sens_Ds18x20.h"
#endif

#ifdef SENSOR_BME280
  #include "Sensors/Sens_Bme280.h"
#endif

#ifdef SENSOR_TSL2561
  #include "Sensors/Sens_Tsl2561.h"
#endif

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
  {0x42,0x44,0xA3},       	 // Device ID
  "UNISENS001",           	 // Device Serial
  {0xF1, 0x03},            	 // Device Model
  0x10,                   	 // Firmware Version
  as::DeviceType::THSensor,  // Device Type
  {0x01, 0x01}             	 // Info Bytes
};

// Configure the used hardware
typedef AvrSPI<10, 11, 12, 13> SPIType;
typedef Radio<SPIType, 2> RadioType;
typedef StatusLed<LED_PIN> LedType;
typedef AskSin<LedType, BatterySensor, RadioType> BaseHal;

class Hal : public BaseHal {
  public:
    void init (const HMID& id) {
      BaseHal::init(id);
      // init real time clock - 1 tick per second
      //rtc.init();
      // measure battery every 12h
      battery.init(seconds2ticks(12UL * 60 * 60), sysclock);
      battery.low(22);      // Low voltage set to 2.2V
      battery.critical(19); // Critical voltage set to 1.9V
    }

    bool runready () {
      return sysclock.runready() || BaseHal::runready();
    }
} hal;

class WeatherEventMsg : public Message {
  public:
    void init(uint8_t msgcnt, int16_t temp, uint16_t airPressure, uint8_t humidity, uint32_t brightness, uint16_t battery, bool batlow) {

      uint8_t t1 = (temp >> 8) & 0x7f;
      uint8_t t2 = temp & 0xff;
      if ( batlow == true ) {
        t1 |= 0x80; // set bat low bit
      }
      Message::init(0x14, msgcnt, 0x70, BCAST, t1, t2);	// first byte determines message length; pload[0] starts at byte 13

              // 1 Byte payload -> length 0x0C
              // 6 Byte payload -> length 0x11
              // 9 Byte payload -> length 0x14
              // max. msg length 0x19 ?

							// BIDI|WKMEUP: erwartet ACK vom Empfänger, ohne ACK wird das Senden wiederholt
              //       LazyConfig funktioniert, d.h. eine anstehende Conf.Änderung von der CCU wird nach dem nächsten Senden übernommen
              //       Aber erhöhter Funkverkehr wegen ACK
							// BCAST: ohne ACK zu Erwarten, Standard für HM Sensoren
              //       LazyConfig funktioniert nicht, d.h. eine anstehende Conf.Änderung von der CCU muss durch den Config Button am Sensor übernommen werden!!
              
              // papa:
              // BIDI - fordert den Empfänger auf ein Ack zu schicken. Das wird auch zwingend für AES-Handling gebraucht.
              // BCAST - signalisiert eine Broadcast-Message. Das wird z.B. verwendet, wenn mehrere Peers vor einen Sensor existieren.
              //   Es wird dann an einen Peer gesndet und zusätzlich das BCAST-Flag gesetzt. So dass sich alle die Nachrricht ansehen.
              //   Ein Ack macht dann natürlich keinen Sinn - es ist ja nicht klar, wer das senden soll.
              // WKMEUP - wird für LazyConfig verwendet. Ist es in einer Message gesetzt, so weiss die Zentrale, dass das Geräte noch kurz auf weitere Nachrichten wartet.
              //   Die Lib setzt diese Flag für die StatusInfo-Message automatisch. Außerdem bleibt nach einer Kommunikation der Empfang grundsätzlich für 500ms angeschalten.

      // airPressure
      pload[0] = (airPressure >> 8) & 0x7f;
      pload[1] = airPressure & 0xff;
      
      // humidity
      pload[2] = humidity;
      
      // brightness (Lux)
      pload[3] = (brightness >> 24) & 0xff;
      pload[4] = (brightness >> 16) & 0xff;
      pload[5] = (brightness >>  8) & 0xff;
      pload[6] = (brightness >>  0) & 0xff;
      
      // battery
      pload[7] = (battery >> 8) & 0x7f;
      pload[8] = battery & 0xff;
    }
};

// die "freien" Register 0x20/21 werden hier als 16bit memory für das Update Intervall in Sek. benutzt
// siehe auch hb_uni_sensor1.xml, <parameter id="Update Intervall"> ..
DEFREGISTER(Reg0, MASTERID_REGS, DREG_TRANSMITTRYMAX, DREG_LOWBATLIMIT, 0x20, 0x21, 0x22, 0x23)
class SensorList0 : public RegList0<Reg0> {
public:
  SensorList0(uint16_t addr) : RegList0<Reg0>(addr) {}
  
  bool updIntervall (uint16_t value) const {
    return this->writeRegister(0x20, (value >> 8) & 0xff) && this->writeRegister(0x21, value & 0xff);
  }
  uint16_t updIntervall () const {
    return (this->readRegister(0x20, 0) << 8) + this->readRegister(0x21, 0);
  }
  
  bool height (uint16_t value) const {
    return this->writeRegister(0x22, (value >> 8) & 0xff) && this->writeRegister(0x23, value & 0xff);
  }
  uint16_t height () const {
    return (this->readRegister(0x22, 0) << 8) + this->readRegister(0x23, 0);
  }

  void defaults () {
    clear();
    transmitDevTryMax(6);
    lowBatLimit(22);
    updIntervall(300);
    height(0);
  }
};

class WeatherChannel : public Channel<Hal, List1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {

    WeatherEventMsg msg;

    int16_t       temperature;
    uint16_t      airPressure;
    uint8_t       humidity;
    uint32_t      brightness;
    uint16_t      battery;
    bool          sensorSetupDone;

    #ifdef SENSOR_DS18X20
      Sens_Ds18x20 ds18x20;
    #endif
    
    #ifdef SENSOR_BME280
      Sens_Bme280  bme280;
    #endif
    
    #ifdef SENSOR_TSL2561
      Sens_Tsl2561 tsl2561;
    #endif
    
  public:
    WeatherChannel () : Channel(), Alarm(seconds2ticks(60)), sensorSetupDone(false) {}
    virtual ~WeatherChannel () {}

    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      // delayed sensor setup
      if (!sensorSetupDone) {
        #ifdef SENSOR_DS18X20
	        ds18x20.init(ONEWIRE_PIN);
	      #endif
        #ifdef SENSOR_BME280
	        bme280.init();
	      #endif
        #ifdef SENSOR_TSL2561
          tsl2561.init();
        #endif
        sensorSetupDone = true;
        DPRINTLN("Sensor setup done");
      }
      uint8_t msgcnt = device().nextcount();
      measure();
      msg.init(msgcnt, temperature, airPressure, humidity, brightness, battery, device().battery().low());
      device().sendPeerEvent(msg, *this);
      // reactivate for next measure
      uint16_t updCycle = this->device().getList0().updIntervall();
      tick = seconds2ticks(updCycle);
      clock.add(*this);
    }

    // here we do the measurement
    void measure () {

      #ifdef SENSOR_DS18X20
        ds18x20.measure();
        temperature = ds18x20.temperature();
      #else
        temperature = 150 + random(50);   // 15C +x
      #endif
      
      #ifdef SENSOR_BME280
        uint16_t height = this->device().getList0().height();
        bme280.measure(height);
        temperature = bme280.temperature();
        airPressure = bme280.pressureNN();
        humidity    = bme280.humidity();
      #else
        airPressure = 1024 + random(9);   // 1024 hPa +x
        humidity    = 66 + random(7);     // 66% +x
      #endif

      #ifdef SENSOR_TSL2561
        tsl2561.measure();
        brightness = tsl2561.brightnessLux(); // also available: brightnessVis(), brightnessIR(), brightnessFull(), but these are dependent on integration time setting
      #else
        brightness  = 67000 + random(1000);   // 67000 Lux +x
      #endif
      
      // ToDo
      battery     = 2750;               // 2,75V
    }

    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr) {
      Channel::setup(dev, number, addr);
      tick = seconds2ticks(5);	        // first message in 5 sec.
      sysclock.add(*this);
    }
    
    void configChanged() {
      //DPRINTLN("Config changed: List1");
    }

    uint8_t status () const {
      return 0;
    }

    uint8_t flags () const {
      return 0;
    }
};

class SensChannelDevice : public MultiChannelDevice<Hal, WeatherChannel, 1, SensorList0> {
  public:
    typedef MultiChannelDevice<Hal, WeatherChannel, 1, SensorList0> TSDevice;
    SensChannelDevice(const DeviceInfo& info, uint16_t addr) : TSDevice(info, addr) {}
    virtual ~SensChannelDevice () {}

    virtual void configChanged () {
      TSDevice::configChanged();
      DPRINTLN("Config Changed: List0");
      
      uint8_t lowBatLimit = this->getList0().lowBatLimit();
      DPRINT("lowBatLimit: "); DDECLN(lowBatLimit);
      battery().low(lowBatLimit);
      
      uint8_t txDevTryMax = this->getList0().transmitDevTryMax();
      DPRINT("transmitDevTryMax: "); DDECLN(txDevTryMax);

      uint16_t updCycle = this->getList0().updIntervall();
      DPRINT("updCycle: "); DDECLN(updCycle);
      
      uint16_t height = this->getList0().height();
      DPRINT("height: "); DDECLN(height);
    }
};

SensChannelDevice sdev(devinfo, 0x20);
ConfigButton<SensChannelDevice> cfgBtn(sdev);

void setup () {
  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if ( worked == false && poll == false ) {
    // deep discharge protection
    // if we drop below critical battery level - switch off all and sleep forever
    if( hal.battery.critical() ) {
      // this call will never return
      hal.activity.sleepForever(hal);
    }
    // if nothing to do - go sleep
    hal.activity.savePower<Sleep<>>(hal);
  }
}

