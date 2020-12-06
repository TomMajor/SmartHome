//---------------------------------------------------------
// Device_Example.h
// 2019-10-09 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

// clang-format off

#ifndef _DEVICE_CFG_H_
#define _DEVICE_CFG_H_


//---------------------------------------------------------
// Definition von Device ID und Device Serial
// Bei mehreren Geräten des gleichen Typs (HB-UNI-Sensor1) muss Device ID und Device Serial unterschiedlich sein!
#define cDEVICE_ID      { 0xA5, 0xA5, 0x11 }
#define cDEVICE_SERIAL  "UNISENS011"


//---------------------------------------------------------
// Aktivierung der verwendeten Sensoren
// Über diese defines werden die real angeschlossenen Sensoren aktiviert.
// Andernfalls verwendet der Sketch Dummy-Werte als Messwerte (zum Testen der Anbindung an HomeMatic/RaspberryMatic/FHEM)
//
#define SENSOR_DS18X20      // ONEWIRE_PIN define weiter unten muss zur HW passen! DS18X20_COUNT muss definiert sein!
#define SENSOR_BME280       // BME280 Library (finitespace) verwendet I2C Addr. 0x76, für 0x77 die Library anpassen!
//#define SENSOR_BMP180
//#define SENSOR_MAX44009   // MAX44009_ADDR define weiter unten muss zur HW passen!
//#define SENSOR_TSL2561    // TSL2561_ADDR define weiter unten muss zur HW passen!
//#define SENSOR_BH1750     // BH1750_ADDR define weiter unten muss zur HW passen!
//#define SENSOR_SHT31      // SHT31_ADDR define weiter unten muss zur HW passen!
//#define SENSOR_SHT21
//#define SENSOR_SHT10      // SHT10_DATAPIN / SHT10_CLKPIN define weiter unten muss zur HW passen!
//#define SENSOR_DIGINPUT   // DIGINPUT_PIN define weiter unten muss zur HW passen!
//#define SENSOR_VEML6070
//#define SENSOR_VEML6075


//---------------------------------------------------------
// Pin Definitionen Allgemein
#define CONFIG_BUTTON_PIN   8
#define LED_PIN             A3


//---------------------------------------------------------
// Pin und Address Definitionen Sensoren
#define ONEWIRE_PIN         A2
#define DS18X20_COUNT       1
//#define MAX44009_ADDR     0x4A
//#define TSL2561_ADDR      TSL2561_ADDR_FLOAT
//#define BH1750_ADDR       0x23    // 0x23 (ADDR connecting to Gnd) or 0x5C (ADDR connecting to Vcc)
//#define SHT31_ADDR        0x44    // by default its 0x44, you can also adjust the sensor for 0x45 and then pass that value in
//#define SHT10_DATAPIN     A4
//#define SHT10_CLKPIN      A5
//#define DIGINPUT_PIN      A1


//---------------------------------------------------------
// Über diese defines wird die Clock festgelegt
// CLOCK_SYSCLOCK: 8MHz Quarz an XTAL oder 8MHz int. RC-Oszillator, Sleep Strom ca. 4uA
// CLOCK_RTC:      8MHz int. RC-Oszillator, 32.768kHz Quarz an XTAL, Sleep Strom ca. 1uA
#define CLOCK_SYSCLOCK
//#define CLOCK_RTC


//---------------------------------------------------------
// Schaltungsvariante und Pins für Batteriespannungsmessung, siehe README
//------------
// 1) Standard: tmBattery, UBatt = Betriebsspannung AVR
//#define BAT_SENSOR tmBattery
//------------
// 2) für StepUp/StepDown: tmBatteryResDiv, sense pin A0, activation pin D9, Faktor = Rges/Rlow*1000, z.B. 470k/100k, Faktor 570k/100k*1000 = 5700
//#define BAT_SENSOR tmBatteryResDiv<A0, 9, 5700>
//------------
// 3) Echte Batteriespannungsmessung unter Last, siehe README und Thema "Babbling Idiot Protection"
// tmBatteryLoad: sense pin A1, activation pin D9, Faktor = Rges/Rlow*1000, z.B. 10/30 Ohm, Faktor 40/10*1000 = 4000, 200ms Belastung vor Messung
#define BAT_SENSOR tmBatteryLoad<A1, 9, 4000, 200>


//---------------------------------------------------------
// Schwellwerte für Batteriespannungsmessung
#define BAT_VOLT_LOW        24  // 2.4V
#define BAT_VOLT_CRITICAL   19  // 1.9V


#endif

// clang-format on
