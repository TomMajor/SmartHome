
# Warnung vor dem Flashen von 3,3V Geräten mit USBasp Klones

:exclamation: AVR und CC1101 können bei 3,3V durch Einsatz eines USBasp Klones zerstört werden :exclamation:

Die weit verbreiteten und billigen (1€) USBasp Klones haben einen Jumper zur 3,3V / 5V Umschaltung.<br>
Leider ist diese Umschaltung nur für die Versorgung des (Programmier-)Ziels zuständig, nicht für die Betriebsspannung des Controllers auf dem USBasp!<br>
Dies führt dazu dass der zu programmierende AVR zwar mit 3,3V betrieben wird, die Programmierleitungen SCK/MISO/MOSI/RESET führen jedoch 5V Pegel was ganz klar eine Verletzung der Spezifikation für den zu programmierenden AVR ist.<br>
Das Gleiche gilt für einen angeschlossenen CC1101.<br>
Es besteht somit die Gefahr, sowohl AVR als auch CC1101 durch Einsatz eines USBasp Klones zu zerstören.<br>
Ich habe Berichte von Usern gelesen wo das Programmieren gut ging, aber sicher ist das nicht. Es gibt auch Berichte wo der USBasp den AVR zerstört hat. :fire:<br>
Der 3,3V Jumper suggeriert dass alles Bestens ist, täuscht damit aber den Benutzer.<br>
**Aus diesen Gründen rate ich vom Einsatz eines USBasp Klones in 3,3V Umgebungen ab.**<br>
Der Thread zu diesem Thema:
[HM Forum](https://homematic-forum.de/forum/viewtopic.php?f=76&t=47361&start=50#p480173)

Absolute Maximum Ratings **ATmega328**

![pic](Images/Absolute_Maximum_Ratings_AVR.png)

<br>

Absolute Maximum Ratings **CC1101**

![pic](Images/Absolute_Maximum_Ratings_CC1101.png)
