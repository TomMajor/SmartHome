# HB-Dis-EP-42BW Platine     [![License: CC BY-NC-SA 3.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%203.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/3.0/)


# Work in Progress - in Bearbeitung...


# Hardware

### Bauteile

#### Reichelt

[Bestellliste](https://www.reichelt.de/my/1564026) (ohne die optionalen Bauteile)

Bauteil                  | Bestellnummer    | Anzahl | Kommentar
------------------------ | ---------------- | ------ | ---------
C1                       | X5R-G0603 10/6   |   1    | -
C2 .. C5, C7             | X7R-G0603 100N   |   5    | -
C6, C8                   | X7R-G0603 1,0/16 |   2    | -
D1                       | KBT L-115WEGW    |   1    | -
R1, R5                   | RND 0603 1 10K   |   2    | -
R2 .. R3                 | RND 0603 1 330   |   2    | -
R4                       | RND 0603 1 1,0M  |   1    | -
R6                       | RND 0603 1 4,7k  |   1    | optional: für Batteriespannungsmessung unter Last
R7                       | RND 155HP05 CI   |   1    | optional: für Batteriespannungsmessung unter Last
R8                       | RND 155HP05 AF   |   1    | optional: für Batteriespannungsmessung unter Last
Q1                       | IRLML 6344       |   1    | optional: für Batteriespannungsmessung unter Last
SW0                      | TASTER 3301      |   1    | optional: Konfigtaster *Achtung, diese Bauhöhe ist für die Montage kopfüber!*
SW1 .. SW10              | JTP-1130         |  10    | *Achtung, evtl. andere Bauhöhe!*
U1                       | ATMEGA 1284P-AU  |   1    | -
Y1                       | CSTCE 8,00       |   1    | -
Verbinder zum CC1101     | MPE 150-1-010    |   1    | -
Verbinder zum ePaper     | MPE 087-1-010    |   1    | -
Verbinder zur Batterie   | -                |   -    | optional, kann auch angelötet werden
Batteriefach             | HALTER 2XUM4-NLF |   -    | optional, für 2x AAA
Batteriefach             | HALTER 2XAAZ     |   -    | optional, für 2x AA



#### Farnell

Bauteil                  | Bestellnummer    | Anzahl | Kommentar
------------------------ | ---------------- | ------ | ---------
U3                       | [MCP111T-270E/TT](https://de.farnell.com/microchip/mcp111t-270e-tt/ic-supervisor-2-63v-low-sot-23b/dp/1851877)  |   1    | optional, Unterspannungserkennung


#### Sonstiges

Bauteil | Bezeichnung                  | Anzahl | Kommentar
------- | ---------------------------- | ------ | ---------
U2      | CC1101 Funkmodul 868 MHz     |   1    | z.B. [eBay](https://www.ebay.de/itm/272455136087)
Display | 4,2" ePaper mit SPI-Anschluss|   1    | z.B. [EXP-R63-045](https://www.exp-tech.de/new/8289/400x300-4.2-e-ink-display-module)

~8,3 cm Draht als Antenne


### Programmieradapter
- 1x ISP (z.B. [diesen hier](https://www.diamex.de/dxshop/USB-ISP-Programmer-fuer-Atmel-AVR-Rev2))


# Software

### Fuses und Lockbits

Fuse     | Wert | Kommentar
-------- | ---- | ---------
Extended | 0xFF |
High     | 0xD4 |
Low      | 0xFF |
Lockbits | 0xEF | optional


`C:\Program Files (x86)\Arduino\hardware\tools\avr\bin> .\avrdude -C ..\etc\avrdude.conf -p m1284p -P com7 -c stk500 -U lfuse:w:0xFF:m -U hfuse:w:0xD4:m -U efuse:w:0xFF:m`


### Firmware

![Einstellungen IDE](https://github.com/jp112sdl/HB-Dis-EP-42BW/blob/master/PCB/Images/ArduinoIDE_Auswahl_Controller.png)



# Bauanleitung

Zuerst den ATmega auflöten, die Markierung (kleiner Punkt) muss zur Beschriftung U1 zeigen.
Danach den Resonator, die Kondensatoren und Widerstände auflöten.

Mit einem Multimeter messen ob kein Kurzschluss zwischen VCC und GND besteht (mehrere 10 K Widerstand sind okay).

Den ISP Programmieradapter an den 6-poligen Anschluss auf der **Rückseite** anschließen.
Nun kann entweder der Sketch direkt geflasht werden ("Hochladen mit Programmer"), oder zuerst der [Bootloader](https://github.com/TomMajor/SmartHome/tree/master/Info/Bootloader#bootloader-atmega1284p-rc-oszillator-oder-quarz-8mhz) und dann der eigentliche Sketch über den FTDI-Anschluss.

Pin am ISP-Kabel | Bedeutung
---------------- | ----------
1                | MISO
2                | VCC
3                | SCK
4                | MOSI
5                | Reset
6                | GND


Anschließend können das Funkmodul, die Taster und die LED eingelötet werden.

An den Antennenanschluss muss noch das 8,3 cm Drahtstück angelötet werden.

Mit einem FTDI USB-seriell-Adapter und einem Terminalprogramm kann man die Ausgabe beobachten. Es sollten schon *Ignore...*-Meldungen empfangen und angezeigt werden.

# ToDo

#### Abstände ePaper zu Platine
#### Höhe der Taster
#### Konfigtaster
Orientierung wie die anderen 10 Taster, oder über Kopf.
#### Ausrichtung LED
#### Abstandsbolzen
#### Befestigung im Gehäuse
#### Batterien 2x AA statt 2x 2x AAA


# Bilder



[![Creative Commons Lizenzvertrag](https://i.creativecommons.org/l/by-nc-sa/3.0/88x31.png)](http://creativecommons.org/licenses/by-nc-sa/3.0/)

Dieses Werk ist lizenziert unter einer [Creative Commons Namensnennung - Nicht-kommerziell - Weitergabe unter gleichen Bedingungen 3.0 International Lizenz](http://creativecommons.org/licenses/by-nc-sa/3.0/).
