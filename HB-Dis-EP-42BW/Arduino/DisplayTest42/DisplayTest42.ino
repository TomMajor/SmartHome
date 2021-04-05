//---------------------------------------------------------
// DisplayTest42
// Version 1.02
// (C) 2019-2021 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSinPP 2016-10-31 papa Creative Commons
// HB-Dis-EP-42BW 2018-12-01 jp112sdl Creative Commons
//---------------------------------------------------------

// MightyCore 1284P, STANDARD pinout, 8MHz

//---------------------------------------------------------
// #define USE_COLOR     // default ist black/white, uncomment this line for 3-colors ePaper black/white/red

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include "Icons.h"

#include <GxEPD.h>
#ifdef USE_COLOR
#include <GxGDEW042Z15/GxGDEW042Z15.h>    // 4.2" b/w/r
#else
#include <GxGDEW042T2/GxGDEW042T2.h>    // 4.2" b/w
#endif

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include "U8G2_FONTS_GFX.h"

#define GxRST_PIN 14     // PD6
#define GxBUSY_PIN 11    // PD3
#define GxDC_PIN 12      // PD4
#define GxCS_PIN 18      // PC2

#define TEXT_LENGTH 16
#define DISPLAY_LINES 10
#define DISPLAY_ROTATE 1    // 0 = 0° , 1 = 90°, 2 = 180°, 3 = 270°

GxIO_Class  io(SPI, GxCS_PIN, GxDC_PIN, GxRST_PIN);
GxEPD_Class display(io, GxRST_PIN, GxBUSY_PIN);

U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;
U8G2_FONTS_GFX        u8g2Fonts(display);

struct {
    bool     Inverted = false;
    uint16_t clFG     = GxEPD_BLACK;
    uint16_t clBG     = GxEPD_WHITE;
} DisplayConfig;

//---------------------------------------------------------
void setup()
{
    initIcons();
    display.init(57600);
    display.drawPaged(initDisplay);
}

//---------------------------------------------------------
void loop()
{
    delay(100);
}

//---------------------------------------------------------
void initDisplay()
{
    u8g2_for_adafruit_gfx.begin(display);
    display.setRotation(DISPLAY_ROTATE);
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(DisplayConfig.clFG);
    u8g2Fonts.setBackgroundColor(DisplayConfig.clBG);
    display.fillScreen(DisplayConfig.clBG);
    u8g2Fonts.setFont(u8g2_font_helvB24_tf);

    const char* title PROGMEM    = "HB-Dis-EP-42BW";
    const char* asksinpp PROGMEM = "AskSinPP";
    const char* version PROGMEM  = "V" ASKSIN_PLUS_PLUS_VERSION_STR;
    const char* c1 PROGMEM       = "DisplayTest42 Tom Major";
    const char* c2 PROGMEM       = "Thanks papa for great AskSinPP";
    const char* c3 PROGMEM       = "Thanks Jerome for cool project";
    const char* c4 PROGMEM       = "Thanks Marco for PCB design";

    u8g2Fonts.setCursor(centerPosition(title), 95);
    u8g2Fonts.print(title);

#ifdef USE_COLOR
    u8g2Fonts.setForegroundColor(GxEPD_RED);
#endif
    u8g2Fonts.setCursor(centerPosition(asksinpp), 170);
    u8g2Fonts.print(asksinpp);
    u8g2Fonts.setCursor(centerPosition(version), 210);
    u8g2Fonts.print(version);
#ifdef USE_COLOR
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
#endif

    u8g2Fonts.setFont(u8g2_font_helvB12_tf);
    int x = centerPosition(c2);
    u8g2Fonts.setCursor(x, 270);
    u8g2Fonts.print(c1);
    u8g2Fonts.setCursor(x, 300);
    u8g2Fonts.print(c2);
    u8g2Fonts.setCursor(x, 330);
    u8g2Fonts.print(c3);
    u8g2Fonts.setCursor(x, 360);
    u8g2Fonts.print(c4);

    display.drawRect(60, 138, 175, 80, DisplayConfig.clFG);
}

//---------------------------------------------------------
uint16_t centerPosition(const char* text)
{
    return (display.width() / 2) - (u8g2Fonts.getUTF8Width(text) / 2);
}
