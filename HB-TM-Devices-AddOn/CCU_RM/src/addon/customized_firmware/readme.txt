
Ab HB-TM-Devices-AddOn Version 2.51 habe ich die Möglichkeiten für benutzerspezifischen Daten bzw. ein benutzerspezifisches Datenlayout erweitert.
Neu ab dieser Version sind 5 weitere Sensoren-Templates HB-UNI-Sensor2..6 die man nach eigen Wünschen gestalten kann.
Diese haben neue Device Model IDs bekommen (F112..F116), eigene install/uninstall-Skripte sowie eigene Firmware-xml.

-----------------------------------------------------------
Ein Benutzer, der das Datenlayout eines Sensors ändern möchte, kann dies mit folgenden Schritten erreichen:

- hb-uni-sensorX.xml auf das gewünschte Datenlayout anpassen, siehe Beispiele hb-uni-sensor2.xml / hb-uni-sensor3.xml
- dazu passend die Senderoutine im Sketch HB-UNI-SensorX.ino ändern so dass die Payload zum xml passt, im init() der WeatherEventMsg
    class WeatherEventMsg : public Message {
    public:
      void init() {..
- weiterhin im Sketch HB-UNI-SensorX.ino sicherstellen das die richtige Device Model ID (F112..F116), passend zum xml, verwendet wird
- das neue hb-uni-sensorX.xml unter
  /usr/local/addons/hb-tm-devices-addon/customized_firmware
  ablegen
- Neustarten der Zentrale
- Anlernen des neuen Sensors

-----------------------------------------------------------
Das AddOn Skript sorgt dann dafür, dass die alternativen Firmware xml-Dateien bei Aktionen wie:

- AddOn Update-Installation,
- Einspielen eines RM/CCU Backups,
- Update der RM/CCU Firmware
wieder in das richtige Verzeichnis kopiert und in der Zentrale berücksichtigt werden.

-----------------------------------------------------------
Ich habe dazu ein paar kurze Beispiele gemacht:

- hb-uni-sensor2.xml
Hier ist der (oft nicht benötigte) Datenpunkt 'Ventilposition' (Digitaler Eingang) weggelassen, so dass dieser HB-UNI-Sensor2 nur 5 Datenpunkte hat.
Den Sketch (HB-UNI-Sensor2.ino) und die Payload darin braucht man dafür nicht unbedingt zu ändern.
siehe:
https://github.com/TomMajor/SmartHome/blob/master/HB-UNI-Sensor1/Images/HB-UNI-Sensor2_WebUI.png

- hb-uni-sensor3.xml
Hier ist der Datenpunkt 'Ventilposition' geändert in 'Wassertemperatur' um eine Frage aus dem Forum aufzugreifen.
Dazu habe ich ein passendes HB-UNI-Sensor3.bsp gemacht in dem als Beispiel diese Wassertemperatur mit 22,4C gesendet wird.
siehe:
https://github.com/TomMajor/SmartHome/blob/master/HB-UNI-Sensor1/Images/HB-UNI-Sensor3_WebUI.png
Dieser sketch liegt mit im /Arduino Verzeichnis. Er muss in .ino umbenannt werden und für die Arduino-IDE muss das Verzeichnis dann HB-UNI-Sensor3 heißen (Sketch-Name und Verzeichnisname müssen gleich sein).

- hb-uni-sensor4.xml
Ist identisch zu hb-uni-sensor2.xml, nur ein Dummy

- hb-uni-sensor5.xml
Ein neuer Datenpunkt für den UV-Sensor VEML6070 wurde ergänzt, der per define im Sketch unterstützt wird.
Der Datenpunkt 'Ventilposition' (Digitaler Eingang) wurde entfernt.

- hb-uni-sensor6.xml
Ein neuer Datenpunkt für den UV-Sensor VEML6075 wurde ergänzt, der per define im Sketch unterstützt wird.
Der Datenpunkt 'Ventilposition' (Digitaler Eingang) wurde entfernt.

-----------------------------------------------------------
Außerdem noch der Hinweis das bei Änderungen am xml immer eventuell vorhandene dazugehörigen Geräte mit dieser Device Model ID in der Zentrale abgelernt/gelöscht und
nach der xml Änderung wieder neu angelernt werden müssen.

-----------------------------------------------------------
Die verwendeten Device Model ID:

HB-UNI-Sensor1      0xF103
HB-UNI-Sensor2      0xF112
HB-UNI-Sensor3      0xF113
HB-UNI-Sensor4      0xF114
HB-UNI-Sensor5      0xF115
HB-UNI-Sensor6      0xF116
