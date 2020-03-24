//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2020-03-23 Tom Major - Nightlight extension
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
// uncomment the following 2 lines if you have a TSL2561 connected at address 0x29
// der BH1750 geht auch
#include <Wire.h>
#include <sensors/Tsl2561.h>

#include <MultiChannelDevice.h>
#include <Motion.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8
// Arduino pin for the PIR
// A0 == PIN 14 on Pro Mini
#define PIR_PIN 14

// Nightlight extension
#define NIGHTLI_PIN 6
#define NIGHTLI_BRIGHTN_THRESHOLD 20
#define NIGHTLI_TIME_SEC 300

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x56,0x78,0x90},       // Device ID
    "papa222222",           // Device Serial
    {0x00,0x4a},            // Device Model
    0x16,                   // Firmware Version
    as::DeviceType::MotionDetector, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef StatusLed<LED_PIN> LedType;
typedef AskSin<LedType,BatterySensor,RadioType> BaseHal;
class Hal : public BaseHal {
public:
  void init (const HMID& id) {
    BaseHal::init(id);
    // set low voltage to 2.2V
    // measure battery every 1h
    battery.init(seconds2ticks(60UL*60),sysclock);
    battery.low(22);
    battery.critical(19);
  }
} hal;

#ifdef _TSL2561_H_
typedef MotionChannel<Hal,PEERS_PER_CHANNEL,List0,Tsl2561<TSL2561_ADDR_LOW> > MChannel;
#else
typedef MotionChannel<Hal,PEERS_PER_CHANNEL,List0> MChannel;
#endif

typedef MultiChannelDevice<Hal,MChannel,1> MotionType;
MotionType sdev(devinfo,0x20);

ConfigButton<MotionType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  motionISR(sdev,1,PIR_PIN);
  sdev.initDone();
}

class LightAlarm : public Alarm {
  public:
  LightAlarm() : Alarm(0) {
    pinMode(NIGHTLI_PIN,OUTPUT);
    digitalWrite(NIGHTLI_PIN,LOW);
  }
  virtual ~LightAlarm() {}

  void setOn () {
    digitalWrite(NIGHTLI_PIN,HIGH);
    set(seconds2ticks(NIGHTLI_TIME_SEC));
    sysclock.add(*this);
  }

  virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
    digitalWrite(NIGHTLI_PIN,LOW);
  }
} nightlight;

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  // falls Bewegung und Helligkeit <= Schwelle und Nachtlicht nicht bereits aktiv, dann Anschalten für definierte Zeit
  if( sdev.channel(1).isMotion() && sdev.channel(1).status() <= NIGHTLI_BRIGHTN_THRESHOLD && nightlight.active()==false ) {
    nightlight.setOn();
  }
  if( worked == false && poll == false ) {
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
