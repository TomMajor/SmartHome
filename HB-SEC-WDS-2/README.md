
## Wassermelder (HB-SEC-WDS-2)

- Demonstriert die Verwendung vom ThreeStateSensor device type mit einer anpassbaren Messroutine, in diesem Fall eine Leitfähigkeitsmessung zwischen den Elektroden mit ADC.

# Benötige Libraries

[AskSinPP Library](https://github.com/pa-pa/AskSinPP)</br>
[EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt)</br>
[Low-Power](https://github.com/rocketscream/Low-Power)

# Schaltung
![pic](Images/Schaltung.png)

# Aufbau Prototyp

![pic](Images/Prototyp.jpg)

# Option: Step-Up Konverter zum Betrieb mit 1 Batterie oder einem Akku AAA

MAX1724 Testaufbau
![pic](Images/MAX1724.jpg)

Ausgangsspannung Restwelligkeit mit 10uF Keramikkondensator an +3V
![pic](Images/step-up_output_AC_with_10u.png)

Ausgangsspannung Restwelligkeit mit 10uF Keramikkondensator + 20uF Tantalelko an +3V
![pic](Images/step-up_output_AC_with_10u_20u.png)

Ausgangsspannung DC
![pic](Images/step-up_output_DC.png)
