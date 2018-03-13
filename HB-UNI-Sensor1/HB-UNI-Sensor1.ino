//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Register.h>
#include <MultiChannelDevice.h>

//#include "Ds18b20.h"

// Arduino pin for the config button
#define CONFIG_BUTTON_PIN 9

// Arduino pin for the LED
#define LED_PIN 6

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
  {0x42,0x44,0xA3},       	 // Device ID
  "UNISENS001",           	 // Device Serial
  {0xF3, 0x01},            	 // Device Model
  0x10,                   	 // Firmware Version
  as::DeviceType::THSensor,  // Device Type
  {0x01, 0x01}             	 // Info Bytes
};

/**
   Configure the used hardware
*/
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
      // measure battery every 1h
      battery.init(seconds2ticks(60UL * 60), sysclock);
      battery.low(22); // Low voltage set to 2.2V
      battery.critical(19); // Critical voltage set to 1.9V
    }

    bool runready () {
      return sysclock.runready() || BaseHal::runready();
    }
} hal;

class WeatherEventMsg : public Message {
  public:
    void init(uint8_t msgcnt, int16_t temp, uint16_t airPressure, uint8_t humidity, uint8_t brightness, uint16_t battery, bool batlow) {

      uint8_t t1 = (temp >> 8) & 0x7f;
      uint8_t t2 = temp & 0xff;
      if ( batlow == true ) {
        t1 |= 0x80; // set bat low bit
      }
      Message::init(0x11, msgcnt, 0x70, BCAST, t1, t2);	// first byte determines message length; pload[0] starts at byte 13
							// BIDI: erwartet ACK vom Empfänger, ohne ACK wird das Senden wiederholt
              //       LazyConfig funktioniert, d.h. eine anstehende Conf.Änderung von der CCU wird nach dem nächsten Senden übernommen
              //       Aber erhöhter Funkverkehr wegen ACK
							// BCAST: ohne ACK zu Erwarten, Standard für HM Sensoren
              //       LazyConfig funktioniert nicht, d.h. eine anstehende Conf.Änderung von der CCU muss durch den Config Button am Sensor übernommen werden!!
              // 1 Byte payload -> length 0x0C
              // 6 Byte payload -> length 0x11
              // max. msg length 0x19 ?
      
      // airPressure
      pload[0] = (airPressure >> 8) & 0x7f;
      pload[1] = airPressure & 0xff;
      
      // humidity
      pload[2] = humidity;
      
      // brightness
      pload[3] = brightness;
      
      // battery
      pload[4] = (battery >> 8) & 0x7f;
      pload[5] = battery & 0xff;
    }
};

// die "freien" Register 0x20/21 werden hier als 16bit memory für das Update Intervall in Sek. benutzt
// siehe auch hb_uni_sensor1.xml, <parameter id="Update Intervall"> ..
DEFREGISTER(Reg0, MASTERID_REGS, DREG_TRANSMITTRYMAX, DREG_LOWBATLIMIT, 0x20, 0x21)
class SensorList0 : public RegList0<Reg0> {
public:
  SensorList0(uint16_t addr) : RegList0<Reg0>(addr) {}
  
  bool updIntervall (uint16_t value) const {
    return this->writeRegister(0x20, (value >> 8) & 0xff) && this->writeRegister(0x21, value & 0xff);
  }
  uint16_t updIntervall () const {
    return (this->readRegister(0x20, 0) << 8) + this->readRegister(0x21, 0);
  }

  void defaults () {
    clear();
    transmitDevTryMax(6);
    lowBatLimit(22);
    updIntervall(300);
  }
};

class WeatherChannel : public Channel<Hal, List1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {

    WeatherEventMsg msg;

    //Ds18b20<3>    ds18b20;
    int16_t       temperature;
    uint16_t      airPressure;
    uint8_t       humidity;
    uint8_t       brightness;
    uint16_t      battery;
    
  public:
    WeatherChannel () : Channel(), Alarm(seconds2ticks(60)) {}
    virtual ~WeatherChannel () {}

    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
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

      // Dummy Werte zum Testen
      temperature = 150 + random(50);   // 15C +x
      airPressure = 1024 + random(9);   // 1024 hPa +x
      humidity    = 66 + random(7);     // 66% +x
      brightness  = 100 + random(20);   // 100 +x
      battery     = 2750;               // 2,75V
    }

    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr) {
      Channel::setup(dev, number, addr);
      tick = seconds2ticks(10);	// first message in 10 sec.
      sysclock.add(*this);
      //ds18b20.init();
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

