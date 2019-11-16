//---------------------------------------------------------
// ePaper29_Test
// 2019-11-14 Tom Major Creative Commons
// 2019-04-03 jp112sdl Creative Commons
//---------------------------------------------------------

// MightyCore 1284P, STANDARD pinout, 8MHz

//---------------------------------------------------------
#include <GxEPD.h>
#include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include "U8G2_FONTS_GFX.h"

//---------------------------------------------------------
#define GxRST_PIN 14     // PD6
#define GxBUSY_PIN 11    // PD3
#define GxDC_PIN 12      // PD4
#define GxCS_PIN 18      // PC2

//#define DISPLAY_ROTATE 1    // 0 = 0° , 1 = 90°, 2 = 180°, 3 = 270°

GxIO_Class  io(SPI, GxCS_PIN, GxDC_PIN, GxRST_PIN);
GxEPD_Class display(io, GxRST_PIN, GxBUSY_PIN);

U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;
U8G2_FONTS_GFX        u8g2Fonts(display);

//---------------------------------------------------------
void setup()
{
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
    //    display.setRotation(DISPLAY_ROTATE);
    u8g2Fonts.setFontMode(1);
    //    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setFont(u8g2_font_helvB12_tf);

    display.drawRect(0, 0, display.width(), display.height(), GxEPD_BLACK);

    const char* t0 PROGMEM = "HB-RC-12-EP";
    const char* t1 PROGMEM = "Display Test";
    const char* t2 PROGMEM = "Tom Major";

    u8g2Fonts.setCursor(centerPosition(t0), display.height() / 2 - 24);
    u8g2Fonts.print(t0);

    u8g2Fonts.setCursor(centerPosition(t2), display.height() / 2 + 24);
    u8g2Fonts.print(t2);

    u8g2Fonts.setForegroundColor(GxEPD_RED);
    u8g2Fonts.setCursor(centerPosition(t1), display.height() / 2);
    u8g2Fonts.print(t1);
}

//---------------------------------------------------------
uint16_t centerPosition(const char* text)
{
    return (display.width() / 2) - (u8g2Fonts.getUTF8Width(text) / 2);
}
