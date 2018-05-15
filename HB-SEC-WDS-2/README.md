
# Wassermelder (HB-SEC-WDS-2)

- Wassermelder mit Leitfähigkeitsmessung zwischen den Elektroden

- Demonstriert die Verwendung vom 'ThreeStateSensor' device type aus der AskSinPP Bibliothek mit einer anpassbaren Messroutine, in diesem Fall die Leitfähigkeitsmessung mit dem integrierten ADC

## Benötige Libraries

[AskSinPP Library](https://github.com/pa-pa/AskSinPP)</br>
[EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt)</br>
[Low-Power](https://github.com/rocketscream/Low-Power)

## Schaltung
![pic](Images/Schaltung.png)

## Aufbau Prototyp

![pic](Images/Prototyp.jpg)

## Das angemeldete Gerät im RaspberryMatic WebUI

![pic](Images/WebUI.png)

## Option 1: Step-Up Konverter zum Betrieb mit 1 Mignon AA Batterie oder Akku

MAX1724 Testaufbau<br>
Entscheidend hier sind die Eignung der Spule für den DC/DC Einsatz (DC-Nennstrom, DC-Widerstand, Güte), das Layout der Bauteile und natürlich kurze Signalwege.
![pic](Images/MAX1724.jpg)


Ausgangsspannung +3V @5mA, Restwelligkeit mit 10uF Keramikkondensator am Ausgang

![pic](Images/step-up_output_AC_with_10u.png)

Ausgangsspannung +3V @5mA, Restwelligkeit mit 10uF Keramikkondensator + 20uF Tantalelko am Ausgang

![pic](Images/step-up_output_AC_with_10u_20u.png)

Ausgangsspannung DC

![pic](Images/step-up_output_DC.png)

## Option 2: Echte Batteriezustandsmessung unter Last, um frühzeit leere Batterien zu erkennen und zu tauschen.

Dazu wird eine neue Batterieklasse nach Vorbild von papas Batterieklassen erstellt. Sie heißt hier Sensor_Battery und befindet sich unter Sensors/Sensor_Battery.h <br>
Mit dieser Klasse wird die Batterie oder der Akku für 200ms mit 75mA belastet. Momentan geschieht das 2 mal am Tag.<br>
Der Belastungsstrom lässt sich durch die Widerstände R5/R6 ändern, das Interval für die Messung in dieser Zeile:
`hal.battery.init(seconds2ticks(60UL*60*12), sysclock, 2000);
// 2x Batt.messung täglich, Spannungsteiler 1:2`

![pic](Images/Class_Sensor_Battery.jpg)
