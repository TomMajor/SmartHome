
### Diverse Infos und Dokumentationen für die RaspberryMatic/CCU AddOn-Entwicklung

*Nur für Entwickler, für Anwender reicht die Installation des AddOn mittels Einstellungen/Systemsteuerung/Zusatzsoftware -> z.B. Datei CCU_RM/HB-UNI-Sensor1-addon.tgz auswählen*
<br><br>
(C) 2018, 2019 Tom Major (Creative Commons)<br>
[CC BY-NC-SA 3.0](https://creativecommons.org/licenses/by-nc-sa/3.0/)<br>
Danke an jp112sdl für seine wertvollen Hinweise.<br>

### Mein AddOn Konzept

|Datei|Gleich bei allen HB-\* Projekten|Bemerkungen|
|---|---|
|update_script|:green_apple: ja|-|
|rc.d Skript|:green_apple: ja|-|
|install\_\*, uninstall\_\*|:apple: nein|Gerätespezifische Anpassungen, kopieren, patchen usw.|
|params|:apple: nein|Gerätespezifische Namen, Links, Version|
|functions|:green_apple: ja|-|

#### Konzepthinweise von jp112sdl

> Und ich finde es so am übersichtlichsten:
- update_script kopiert lediglich beim Runterfahren erstmal die Addon-Inhalte ins /usr/local<br>
  :green_apple: An dieser Datei muss man eigentlich nie was ändern
- rc-Script schaut, ob überhaupt eine Installation stattfinden muss und kümmert sich um die Vorbereitung des Systems (ggf. anhalten von Diensten, rw-mounten) und stellt dem RFD vom Benutzer editierte XMLs dem RFD zur Verfügung, sofern er sie in customized_firmware abgelegt hat<br>
  :large_blue_circle: Hier ist die zentrale Baustelle, wenn es CCU FW Updates gibt
- install\_\*-Files führen dann alle das jeweilige Geräte betreffenden Modifikationen durch.<br>
  :green_apple: Auch hier muss über CCU-FW-Generationen hinweg nichts geändert werden

<!-- -->

#### Warum kann man nicht die gesamte Installation im update_script erledigen?

> Bei einer CCU2 und CCU3 findet das installieren des Addons via update_script in einer chroot Umgebung statt. Es ist zwar über Umwege auch da möglich am read-only rootfs Änderungen vorzunehmen. Gewollt ist es da allerdings nicht.
(jmaus)

<!-- -->

#### Custom devices sind nach RM firmware update weg (befinden sich aber im Posteingang)

>Das Problem ist Folgendes, nach einem Firmware-Update  fährt das System erstmal unmodifiziert hoch, ..., rfd startet, ReGaHss startet.<br>
Danach erst werden die Addon-Skripts ausgeführt, somit auch dein rc.d/hb-uni-sensor1.<br>
RFD und ReGaHss laufen dabei jedoch schon und sie wurden gestartet, bevor sie deine Custom-Geräte kennen konnten.<br>
Die Geräte bekommen dabei das "dirty"-Flag und landen im Posteingang.<br>
Sie dürften auch aus allen Programmen rausfliegen. Bin mir aber nicht 100% sicher.<br>
Aus diesem Grund halte ich vor der Installation meines Addons die beiden Dienste an und starte sie erst zum Schluss wieder.<br>
(jp112sdl)

<!-- -->

#### RaspberryMatic rc.d Skript 'init' Zweig

> Beim Hochfahren gibt es mit RaspberryMatic die Möglichkeit Dinge die ein Addon VOR ausführen des ReGaHss, rfd, etc. Dienstes erledigen muss in einen "init" Zweig des Addon rc Skriptes zu stecken. Das nutzt z.B. CUxD damit es VOR dem start von ReGaHss&Co einige Dinge erledigen kann (siehe https://github.com/jens-maus/cuxd/blob/master/ccu3/rc.d/cuxdaemon#L61).<br>
Damit sollte es dann z.B. möglich sein das du die Dinge die du VOR ReGaHss erledigen musst in dem "init" Zweig deines rc Skriptes abhandelst und damit eigentlich nicht mehr selbst ReGaHss stoppen/starten musst (was eben vermieden werden sollte).
(jmaus)


### neues xml testen:
    mount -o remount,rw /
    # restart_rm nach /root kopieren
    chmod 700 restart_rm
    # neues xml nach /firmware/rftypes kopieren
    mount -o remount,ro /
    # ro status testen mit:
    grep "\sro[\s,]" /proc/mounts
    /root/restart_rm


### HB-UNI-Sensor1-addon.tgz bauen:
    # ggf. xml in (Repo)/CCU_RM/src/addon/firmware/rftypes anpassen
    # (Repo)CCU_RM/src/addon/params prüfen/anpassen, Versionsnummer erhöhen
    # (Repo)CCU_RM nach /usr/local/tmp kopieren
    cd /usr/local/tmp/CCU_RM
    chmod +x build.sh
    ./build.sh
    # /usr/local/tmp/CCU_RM nach <Repo> zurückkopieren
    # durch build.sh geänderte Dateien: HB-UNI-Sensor1-addon.tgz, VERSION, update-check.cgi


### Script restart_rm
    #!/bin/sh
    echo
    /etc/init.d/S50lighttpd restart
    sleep 1
    /etc/init.d/S61rfd restart
    sleep 1
    /etc/init.d/S70ReGaHss restart


### Versionierung Firmware(Sketch) <-> CCU Addon xml (jp112sdl)
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


### custom HomeMatic data type 'Digitaler Eingang'
**Achtung: obsolete, nicht mehr verwendet, da der geänderte Datentyp zwar in der Zentrale aber nicht in Systemen wie ioBroker funktioniert**

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


### Die Registerklassen (Listen) eines HomeMatic-Gerätes

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
