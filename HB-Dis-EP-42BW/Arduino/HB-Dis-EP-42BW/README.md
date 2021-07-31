# HB-Dis-EP-42BW (jetzt auch mit Color-Display Unterstützung)

<img src="Images/IMG_9206.jpg" width=300 height=350></img><img src="Images/IMG_9965.jpg" width=300 height=350></img>

## Infos
Thread im Homematic-Forum: https://homematic-forum.de/forum/viewtopic.php?f=76&t=48153

## Parts
- ATMega **1284P** (für den [Prototypen](https://github.com/jp112sdl/HB-Dis-EP-42BW/blob/master/Images/IMG_9180.jpg) habe ich die [HM-ES-PWMSw1-Pl_GosundSP1-Platine](https://raw.githubusercontent.com/stan23/HM-ES-PMSw1-Pl_GosundSP1/master/Bilder/Platine_V2_bestückt.jpg) missbraucht ^^)
- 11 Taster (10 Tasterkanäle + 1 Anlerntaster)
- 2 Widerstände 330 Ohm
- 2 LEDs oder 1 Dual-Color-LED
- 4.2" ePaper Display
  - [mit SPI Interface](https://www.exp-tech.de/new/8289/400x300-4.2-e-ink-display-module), passt dann jedoch nicht in die [Frontabdeckung](Images/ePaper-Front.stl) - da ist dann eigene Kreativität gefragt
  - [mit RAW Interface](https://www.exp-tech.de/displays/e-paper-e-ink/8885/waveshare-4.2-e-ink-raw-display-400x300), setzt jedoch noch ein SPI-Treiberboard voraus
    - [Universal e-Paper Raw Panel Driver HAT](https://www.exp-tech.de/displays/e-paper-e-ink/8525/universal-e-paper-raw-panel-driver-hat) oder
    - [CrystalFontz ePaper Adapter Board](https://www.crystalfontz.com/product/cfa10084-epaper-adapter-board)
- 2x 2fach AAA Batteriehalter (werden parallel betrieben; passte am besten ins [Gehäuse](https://github.com/jp112sdl/HB-Dis-EP-42BW/blob/master/Images/IMG_9181.jpg))    

## Arduino
- [MCUDude/MightyCore Board](https://github.com/MCUdude/MightyCore)-Unterstützung für den 1284P
  - Pinout: Standard
  - Clock: 8MHz external
  - BOD: Disabled
  - Compiler LTO: Disabled
  - Variant: 1284P
- Bibliotheken:  
  - [AskSinPP](https://github.com/pa-pa/AskSinPP) (master-Branch verwenden!)
  - [Low-Power](https://github.com/rocketscream/Low-Power)
  - [EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt)
  - [GxEPD](https://github.com/ZinggJM/GxEPD) 
    - :arrow_right: um die Display-Aktualisierungsdauer zu verringern, lohnt es sich, in der `GxGDEW042T2.h` den Wert für [`GxGDEW042T2_PAGES`](https://github.com/ZinggJM/GxEPD/blob/master/src/GxGDEW042T2/GxGDEW042T2.h#L25) auf `3` zu reduzieren
  - [Adafruit-GFX](https://github.com/adafruit/Adafruit-GFX-Library)
  - [U8g2_for_Adafruit_GFX](https://github.com/olikraus/U8g2_for_Adafruit_GFX)

## Gehäuse
### Stand
https://www.thingiverse.com/thing:3331473

### Front
[Front](Images/ePaper-Front.stl)
