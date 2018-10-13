
## Universalsensor (HB-UNI-Sensor1)

- Demonstriert einen HomeMatic/RaspberryMatic/FHEM Universalsensor für Temperatur, Luftdruck, Luftfeuchte, Helligkeit usw.
- modifizierbar für andere Sensoren
- konfigurierbares Sendeintervall über WebUI
- konfigurierbare Höhe (für Berechnung des Luftdrucks auf Meeresniveau/Normaldruck) über WebUI
- über die defines am Anfang des Sketches können entweder Dummy Werte für die Sensoren (zum Testen der Anbindung an HomeMatic/RaspberryMatic/FHEM) oder reale Sensoren aktiviert werden
Beispiel:<br>
`#define SENSOR_BME280    // realer BME280 angeschlossen`<br>
`//#define SENSOR_BME280    // es werden BME280 Dummy Werte gesendet`
- Es können mehrere HB-UNI-Sensor1 an einer Zentrale verwendet werden. Dafür müssen sich die einzelnen Sensoren nur in Device ID und Device Serial unterscheiden:<br>
`const struct DeviceInfo PROGMEM devinfo = `<br>
`  { 0xA5, 0xA5, 0x00 },    // Device ID`<br>
`  "UNISENS001",            // Device Serial`<br>
- Ich empfehle den MAX44009 Helligkeitssensor anstatt dem TSL2561, siehe<br>
[SensorTest_Lux](https://github.com/TomMajor/AskSinPP_Examples/tree/master/Info/SensorTest_Lux)


# Benötige Libraries

[AskSinPP Library](https://github.com/pa-pa/AskSinPP)</br>
[EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt)</br>
[Low-Power](https://github.com/rocketscream/Low-Power)


Für einen DS18x20 Sensor (Temperatur):</br>
[OneWire](https://github.com/PaulStoffregen/OneWire)

Für einen BME280 Sensor (Temperatur/Druck/Feuchte):</br>
[BME280](https://github.com/finitespace/BME280)

Für einen TSL2561 Sensor (Helligkeit in Lux):</br>
[TSL2561](https://github.com/adafruit/TSL2561-Arduino-Library)

Für einen MAX44009 Sensor (Helligkeit in Lux):</br>
keine zusätzliche Library nötig.

Für einen SHT10 Sensor (Feuchte):</br>
[SHT10](https://github.com/spease/Sensirion)


# Schaltung
![pic](Images/Schaltung.png)


# Messung der Batteriespannung

![pic](Images/Batteriemessung.png)

- Option1: Batteriespannungsmessung Standard (UBatt = Betriebsspannung AVR)<br>
keine zusätzliche Hardware notwendig<br>
`typedef AskSin<LedType, BatterySensor, RadioType> BaseHal;`<br>

- Option2: Batteriespannungsmessung für Step-Up<br>
2 zusätzliche Widerstände notwendig, Verwendung der Batterieklasse BatterySensorUni<br>
`BatterySensorUni<17,7,3000>`<br>
`// sense pin = A1 = 15, activation pin = D7 = 7, Vcc nominal 3,0V`

- Option3: Echte Batteriespannungsmessung unter Last<br>
Sie dient u.a. dem Schutz vor einem "Babbling Idiot, siehe
[Babbling Idiot Protection](https://github.com/TomMajor/AskSinPP_Examples/tree/master/Info/Babbling%20Idiot%20Protection)
<br><br>
Aus meiner Sicht würde es sehr helfen, eine echte Messung des Batteriezustands unter Last zu haben, um frühzeitig leere Batterien zu erkennen und zu tauschen. Bekanntermaßen sagt eine Spannungsmessung an unbelasteter Batterie, je nach Batterie- bzw. Akkutyp, nicht viel über den Ladezustand aus.
<br><br>
Die Schaltung belastet die Batterie bzw. den Akku für einige Hundert Millisekunden und misst dabei die Spannung.
Dies führt meiner Meinung nach zu realistischeren Werten über den Batteriezustand als eine asynchrone und unbelastete Messung.
<br><br>
Dazu wurde eine neue Batterieklasse nach Vorbild von papas Batterieklassen erstellt. Sie heißt hier BatterySensorLoad und befindet sich unter Sensors/BatterySensorLoad.h <br>
Mit dieser Klasse und der Schaltung wird der 1,2V Akku mit ca. 75mA für die kurze Zeit der Messung belastet. Anpassungen an andere Spannungen und Ströme sind natürlich leicht über die Widerstände R5/R6 möglich. Momentan geschieht das 2 mal am Tag.<br>
`TODO code für Dekl. der Klasse`<br>
`hal.battery.init(seconds2ticks(60UL*60*12), sysclock, 2000);`<br>
`// 2x Batt.messung täglich, Spannungsteiler 1:2`<br>

Das Bild zeigt den Einbruch der Batteriespannung wenn für 200ms mit 75mA belastet wird. Die Spannung bricht um 142mV ein und wird am Ende der 200ms gemessen.
<br>
![pic](Images/BatterySensorLoad.png)


# Prototyp
![pic](Images/Prototyp_HB-UNI-Sensor1.jpg)


# CCU2/RaspberryMatic Installation
Einstellungen/Systemsteuerung/Zusatzsoftware -> Datei CCU_RM/HB-UNI-Sensor1-addon.tgz installieren.

![pic](Images/HB-UNI-Sensor1_Install.png)


# RaspberryMatic WebUI
Der angemeldete Sensor auf der RaspberryMatic:

![pic](Images/HB-UNI-Sensor1_WebUI.png)

![pic](Images/HB-UNI-Sensor1_Parameter.png)


# FHEM Installation
Die Datei FHEM/HMConfig_UniSensor1.pm nach /opt/fhem/FHEM kopieren, dann FHEM neustarten.

![pic](Images/fhem1.png)

![pic](Images/fhem2.png)

**Parameter Höhe einstellen:**<br>
"Ich konnte bei meinem THPL Sensor mit BME280 und MAX44009 über getConfig // **Config drücken** // set regSet altitude 590 // **Config drücken** // getConfig // **Config drücken** die Höhe einstellen. Bitte berichtigt mich, wenn ich da zu viel drücke und mache- so hat es jedenfalls funktioniert ::)"<br>
_FHEM user kpwg_
