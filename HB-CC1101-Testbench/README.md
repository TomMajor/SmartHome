
# HB-CC1101-Testbench

- Testbench für CC1101 Frequenztests und für weitere Anwendungen.

Features:<br>
- 3V Spannungsquelle für Testaufbauten (aus einer Zelle)
- inkl. Verpolschutz

Teil A (links)<br>
- Arduino Pro Mini Ruhestrom Tester
- Arduino Pro Mini ISP Programmierung
- I2C Connector für Breakout-Board Tests

[Sketch Ruhestromtest](https://github.com/TomMajor/SmartHome/tree/master/Info/Ruhestrom#%C3%BCberpr%C3%BCfung-des-avr-ruhestroms-power-down-mode)

Teil B (rechts)<br>
- CC1101 Frequenz Tester

[Sketch Frequenztest](https://github.com/pa-pa/AskSinPP/tree/master/examples/FreqTest)

Man braucht nicht unbedingt die 3V Spannungsquelle mit MAX1724 für Testaufbauten aus einer Zelle aufzubauen.<br>
Genauso gut kann man den Pro Mini über den FTDI Adapter versorgen oder am Power Jumper oben rechts eine andere Versorgung einspeisen.


## Bilder

![pic](Images/HB-CC1101-Testbench_1.jpg)

![pic](Images/HB-CC1101-Testbench_2.jpg)

![pic](Images/HB-CC1101-Testbench_3.jpg)

![pic](Images/HB-CC1101-Testbench_4.jpg)

![pic](Images/HB-CC1101-Testbench_5.png)


## Schaltplan

[:arrow_right: Schaltplan](PCB/Files/HB-CC1101-TestBench.pdf)


## Platine

[:arrow_right: PCB Gerber](PCB)

![pic](Images/HB-CC1101-Testbench_PCB.png)


## Aufbau / Stückliste

- Der CC1101 Halter war eine schnelle Idee von mir, den *Jan_von_neben_an* dankenswerterweise umgesetzt hat.<br>
Er funktioniert gut, ich bin aber sicher dass ein findiger Konstrukteur eine für den CC1101 Wechsel schnellere Lösung (Schnell-Klemmvorrichtung o.ä.) bauen könnte.<br>

- Momentan muss man den CC1101 auf die Pogo-Pins auflegen, dann den Halter auf den CC1011 und dann die 2 Schrauben ein paar wenige Umdrehungen eindrehen.<br>
[:arrow_right: Halter by Schmelzerboy](https://www.thingiverse.com/thing:4002846)

- Die Pogo-Pins **P75-E2** für die Testbench haben einen kleinen Kegel an der Spitze:<br>
![pic](Images/Pogo_pin_P75_E2.png)

[:arrow_right: P75-E2 bei aliexpress](https://de.aliexpress.com/item/32874022638.html?transAbTest=ae803_3)

- Wichtig bei anderen  Pogo-Pins wäre dass sie bei den 2mm Padabstand des CC1101 nicht aneinander stoßen, andererseits dürfen sie nicht zu dünn sein, damit sie nicht durch die Löcher in den CC1101 Pads durchrutschen.

- Aus eigener Erfahrung :crying_cat_face: würde ich beim Aufbau unbedingt mit den Pogo-Pins beginnen, sonst kann es später fummelig werden.
  - Dazu die Platine **auf eine wärmefeste Unterlage legen** (damit sich die erhitzen Pogo-Pins beim Einlöten nicht unten einbrennen) und mit den äußeren Pogo-Pins anfangen.
  - Einen CC1101 auflegen, leicht andrücken, die Pogo-Pins ausrichten und einlöten.
  - Dann mit den inneren Pins weitermachen, dazu beim Einlöten eines jeden Pins den CC1101 leicht Andrücken damit sich die Pins automatisch auf die Löcher in den CC1101 Pads ausrichten.
  - Fehler in der Ausrichtung der Pins am Besten sofort bei jedem Pin korrigieren, nicht erst am Ende wenn alle Pins eingelötet sind.

![pic](Images/HB-CC1101-Testbench_Aufbau.jpg)

#### Stückliste

| Anzahl	| Name	    | Wert	            | Gehäuse       | Bemerkungen |
|---|---|---|---|---|
|| **CC1101 Frequenz Tester** ||||
| 1 | K4   | Stiftleiste 1x2  | 2,54mm | Power/Strommessung Teil B |
| 1 | IC3  | Arduino Pro Mini | | 3,3V / 8MHz Version |
| 1 | R5   | 1,5k             | 805	| |
| 1 | R6   | 10k              | 805	| |
| 1 | C7   | 100n             | 805	| |
| 1 | C6   | 1µ               | 805	| |
| 1 | LED2 | KP2012SURC       | 805	| |
| 1 | ANT  | CC1101 Antenne   | | |
|| **Arduino Pro Mini Ruhestrom Tester / ISP / I2C** ||||
| 1 | K3   | Stiftleiste 1x2  | 2,54mm | Power/Strommessung Teil A |
| 1 | R1   | 1,5k             | 805	| |
| 2 | R3, R4 | 10k              | 805	| I2C pull-up |
| 1 | C5   | 100n             | 805	| |
| 1 | LED1 | KP2012SURC       | 805	| |
| 1 | SV1  | Stiftleiste 1x4  | 2,54mm | I2C |
|| **3V Spannungsquelle aus einer Zelle** ||||
| 1 | K1   | Stiftleiste 1x2  | 2,54mm | Power/Strommessung Alles |
| 2 | C1, C1   | 10µ          | 805	| |
| 2 | C3, C4   | 47µ          | bedrahtet, C4 optional | |
| 1 | L1   | 10µ              | | LQH43CN100K03L |
| 1 | IC2  | MAX1724EZK30     | | auch MAX1724EZK33 möglich |
| 1 | BAT1 | Batteriehalter AA | | KEYSTONE 1028 |
| 1 | F1   | T100mA           | TR5 (5,08mm) | optional, 3V Quelle |
| 1 | K5   | Stiftleiste 1x6  | 2,54mm | optional, 3V Quelle |
| 1 | R2   | 100k             | 0805 | optional, Verpolschutz |
| 1 | T1   | DMG3415U         | SOT23 | optional, Verpolschutz |


## Benötige Libraries

[AskSinPP Library](https://github.com/pa-pa/AskSinPP)</br>
[EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt)</br>
[Low-Power](https://github.com/rocketscream/Low-Power)


## Lizenz

**Creative Commons BY-NC-SA**<br>
Give Credit, NonCommercial, ShareAlike

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
