
# SmartHome

**Various SmartHome Projects, Devices, Information and Examples including AskSinPP usage**


## AddOn Kompatibilität

| |AddOn|Status|letzte getestete Firmware (RaspberryMatic)|
|---|---|---|---|
|:heavy_check_mark:|HB-UNI-Sensor1|lauffähig unter CCU / RaspberryMatic|3.45.7.20190511|
|:heavy_check_mark:|HB-SEN-LJet|lauffähig unter CCU / RaspberryMatic|3.45.7.20190511|
|:heavy_check_mark:|HB-DIS-EP-42BW|lauffähig unter CCU / RaspberryMatic|3.45.7.20190511|


## Disclaimer

|Bei meinen SmartHome-Geräten und AddOn handelt es sich um reine Hobby-Projekte.|
|---|
|**Ich kann keine Garantie für Fehlerfreiheit, (umfangreichen) Support, Realisierung von individuellen Wünschen oder zeitnahe Unterstützung für neue CCU-Firmwares geben!**|
|**Die Verwendung erfolgt auf eigene Gefahr (z.B. unerwünschte Nebeneffekte in der CCU-Firmware).**|

<br>


## Universalsensor HB-UNI-Sensor1

- Demonstriert einen HomeMatic/RaspberryMatic/FHEM Universalsensor für Temperatur, Luftdruck, Luftfeuchte, Helligkeit usw.
- modifizierbar für andere Sensoren
- konfigurierbares Sendeintervall über WebUI
- konfigurierbare Höhe (für Berechnung des Luftdrucks auf Meeresniveau/Normaldruck) über WebUI
- Der Status eines digitalen Eingangs kann mit übertragen werden.

<p align="center"><img src="Images/HB-UNI-Sensor1_HW3_small.jpg?raw=true"/></p>

-> [Projektseite HB-UNI-Sensor1](https://github.com/TomMajor/SmartHome/tree/master/HB-UNI-Sensor1)


## Wassermelder HB-SEC-WDS-2

- Wassermelder mit Leitfähigkeitsmessung zwischen den Elektroden
- Demonstriert die Verwendung vom 'ThreeStateSensor' device type aus der AskSinPP Bibliothek mit einer anpassbaren Messroutine, in diesem Fall die Leitfähigkeitsmessung mit dem integrierten ADC

<p align="center"><img src="Images/Prototyp_Wassermelder_small.jpg?raw=true"/></p>

-> [Projektseite HB-SEC-WDS-2](https://github.com/TomMajor/SmartHome/tree/master/HB-SEC-WDS-2)


## Füllstandsanzeige Öltank/Wassertank HB-SEN-LJet

- Kopplung der Projet LevelJET Ultraschall-Füllstandsanzeige mit HomeMatic für Rotex-Öltanks
- Anpassung an beliebige Tankformen durch Einsatz einer Peiltabelle möglich

<p align="center"><img src="Images/LevelJet_small.jpg?raw=true"/></p>

-> [Projektseite HB-SEN-LJet](https://github.com/TomMajor/SmartHome/tree/master/HB-SEN-LJet)


## Infos und AddOn zum 4,2" ePaper Display HB-DIS-EP-42BW

- Sketch für DisplayTest, AddOn, Fuses, Bootloader

<p align="center"><img src="Images/EP42_small.jpg?raw=true"/></p>

-> [Projektseite HB-DIS-EP-42BW](https://github.com/TomMajor/SmartHome/tree/master/HB-Dis-EP-42BW)


## 3-Kanal Kontakt-Interface für Öffner und Schließerkontake HB-SCI-3-FM

- Nachbau HM-SCI-3-FM mit AskSinPP Library

<p align="center"><img src="Images/HB_SCI_3_FM_small.jpg?raw=true"/></p>

-> [Projektseite HB-SCI-3-FM](https://github.com/TomMajor/SmartHome/tree/master/HB-SCI-3-FM)


## Schutz vor "Babbling Idiot" (BI)

- Betrachtungen und Lösungen zum Schutz gegen einen "Babbling Idiot" (Dauersender) im HomeMatic Netzwerk bei Selbstbaugeräten

<p align="center"><img src="Images/BI_small.jpg?raw=true"/></p>

-> [Projektseite BI Schutz](https://github.com/TomMajor/SmartHome/tree/master/Info/Babbling%20Idiot%20Protection)


## PLHT Sensor Version 2.01

- HB-UNI-Sensor1 für Außen- oder Innenanwendungen, 10/2018
- Redesign von Dirks/PeMue's Platine (FHEM)

<p align="center"><img src="Images/PLHT_small.png?raw=true"/></p>

-> [Projektseite PLHT Sensor 2.01](https://github.com/TomMajor/SmartHome/tree/master/PCB/01_Sensor_PLHT_V2.01)


## PLHT Sensor Version i2.20

- HB-UNI-Sensor1 für Außen- oder Innenanwendungen, 07/2019
- Weiterentwicklung vom PLHT Sensor Version 2.01 mit anderen Gehäusevarianten

<p align="center"><img src="Images/PLHT_small.png?raw=true"/></p>

-> [Projektseite PLHT Sensor i2.20](https://github.com/TomMajor/SmartHome/tree/master/PCB/02_Sensor_PLHT_Vi2.20)


## Ruhestrom

- Hinweise zur Verringerung des Ruhestromverbrauchs
- Entfernen des LDOs bei Sensor-Boards
- Sketch (SleepTest.ino) zur Überprüfung von Aktiv- und power-down Strom eines Arduino Pro Mini 328 (3.3V/8MHz) mit angeschlossenem CC1101 (das wäre ein Basis-AskSinPP Gerät ohne angeschlossene Sensoren oder andere Zusatz-HW)
- Sketch (SleepTestRTC.ino) zur Überprüfung des AVR power-save Mode mit 32,768kHz RTC, damit kann man weniger als 1µA Ruhestrom erreichen.

<p align="center"><img src="Images/SensorBreakoutBoards_small.jpg?raw=true"/></p>

-> [Projektseite Ruhestrom](https://github.com/TomMajor/SmartHome/tree/master/Info/Ruhestrom)


## Bootloader & Fuses

- diverse Informationen zu Bootloadern, Fuses, Flashen

<p align="center"><img src="Images/fuses_small.jpg?raw=true"/></p>

-> [Projektseite Bootloader & Fuses](https://github.com/TomMajor/SmartHome/tree/master/Info/Bootloader)


## HomeMatic Skripte und Lösungen

-> [Projektseite HomeMatic Skripte und Lösungen](https://github.com/TomMajor/SmartHome/tree/master/Info/Skripte)


## HomeMatic AddOn Entwicklung und Dokumentation

- Diverse Infos und Dokumentationen für die RaspberryMatic/CCU AddOn-Entwicklung

<p align="center"><img src="Images/AddOn_small.jpg?raw=true"/></p>

-> [Projektseite HomeMatic AddOn Entwicklung und Dokumentation](https://github.com/TomMajor/SmartHome/tree/master/Info/RaspberryMatic_CCU_AddOn)


## SensorTest_Lux

- Messungen und Betrachtungen zu den Helligkeitssensoren TSL2561 und MAX44009

<p align="center"><img src="Images/SensorTest_Lux_small.jpg?raw=true"/></p>

-> [Projektseite SensorTest_Lux](https://github.com/TomMajor/SmartHome/tree/master/Info/SensorTest_Lux)


## WDT_Frequenz

- Misst die Abweichung der Watchdog-Timer Frequenz bei einen ATmega328 in Bezug zur Quarzfrequenz, wichtig für das Aufwach-Intervall

<p align="center"><img src="Images/WDT_Frequenz_small.jpg?raw=true"/></p>

-> [Projektseite WDT_Frequenz](https://github.com/TomMajor/SmartHome/tree/master/Info/WDT_Frequenz)


## Lizenz

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons Lizenzvertrag" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />Dieses Werk ist lizenziert unter einer <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Namensnennung - Nicht-kommerziell - Weitergabe unter gleichen Bedingungen 4.0 International Lizenz</a>.

[comment]: # (:large_blue_circle:)
