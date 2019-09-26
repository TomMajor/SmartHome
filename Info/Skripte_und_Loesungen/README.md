
# HomeMatic / RaspberryMatic Skripte und Lösungen


## Wiederherstellung der alten, nicht-alphabetische Menureihenfolge ab RaspberryMatic Ver. 3.47.18.20190918

Die Datei [color.map](https://github.com/TomMajor/SmartHome/tree/master/Info/Skripte_und_Loesungen/Files/color.map) nach /usr/local/etc/config kopieren und Permissions 0644 dafür setzen.<br>
Danach den Browsercache löschen.<br>
Referenzen:<br>
[706](https://github.com/jens-maus/RaspberryMatic/issues/706)<br>
[709](https://github.com/jens-maus/RaspberryMatic/pull/709)<br>


## "No internet connection detected" - Überwachung deaktivieren (obsolet ab RaspberryMatic Ver. 3.47.18.20190918) 

- Die Überwachung der Internetverbindung mittels monit hat in RaspberryMatic ab Version 3.45.5.20190330 Einzug gehalten.
- mein Router wird nachts automatisch ausgeschaltet, deswegen kann ich den dadurch entstehenen Alarm gar nicht brauchen da er unter Umständen andere und wichtigere Alarme auf meinem ePaper "überdeckt" (Anzahl der Servicemeldungen und Alarme als Dezimalzahlen unten rechts).<br>
[HB-Dis-EP-42BW](https://github.com/TomMajor/SmartHome/tree/master/HB-Dis-EP-42BW)
- ich habe mich mit dem Skript unten beholfen (gleichzeitig wird die Startzeit der Zentrale in eine Systemvariable CCU.StartTime geschrieben).
- CUxD/CMD_EXEC wie in den Bsp. braucht man dabei nicht zwingend. Man kann das auch mit system.Exec() aufrufen.
- Meine Versuche davor, den monit Befehl mittels Bootskript in /usr/local/etc/config/rc.d zu erledigen waren nicht erfolgreich, monit läuft zu diesem Zeitpunkt noch nicht, deswegen ein HomeMatic-Skript.
- Die Auslösebedingung für das Skript muss leer gelassen werden!
```
! -------------------------------------
! CCU.OnStart.hms
!
! Beim Reboot der CCU2 wird jedes Programm ausgeführt, das keine Bedingung hat.
! Ist gleichzeitig das Objekt "$src$" nicht definiert, handelt es sich sicher um einen Reboot und keinen z. B. manuellen Programmaufruf.
! Systemvariable CCU.StartTime muss existieren
!
object src = dom.GetObject("$src$");
if (!src) {
    string timeStr = system.Date("%d.%m.%Y %H:%M:%S");
    dom.GetObject("CCU.StartTime").State(timeStr);
    dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("monit unmonitor hasInternet");
}
```

![pic](Images/monit_webui.png)

![pic](Images/monit_status.png)
