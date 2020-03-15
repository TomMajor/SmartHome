//---------------------------------------------------------
// ePaper29_DisplayTest
// 2020-03-14 Tom Major Creative Commons
// 2019-04-03 jp112sdl Creative Commons
//---------------------------------------------------------

// MightyCore 1284P, STANDARD pinout, 8MHz int. RC-Osc.

//---------------------------------------------------------
#include <GxEPD.h>
#include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include "U8G2_FONTS_GFX.h"

//---------------------------------------------------------
#define GxRST_PIN     22 // PC6
#define GxBUSY_PIN    23 // PC7
#define GxDC_PIN      21 // PC5
#define GxCS_PIN      20 // PC4

#define LED_PIN_1          19   // PC3
#define LED_PIN_2          10   // TomMajor PCB 1.00, LED2 on INT0

#define DISPLAY_ROTATE 0    // 0 = 0° , 1 = 90°, 2 = 180°, 3 = 270°

GxIO_Class  io(SPI, GxCS_PIN, GxDC_PIN, GxRST_PIN);
GxEPD_Class display(io, GxRST_PIN, GxBUSY_PIN);

U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;
U8G2_FONTS_GFX        u8g2Fonts(display);

//---------------------------------------------------------
void setup()
{
    pinMode(LED_PIN_1, OUTPUT);
    pinMode(LED_PIN_2, OUTPUT);
    digitalWrite(LED_PIN_1, LOW);
    digitalWrite(LED_PIN_2, LOW);

    display.init(57600);
    display.drawPaged(initDisplay);
}

//---------------------------------------------------------
void loop()
{
    digitalWrite(LED_PIN_1, HIGH);
    digitalWrite(LED_PIN_2, LOW);
    delay(500); // green

    digitalWrite(LED_PIN_1, LOW);
    digitalWrite(LED_PIN_2, HIGH);
    delay(500); // red

    digitalWrite(LED_PIN_1, HIGH);
    digitalWrite(LED_PIN_2, HIGH);
    delay(500); // orange

    digitalWrite(LED_PIN_1, LOW);
    digitalWrite(LED_PIN_2, LOW);
    delay(500); // off
}

//---------------------------------------------------------
void initDisplay()
{
    u8g2_for_adafruit_gfx.begin(display);
    display.setRotation(DISPLAY_ROTATE);
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
