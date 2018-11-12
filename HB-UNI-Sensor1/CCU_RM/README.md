
# Diverse Infos und Dokumentationen zur HM Anbindung
// 2018-10-23 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// Danke an jp112sdl für seine wertvollen Hinweise


# neues xml testen:
mount -o remount,rw /
restart_rm nach /root kopieren
chmod 700 restart_rm
neues xml nach /firmware/rftypes kopieren
mount -o remount,ro /
ro status testen mit  grep "\sro[\s,]" /proc/mounts
/root/restart_rm


# HB-UNI-Sensor1-addon.tgz bauen:
ggf. neues xml nach <Repo>CCU_RM/src/addon/firmware/rftypes kopieren
<Repo>CCU_RM/src/addon/params prüfen/anpassen, Versionsnummer erhöhen
mount -o remount,rw /
<Repo>CCU_RM nach /root kopieren
cd CCU_RM/
chmod 700 build.sh
./build.sh
mount -o remount,ro /
/root/CCU_RM nach <Repo> zurückkopieren
(durch build.sh geänderte Dateien: HB-UNI-Sensor1-addon.tgz, VERSION, update-check.cgi)


# Script restart_rm
#!/bin/sh
echo
/etc/init.d/S50lighttpd restart
sleep 1
/etc/init.d/S61rfd restart
sleep 1
/etc/init.d/S70ReGaHss restart


# custom HM data type 'Digitaler Eingang'
Durch diese Änderungen wird der plotbare HomeMatic Datentyp ILLUMINATION für den HB-UNI-Sensor1 in den Datentyp 'Digitaler Eingang' umgewandelt.
So erhält man in der Geräteansicht *und* in der Diagramm-Legende einen Namen der besser zu einem beliebigen (binären) Status 0/1 passt.
Dieser Status ist in einem Diagramm plotbar (dies ist nicht mit allen HM Datentypen möglich).
Die Wahl fiel auf Datentyp ILLUMINATION weil dieser in den Device xml-Dateien der diversen HM Geräte kaum bzw. nicht verwendet wird.
Ein Abschnitt im install bzw. uninstall Script sorgt für die Änderungen, diese sind im Einzelnen:

neue Zeile:
/www/config/stringtable_de.txt
WEATHER|ILLUMINATION\t\t\${stringTableWeatherIllumination$JAVA_DEVICE_NAME}
// wichtig, Tabs müssen sein, nur mit space wird der String-Namen nicht eingelesen!

neue Zeile:
/www/webui/js/lang/de/translate.lang.stringtable.js
"stringTableWeatherIllumination$JAVA_DEVICE_NAME" : "Digitaler Eingang",

Ersetzen:
/www/webui/js/lang/de/translate.lang.diagram.js
"diagramValueTypeILLUMINATION": "Digitaler Eingang",


# Versionierung Firmware(Sketch) <-> CCU Addon xml (jp112sdl)
> Also die VERSION habe ich in der Addon source natürlich hochgesetzt.
Diese Versionsangabe dient lediglich der Anzeige in der WebUI, um den Anwender über den Versionsstand des jeweiligen Addons zu informieren.
===
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
