# Babbling Idiot Protection

Auf dieser Seite sollen Lösungen zum Schutz gegen einen "Babbling Idiot" im Homematic Netzwerk bei Selbstbaugeräten vorgestellt werden.

## Problem

Zitat Wikipedia:<br>
"Ein Babbling Idiot ist ein Knoten eines Netzwerks, der die Kommunikation durch Fehlverhalten stört. Dies kann durch das Senden von Nachrichten zu willkürlichen, nicht mit dem Netzwerk synchronisierten Zeitpunkten oder durch fehlerhaften Inhalt von Nachrichten hervorgerufen werden.<br>
Er kann somit die Knoten stören, die ihre Nachrichten synchron mit dem Netzwerk übertragen. Als Folge kann es zu Fehlkommunikation bis hin zu einem vollständigen Kommunikationsverlust im Netzwerk kommen. Um dem vorzubeugen, kann beispielsweise jeder Netzwerkteilnehmer einen sogenannten Buswächter (Bus Guardian) besitzen. Dieser stellt sicher, dass der Netzwerkteilnehmer nur innerhalb seines statisch zugewiesenen Zeitslots sendet."
<br>
[Wikipedia - Babbling Idiot](https://de.wikipedia.org/wiki/Babbling_idiot)</br>

Beim Homematic Funknetzwerk kann ein Babbling Idiot wegen der 1% Regel sehr schnell das ganze System über Stunden lahmlegen.

Dies wurde bereits von einigen Usern im FHEM und homematic-forum beschrieben, z.B. von jp112sdl hier:<br>
[Antw:AskSin++ Library](https://forum.fhem.de/index.php/topic,57486.msg783197.html#msg783197)
<br>
[Antw:AskSin++ Library](https://forum.fhem.de/index.php/topic,57486.msg783235.html#msg783235)

<font color="red">TODO</font><br>
8fach-DS18B20 Projekt link oder infos?

<font color="red">TODO</font><br>
weitere links aus homematic-forum?

## Massnahmen

Ich stelle hier 3 Massnahmen vor, um sich vor diesem Effekt zu schützen, anhand meines Beispiels für den Wassrmelder<br>
[HB-SEC-WDS-2](https://github.com/TomMajor/AskSinPP_Examples/tree/master/HB-SEC-WDS-2)

Der Babbling Idiot könnte entstehen, wenn beim Senden die Batteriespannung so weit zusammenbricht, das ein normaler Programmablauf nicht mehr gegeben ist, dadurch der angefangene Sendevorgang nicht abgeschlossen werden kann, die Spannung aber noch für den Betrieb der Sendeendstufe im CC1101 reicht. <br>Dieser Zustand ist kritisch und soll mit den hier gezeigten Massnahmen vermieden werden.

Ich möchte ausserdem anmerken dass die Störung des Programmablaufes beim Senden nicht nur durch den AVR entstehen kann sondern wahrscheinlich auch durch den CC1101 selbst, dieser hat ebenso einen Mikrokontroller mit Firmware integriert.<br>
Ob ein Babbling Idiot also im AVR oder CC1101 entsteht kann nur durch Messung im Falle des Falles am Gerät bewiesen werden, wegen der Reproduzierbarkeit ein schwieriges Unterfangen.

## Schaltung A

Aus meiner Sicht würde es sehr helfen, eine echte Batteriezustandsmessung unter Last zu haben, um frühzeit leere Batterien zu erkennen und zu tauschen. Der Schaltungsteil belastet die Batterie bzw. den Akku für einige Hundert Millisekunden und misst dabei die Spannung.
Dies führt meiner Meinung nach zu realistischeren Werten über den Batteriezustand als eine asynchrone und unbelastete Messung wie z.B. mittels hal.battery.init(seconds2ticks(3600).

## Schaltung B

Am AVR wird ein Baustein zur Spannungsüberwachung (BU48xx oder MCP111) angeschlossen. Fällt die Spannung unter den Schwellwert wird das RESET Signal für den AVR aktiv. Falls der AVR der Verursacher des Babbling Idiot ist wird er damit ruhiggestellt.
Alternativ zum externen Baustein kann man auch die interne Brown-Out Detection des AVR mit der 2,7V Schwelle verwenden. Allerdings hebt diese den Strom im Sleep von ca. 4uA auf 18uA an.
Die externen Bausteine haben um 1uA, deswegen verwende ich die lieber.

## Schaltung C

Diese Massnahme setzt Schaltung B voraus. Fällt die Spannung unter den Schwellwert wird das RESET Signal für den AVR aktiv. Dadurch werden hardwaremässig beim AVR alle I/O auf Input Mode gesetzt. Der Ausgang D4 kann dadurch keinesfalls mehr den Mosfet T2 schalten, als Resultat wird der CC1101 von der Spannung abgeklemmt und ruhiggestellt.

## Fazit

Im momentanen Stadium der Protoypen verwende ich die Massnahmen A und B, die die Sicherheit vor dem Babbling Idiot Zustand meines Erachtens deutlich erhöhen. Massnahmen C würde da noch mal eins draufsetzen, schwer zu sagen ob sich das lohnt.

![pic](Images/Schaltung.png)
