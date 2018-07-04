
## SensorTest_Lux

- Erster Test der digitalen Helligkeitssensoren TSL2561 und MAX44009 gegeneinander, der analoge Sensor GA1A12S202 läuft außer Komkurenz mit.
Messung war 18:30 abends, also noch keine maximalen Helligkeiten.
- Der TSL2561 hat ein Problem bei hohen Helligkeiten in der Sonne. Meine Sensorklasse gibt dann -1 zurück, ich habe die Stellen rot markiert.
Dies tritt in etwa bei ca. 30000 Lux auf, wie man im Diagramm sieht kann es aber auch bei kleineren Helligkeiten passieren. Eine Autoranging-Funktion, die zwischen verschiedenen Integrationszeiten umschalten kann, ist beim TSL2561 aktiv.
- Test-Sketch und MAX44009 Sensorklasse werden noch eingecheckt.

![pic](Messwerte1.png)

![pic](Hardware.jpg)
