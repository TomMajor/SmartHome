
# 3-Kanal-Funk-Schließerkontakt (HB-SCI-3-FM)

- von 'Position' abgeleitete Klasse 'OnePinPosition' für das Einlesen der Schließerkontakte
- für die 3 Eingänge werden im Sketch pull-up Widerstände konfiguriert
- Das Gerät HB-SCI-3-FM wird durch die Zentrale standardmäßig direkt beim Anlernen auf "gesicherte Übertragung" eingestellt. Damit dieser Eigenbau funktioniert müssen **alle 3 Kanäle** nach dem Anlernen auf "Standard Übertragung" umgestellt werden.<br>Oder man aktiviert AES für den Sketch:
[AskSinPP AES Support](https://github.com/pa-pa/AskSinPP#enable-aes-support)

## Schaltung

- nur ein normales AskSinPP Gerät (z.B. Arduino Pro Mini + CC1101 + LED + Config Taster)
- einfach 3 freie Pins für die Schließerkontakte verwenden

## Das angemeldete Gerät im RaspberryMatic WebUI

![pic](Images/HB_SCI_3_FM.png)

## Benötige Libraries

[AskSinPP Library](https://github.com/pa-pa/AskSinPP)</br>
[EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt)</br>
[Low-Power](https://github.com/rocketscream/Low-Power)
