
### Bootloader/Fuses/Flashspeicher flashen mit avrdude

#### Fuses lesen
    avrdude -c stk500 -p m328p -P COM5 -b 115200 -U lfuse:r:lowfuse.hex:h -U hfuse:r:highfuse.hex:h -U efuse:r:extfuse.hex:h

#### Fuses schreiben
    avrdude -c stk500 -p m328p -P COM5 -b 115200 -U lfuse:w:0xE2:m -U hfuse:w:0xD2:m -U efuse:w:0xFF:m

#### Flashspeicher lesen
    avrdude -c stk500 -p m328p -P COM5 -b 115200 -U flash:r:readtest.hex:i

#### Bootloader/Flashspeicher schreiben
    avrdude -c stk500 -p m328p -P COM5 -b 115200 -e -U flash:w:C:\temp\ATmegaBOOT_168_atmega328_pro_8MHz.hex:i

![pic](Images/Flash_Bootloader.png)

<br>
#### AVR ISP 6-polig

![pic](Images/AVR_ISP.jpg)

<br>
#### Bootloader ATmega328p Quarz/RC-Oszillator
[Quarz-Osc 328p](https://github.com/TomMajor/SmartHome/tree/master/Info/Bootloader/mega328/Quarz_Osc)

#### Bootloader ATmega328p RC-Oszillator mit Kalibrierung
[RC-Osc Cal. 328p](https://github.com/TomMajor/SmartHome/tree/master/Info/Bootloader/mega328/RC_Osc_Cal)

#### Bootloader ATmega1284p
[Quarz_Osc 1284p](https://github.com/TomMajor/SmartHome/tree/master/Info/Bootloader/mega1284)

#### Atmel AVR Fuse Calculator

[Engbedded Atmel AVR® Fuse Calculator](http://www.engbedded.com/fusecalc/)
