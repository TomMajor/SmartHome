
# Warnung vor dem Flashen von 3,3V Geräten mit USBasp Klones

:exclamation: AVR und CC1101 können bei 3,3V durch Einsatz eines USBasp Klones zerstört werden :exclamation:

Die weit verbreiteten und billigen (1€) USBasp Klones haben einen Jumper zur 3,3V / 5V Umschaltung.<br>
Leider ist diese Umschaltung nur für die Versorgung des (Programmier-)Ziels zuständig, nicht für die Betriebsspannung des Controllers auf dem USBasp!<br><br>
Dies führt dazu dass der zu programmierende AVR zwar mit 3,3V betrieben wird, die Programmierleitungen SCK/MISO/MOSI führen jedoch 5V Pegel was ganz klar eine Verletzung der Spezifikation für den zu programmierenden AVR ist.<br>
Das Gleiche gilt für einen angeschlossenen CC1101.<br><br>
Es besteht somit die Gefahr, sowohl AVR als auch CC1101 durch Einsatz eines USBasp Klones zu zerstören.<br>
Ich habe Berichte von Usern gelesen wo das Programmieren gut ging, aber sicher ist das nicht.<br>
Es gibt Berichte wo der USBasp den AVR zerstört hat. :fire:<br>
Zwei Beispiele:<br>

HomeMatic user *Alveran*:
> Nein auf die restlichen komm ich auch mit dem Diamex nicht drauf. Die hat wohl der USBASP gegrillt.<br>
> **Also noch mal ganz große Warnung vor USBASP.**<br>
> Ich muss erstmal sehen wie ich die defekten runter gelötet bekomme.<br>
[Quelle: HM Forum](https://homematic-forum.de/forum/viewtopic.php?f=76&t=55262&start=30#p549991)

FHEM user *maclovlin*
> man kann wirklich Pech mit dem USBasp oder Arduino UNO ISP haben, so<br>
das die ATMega's gegrillt werden ( im Nachhinein Fällt mir ein das ich auch einen 3.3V Arduino Pro Mini mit dem Uno gekillt habe, da rührt sich nix mehr ).<br>
[Quelle: FHEM Forum](https://forum.fhem.de/index.php/topic,71413.msg1005356.html#msg1005356)

---<br>

Der 3,3V Jumper suggeriert dass alles Bestens ist, täuscht damit aber den Benutzer.<br>
:fire: **Aus diesen Gründen rate ich vom Einsatz eines USBasp Klones in 3,3V Umgebungen ab.** :fire:<br><br>
Der Thread zu diesem Thema im [HM Forum](https://homematic-forum.de/forum/viewtopic.php?f=76&t=47361&start=50#p480173)

## Absolute Maximum Ratings **ATmega328**

![pic](Images/Absolute_Maximum_Ratings_AVR.png)

<br>

## Absolute Maximum Ratings **CC1101**

![pic](Images/Absolute_Maximum_Ratings_CC1101.png)
