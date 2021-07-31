//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2018-12-01 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
// ci-test=yes board=1284pSTD aes=no

// use Arduino IDE Board Setting: STANDARD Layout

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER
// #define USE_HW_SERIAL
// #define NDEBUG
// #define NDISPLAY
// #define USE_COLOR

#define BATTERY_MODE

#ifdef BATTERY_MODE
#define USE_WOR
#endif

//////////////////// DISPLAY DEFINITIONS /////////////////////////////////////
#include <GxEPD.h>
#ifdef USE_COLOR
#include <GxGDEW042Z15/GxGDEW042Z15.h>      // 4.2" color
#else
#include <GxGDEW042T2/GxGDEW042T2.h>        // 4.2" b/w
#endif

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include "U8G2_FONTS_GFX.h"

#define GxRST_PIN  14   // PD6
#define GxBUSY_PIN 11   // PD3
#define GxDC_PIN   12   // PD4
#define GxCS_PIN   18   // PC2

GxIO_Class io(SPI, GxCS_PIN, GxDC_PIN, GxRST_PIN);
GxEPD_Class display(io, GxRST_PIN, GxBUSY_PIN);

U8G2_FONTS_GFX u8g2Fonts(display);
//////////////////////////////////////////////////////////////////////////////

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>

#include <AskSinPP.h>
#include <LowPower.h>

#include <Register.h>
#include <Remote.h>
#include <MultiChannelDevice.h>

// make compatible with v5.0.0
#ifndef ASKSIN_PLUS_PLUS_VERSION_STR
  #define ASKSIN_PLUS_PLUS_VERSION_STR ASKSIN_PLUS_PLUS_VERSION
#endif

#define CC1101_CS_PIN       4   // PB4
#define CC1101_GDO0_PIN     2   // PB2
#define CC1101_SCK_PIN      7   // PB7
#define CC1101_MOSI_PIN     5   // PB5
#define CC1101_MISO_PIN     6   // PB6
#define CONFIG_BUTTON_PIN  15   // PD7
#define LED_PIN_1           0   // PB0
#define LED_PIN_2           1   // PB1
#define BTN1_PIN            3   // PB3
#define BTN2_PIN           A1   // PA1
#define BTN3_PIN           A2   // PA2
#define BTN4_PIN           A3   // PA3
#define BTN5_PIN           A4   // PA4
#define BTN6_PIN           A5   // PA5
#define BTN7_PIN           A6   // PA6
#define BTN8_PIN           A7   // PA7
#define BTN9_PIN           23   // PC7
#define BTN10_PIN          22   // PC6

#define TEXT_LENGTH        16
#define DISPLAY_LINES      10

#define DISPLAY_ROTATE     3 // 0 = 0° , 1 = 90°, 2 = 180°, 3 = 270°

#define PEERS_PER_CHANNEL   8
#define NUM_CHANNELS        11
#define DEF_LOWBAT_VOLTAGE  24
#define DEF_CRITBAT_VOLTAGE 22

#define MSG_START_KEY     0x02
#define MSG_TEXT_KEY      0x12
#define MSG_ICON_KEY      0x13
#define MSG_COLOR_OFFSET  0x04
#define MSG_CLR_LINE_KEY  0xFE
#define MSG_MIN_LENGTH    13
#define MSG_BUFFER_LENGTH 224
#define MSG_OUTOFHOUSE    0xFD

#include "Icons.h"

// all library classes are placed in the namespace 'as'
using namespace as;

const struct DeviceInfo PROGMEM devinfo = {
  {0xf3, 0x43, 0x00},          // Device ID
  "JPDISEP000",                // Device Serial
#ifdef BATTERY_MODE
  {0xf3, 0x43},                // Device Model
#else
  {0xf3, 0x53},                // Device Model
#endif
  0x11,                        // Firmware Version
  as::DeviceType::Remote,      // Device Type
  {0x01, 0x01}                 // Info Bytes
};

enum Alignments {AlignRight = 0, AlignCenterIconRight = 1, AlignCenterIconLeft = 3, AlignLeft = 2};

typedef struct {
  uint8_t Alignment = AlignRight;
  uint8_t Icon      = 0xff;
  bool IconColored  = false;
  String Text       = "";
  bool TextColored  = false;
  bool showLine     = false;
} DisplayLine;
DisplayLine DisplayLines[DISPLAY_LINES];

String List1Texts[DISPLAY_LINES * 2];

bool runSetup          = true;

/**
   Configure the used hardware
*/
typedef AvrSPI<CC1101_CS_PIN, CC1101_MOSI_PIN, CC1101_MISO_PIN, CC1101_SCK_PIN> SPIType;
typedef Radio<SPIType, CC1101_GDO0_PIN> RadioType;
typedef StatusLed<LED_PIN_1> LedType;

#ifdef BATTERY_MODE
typedef AskSin<LedType, BatterySensor, RadioType> BaseHal;
#else
typedef AskSin<LedType, NoBattery, RadioType> BaseHal;
#endif

class Hal: public BaseHal {
  public:
    void init(const HMID& id) {
      BaseHal::init(id);
#ifdef BATTERY_MODE
      battery.init(seconds2ticks(60UL * 60 * 21), sysclock); //battery measure once an day
      battery.low(DEF_LOWBAT_VOLTAGE);
      battery.critical(DEF_CRITBAT_VOLTAGE);
      activity.stayAwake(seconds2ticks(15));
#endif
    }

    bool runready () {
      return sysclock.runready() || BaseHal::runready();
    }
} hal;


void initDisplay();
void updateDisplay();
void emptyBatteryDisplay();
void outOfHouseDisplay();
class ePaperType : public Alarm {
  class ePaperWorkingLedType : public StatusLed<LED_PIN_2>  {
  private:
    bool enabled;
  public:
    ePaperWorkingLedType () : enabled(true) {}
    virtual ~ePaperWorkingLedType () {}
    void Enabled(bool e) {
      enabled = e;
    }
    bool Enabled() {
      return enabled;
    }
  } workingLed;
private:
  bool                 mUpdateDisplay;
  bool                 shInitDisplay;
  bool                 shOutOfHouseDisplay;
  bool                 inverted;
  bool                 waiting;
  uint16_t             clFG;
  uint16_t             clBG;
public:
  ePaperType () :  Alarm(0), mUpdateDisplay(false), shInitDisplay(false), shOutOfHouseDisplay(false), inverted(false), waiting(false), clFG(GxEPD_BLACK), clBG(GxEPD_WHITE)  {}
  virtual ~ePaperType () {}

  uint16_t ForegroundColor() {
    return clFG;
  }

  void ForegroundColor(uint16_t c) {
    clFG = c;
  }

  uint16_t BackgroundColor() {
    return clBG;
  }

  void BackgroundColor(uint16_t c) {
    clBG = c;
  }

  bool Inverted() {
    return inverted;
  }

  void Inverted(bool i) {
    inverted = i;
  }

  bool showInitDisplay() {
    return shInitDisplay;
  }

  void showInitDisplay(bool s) {
    shInitDisplay = s;
  }

  bool showOutOfHouseDisplay() {
    return shOutOfHouseDisplay;
  }

  void showOutOfHouseDisplay(bool s) {
    shOutOfHouseDisplay = s;
  }

  bool mustUpdateDisplay() {
    return mUpdateDisplay;
  }

  void mustUpdateDisplay(bool m) {
    if (m == true && workingLed.Enabled() == true) workingLed.set(LedStates::pairing);
    mUpdateDisplay = m;
  }

  void init() {
    u8g2Fonts.begin(display);
    display.setRotation(DISPLAY_ROTATE);
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    workingLed.init();
  }

  void setWorkingLedEnabled(bool en) {
    workingLed.Enabled(en);
  }

  void setDisplayColors() {
    u8g2Fonts.setForegroundColor(ForegroundColor());
    u8g2Fonts.setBackgroundColor(BackgroundColor());
  }

  void isWaiting(bool w) {
    waiting = w;
    DPRINT("wait:"); DDECLN(waiting);
  }

  bool isWaiting() {
    return waiting;
  }

  void setRefreshAlarm (uint32_t t) {
    isWaiting(true);
    sysclock.cancel(*this);
    Alarm::set(millis2ticks(t));
    sysclock.add(*this);
  }
  virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
    isWaiting(false);
    if (this->mustUpdateDisplay()) {
      this->mustUpdateDisplay(false);
  #ifndef NDISPLAY
      if (workingLed.Enabled() == true) {
        workingLed.set(LedStates::nothing);
        workingLed.ledOn();
      }
      setDisplayColors();
      if (this->showInitDisplay() == true) {
        this->showInitDisplay(false);
        display.drawPaged(initDisplay);
      } else if (this->showOutOfHouseDisplay() == true ) {
          display.drawPaged(outOfHouseDisplay);
          this->showOutOfHouseDisplay(false);
        } else {
          display.drawPaged(updateDisplay);
        }

      workingLed.ledOff();
  #else
      DPRINTLN("UPDATEDISPLAY!");
  #endif
    }
  }
} ePaper;

DEFREGISTER(Reg0, MASTERID_REGS, DREG_TRANSMITTRYMAX, DREG_LEDMODE, DREG_LOWBATLIMIT, 0x06, 0x07, 0x34, 0x35)
class DispList0 : public RegList0<Reg0> {
  public:
    DispList0(uint16_t addr) : RegList0<Reg0>(addr) {}

    bool displayInvertingHb(bool v) const { return this->writeRegister(0x06, 0x01,0,v); }
    bool displayInvertingHb() const { return this->readRegister(0x06, 0x01,0,false); }

    uint8_t displayRefreshWaitTime () const { return this->readRegister(0x07,0); }
    bool displayRefreshWaitTime (uint8_t value) const { return this->writeRegister(0x07,value); }

    uint8_t powerUpMode () const { return this->readRegister(0x34,0x03,0); }
    bool powerUpMode (uint8_t value) const { return this->writeRegister(0x34,0x03,0,value); }

    uint8_t powerUpKey () const { return this->readRegister(0x34,0x0f,2); }
    bool powerUpKey (uint8_t value) const { return this->writeRegister(0x34,0x0f,2,value); }

    uint8_t critBatLimit () const { return this->readRegister(0x35,0); }
    bool critBatLimit (uint8_t value) const { return this->writeRegister(0x35,value); }

    void defaults () {
      clear();
      displayInvertingHb(false);
      ledMode(1);
      transmitDevTryMax(2);
      displayRefreshWaitTime(50);
      powerUpMode(0);
      powerUpKey(0);
#ifdef BATTERY_MODE
      lowBatLimit(DEF_LOWBAT_VOLTAGE);
      critBatLimit(DEF_CRITBAT_VOLTAGE);
#endif
    }
};

DEFREGISTER(DispReg1)
class DispList1 : public RegList1<DispReg1> {
public:
  DispList1 (uint16_t addr) : RegList1<DispReg1>(addr) {}
  void defaults () {
    clear();
  }
};

DEFREGISTER(RemReg1, CREG_AES_ACTIVE, CREG_LONGPRESSTIME ,CREG_DOUBLEPRESSTIME, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x90, 0x91)
class RemList1 : public RegList1<RemReg1> {
  public:
    RemList1 (uint16_t addr) : RegList1<RemReg1>(addr) {}

    bool showLine (uint8_t value) const {
      return this->writeRegister(0x90, 0x01, 0, value & 0xff);
    }
    bool showLine () const {
      return this->readRegister(0x90, 0x01, 0, false);
    }

    bool Alignment (uint8_t value) const {
      return this->writeRegister(0x91, value & 0xff);
    }
    uint8_t Alignment () const {
      return this->readRegister(0x91, 0);
    }

    bool TEXT1 (uint8_t value[TEXT_LENGTH]) const {
      for (int i = 0; i < TEXT_LENGTH; i++) {
        this->writeRegister(0x36 + i, value[i] & 0xff);
      }
      return true;
    }
    String TEXT1 () const {
      String a = "";
      for (int i = 0; i < TEXT_LENGTH; i++) {
        byte b = this->readRegister(0x36 + i, 0x20);
        if (b == 0x00) b = 0x20;
        a += char(b);
      }
      return a;
    }

    bool TEXT2 (uint8_t value[TEXT_LENGTH]) const {
      for (int i = 0; i < TEXT_LENGTH; i++) {
        this->writeRegister(0x46 + i, value[i] & 0xff);
      }
      return true;
    }
    String TEXT2 () const {
      String a = "";
      for (int i = 0; i < TEXT_LENGTH; i++) {
        byte b = this->readRegister(0x46 + i, 0x20);
        if (b == 0x00) b = 0x20;
        a += char(b);
      }
      return a;
    }

    void defaults () {
      clear();
      //aesActive(false);
      longPressTime(1);
      doublePressTime(0);
      uint8_t initValues[TEXT_LENGTH];
      memset(initValues, 0x00, TEXT_LENGTH);
      TEXT1(initValues);
      TEXT2(initValues);
      showLine(false);
      Alignment(AlignRight);
    }
};

class RemChannel : public RemoteChannel<Hal,PEERS_PER_CHANNEL,DispList0, RemList1>  {
public:
  RemChannel () : RemoteChannel() {}
    virtual ~RemChannel () {}
    void configChanged() {
      uint16_t _longpressTime = 300 + (this->getList1().longPressTime() * 100);
      setLongPressTime(millis2ticks(_longpressTime));

      List1Texts[(number() - 1)  * 2] = this->getList1().TEXT1();
      List1Texts[((number() - 1) * 2) + 1] = this->getList1().TEXT2();

      bool somethingChanged = (
                                DisplayLines[(number() - 1)].showLine != this->getList1().showLine() ||
                                DisplayLines[(number() - 1)].Alignment != this->getList1().Alignment()
                              );

      DisplayLines[(number() - 1)].showLine = this->getList1().showLine();
      DisplayLines[(number() - 1)].Alignment = this->getList1().Alignment();
      DPRINT(number() < 10 ? "0":"");DDEC(number()); DPRINT(F(" - TEXT1 = ")); DPRINT(this->getList1().TEXT1());DPRINT(F(" - TEXT2 = ")); DPRINT(this->getList1().TEXT2());DPRINT(F(" - Line  = ")); DDEC(this->getList1().showLine());DPRINT(F(" - Align = ")); DDECLN(this->getList1().Alignment());

      if (!runSetup && somethingChanged) ePaper.mustUpdateDisplay(true);
    }

    uint8_t flags () const {
      return hal.battery.low() ? 0x80 : 0x00;
    }
};

class DispChannel : public RemoteChannel<Hal,PEERS_PER_CHANNEL,DispList0, DispList1>  {
private:
  uint8_t       msgBufferIdx;
  uint8_t       msgBuffer[MSG_BUFFER_LENGTH];
public:
  DispChannel () : RemoteChannel(), msgBufferIdx(0) {}
    virtual ~DispChannel () {}

    void configChanged() { }

    bool validLineCount(){
      uint8_t cnt = 0;
      for (int i = 0; i < msgBufferIdx; i++) {
        if (msgBuffer[i] == AS_ACTION_COMMAND_EOL) cnt++;
      }
      if (cnt != 10) {DPRINT("ERROR: EOL Count = ");DDECLN(cnt);}
      return cnt == DISPLAY_LINES;
    }

    uint8_t resetMessageBuffer() {
      //DPRINTLN("reset msgBuffer");
      msgBufferIdx = 0;
      memset(msgBuffer, 0, sizeof(msgBuffer));
      return msgBufferIdx;
    }

    bool process (const ActionCommandMsg& msg) {
      static bool getText = false;
      static uint8_t currentLine = 0;

      String Text = "";
      for (int i = 0; i < msg.len(); i++) {
        if (msg.value(i) == MSG_START_KEY) {
          currentLine = resetMessageBuffer();
        }

        if (msgBufferIdx < MSG_BUFFER_LENGTH) {
          msgBuffer[msgBufferIdx] = msg.value(i);
          msgBufferIdx++;
        } else {
          currentLine = resetMessageBuffer();
        }
      }

      if (
          msg.eot(AS_ACTION_COMMAND_EOT) &&
          msgBufferIdx >= MSG_MIN_LENGTH &&
          validLineCount() == true &&
          msgBuffer[0] == MSG_START_KEY
          ) {
        DPRINT("RECV: ");
        for (int i = 0; i < msgBufferIdx; i++) {
          DHEX(msgBuffer[i]); DPRINT(" ");

          if (msgBuffer[i] == AS_ACTION_COMMAND_EOL) {
            if (Text != "") DisplayLines[currentLine].Text = Text;
            //DPRINT("EOL DETECTED. currentLine = ");DDECLN(currentLine);
            currentLine++;
            Text = "";
            getText = false;
          }

          if (getText == true) {
            if ((msgBuffer[i] >= 0x20 && msgBuffer[i] < 0x80) || msgBuffer[i] == 0xb0 ) {
              char c = msgBuffer[i];
              Text += c;
            } else if (msgBuffer[i] >= 0x80 && msgBuffer[i] < 0x80 + (DISPLAY_LINES * 2)) {
              uint8_t textNum = msgBuffer[i] - 0x80;
              String fixText = List1Texts[textNum];
              fixText.trim();
              Text += fixText;
              //DPRINTLN(""); DPRINT("USE PRECONF TEXT NUMBER "); DDEC(textNum); DPRINT(" = "); DPRINTLN(List1Texts[textNum]);
            }
          }

          if (msgBuffer[i] == MSG_TEXT_KEY || msgBuffer[i] == (MSG_TEXT_KEY + MSG_COLOR_OFFSET) ) {
            getText = true;
            DisplayLines[currentLine].Icon = 0xff;  //clear icon
#ifdef USE_COLOR
            DisplayLines[currentLine].TextColored = (msgBuffer[i] == (MSG_TEXT_KEY + MSG_COLOR_OFFSET));
#endif
          }

          if (msgBuffer[i] ==  MSG_ICON_KEY || msgBuffer[i] == (MSG_ICON_KEY + MSG_COLOR_OFFSET) ) {
            getText = false;
            uint8_t iconPos = i + 1;
            if (iconPos < MSG_BUFFER_LENGTH) {
              if (msgBuffer[iconPos] >= 0x80) {
                uint8_t iconNumber = msgBuffer[iconPos] - 0x80;
                if (iconNumber < ICON_COUNT) {
                  DisplayLines[currentLine].Icon = iconNumber;
#ifdef USE_COLOR
                  DisplayLines[currentLine].IconColored = msgBuffer[i] == (MSG_ICON_KEY + MSG_COLOR_OFFSET);
#endif
                } else {
                  DPRINT(F("ICON Number out of range. ")); DDECLN(iconNumber);
                }
              }
            }
          }

          if (msgBuffer[i] == MSG_CLR_LINE_KEY) {
            getText = false;
            for (uint8_t i = 0; i < TEXT_LENGTH; i++)
              Text += F(" ");
            DisplayLines[currentLine].Icon = 0xff;
          }
          if (msgBuffer[i] == MSG_OUTOFHOUSE) {
            ePaper.showOutOfHouseDisplay(true);
          }
        }

        DPRINTLN("");

        for (int i = 0; i < DISPLAY_LINES; i++) {
         DPRINT("LINE "); DDEC(i + 1); DPRINT(" ICON_COLOR = "); DDEC(DisplayLines[i].IconColored); DPRINT(" ICON = "); DDEC(DisplayLines[i].Icon); DPRINT(" TEXT_COLOR = "); DPRINT(DisplayLines[i].TextColored); DPRINT(" TEXT = "); DPRINT(DisplayLines[i].Text); DPRINTLN("");
        }
        ePaper.mustUpdateDisplay(true);
      }
      return true;
    }

    bool process (const Message& msg) {
      return process(msg);
    }

    bool process (const RemoteEventMsg& msg) {
      return process(msg);
    }

    uint8_t flags () const {
      return hal.battery.low() ? 0x80 : 0x00;
    }
};

class DisplayDevice : public ChannelDevice<Hal, VirtBaseChannel<Hal, DispList0>, NUM_CHANNELS, DispList0> {
  public:
  VirtChannel<Hal, RemChannel,  DispList0> c[NUM_CHANNELS - 1];
  VirtChannel<Hal, DispChannel, DispList0> d;
  public:
    typedef ChannelDevice<Hal, VirtBaseChannel<Hal, DispList0>, NUM_CHANNELS, DispList0> DeviceType;
    DisplayDevice (const DeviceInfo& info, uint16_t addr) : DeviceType(info, addr) {
      for (uint8_t i = 0; i < NUM_CHANNELS - 1; i++) {
        DeviceType::registerChannel(c[i], i+1);
      }
      DeviceType::registerChannel(d, NUM_CHANNELS);
    }
    virtual ~DisplayDevice () {}

    RemChannel& remChannel (uint8_t num)  {
      return c[num - 1];
    }

    DispChannel& dispChannel ()  {
      return d;
    }

    bool process(Message& msg) {
      HMID devid;
      this->getDeviceID(devid);
      if (msg.to() == devid) {
        uint16_t rtime = this->getList0().displayRefreshWaitTime() * 100;
        ePaper.setRefreshAlarm(rtime);
      }
      return ChannelDevice::process(msg);
    }

    virtual void configChanged () {
      DPRINTLN(F("CONFIG LIST0 CHANGED"));

#ifdef BATTERY_MODE
      uint8_t lowbat = getList0().lowBatLimit();
      uint8_t critbat = getList0().critBatLimit();
      if( lowbat > 0 ) battery().low(lowbat);
      if( critbat > 0 ) battery().critical(critbat);
      DPRINT(F("lowBat          : ")); DDECLN(lowbat);
      DPRINT(F("critBat         : ")); DDECLN(critbat);
#endif

      uint8_t ledmode = this->getList0().ledMode();
      DPRINT(F("ledMode         : ")); DDECLN(ledmode);
      ePaper.setWorkingLedEnabled(ledmode);

      if (this->getList0().displayInvertingHb()) {
        ePaper.ForegroundColor(GxEPD_WHITE);
        ePaper.BackgroundColor(GxEPD_BLACK);
      } else {
        ePaper.ForegroundColor(GxEPD_BLACK);
        ePaper.BackgroundColor(GxEPD_WHITE);
      }
      bool invertChanged = (ePaper.Inverted() != this->getList0().displayInvertingHb());
      ePaper.Inverted(this->getList0().displayInvertingHb());
      DPRINT(F("displayInverting: ")); DDECLN(this->getList0().displayInvertingHb());

      DPRINT(F("RefreshWaitTime : ")); DDECLN(this->getList0().displayRefreshWaitTime());
      DPRINT(F("PowerUpMode     : ")); DDECLN(this->getList0().powerUpMode());
      DPRINT(F("PowerUpKey      : ")); DDECLN(this->getList0().powerUpKey());

      if (this->getList0().masterid().valid() == false || runSetup == true) {
        if (this->getList0().powerUpMode() == 0) {
          ePaper.showInitDisplay(true);
          ePaper.mustUpdateDisplay(true);
          ePaper.setRefreshAlarm(20);
        }
      }

      if (!runSetup && invertChanged) ePaper.mustUpdateDisplay(true);
    }
};
DisplayDevice sdev(devinfo, 0x20);
class ConfBtn : public ConfigButton<DisplayDevice>  {
public:
  ConfBtn (DisplayDevice& i) : ConfigButton(i)  {}
  virtual ~ConfBtn () {}

  virtual void state (uint8_t s) {
    if( s == ButtonType::longreleased ) {
      ePaper.mustUpdateDisplay(true);
      ePaper.setRefreshAlarm(20);
    }
    ConfigButton::state(s);
  }
};
ConfBtn cfgBtn(sdev);

void setup () {
  runSetup = true;
  for (int i = 0; i < DISPLAY_LINES; i++) {
    DisplayLines[i].Icon = 0xff;
    DisplayLines[i].Text = "";
  }

  initIcons();
#ifndef NDISPLAY
  display.init(57600);
#endif

  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  remoteChannelISR(sdev.remChannel(1),BTN1_PIN);
  remoteChannelISR(sdev.remChannel(2),BTN2_PIN);
  remoteChannelISR(sdev.remChannel(3),BTN3_PIN);
  remoteChannelISR(sdev.remChannel(4),BTN4_PIN);
  remoteChannelISR(sdev.remChannel(5),BTN5_PIN);
  remoteChannelISR(sdev.remChannel(6),BTN6_PIN);
  remoteChannelISR(sdev.remChannel(7),BTN7_PIN);
  remoteChannelISR(sdev.remChannel(8),BTN8_PIN);
  remoteChannelISR(sdev.remChannel(9),BTN9_PIN);
  remoteChannelISR(sdev.remChannel(10),BTN10_PIN);

  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  sdev.initDone();
  DDEVINFO(sdev);

#ifndef NDISPLAY
  ePaper.init();
#endif

  uint8_t powerupmode = sdev.getList0().powerUpMode();
  uint8_t powerupkey  = sdev.getList0().powerUpKey();
  if (powerupmode > 0) {
    sdev.remChannel(powerupkey + 1).state(powerupmode == 1 ? Button::released: Button::longreleased);
    sdev.remChannel(powerupkey + 1).state(Button::none);
  } else {
    sdev.dispChannel().changed(true);
  }

  runSetup = false;
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if ( worked == false && poll == false ) {
#ifdef BATTERY_MODE
    if (hal.battery.critical()) {
      display.drawPaged(emptyBatteryDisplay);
      hal.activity.sleepForever(hal);
    }
    if (ePaper.isWaiting()) {
      hal.activity.savePower<Idle<>>(hal);
    } else {
      hal.activity.savePower<Sleep<>>(hal);
    }
#else
    hal.activity.savePower<Idle<>>(hal);
#endif
  }
}

uint16_t centerPosition(const char * text) {
  return (display.width() / 2) - (u8g2Fonts.getUTF8Width(text) / 2);
}

void PrintTextWithPosition(String text, int y)
{
  while (text.length()) {
    int pos1 = text.lastIndexOf("@p");
    int pos2 = text.lastIndexOf("@f");
    int pos = max(pos1, pos2);
    if (pos >= 0) {
      String tPos = text.substring(pos+2, pos+4); // Position als String
      String tText = text.substring(pos+4);  // Ausgabetext
      text.remove(pos);
      if (tText.length()) {
        uint16_t iPos = tPos.toInt();  // Position als int
        //Serial.print(iPos); Serial.print(" ");
        //Serial.print(tText); Serial.println(" ");
        uint16_t left = iPos * display.width() / 100u;
        u8g2Fonts.setCursor(left, y);
#ifdef USE_COLOR
        if (pos2 > pos1) {
          u8g2Fonts.setForegroundColor(GxEPD_RED);
        } else {
          u8g2Fonts.setForegroundColor(ePaper.ForegroundColor());
        }
#endif
        u8g2Fonts.print(tText);
      }
    } else {
      break;
    }
  }
#ifdef USE_COLOR
  ePaper.setDisplayColors();    // restore default colors for possible next line in loop updateDisplay()
#endif
}

void updateDisplay() {
  display.fillScreen(ePaper.BackgroundColor());
  u8g2Fonts.setFont(u8g2_font_helvB18_tf);

  for (uint16_t i = 0; i < 10; i++) {
    if (DisplayLines[i].showLine && i < 10) display.drawLine(0, ((i + 1) * 40), display.width(), ((i + 1) * 40), ePaper.ForegroundColor());
    //DisplayLines[i].Text.trim();
    String viewText = DisplayLines[i].Text;
    viewText.replace("{", "ä");
    viewText.replace("|", "ö");
    viewText.replace("}", "ü");
    viewText.replace("[", "Ä");
    viewText.replace("#", "Ö");
    viewText.replace("$", "Ü");
    viewText.replace("~", "ß");
    viewText.replace("'", "=");

    uint8_t icon_number = DisplayLines[i].Icon;
    uint16_t icon_top = (24 - ( Icons[icon_number].height / 2)) + (i * 40) - 4;

    uint16_t leftTextPos = 0xffff;
    uint16_t fontWidth = u8g2Fonts.getUTF8Width(viewText.c_str());

    bool icon_colored = DisplayLines[i].IconColored;

    uint16_t y = i * 40 + 30;

    switch (DisplayLines[i].Alignment) {
      case AlignLeft:
        // TomMajor Erweiterung x-Textposition
        if (viewText.startsWith("@p") || viewText.startsWith("@f")) {
            PrintTextWithPosition(viewText, y);
            continue;
        }
        leftTextPos = 40;
        if (icon_number != 255) display.drawBitmap(Icons[icon_number].Icon, (( 24 - Icons[icon_number].width ) / 2) + 8, icon_top, Icons[icon_number].width, Icons[icon_number].height, icon_colored ? GxEPD_RED : ePaper.ForegroundColor(), icon_colored ? GxEPD::bm_normal | GxEPD::bm_transparent | GxEPD::bm_invert : GxEPD::bm_default);
        break;
      case AlignCenterIconRight:
        leftTextPos = (display.width() / 2) - (fontWidth / 2);
        if (icon_number != 255) {
          leftTextPos -= ((Icons[icon_number].width  / 2) + 4);
          display.drawBitmap(Icons[icon_number].Icon, leftTextPos + u8g2Fonts.getUTF8Width(viewText.c_str()) + 8 + (( 24 - Icons[icon_number].width ) / 2) , icon_top, Icons[icon_number].width, Icons[icon_number].height, icon_colored ? GxEPD_RED : ePaper.ForegroundColor(), icon_colored ? GxEPD::bm_normal | GxEPD::bm_transparent | GxEPD::bm_invert : GxEPD::bm_default);
        }
        break;
      case AlignCenterIconLeft:
        leftTextPos = (display.width() / 2) - (fontWidth / 2);
        if (icon_number != 255) {
          leftTextPos += ((Icons[icon_number].width  / 2) + 4);
          display.drawBitmap(Icons[icon_number].Icon, leftTextPos - Icons[icon_number].width - 8 , icon_top, Icons[icon_number].width, Icons[icon_number].height, icon_colored ? GxEPD_RED : ePaper.ForegroundColor(), icon_colored ? GxEPD::bm_normal | GxEPD::bm_transparent | GxEPD::bm_invert : GxEPD::bm_default);
        }
        break;
      case AlignRight:
      default:
        leftTextPos = display.width() - 40 - fontWidth;
        if (icon_number != 255) display.drawBitmap(Icons[icon_number].Icon, display.width() - 32 + (( 24 - Icons[icon_number].width ) / 2) , icon_top, Icons[icon_number].width, Icons[icon_number].height, icon_colored ? GxEPD_RED : ePaper.ForegroundColor(), icon_colored ? GxEPD::bm_normal | GxEPD::bm_transparent | GxEPD::bm_invert : GxEPD::bm_default);
        break;
    }

    u8g2Fonts.setCursor(leftTextPos, y);


#ifdef USE_COLOR
    if (DisplayLines[i].TextColored == true) u8g2Fonts.setForegroundColor(GxEPD_RED);
#endif

    u8g2Fonts.print(viewText);

#ifdef USE_COLOR
    ePaper.setDisplayColors();
#endif
  }
}

void initDisplay() {
  display.fillScreen(ePaper.BackgroundColor());

  uint8_t serial[11];
  sdev.getDeviceSerial(serial);
  serial[10] = 0;

  #define TOSTRING(x) #x
  #define TOSTR(x) TOSTRING(x)

  const char * title        PROGMEM = "HB-Dis-EP-42BW";
  const char * asksinpp     PROGMEM = "AskSin++";
  const char * version      PROGMEM = "V " ASKSIN_PLUS_PLUS_VERSION_STR;
  const char * compiledMsg  PROGMEM = "compiled on";
  const char * compiledDate PROGMEM = __DATE__ " " __TIME__;
#ifdef USE_COLOR
  const char * pages        PROGMEM = "GxGDEW042Z15_PAGES: " TOSTR(GxGDEW042Z15_PAGES);
#else
  const char * pages        PROGMEM = "GxGDEW042T2_PAGES: " TOSTR(GxGDEW042T2_PAGES);
#endif
  const char * ser                  = (char*)serial;
  const char * nomaster1    PROGMEM = "- keine Zentrale -";
  const char * nomaster2    PROGMEM = "- angelernt -";

  u8g2Fonts.setFont(u8g2_font_helvB24_tf);
  u8g2Fonts.setCursor(centerPosition(title), 95);
#ifdef USE_COLOR
  u8g2Fonts.setForegroundColor(GxEPD_RED);
#endif
  u8g2Fonts.print(title);

  u8g2Fonts.setCursor(centerPosition(asksinpp), 170);
#ifdef USE_COLOR
    u8g2Fonts.setForegroundColor(ePaper.ForegroundColor());
#endif
  u8g2Fonts.print(asksinpp);

  u8g2Fonts.setCursor(centerPosition(version), 210);
  u8g2Fonts.print(version);

  u8g2Fonts.setFont(u8g2_font_helvB12_tf);
  u8g2Fonts.setCursor(centerPosition(compiledMsg), 235);
  u8g2Fonts.print(compiledMsg);
  u8g2Fonts.setCursor(centerPosition(compiledDate), 255);
  u8g2Fonts.print(compiledDate);
#ifdef USE_COLOR
    ePaper.setDisplayColors();
#endif

  u8g2Fonts.setFont(u8g2_font_helvR10_tf);
  u8g2Fonts.setCursor(centerPosition(pages), 275);
  u8g2Fonts.print(pages);

  u8g2Fonts.setFont(u8g2_font_helvB18_tf);
  u8g2Fonts.setCursor(centerPosition(ser), 320);
  u8g2Fonts.print(ser);

  if (sdev.getList0().masterid().valid() == false) {
    u8g2Fonts.setFont(u8g2_font_helvB18_tf);
    u8g2Fonts.setCursor(centerPosition(nomaster1), 360);u8g2Fonts.print(nomaster1);
    u8g2Fonts.setCursor(centerPosition(nomaster2), 386);u8g2Fonts.print(nomaster2);
  }

#ifdef USE_COLOR
  display.drawRect(50, 138, 200, 145, GxEPD_RED);
#else
  display.drawRect(50, 138, 200, 145, ePaper.ForegroundColor());
#endif
}

void emptyBatteryDisplay() {
  display.fillScreen(ePaper.BackgroundColor());
#ifdef USE_COLOR
  uint16_t fg = GxEPD_RED;
#else
  uint16_t fg = ePaper.ForegroundColor();
#endif
  display.fillRect(120, 80, 60, 40, fg);

  uint8_t batt_x = 80;
  uint8_t batt_y = 120;
  uint8_t batt_w = 140;
  uint8_t batt_h = 230;
  uint8_t line_w = 4;

  for (uint8_t i = 0 ; i < line_w; i++) {
    display.drawRect(batt_x + i, batt_y + i, batt_w - i*2, batt_h - i*2, fg);
    display.drawLine(batt_x + i, batt_y + batt_h - 1, batt_x + batt_w - line_w + i, batt_y + 1,fg);
  }
}

void outOfHouseDisplay() {
  display.fillScreen(ePaper.BackgroundColor());
#ifdef USE_COLOR
 uint16_t fg = GxEPD_RED;
#else
 uint16_t fg = ePaper.ForegroundColor();
#endif


 uint8_t home_x = 50;
 uint8_t home_y = 150;
 uint8_t home_w = 200;
 uint8_t home_h = 230;
 uint8_t line_w = 4;

 for (uint8_t i = 0 ; i < line_w; i++) {
   display.drawRect(home_x + i, home_y + i, home_w - i*2, home_h - i*2, fg);
   display.drawLine(home_x + i, home_y + i, home_y + i , home_x + i, fg);
   display.drawLine(home_y +i , home_x + i , home_x + home_w - i, home_y - i,  fg);
 }
}
