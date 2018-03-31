
## Universalsensor (HB-UNI-Sensor1)

- Demonstriert einen HomeMatic/RaspberryMatic/FHEM Universalsensor für Temperatur, Luftdruck, Luftfeuchte, Helligkeit usw.
- modifizierbar für andere Sensoren
- konfigurierbares Sendeintervall über WebUI
- konfigurierbare Höhe (für Berechnung des Luftdrucks auf Meeresniveau/Normaldruck) über WebUI
- über die defines am Anfang des Sketches können entweder Dummy Werte für die Sensoren (zum Testen der Anbindung an HomeMatic/RaspberryMatic/FHEM) oder reale Sensoren aktiviert werden

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

# Schaltung
![pic](Images/Schaltung.png)

# Prototyp
![pic](Images/Prototyp_HB-UNI-Sensor1.jpg)

# RaspberryMatic WebUI
![pic](Images/HB-UNI-Sensor1_WebUI.png)

![pic](Images/HB-UNI-Sensor1_Parameter.png)
