
# AskSin Analyzer XS Board

- Ein Board für Christophs [AskSin Analyzer XS](https://homematic-forum.de/forum/viewtopic.php?f=76&t=56395), die Minimalversion des "großen" [AskSinAnalyzer](https://homematic-forum.de/forum/viewtopic.php?f=76&t=51161) - ohne ESP32 und ohne großes Display - zum Betrieb über den USB Port eines Computers.

- Außerdem lässt sich damit die [Idee](https://homematic-forum.de/forum/viewtopic.php?f=76&t=56881) von der "868 MHz Wünschelrute" zur Suche nach einem [Babbling Idiot](https://github.com/TomMajor/SmartHome/tree/master/Info/Babbling%20Idiot%20Protection) (BI) Dauersender realisieren.<br>
Dabei wird das Gerät von einer USB-Powerbank versorgt und man kann sich damit auf Wanderung im Haus begeben um durch die Anzeige der Signalstärke auf einem kleinen OLED-Display das fehlerhaft sendende Gerät zu identifizieren.


## Bilder

![pic](Images/AnalyzerXS_pcb1.png)

![pic](Images/AnalyzerXS_pcb2.png)

![pic](Images/AnalyzerXS_pcb3.png)

![pic](Images/BI-Wuenschelrute.jpg)

![pic](Images/oled-module.jpg)


## Schaltplan

[:arrow_right: AskSin Analyzer XS 1.01](https://github.com/TomMajor/SmartHome/tree/master/PCB/AskSin-Analyzer-XS/Files/Analyzer_XS_101.pdf)


## Platine

[:arrow_right: PCB Gerber](Gerber)

**WIP**


## Aufbau / Stückliste

| Anzahl    | Name      | Wert              | Gehäuse       | Bemerkungen |
|---|---|---|---|---|
| 5 | R1, R5-R8         | 10k               | 0805          | |
| 1 | IC4               | 4-fach I2C Levelshifter | 2x6     | [Beispiel bei ebay](https://www.ebay.de/itm/162352091615) |

**WIP**


## Links

[HomeMatic Forum: AskSin Analyzer XS - Der Analyzer als Desktop-App ohne ESP](https://homematic-forum.de/forum/viewtopic.php?f=76&t=56395)

[HomeMatic Forum: AskSinAnalyzer-Sniffer mit Display (BI-Wünschelrute)](https://homematic-forum.de/forum/viewtopic.php?f=76&t=56881)


## Lizenz

**Creative Commons BY-NC-SA**<br>
Give Credit, NonCommercial, ShareAlike

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
