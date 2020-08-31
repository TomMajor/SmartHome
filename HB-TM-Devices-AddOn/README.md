
# HB-TM-Devices-AddOn


## Unterstützte Geräte

- [HB-UNI-Sensor1](https://github.com/TomMajor/SmartHome/tree/master/HB-UNI-Sensor1)
- [HB-UNI-Sensor2..6](https://github.com/TomMajor/SmartHome/tree/master/HB-UNI-Sensor1#benutzerspezifische-sensordaten) (Benutzerspezifische Sensordaten)
- [HB-SEN-LJet](https://github.com/TomMajor/SmartHome/tree/master/HB-SEN-LJet)
- [HB-UNI-Sensor-Heizung](https://github.com/TomMajor/SmartHome/tree/master/HB-UNI-Sensor-Heizung)

[comment]: # (rc.d filename: hb-tm-devices-addon, github name: HB-TM-Devices-AddOn, info name: HB-TM-Devices AddOn)

## Change Log

- 2.52 - 31.08.2020
  - Bugfix HB-UNI-Sensor6, UV-Index beim Sensor VEML6075 wurde im WebUI nicht korrekt angezeigt

- 2.51 - 01.08.2020
  - Unterstützung für [HB-UNI-Sensor2..6](https://github.com/TomMajor/SmartHome/tree/master/HB-UNI-Sensor1#benutzerspezifische-sensordaten) (Benutzerspezifische Sensordaten)

- 2.50 - 17.05.2020
  - das neue HB-TM-Devices-AddOn orientiert sich an Jérômes JP-HB-Devices-addon und fasst die Unterstützung für meine HomeBrew-Geräte in einem AddOn zusammen.
  - in der ersten Version unterstützt das AddOn die Geräte HB-UNI-Sensor1, HB-SEN-LJet und HB-UNI-Sensor-Heizung.
  - die bisherigen separaten AddOn für diese Geräte sind dadurch hinfällig und werden nicht mehr supported.
  - :warning: bitte unbedingt vor Installation des HB-TM-Devices-AddOn die alten HB-UNI-Sensor1 AddOn und HB-SEN-LJet AddOn deinstallieren, falls vorhanden!
  - HB-UNI-Sensor-Heizung hat ab sofort ein eigenes Device Model bekommen (0xF105) da ich im xml einen neuen Datenpunkt *STATE* eingeführt habe.


## Eingestellte AddOn

###### Change Log HB-UNI-Sensor1 AddOn (nicht mehr supported)

- 2.04 - 01.01.2020
  - Im xml die Einträge für `<special_value id="NO_SENSOR" value=..` entfernt (wirkungslos, Danke Jérôme).
  - Im xml den HUMIDITY Eintrag auf Bereich 0..100% geändert (% erzwingt Basis 100 in HM, Bereich 1..99% führte zu minimal falschen Kommastellen in ioBroker, Danke harvey).

- (2.50) - 17.05.2020
  - Das AddOn wird nicht mehr fortgeführt. Bitte das neue HB-TM-Devices-AddOn verwenden.

###### Change Log HB-SEN-LJet AddOn (nicht mehr supported)

- (2.50) - 17.05.2020
  - Das AddOn wird nicht mehr fortgeführt. Bitte das neue HB-TM-Devices-AddOn verwenden.
