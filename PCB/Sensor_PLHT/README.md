
## PLHT Sensor Ver 2.01

Redesign von Dirks/PeMue's Platinen für Außen- oder Innenanwendungen, 10/2018

# Features

- Bestückung mit Arduino Pro Mini oder alternativ mit ATmega328P
- RC- oder Quarzoszillator möglich
- Sensoren für Temperatur (DS18x20, BME280), Luftdruck/Luftfeuchte (BME280), Helligkeit (MAX44009) bestückbar
- diese Sensoren können als SMD mit Reflow oder Heisßluft gelötet oder auf Breakout-Boards an den zwei I2C Stiftleisten angeschlossen werden
- andere I2C Sensoren über Breakout-Boards anschliessbar
- Spannungsversorgung: Batterien 2 Zellen / Batterie 1 Zelle mit MAX1724 / Netzteil

# Changelog Ver 2.01

- MAX44009 hinzugefügt
- BMP180 entfernt (Ersatz durch BME280)
- TSL2561 entfernt (Ersatz durch MAX44009) warum? [Vergleich TSL2561 MAX44009](https://github.com/TomMajor/AskSinPP_Examples/tree/master/Info/SensorTest_Lux)
- Zweite I2C Stiftleiste für zusätzliche Sensoren auf Breakout-Boards
- Schaltung für echte Batteriespannungsmessung unter Last hinzugefügt
- 10k pull-up Widerstand am ChipSelect des CC1101 für sicheres ISP-Flashen (Bootloader) mit verbautem CC1101
- Option: Miniatur-Quarz 8MHz bestückbar (anstatt int. RC-Osc.)
- alle Abstände der Leiterbahnen gegeneinander auf min. 0,25mm vergrößert
- Layout entflochten und verbessert
- Lötpads für BME280 und MAX44009 nach außen verlängert falls man versuchen will diese ohne Reflow oder Heisßluft zu löten

# Software

[HB-UNI-Sensor1](https://github.com/TomMajor/AskSinPP_Examples/tree/master/HB-UNI-Sensor1)

# Schaltung

[Sensor PLHT](https://github.com/TomMajor/AskSinPP_Examples/tree/master/PCB/Sensor_PLHT/Files/SensorPLHT_v201.pdf)

# Bilder

![pic](Images/Top.png)

![pic](Images/Bottom.png)

![pic](Images/HB-UNI-Sensor1_HW1.jpg)

# Spezielle Bauelemente

- Gehäuse mit Klarsichtdeckel: GAINTA G201C (ohne Befestigunslaschen), GAINTA G201CMF (mit Befestigunslaschen)

- Batteriehalter 2xAA: COMF BH-321-1A

- MOSFET für Option 'Echte Batteriespannungsmessung unter Last'
    * Meine Wahl: IRML6344 (Gate Threshold Voltage 0,8V, RDSon 27mOhm, 5A, SOT-23 Gehäuse, Pinout Gate 1, Source 2, Drain 3, N-Channel)
    * Es gehen natürlich auch günstigere bzw. leichter beschaffbare MOSFETs, wichtig ist das dieser bei minimaler Betriebsspannung des Sensors (nehmen wir mal 2V an) sicher durchschalten kann (Gate Threshold Voltage) und das der RDSon möglichst klein im Bezug zu den Widerständen des Spannungsteilers R2/R3 ist.
    Und natürlich muss er den gewählten Laststrom aushalten und das Pinout muss passen.
    * Der Si2302 zum Beispiel wäre auch gut geeignet, ist leichter beschaffbar und günstiger (aliexpress), Gate Threshold Voltage 0,7V, RDSon 85mOhm, 2,6A.

- Spule für optionalen Step-Up Wandler: Murata LQH43CN100K03L oder vergleichbare, unbedingt Strom und DC-Widerstand beachten, aus Produktrange 'Power lines', 'DC/DC' o.ä.

# Verbesserungen fürs Redesign

- das linke Bohrloch D2,5mm besser zu den vorhandenen Löchern im Batteriehalter ausrichten, zweites Bohrloch rechts vorsehen
- MAX44001 und BME280 Pin 1 Markierungen nachtragen
- SHT10 optional?
- Arduino Pro Mini Leiste JP2/U-Bat: Sollte da nicht ein Jumper dazwischen sein? Warum hat Dirk U-Bat fest auf 3,3V geklemmt wenn kein Step-Up verbaut ist?

