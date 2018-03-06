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

#define INTERVAL 180    // alle 3min senden

#define TEMP_SENSORS 4  // 4x DS18x20

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
  as::DeviceType::THSensor, 	 // Device Type
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
    void init(uint8_t msgcnt, int16_t temps[TEMP_SENSORS], uint16_t airPressure, uint8_t humidity, uint16_t battery, bool batlow) {

      uint8_t t1 = (temps[0] >> 8) & 0x7f;
      uint8_t t2 = temps[0] & 0xff;
      if ( batlow == true ) {
        t1 |= 0x80; // set bat low bit
      }
      Message::init(0x16, msgcnt, 0x70, BIDI, t1, t2); // first byte determines message length; pload[0] starts at byte 13

      int idx = 0;
      
      // temps
      for (int i = 1; i < TEMP_SENSORS; i++) {
        pload[idx++] = (temps[i] >> 8) & 0x7f;
        pload[idx++] = temps[i] & 0xff;
      }
      
      // airPressure
      pload[idx++] = (airPressure >> 8) & 0x7f;
      pload[idx++] = airPressure & 0xff;
      
      // humidity
      pload[idx++] = humidity;
      
      // battery
      pload[idx++] = (battery >> 8) & 0x7f;
      pload[idx++] = battery & 0xff;
    }
};

DEFREGISTER(WeatherRegsList0, DREG_BURSTRX)
typedef RegList0<WeatherRegsList0> WeatherList0;

class WeatherChannel : public Channel<Hal, List1, EmptyList, List4, PEERS_PER_CHANNEL, WeatherList0>, public Alarm {

    WeatherEventMsg msg;

    //Ds18b20<3>    ds18b20;
    int16_t       temperatures[TEMP_SENSORS];
    uint16_t      airPressure;
    uint8_t       humidity;
    uint16_t      battery;
    
  public:
    WeatherChannel () : Channel(), Alarm(seconds2ticks(INTERVAL)) {}
    virtual ~WeatherChannel () {}

    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      uint8_t msgcnt = device().nextcount();
      measure();
      msg.init(msgcnt, temperatures, airPressure, humidity, battery, device().battery().low());
      device().sendPeerEvent(msg, *this);
      // reactivate for next measure
      tick = seconds2ticks(INTERVAL);
      clock.add(*this);
    }

    // here we do the measurement
    void measure () {
      /*memset(temperatures, 0, sizeof(temperatures));
      for (int i = 0; i < TEMP_SENSORS; i++) {
        ds18b20.measure(i);
        temperatures[i] = ds18b20.temperature();
        DPRINT("measure(");DDEC(i);DPRINT(") = ");DDECLN(temperatures[i]);
      }*/
      // Dummy Werte zum Testen
      temperatures[0] = 50 + random(200);   // 5C +x
      temperatures[1] = 560;                // 56C
      temperatures[2] = 1055;               // 105,5C
      temperatures[3] = -18;                // -1,8C
      airPressure     = 1020;               // 1020 hPa
      humidity        = 66;                 // 66%
      battery         = 2750;               // 2,75V
    }

    void setup(Device<Hal, WeatherList0>* dev, uint8_t number, uint16_t addr) {
      Channel::setup(dev, number, addr);
      tick = seconds2ticks(10);	// first message in 10 sec.
      sysclock.add(*this);
      //ds18b20.init();
    }

    uint8_t status () const {
      return 0;
    }

    uint8_t flags () const {
      return 0;
    }
};

typedef MultiChannelDevice<Hal, WeatherChannel, 1, WeatherList0> WeatherType;
WeatherType sdev(devinfo, 0x20);
ConfigButton<WeatherType> cfgBtn(sdev);

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

