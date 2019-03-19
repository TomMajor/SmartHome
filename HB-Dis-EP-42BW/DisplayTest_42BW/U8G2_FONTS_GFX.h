#ifndef _U8G2_FONTS_GFX_H_
#define _U8G2_FONTS_GFX_H_

#include <Adafruit_GFX.h>
#include <U8g2_for_Adafruit_GFX.h>

class U8G2_FONTS_GFX : public U8G2_FOR_ADAFRUIT_GFX
{
  public:
    U8G2_FONTS_GFX(Adafruit_GFX& gfx) : _gfx(gfx)
    {
      begin(gfx);
    };
    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
      _gfx.drawPixel(x, y, color);
    };
    size_t write(uint8_t v)
    {
      if (v == '\n') // Newline?
      {
        switch (u8g2.font_decode.dir)
        {
          case 0:
            tx = 0;
            ty += u8g2.font_info.max_char_height;
            break;
          // these need to be verified
          case 1:
            tx += u8g2.font_info.max_char_height;
            ty = 0;
            break;
          case 2:
            tx = 0;
            ty -= u8g2.font_info.max_char_height;
            break;
          case 3:
            tx -= u8g2.font_info.max_char_height;
            ty = 0;
            break;
        }
        return 1;
      }
      else
      {
        return U8G2_FOR_ADAFRUIT_GFX::write(v);
      }
    };
  private:
    Adafruit_GFX& _gfx;
};


#endif
