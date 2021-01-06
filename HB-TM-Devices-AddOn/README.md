
# HB-TM-Devices-AddOn


## Unterstützte Geräte

- [HB-UNI-Sensor1](https://github.com/TomMajor/SmartHome/tree/master/HB-UNI-Sensor1)
- [HB-UNI-Sensor2..6](https://github.com/TomMajor/SmartHome/tree/master/HB-UNI-Sensor1#benutzerspezifische-sensordaten) (Benutzerspezifische Sensordaten)
- [HB-SEN-LJet](https://github.com/TomMajor/SmartHome/tree/master/HB-SEN-LJet)
- [HB-UNI-Sensor-Heizung](https://github.com/TomMajor/SmartHome/tree/master/HB-UNI-Sensor-Heizung)
- [HB-UNI-Sensor-Blitz](https://github.com/TomMajor/SmartHome/tree/master/HB-UNI-Sensor-Blitz)
- [HB-Sec-WDS-2](https://github.com/TomMajor/SmartHome/tree/master/HB-Sec-WDS-2)

[comment]: # (rc.d filename: hb-tm-devices-addon, github name: HB-TM-Devices-AddOn, info name: HB-TM-Devices AddOn)

###### Die verwendeten Device Model IDs:

| Sensor | Device Model |
|---|---|
| HB-UNI-Sensor1        | 0xF103 |
| HB-UNI-Sensor2        | 0xF112 |
| HB-UNI-Sensor3        | 0xF113 |
| HB-UNI-Sensor4        | 0xF114 |
| HB-UNI-Sensor5        | 0xF115 |
| HB-UNI-Sensor6        | 0xF116 |
| HB-SEN-LJet           | 0xF104 |
| HB-UNI-Sensor-Heizung | 0xF105 |
| HB-UNI-Sensor-Blitz   | 0xF120 |
| HB-Sec-WDS-2          | 0xF121 |


## Change Log

- 2.53 - 01.11.2020
  - Unterstützung für HB-UNI-Sensor-Blitz

- 2.52 - 31.08.2020
  - Bugfix HB-UNI-Sensor6, UV-Index beim Sensor VEML6075 wurde im WebUI nicht korrekt angezeigt

- 2.51 - 01.08.2020
  - Unterstützung für [HB-UNI-Sensor2..6](https://github.com/TomMajor/SmartHome/tree/master/HB-UNI-Sensor1#benutzerspezifische-sensordaten) (Benutzerspezifische Sensordaten)

- 2.50 - 17.05.2020
  - das neue HB-TM-Devices-AddOn orientiert sich an Jérômes JP-HB-Devices-addon und fasst die Unterstützung für meine HomeBrew-Geräte in einem AddOn zusammen.
  - in der ersten Version unterstützt das AddOn die Geräte HB-UNI-Sensor1, HB-SEN-LJet und HB-UNI-Sensor-Heizung.
  - die bisherigen separaten AddOn für diese Geräte sind dadurch hinfällig und werden nicht mehr unterstützt.
  - :warning: bitte unbedingt vor Installation des HB-TM-Devices-AddOn die alten HB-UNI-Sensor1 AddOn und HB-SEN-LJet AddOn deinstallieren, falls vorhanden!
  - HB-UNI-Sensor-Heizung hat ab sofort ein eigenes Device Model bekommen (0xF105) da ich im xml einen neuen Datenpunkt *STATE* eingeführt habe.


## Eingestellte AddOn

###### Change Log HB-UNI-Sensor1 AddOn (nicht mehr unterstützt)

- (2.50) - 17.05.2020
  - :warning: Das AddOn wird nicht mehr fortgeführt. Bitte das neue **HB-TM-Devices-AddOn** verwenden.

- 2.04 - 01.01.2020
  - Im xml die Einträge für `<special_value id="NO_SENSOR" value=..` entfernt (wirkungslos, Danke Jérôme).
  - Im xml den HUMIDITY Eintrag auf Bereich 0..100% geändert (% erzwingt Basis 100 in HM, Bereich 1..99% führte zu minimal falschen Kommastellen in ioBroker, Danke harvey).

###### Change Log HB-SEN-LJet AddOn (nicht mehr unterstützt)

- (2.50) - 17.05.2020
  - :warning: Das AddOn wird nicht mehr fortgeführt. Bitte das neue **HB-TM-Devices-AddOn** verwenden.

<br>

## Ab hier allgemeine Dokumentation der HomeMatic AddOn-Entwicklung

*Nur für Entwickler, für Anwender reicht die Installation des AddOn mittels Einstellungen/Systemsteuerung/Zusatzsoftware -> Datei [hb-tm-jp-addon-reduced.tgz](https://github.com/TomMajor/SmartHome/tree/master/HB-TM-JP-AddOn-Reduced/CCU_RM) auswählen*
<br><br>
(C) 2018-2021 Tom Major / Creative Commons (CC BY-NC-SA 4.0)<br>
Vielen Dank an Jérôme für seine wertvollen Vorlagen und Hinweise. :thumbsup: <br>


## Patch Konzepte

#### Mein Patch Konzept

- Hinweis: ${BACKUP_DIR} ist z.B. ${ADDON_DIR}/backup_3.51.6.20200229

- folgende Fälle sind möglich:

- A) Installation des AddOn
  - Merkmal: ${BACKUP_DIR}/saved existiert nicht
  - die zu patchenden CCU-Originaldateien nach ${BACKUP_DIR} sichern
  - Datei ${BACKUP_DIR}/saved anlegen
  - Patches anwenden (die Original Patches von Jérôme soweit für die ausgewählten 3 Geräte nötig)
<br><br>
- B) Update der RaspberryMatic Firmware, AddOn ist installiert
  - Merkmal: ${BACKUP_DIR}/saved existiert nicht (gemeint ist für den neuen Firmware String, für den alten ggf. schon)
  - weiter wie 1)
<br><br>
- C) Update des AddOn ('Drüber'-Installation)
  - Merkmal: ${BACKUP_DIR}/saved existiert
  - die gesicherten CCU-Originaldateien aus ${BACKUP_DIR} wiederherstellen
  - Patches erneut anwenden
<br><br>
- D) Deinstallation AddOn
  - Merkmal: n/a
  - die gesicherten CCU-Originaldateien aus ${BACKUP_DIR} wiederherstellen
<br><br>

- das ist so im [patchworker](https://github.com/TomMajor/SmartHome/blob/master/HB-TM-JP-AddOn-Reduced/CCU_RM/src/addon/patchworker) umgesetzt

- Jérômes Anmerkungen:
```
So wollte ich es anfangs mal machen. Aber irgendwelche Addons (war es HVL von THKL?) modifizieren u.U. die
selben Files. 
Wenn man dann seine eigenen Addon-Backup-Files wieder zurückspielt, gehen andere User-(AddOn)-Modifikationen
möglicherweise verloren.
Vielleicht denke ich da zu abstrakt und es wäre völlig egal. Aber die Möglichkeit bestünde.
Nur das war der Grund, weshalb ich immer nur meinen Teil De-Patchen wollte.
Die Variante mit den Backup-Files ist natürlich viel viel schneller und hängt letztendlich auch an der SD-
Kartengeschwindigkeit. Andererseits erfolgt das ganze Gepatche ja auch nur 1x (z.B. nach einem CCU-FW-Update)
und nicht bei jedem Hochfahren.
In einer VM macht das wesentlich mehr Spaß. Da ist der Kram in 1-2 Sekunden durch :-) 
```

#### Jérômes Patch Konzept

- Jérôme pflegt einen revoke Ordner, in dem die ganze History seiner Patches, die in der Vergangenheit jemals gemacht wurden, enthalten ist.

- Dieser revoke Ordner ist einerseits für die Deinstallation des AddOn, andererseits auch für eine Addon 'Drüber'-Installation:
```
Angenommen der Kunde kommt von AddOn Version 2.19 und geht auf 2.20. In der ic_common.tcl hat sich eine
weitere Änderung zum Patchen ergeben.
Damit der neue Patch angewendet werden kann, muss die CCU-Originaldatei beim Kunden existieren, denn die
Patches beziehen sich immer auf die Originaldatei und nicht auf die bereits gepatchte Vorgängerversion.
Das geht nur, wenn man alle möglichen vorherigen Patches zurückfährt. 
Die meisten werden fehlschlagen, aber ich weiß nie, von welcher Version jemand kommt.
Daher werden erstmal vorsichtshalber alle Patches, die es für eine Datei mal gab, zurückgefahren.
```

#### Patchen

- Nur der Vollständigkeit noch mal gesagt:<br>
So lange sich nur die Stelle (Zeilennummer) in der CCU-Originaldatei ändert, hat Patch kein Problem. Da ändere ich dann auch nix.<br>
Es kommt beim Patchen nur ein Hinweis, dass der Patch an anderer Stelle applied wurde.<br>
Ich muss nur dann was anfassen, wenn direkt an den für mich relevanten Abschnitten was geändert wurde.<br>
  *jp112sdl*


## Diverse Informationen

#### AddOn Konzept - Dateien

- Der update_script kopiert lediglich beim Runterfahren erstmal die Addon-Inhalte ins /usr/local.<br>
An dieser Datei muss man eigentlich nie was ändern.

- Der rc-Script schaut, ob überhaupt eine Installation stattfinden muss und kümmert sich um die Vorbereitung des Systems (ggf. anhalten von Diensten, rw-mounten) und stellt dem RFD die vom Benutzer editierte XMLs zur Verfügung, sofern er sie in customized_firmware abgelegt hat<br>
Hier ist die zentrale Baustelle, wenn es CCU FW Updates gibt

- install\_\*-Files führen dann alle das jeweilige Geräte betreffenden Modifikationen durch.<br>
Auch hier muss über CCU-FW-Generationen hinweg nichts geändert werden.<br>
  *jp112sdl*

#### AddOn Installation von der Kommandozeile

    # Das Addon nach /tmp kopieren. Dann über SSH:
    cd /tmp
    tar xzf tm-jp-hb-reduced-addon.tgz
    ./update_script
    /etc/config/rc.d/tm-jp-hb-reduced-addon start

Der Unterschied zum RM restart ist, dass hier jetzt die Prozesse ReGaHss und RFD neu gestartet werden.<br>
Das fällt beim Ausführen zur Bootzeit weg, weil das rc-Skript vor dem Starten der anderen Prozesse ausgeführt wird.

#### Warum kann man nicht die gesamte Installation im update_script erledigen?

- Bei einer CCU2 und CCU3 findet das installieren des Addons via update_script in einer chroot Umgebung statt. Es ist zwar über Umwege auch da möglich am read-only rootfs Änderungen vorzunehmen. Gewollt ist es da allerdings nicht.<br>
  *jmaus*

#### RaspberryMatic rc.d Skript 'init' Zweig

- Beim Hochfahren gibt es mit RaspberryMatic die Möglichkeit Dinge die ein Addon VOR ausführen des ReGaHss, rfd, etc. Dienstes erledigen muss in einen "init" Zweig des Addon rc Skriptes zu stecken.<br>
Das nutzt z.B. CUxD damit es VOR dem start von ReGaHss&Co einige Dinge erledigen kann (siehe https://github.com/jens-maus/cuxd/blob/master/ccu3/rc.d/cuxdaemon#L61).<br>
Damit sollte es dann z.B. möglich sein das du die Dinge die du VOR ReGaHss erledigen musst in dem "init" Zweig deines rc Skriptes abhandelst und damit eigentlich nicht mehr selbst ReGaHss stoppen/starten musst (was eben vermieden werden sollte).<br>
  *jmaus*

#### Custom devices sind nach RM firmware update weg (befinden sich aber im Posteingang)

- Das Problem ist Folgendes, nach einem Firmware-Update fährt das System erstmal unmodifiziert hoch, ..., rfd startet, ReGaHss startet.<br>
Danach erst werden die Addon-Skripts ausgeführt, somit auch dein rc.d/hb-uni-sensor1.<br>
RFD und ReGaHss laufen dabei jedoch schon und sie wurden gestartet, bevor sie deine Custom-Geräte kennen konnten.<br>
Die Geräte bekommen dabei das "dirty"-Flag und landen im Posteingang.<br>
Sie dürften auch aus allen Programmen rausfliegen. Bin mir aber nicht 100% sicher.<br>
Aus diesem Grund halte ich vor der Installation meines Addons die beiden Dienste an und starte sie erst zum Schluss wieder.<br>
  *jp112sdl*

#### neues xml testen:

    mount -o remount,rw /
    # restart_rm nach /root kopieren
    chmod 700 restart_rm
    # neues xml nach /usr/local/addons/hb-tm-devices-addon/firmware/rftypes kopieren (SymLink existiert aus /firmware/rftypes)
    mount -o remount,ro /
    # ro status testen mit:
    grep "\sro[\s,]" /proc/mounts
    /root/restart_rm

#### hb-tm-devices-addon.tgz bauen:

    # ggf. xml in (Repo)/CCU_RM/src/addon/firmware/rftypes anpassen
    # (Repo)CCU_RM/src/addon/VERSION, Versionsnummer erhöhen
    # (Repo)CCU_RM nach /tmp kopieren
    cd /tmp/CCU_RM
    chmod +x build.sh
    ./build.sh
    # hb-tm-devices-addon.tgz nach <Repo> zurückkopieren

#### Script restart_rm

    #!/bin/sh
    echo
    /etc/init.d/S50lighttpd restart
    sleep 1
    /etc/init.d/S61rfd restart
    sleep 1
    /etc/init.d/S70ReGaHss restart


## Versionierung Firmware (Sketch) <-> CCU Addon xml

Also die VERSION habe ich in der Addon source natürlich hochgesetzt -
Diese Versionsangabe dient lediglich der Anzeige in der WebUI, um den Anwender über den Versionsstand des jeweiligen Addons zu informieren.
Was ich meine, ist die Firmware-Version des Gerätes.

Du hast zum einen die FW-Versionsangabe im Sketch

    0x10, // Firmware Version
In dem Fall `0x10` was Version "1.0" entspricht und in der WebUI in den Geräteeinstellungen auch "Version 1.0" anzeigen lässt.
Die FW Version wird beim Anlernen mit übertragen.

Auf der Gegenseite, der CCU, liegt die XML.
Dort ist die Firmware-Version, für die diese XML gültig ist, beschrieben in

    <parameter index="9.0" size="1.0" cond_op="GE" const_value="12" />

Diese Gerätebeschreibungsdatei gilt für Geräte mit einem FW-Stand GE (>=) 12(dec).
Da 10(hex) größer als 12(dec) ist, wird diese XML beim Anlernen gezogen.

Es wäre nun also ratsam
- die Vorgängerversion der XML auf die FW-Version `0x10` festzunageln (`cond_op="E" const_value="0x10"`) und umzubenennen (z.B. `hb-uni-sensor1_e_0x10.xml`)   
- die `//Firmware Version` im aktuellen Sketch zu ändern, z.B. auf `0x11` und die XML genau an diese FW-Version zu binden mit `cond_op="E" const_value="0x11"`

Anschließend beide Versionen ausliefern.
Du hast ein Addon (Anzahl 1) und packst da einfach mehrere XML-Files mit rein.
So können dann nach wie vor auch Geräte mit altem FW-Stand wieder korrekt angelernt werden, ohne sie neu flashen zu müssen.
  *jp112sdl*


## custom HomeMatic data types

:warning: **Achtung: für mich obsolete, nicht mehr verwendet, da der geänderte Datentyp zwar in der Zentrale aber nicht in Systemen wie ioBroker funktioniert**

- Beispiel 'Digitaler Eingang'

Durch diese Änderungen wird der plotbare HomeMatic Datentyp ILLUMINATION für den HB-UNI-Sensor1 in den Datentyp 'Digitaler Eingang' umgewandelt.<br>
So erhält man in der Geräteansicht **und** in der Diagramm-Legende einen Namen der besser zu einem beliebigen (binären) Status 0/1 passt.<br>
Dieser Status ist in einem Diagramm plotbar (dies ist nicht mit allen HM Datentypen möglich).<br>
Die Wahl fiel auf Datentyp ILLUMINATION weil dieser in den Device xml-Dateien der diversen HM Geräte kaum bzw. nicht verwendet wird.<br>
Ein Abschnitt im install bzw. uninstall Script sorgt für die Änderungen, diese sind im Einzelnen:<br>

    # neue Zeile in:
    /www/config/stringtable_de.txt
    WEATHER|ILLUMINATION**TAB CHAR**\${stringTableWeatherIllumination$JAVA_DEVICE_NAME}
    # wichtig, der Tab muss sein, nur mit space wird der String-Namen nicht eingelesen!
    # Wegen alter Version vom Tool sed auf der CCU2 muss ein richtiger Tab char 0x09 in der Zeile erzeugt werden, ein \t geht auf RM aber nicht auf CCU2

    # neue Zeile in:
    /www/webui/js/lang/de/translate.lang.stringtable.js
    "stringTableWeatherIllumination$JAVA_DEVICE_NAME" : "Digitaler Eingang",

    # Ersetzen in:
    /www/webui/js/lang/de/translate.lang.diagram.js
    "diagramValueTypeILLUMINATION": "Digitaler Eingang",

In diesem Beispiel wird *stringTableWeatherIllumination* nur für den Datenpunkt *ILLUMINATION* übersetzt falls der Kanal vom Typ *WEATHER* ist.<br>
Ohne das vorangestellte *WEATHER|* würden alle Kanaltypen mit Datenpunkt *ILLUMINATION* mit *stringTableWeatherIllumination* übersetzt werden.


## Die Liste der internen Datentypen von HomeMatic (TEMPERATURE, AIR_PRESSURE, VALVE_STATE usw.) die man in einem Diagramm verwenden kann

In /www/webui/js/lang/en/translate.lang.diagram.js findet man die Datentypen die in Diagrammen verwendet werden können (dort mit diagramValueTypeXXXXXX gekennzeichnet):
```
ENERGY_COUNTER
IEC_ENERGY_COUNTER
GAS_ENERGY_COUNTER
POWER
IEC_POWER
GAS_POWER
CURRENT
VOLTAGE
FREQUENCY
TEMPERATURE
HUMIDITY
BRIGHTNESS
ILLUMINATION
LUX
WIND_SPEED
WIND_DIRECTION
RAINING
RAIN_COUNTER
SET_TEMPERATURE
SET_POINT_TEMPERATURE
ACTUAL_TEMPERATURE
VALVE_STATE
FILLING_LEVEL
LEVEL
AIR_PRESSURE
SUNSHINEDURATION
SUNSHINE_THRESHOLD_OVERRUN
WIND_DIR
HIGHEST_ILLUMINATION
CURRENT_ILLUMINATION
LOWEST_ILLUMINATION
AVERAGE_ILLUMINATION
```


## Die Liste aller internen Datentypen von HomeMatic (TEMPERATURE, UNREACH, ERROR usw.)

Alle vorhandenen internen Datentypen kann man aus den Firmware xmls extrahieren (*jp112sdl*):
```
xmllint --format /firmware/rftypes/*.xml|grep "operations=\"read,event\"" | awk -F "id=" '{print $2}' | awk -F " " '{print $1}' | sort | uniq
```

```
"ACTUAL_HUMIDITY"
"ACTUAL_TEMPERATURE"
"ADJUSTING_COMMAND"
"ADJUSTING_DATA"
"AIR_PRESSURE"
"BATTERY_STATE"
"BOOST_STATE"
"BOOST_TIME"
"BOOT"
"BRIGHTNESS"
"COMMUNICATION_REPORTING"
"CONFIG_PENDING"
"CONTROL_MODE"
"COUNTERREADING1"
"COUNTERREADING10"
"COUNTERREADING2"
"COUNTERREADING3"
"COUNTERREADING4"
"COUNTERREADING5"
"COUNTERREADING6"
"COUNTERREADING7"
"COUNTERREADING8"
"COUNTERREADING9"
"CURRENT"
"DECISION_VALUE"
"DEVICE_IN_BOOTLOADER"
"DIRECTION"
"DIRECTION_SLATS"
"DUTYCYCLE"
"ENERGY_COUNTER"
"ERROR"
"ERROR_ALARM_TEST"
"ERROR_BATTERY"
"ERROR_M1"
"ERROR_M2"
"ERROR_M3"
"ERROR_OVERHEAT"
"ERROR_OVERLOAD"
"ERROR_POWER"
"ERROR_REDUCED"
"ERROR_SABOTAGE"
"ERROR_SMOKE_CHAMBER"
"ERR_DETECT_EIA485_SERVICE"
"ERR_TTCU_INTERNAL_TEST"
"ERR_TTCU_LOCK_ROLLERS_SHORTED"
"ERR_TTCU_MAGNET_ERROR"
"ERR_TTCU_POWER_ONTIME_EXCEEDED"
"ERR_TTCU_SENSOR_STRIP_DISCONNECTED"
"ERR_TTCU_SENSOR_STRIP_SHORTED"
"ERR_TTCU_STOP_AFTER_10_CLOSING_TRIES"
"ERR_TTCU_TURN_TILT_ACT_ALLOY_MOSFET"
"ERR_TTCU_TURN_TILT_ACT_ASYNCHRON"
"ERR_TTCU_TURN_TILT_ACT_BLOCKED"
"ERR_TTCU_TURN_TILT_ACT_CONTACT_PROBLEM"
"ERR_TTCU_TURN_TILT_ACT_NO_SPEED_SIGNAL"
"ERR_TTCU_TURN_TILT_ACT_OVERCURRENT"
"ERR_TTCU_TURN_TILT_ACT_SHORTED"
"ERR_TTCU_WRONG_VOLTAGE_POLARITY"
"ERR_TTM_INTERNAL"
"ERR_TTM_OVERVOLT"
"ERR_TTM_UNDERVOLT"
"ERR_WINDOW_NOT_FOUND"
"ERR_WIN_STAY_IN_INITIAL_OPERATION"
"FAULT_REPORTING"
"FILLING_LEVEL"
"FREE_TO_USE"
"FREQUENCY"
"GAS_ENERGY_COUNTER"
"GAS_POWER"
"HUMIDITY"
"IEC_ENERGY_COUNTER"
"IEC_POWER"
"LEVEL"
"LEVEL_REAL"
"LOWBAT"
"LOWBAT_REPORTING"
"LOWBAT_SENSOR"
"LUX"
"MOTION"
"POWER"
"RAINING"
"RAIN_COUNTER"
"RSSI"
"RSSI_DEVICE"
"RSSI_PEER"
"STATE"
"STATE_UNCERTAIN"
"STATUS"
"SUNSHINEDURATION"
"TEMPERATURE"
"TIPTRONIC_STATE"
"UNREACH"
"UPDATE_PENDING"
"VALVE_STATE"
"VOLTAGE"
"WINDOW_OPEN_REPORTING"
"WINDOW_STATE"
"WINDOW_TYPE"
"WIND_DIRECTION"
"WIND_DIRECTION_RANGE"
"WIND_SPEED"
"WIN_RELEASE"
"WORKING"
"WORKING_SLATS"
```


## Spezielle Datentypen (Servicemeldungen)

- Wie ein Datentyp im WebUI angezeigt wird, hängt hauptsächlich von *control="..."* ab.
  - Bei **NONE** steht einfach nur der **Bezeichner: Wert** dann in der WebUI (graues Feld bei *Status und Bedienung/Geräte*)
  - Bei **SWITCH.STATE** bekommt man einen EIN-AUS-Button.
  - Bei **BUTTON.SHORT** einen Taster "kurzer Tastendruck".

- "ERROR" ist ein interner Datentyp (*ui_flags="service"*), der Servicemeldungen generiert (z.B. Sabotage bei Fensterkontakten).

- Beispiel für "ERROR" Verwendung: Jérômes Katzenklappen-Projekt mit Magnetwinkelsensor, seine Hinweise dazu:
  - dort wird der ERROR-State genutzt, um bestimmte Fehlermeldungen des Magnetwinkelsensors zu übertragen, Magnetabstand zu dicht, zu nah, genereller Sensorfehler usw.
  - In der XML sieht das so aus: [Link](https://github.com/jp112sdl/JP-HB-Devices-addon/blob/master/src/addon/firmware/rftypes/hb-uni-sen-pf-sc.xml#L179-L192)
  - Nicht zu vergessen, "ERROR" auch im [Frame](https://github.com/jp112sdl/JP-HB-Devices-addon/blob/master/src/addon/firmware/rftypes/hb-uni-sen-pf-sc.xml#L30-L39) zu berücksichtigen!
  - Und auch die [Übersetzungen](https://github.com/jp112sdl/JP-HB-Devices-addon/blob/master/src/addon/install_hb-uni-sen-pf-sc) nicht vergessen.
  - Im Sketch mache ich das so, dass ich [alle 5 Sekunden den Fehlerstatus des AS5600 auslese](https://github.com/jp112sdl/HB-UNI-Sen-PF-SC/blob/master/HB-UNI-Sen-PF-SC.ino#L165-L170).
  - Wenn sich der Status ändert, wird ein [INFO-Telegram mit dem gesetzten ERROR-Flag gesendet](https://github.com/jp112sdl/HB-UNI-Sen-PF-SC/blob/master/HB-UNI-Sen-PF-SC.ino#L205-L229).
  - Ich bin erst viel zu spät auf diese Möglichkeit gestoßen, sonst hätte ich das in anderen Projekten schon für die Übertragung von Fehlern genutzt.
  - Beim HB-UNI-Sensor1 könnte man damit übertragen, ob eine Sensorinitialisierung fehlgeschlagen ist. Dann sieht der Anwender sofort, wenn was nicht stimmt.
  - P.S.: Wenn man aber den Aufwand gering halten möchte (ohne custom Error Message), kann man auch alles mit "ERROR" aus dem [Fensterkontakt](https://github.com/AskSinPP/asksinpp.de/blob/master/rftypes/xmls/rf_sc.xml) rüberholen und dann einfach das [Sabotage-Bit](https://github.com/pa-pa/AskSinPP/blob/master/ContactState.h#L71-L75) setzen wenn ein Fehler auftritt.


## Die Registerklassen (Listen) eines HomeMatic-Gerätes

Gerätebezogene Register<br>
Gerätebezogene Register existieren für jedes HomeMatic-Gerät nur einmal und
werden in der sogenannten List0 gespeichert.

Kanalbezogene Register<br>
Kanalbezogene Register existieren für jeden Kanal eines Gerätes einmal und
werden in der sogenannten List1 gespeichert.

Verknüpfungsbezogene Register<br>
Diese Register sind am umfangreichsten und werden für jeden Verknüpfungspartner
(peer) einzeln separat angelegt in der List3 (RegL_03.<peer>). Die
grundsätzlichen Funktionen und ihre Zusammenhänge sind auch ausführlich in der
Einsteigerdokumentation erklärt, inklusive Skizzen für die sogenannte state
machine.

https://wiki.fhem.de/wiki/Homematic-Register_von_A-Z_(Namen,_Erkl%C3%A4rung)
https://wiki.fhem.de/wiki/HomeMatic_Register_programmieren


## Lizenz

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
