# Schutz vor "Babbling Idiot" (BI)

Auf dieser Seite sollen Lösungen zum Schutz gegen einen "Babbling Idiot" im Homematic Netzwerk bei Selbstbaugeräten vorgestellt werden.<br>
Ein "Babbling Idiot", im folgenden mit **BI** abgekürzt, ist hier ein Dauersender, der das Funknetzwerk lahmgelegt, da die anderen Teilnehmer durch den Störsender nicht mehr miteinander kommunizieren können.

Beim Homematic Funknetzwerk kann ein BI wegen der 1% Regel sehr schnell das ganze System über Stunden lahmlegen.

#### Problem

Der Auslöser der Überlegungen hier war der Post von jp112sdl im FHEM Forum über die "Stilllegung" seines Homematic Netzwerkes im Haus:

> *Hat schon mal jemand das Phänomen beobachtet, dass ein Arduino/CC1101 auf Dauersendung war?<br>
> Ist mir nun schon ein zweites Mal passiert...  :-[<br>
> In der ganzen Bude ging nix (Funkgesteuertes) mehr.<br>
Hab zum Glück nen SDR und konnte den Störer schnell ausfindig machen.<br>
Ein nachgebauter WDS 10 TH O.<br>
Die CCU sagte mir "Kommunikationsstörung" zu so ziemlich allen meinen Geräten.*

[[Quelle: FHEM Forum, Antw: AskSin++ Library]](https://forum.fhem.de/index.php/topic,57486.msg783197.html#msg783197)


#### Eine weitere Meldungen zum Thema:

> *Mich hat jemand angeschrieben, der das 8fach-DS18B20 Projekt nachgebaut hat und - oh Wunder - bei leerer Batterie einen BI hatte! :<br>
Du sag mal, wenn der Temperatursensor leer wird, also die Batterien zu schwach sind, warum geht dann hier keine Funkfernbedienung mehr?*

[Quelle: Projekt 8fach-DS18B20, Info über Private Nachricht erhalten]

#### Und noch ein passender Post

> *Re: schlagartig Kommunikationsstörungen<br>
Schließlich habe ich die Ursache gefunden: Ein einziger Sensor HM-SCI-3-FM hat den Funkverkehr weitgehend lahmgelegt. Ob nur die Batterie entladen ist oder der Sensor defekt muss ich noch untersuchen.<br>
Auf die Spur hat mich CCU-Historian gebracht, der bei mir auch alle (Alchy-)Systemnachrichten mitschreibt. Der betroffenen Sensor ist in den letzten Wochen auffällig oft genannt worden, so habe ich ihn kurzerhand durch Batterieentzug stillgelegt. Jetzt ist wieder Ruhe ... bis zum nächsten mal.*

[[Quelle: homematic-forum, schlagartig Kommunikationsstörungen]](https://homematic-forum.de/forum/viewtopic.php?f=65&t=43150)

#### BI bei Wikipedia

> Ein Babbling Idiot ist ein Knoten eines Netzwerks, der die Kommunikation durch Fehlverhalten stört. Dies kann durch das Senden von Nachrichten zu willkürlichen, nicht mit dem Netzwerk synchronisierten Zeitpunkten oder durch fehlerhaften Inhalt von Nachrichten hervorgerufen werden.<br>
Er kann somit die Knoten stören, die ihre Nachrichten synchron mit dem Netzwerk übertragen. Als Folge kann es zu Fehlkommunikation bis hin zu einem vollständigen Kommunikationsverlust im Netzwerk kommen. Um dem vorzubeugen, kann beispielsweise jeder Netzwerkteilnehmer einen sogenannten Buswächter (Bus Guardian) besitzen. Dieser stellt sicher, dass der Netzwerkteilnehmer nur innerhalb seines statisch zugewiesenen Zeitslots sendet.
<br>

[[Quelle: Wikipedia - Babbling Idiot]](https://de.wikipedia.org/wiki/Babbling_idiot)</br>


#### Massnahmen

jp112sdl und ich haben uns dann in letzter Zeit etwas intensiver zu dem Thema ausgetauscht und Lösungen entwickelt.

Ich stelle hier 3 Massnahmen vor, um sich vor diesem Effekt zu schützen, anhand meines Beispiels für den Wassrmelder<br>
[HB-SEC-WDS-2](https://github.com/TomMajor/AskSinPP_Examples/tree/master/HB-SEC-WDS-2)

Der BI könnte entstehen, wenn beim Senden die Batteriespannung so weit zusammenbricht, das ein normaler Programmablauf nicht mehr gegeben ist, dadurch der angefangene Sendevorgang nicht abgeschlossen werden kann, die Spannung aber noch für den Betrieb der Sendeendstufe im CC1101 reicht. <br>Dieser Zustand ist kritisch und soll mit den hier gezeigten Massnahmen vermieden werden.

Ich möchte ausserdem anmerken dass die Störung des Programmablaufes beim Senden nicht nur durch den AVR entstehen kann sondern wahrscheinlich auch durch den CC1101 selbst.<br>
Dieser hat ebenso einen Mikrokontroller mit Firmware integriert, die beim Senden aufgrund der Spannung genau so wie der AVR abstürzen könnte.<br>
Ob ein BI also im AVR oder CC1101 entsteht kann nur durch Messung im Falle des Falles am Gerät bewiesen werden, wegen der Reproduzierbarkeit ein schwieriges Unterfangen.

### Schaltung A

Aus meiner Sicht würde es sehr helfen, eine echte Batteriezustandsmessung unter Last zu haben, um frühzeit leere Batterien zu erkennen und zu tauschen. Der Schaltungsteil belastet die Batterie bzw. den Akku für einige Hundert Millisekunden und misst dabei die Spannung.
Dies führt meiner Meinung nach zu realistischeren Werten über den Batteriezustand als eine asynchrone und unbelastete Messung, als z.B. die Standard-Initialisierung
`hal.battery.init(seconds2ticks(3600)`<br>
Im Schaltungsbeispiel wird der 1,2V Akku mit ca. 75mA für die kurze Zeit der Messung belastet.

### Schaltung B

Am AVR wird ein Reset-Baustein zur Spannungsüberwachung (BU48xx oder MCP111) angeschlossen. Fällt die Spannung unter den Schwellwert wird das RESET Signal für den AVR aktiv. Falls der AVR der Verursacher des BI ist wird er damit ruhiggestellt.<br>
Alternativ zum externen Baustein kann man auch die interne Brown-Out Detection (BOD) des AVR mit der 2,7V Schwelle verwenden. Allerdings hebt diese den Strom im Sleep von ca. 4uA auf 18uA (@3V) an.
Die angesprochenen externen Reset-Bausteine haben eine Stromaufnahme um 1uA, deswegen verwende ich die wesentlich lieber.

### Schaltung C

Diese Massnahme setzt Schaltung B voraus. Fällt die Spannung unter den Schwellwert wird das RESET Signal für den AVR aktiv. Dadurch werden hardwaremässig beim AVR alle I/O auf Input Mode gesetzt. Der Ausgang D4 kann dadurch keinesfalls mehr den Mosfet T2 schalten, als Resultat wird der CC1101 von der Spannung abgeklemmt und ruhiggestellt.<br>
Diese Massnahme verwendet man am Besten wenn sicher ist dass der CC1101 und nicht der AVR bei Einbruch der Batteriespannung zum BI wird bzw. wenn man alle Fälle abdecken möchte.

### Fazit

Im momentanen Stadium der Protoypen verwende ich die Massnahmen A und B, die die Sicherheit vor dem BI Zustand meines Erachtens deutlich erhöhen.<br>
Massnahme C würde da noch mal eins draufsetzen und den CC1101 ebenfalls abschalten.

![pic](Images/Schaltung.png)
