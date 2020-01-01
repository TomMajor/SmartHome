
# Meine AddOn


## AddOn Kompatibilität

| |AddOn|Status|letzte getestete Firmware (RaspberryMatic)|
|---|---|---|---|
|:heavy_check_mark:|HB-UNI-Sensor1|lauffähig unter CCU / RaspberryMatic|3.47.18.20191225|
|:heavy_check_mark:|HB-SEN-LJet|lauffähig unter CCU / RaspberryMatic|3.47.18.20190918|
|:heavy_check_mark:|HB-DIS-EP-42BW|lauffähig unter CCU / RaspberryMatic|3.47.18.20190918|


## AddOn Change Log

Dieser Changelog wird erst seit 2020 geführt, Änderungen vorher sind nur im git dokumentiert.

###### HB-UNI-Sensor1

- 2.04 - 01.01.2020
  - Im xml die Einträge für `<special_value id="NO_SENSOR" value=..` entfernt (wirkungslos, Danke Jerome)
  - Im xml den HUMIDITY Eintrag auf Bereich 0..100% geändert (% erzwingt Basis 100 in HM, Bereich 1..99% führte zu minimal falschen Kommastellen in ioBroker, Danke harvey)

