
Im HB-UNI-Sensor1 können ab Firmware 0x13 zwei extra Byte 'customData' in der Payload mit benutzerspezifischen Daten belegt 
und mit einer alternativen Firmware xml-Datei der Zentrale zur Verfügung gestellt werden.

Diese alternative Firmware xml-Datei muss für die Behandlung von 'customData' angepasst werden und im Verzeichnis
/usr/local/addons/hb-tm-devices-addon/customized_firmware
liegen.

Das AddOn Skript sorgt dann dafür, dass die alternativen Firmware xml-Dateien bei Aktionen wie
- AddOn Update-Installation,
- Einspielen eines RM/CCU Backups,
- Update der RM/CCU Firmware
wieder in das richtige Verzeichnis kopiert und in der Zentrale berücksichtigt werden.

Ein Beispiel für eine alternative Firmware xml-Datei, die zusätzlich den gemessenen UV-Index zur Verfügung stellt, liegt im Verzeichnis
/usr/local/addons/hb-tm-devices-addon/customized_firmware_bsp.

Zur Aktivierung muss das Verzeichnis in customized_firmware umbenannt und anschließend die Zentrale neugestartet werden.

Danach (und bei jeder weiteren Änderung in der alternativen Firmware xml-Datei) muss das Gerät abgelernt/gelöscht und wieder neu angelernt werden!
Nur so werden die xml Änderungen in die Zentrale übernommen.

-----

Beispiele 'customData' für UV-Index vom VEML6070/VEML6075 in der xml-Datei:

VEML6070:
...
<parameter id="UV-Index" operations="read,event">
  <logical type="integer" min="0" max="11" />
  <physical type="integer" interface="command" value_id="UV-Index" no_init="true">
    <event frame="WEATHER_EVENT" />
  </physical>
</parameter>
...
<parameter type="integer" index="22" size="0.4" param="UV-Index" />

VEML6075:
...
<parameter id="UV-Index" operations="read,event">
  <logical type="float" min="0" max="110" />
  <physical type="integer" interface="command" value_id="UV-Index" no_init="true">
    <event frame="WEATHER_EVENT" />
  </physical>
  <conversion type="float_integer_scale" factor="10.0" />
</parameter>
...
<parameter type="integer" index="22" size="1.0" param="UV-Index" />
