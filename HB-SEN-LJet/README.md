
# Füllstandsanzeige Öltank/Wassertank (HB-SEN-LJet)

- Kopplung der Projet LevelJET Ultraschall-Füllstandsanzeige mit HomeMatic für Rotex-Öltanks
- Anpassung an beliebige Tankformen durch Einsatz einer Peiltabelle möglich
- Volumenberechnung erfolgt Millimetergenau aus der Füllhöhe (Interpolation des Volumens zwischen den cm-Stützstellen)

![pic](Images/LevelJet_WebUI.jpg)


## Schaltung / Platine

![pic](Images/Blockschaltbild_HB-SEN-LJet.png)

[Schaltplan](https://github.com/TomMajor/SmartHome/tree/master/PCB/03_HB-SEN-LJet/Files/HB-SEN-LJet.pdf)

[Platine](https://github.com/TomMajor/SmartHome/tree/master/PCB/03_HB-SEN-LJet)


## Benötige Libraries

[AskSinPP Library](https://github.com/pa-pa/AskSinPP)</br>
[EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt)</br>
[Low-Power](https://github.com/rocketscream/Low-Power)


## CCU2/CCU3/RaspberryMatic Installation

Einstellungen/Systemsteuerung/Zusatzsoftware -> Datei CCU_RM/HB-SEN-LJet-addon.tgz installieren.


## Lizenz

**Creative Commons BY-NC-SA**<br>
Give Credit, NonCommercial, ShareAlike

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
