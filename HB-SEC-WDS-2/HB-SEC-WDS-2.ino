//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-10-19 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// HB-SEC-WDS-2
// 2018-04-07 Tom Major (Creative Commons)
//- -----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------
// !! NDEBUG should be defined when the sensor development and testing ist done and the device moves to serious operation mode
// With BME280 and TSL2561 activated, this saves 2k Flash and 560 Bytes RAM (especially the RAM savings are important for stability / dynamic memory allocation etc.)
//#define NDEBUG

//----------------------------------------------
// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Register.h>
#include <ThreeState.h>

//----------------------------------------------
// Pin definitions
#define CONFIG_BUTTON_PIN  5
#define LED_PIN            6
#define SENS1_PIN          14    // ADC sensor pin: A0

// number of available peers per channel
#define PEERS_PER_CHANNEL 8

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x49,0x29,0xd3},       // Device ID
    "WATERCHK_1",           // Device Serial
    {0x00,0xb2},            // Device Model
    0x13,                   // Firmware Version
    as::DeviceType::ThreeStateSensor, // Device Type
    {0x01,0x00}             // Info Bytes
};


// --------------------------------------------------------
// meine Wassermelder ADC extension
//
// Sensor pin 1: 4.7k nach Masse
// Sensor pin 2: 4.7k zum ADC Eingang A0, 100k vom ADC Eingang A0 nach +3V
//
// Momentan sind mir nur der WASSER und TROCKEN Status wichtig, deswegen wird nur ein ADC Kanal benutzt
// Falls der FEUCHT Status ebenfalls ben�tigt wird, einfach einen weiteren ADC Kanal nehmen und State::PosB f�r den FEUCHT Fall senden
//
// ADC Werte mit 10mm Abstand zwischen den Sensorpins, Batteriespannung 3V:
// offen          1023
// gebr�ckt       88
// Wasser         550-650
// Mineralwasser  ca. 550
// Sekt           ca. 500

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
    uint8_t  samples, timeout;
    uint16_t adc;
    
    // complete ADC init in case other modules have chamged this
    ADCSRA = 1<<ADEN | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0;	// enable ADC, prescaler 128 = 62.5kHz ADC clock @8MHz (range 50..1000 kHz)
    ADMUX  = 1<<REFS0;					// AVCC with external capacitor at AREF pin
    ADCSRB = 0;
    uint8_t channel = m_SensePin - PIN_A0;
    ADMUX  |= (channel & 0x0F);          		// select channel
    delay(25);						                  // load CVref 100nF, 5*Tau = 25ms

    // 1x dummy read
    ADCSRA |= 1<<ADSC; timeout = 50;
    while (ADCSRA & (1<<ADSC)) { delayMicroseconds(10); timeout--; if (timeout==0) break; }

    // Mittelwert aus 4 samples
    adc = 0; samples = 0;
    for (uint8_t i = 0; i < 4; i++) {
      ADCSRA |= 1<<ADSC; timeout = 50;			// start ADC
      while (ADCSRA & (1<<ADSC)) { 
        delayMicroseconds(10);
        timeout--;
        if (timeout == 0)
          break; 
      }
      if (timeout > 0) {
        adc += (ADC & 0x3FF);
        samples++;
      }
    }
    if (samples == 4) {
      adc = adc >> 2;
      DPRINT("ADC: "); DDECLN(adc); 
      if (adc < 800) { _position = State::PosC; } 	// WATER
      else           { _position = State::PosA; } 	// DRY
    }
    else {
      DPRINTLN("ADC Error");
    }
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
typedef StatusLed<LED_PIN> LedType;
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
  hal.battery.init(seconds2ticks(60UL*60*24), sysclock);	// at least one message per day
  hal.battery.low(10);                 // mit Step-up MAX1724, NiMH Akku, Batt.warnung ab 1,0V
  hal.battery.critical(9);
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
