//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-10-19 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Register.h>
#include <ThreeState.h>

// we use a Pro Mini
// Arduino pin for the LED
#define LED1_PIN 6
#define LED2_PIN 8 
// Arduino pin for the config button
#define CONFIG_BUTTON_PIN 9
// ADC sensor pin
#define SENS1_PIN 14    // A0

// number of available peers per channel
#define PEERS_PER_CHANNEL 8

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0xAA,0x10,0x00},       // Device ID
    "tomM3000d3",           // Device Serial
    {0x00,0xb2},            // Device Model
    0x13,                   // Firmware Version
    as::DeviceType::ThreeStateSensor, // Device Type
    {0x01,0x00}             // Info Bytes
};


// --------------------------------------------------------
// meine Wassermelder ADC extension

// --------------------------------------------------------
class ADCPosition : public Position {
  uint8_t m_SensePin;
public:
  ADCPosition () : m_SensePin(0) { _present = true; }

  void init (uint8_t adcpin) { 
    m_SensePin = adcpin;
    pinMode(m_SensePin, INPUT);
    digitalWrite(m_SensePin, LOW);  // kein pull-up
  }

  void measure (__attribute__((unused)) bool async=false) {
    uint8_t timeout;
    //ADMUX  &= ~(ADMUX_REFMASK | ADMUX_ADCMASK);
    ADCSRA = 1<<ADEN | 1<<ADPS2;        // enable ADC, prescaler 16 = 62.5kHz ADC clock @1MHz (range 50..1000 kHz)
    ADMUX  = 1<<REFS1 | 1<<REFS0;        // Internal 1.1V Voltage Reference with external capacitor at AREF pin
//todo AVCC als ref
    uint8_t channel = m_SensePin - 14;
    ADMUX  |= (channel & 0x0F);          // select channel

    _delay_ms(10);
    ADCSRA |= 1<<ADSC; timeout = 50;
    while (ADCSRA & (1<<ADSC)) { delayMicroseconds(10); timeout--; if (timeout==0) break; }
    ADCSRA |= 1<<ADSC; timeout = 50;
    while (ADCSRA & (1<<ADSC)) { delayMicroseconds(10); timeout--; if (timeout==0) break; }
//todo filter

    uint16_t value = ADC & 0x3FF;
    DDECLN(value);
    if (value > 250) { _position = State::PosC; } // WATER
    else             { _position = State::PosA; } // DRY
  }
  
  uint32_t interval () { return seconds2ticks(60); }  // alle 60sec messen
};

// --------------------------------------------------------
template <class HALTYPE,class List0Type,class List1Type,class List4Type,int PEERCOUNT>
class ADCThreeStateChannel : public ThreeStateGenericChannel<ADCPosition,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> {
public:
  typedef ThreeStateGenericChannel<ADCPosition,HALTYPE,List0Type,List1Type,List4Type,PEERCOUNT> BaseChannel;

  ADCThreeStateChannel () : BaseChannel() {};
  ~ADCThreeStateChannel () {}

  void init (uint8_t adcpin) {
    BaseChannel::init();
    BaseChannel::possens.init(adcpin);
  }
};

// --------------------------------------------------------


/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef DualStatusLed<LED2_PIN,LED1_PIN> LedType;
typedef AskSin<LedType,BatterySensor,RadioType> HalType;

DEFREGISTER(Reg0,DREG_CYCLICINFOMSG,MASTERID_REGS,DREG_TRANSMITTRYMAX)
class WDSList0 : public RegList0<Reg0> {
public:
  WDSList0(uint16_t addr) : RegList0<Reg0>(addr) {}
  void defaults () {
    clear();
    cycleInfoMsg(false);
    transmitDevTryMax(6);
  }
};

DEFREGISTER(Reg1,CREG_AES_ACTIVE,CREG_MSGFORPOS,CREG_EVENTFILTERTIME,CREG_TRANSMITTRYMAX)
class WDSList1 : public RegList1<Reg1> {
public:
  WDSList1 (uint16_t addr) : RegList1<Reg1>(addr) {}
  void defaults () {
    clear();
    msgForPosA(1); // DRY
    msgForPosB(3); // WET
    msgForPosC(2); // WATER
    aesActive(false);
    eventFilterTime(5);
    transmitTryMax(6);
  }
};

typedef ADCThreeStateChannel<HalType,WDSList0,WDSList1,DefList4,PEERS_PER_CHANNEL> ChannelType;
typedef ThreeStateDevice<HalType,ChannelType,1,WDSList0> DevType;

HalType hal;
DevType sdev(devinfo,0x20);
ConfigButton<DevType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  sdev.channel(1).init(SENS1_PIN);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  hal.battery.init(seconds2ticks(60UL*60), sysclock);	// measure battery every 1h
  hal.battery.low(22);
  hal.battery.critical(19);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    // deep discharge protection
    // if we drop below critical battery level - switch off all and sleep forever
    if( hal.battery.critical() ) {
      // this call will never return
      hal.activity.sleepForever(hal);
    }
    // if nothing to do - go sleep
    hal.activity.savePower<Sleep<> >(hal);
  }
}
