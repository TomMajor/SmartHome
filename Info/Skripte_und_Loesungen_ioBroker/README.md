
# ioBroker - Skripte und Lösungen


## ioBroker zeigt bestimmte Datenpunkte eines Sensors nicht an

- Dies kann zum Beispiel passieren wenn ein Homebrew-Sensor ein Update für neue Datenpunkte bekommen hat. Nach meinen Erfahrungen bekommt der ioBroker nie automatisch eine geänderte Sensordaten-Konfiguration mit.

- Mögliche Lösungen:

1. Zunächst immer den Sensor in der Zentrale ablernen und neu anlernen.

2. Dann den fraglichen Sensor unter ioBroker/Objekte innerhalb von hm-rpc.meta löschen.<br>
Falls hm-rpc.meta nicht sichtbar ist, den Button oben "Wechseln Sie in die Statusansicht" (Viereck mit einer 1 drin) ggf. mehrmals umschalten.

![pic](Images/ioBroker1.png)

3. Nach dem Löschen des Sensors in hm-rpc.meta die Geräte neu synchronisieren lassen (ioBroker/Instanzen, hm-rpc.0 konfigurieren, "Geräte neu einlesen" aktivieren).

4. Alternativ kann man versuchen, die fehlenden Datenpunkte mit dem richtigen Namen selber neu anzulegen (nicht getestet, bei mir funktionierte immer die Meta Methode).
