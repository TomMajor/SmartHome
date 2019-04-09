
Im HB-UNI-Sensor1 können ab Firmware 0x13 zwei extra Byte 'customData' in der Payload mit benutzerspezifischen Daten belegt 
und mit einer alternativen Firmware xml-Datei der Zentrale zur Verfügung gestellt werden.

Diese alternative Firmware xml-Datei muss für die Behandlung von 'customData' angepasst werden und im Verzeichnis
/usr/local/addons/hb-uni-sensor1/custom_firmware
liegen.

Das AddOn Skript sorgt dann dafür, dass die alternativen Firmware xml-Dateien bei Aktionen wie
- AddOn Update-Installation,
- Einspielen eines RM/CCU Backups,
- Update der RM/CCU Firmware
wieder in das richtige Verzeichnis kopiert und in der Zentrale berücksichtigt werden.

Ein Beispiel für eine alternative Firmware xml-Datei, die zusätzlich den gemessenen UV-Index zur Verfügung stellt, liegt im Verzeichnis
/usr/local/addons/hb-uni-sensor1/custom_firmware_bsp.

Zur Aktivierung muss das Verzeichnis in custom_firmware umbenannt und anschließend die Zentrale neugestartet werden.

Danach (und bei jeder weiteren Änderung in der alternativen Firmware xml-Datei) muss das Gerät abgelernt/gelöscht und wieder neu angelernt werden!
Nur so werden die xml Änderungen in die Zentrale übernommen.
