
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


## ioBroker Backup

- Credits an Looxer01 für wertvolle Anregungen aus seinem Backup-Konzept.

- An meinem ioBroker Raspberry Pi 3B ist eine USB-SSD angeschlossen auf die ich die Ablage der History-Daten verschoben habe um die SD-Karte bezüglich ihrer Schreiblast zu schonen.

- Auf die gleiche SSD lege ich 1x pro Woche ein gepacktes Backup vom ioBroker-Verzeichnis mit allen ioBroker Daten und Einstellungen sowie zusätzlich die (verschobenen) History-Daten ab.

- Das ist der Backup Ablageort Nummer 1, die USB-SSD. Zusätzlich hole ich per Skript die Backups wöchentlich auf den PC bzw. das NAS um einen zusätzlichen Backup Ablageort zu haben.

###### Vorbereitung

```
# Vorhandensein von rsync prüfen
rsync --version
# falls nicht vorhanden nachinstallieren mit sudo apt-get install rsync

# temporäres Verzeichnis für die Backup-Operationen mit rsync erstellen
sudo mkdir /backup_tmp
sudo chown -R pi:pi /backup_tmp
sudo chmod 775 /backup_tmp

# das eigentliche Backup-Verzeichnis erstellen
mkdir /media/usb/backup
sudo chown -R pi:pi /media/usb/backup
sudo chmod 775 /media/usb/backup

# Backup-Skript erstellen
cd ~
touch backup_iobroker.sh
nano backup_iobroker.sh
# entweder a) den Inhalt vom Backup-Skript unten im Nano-Editor kopieren, dann mit Ctrl-X Save/Exit
# oder b) die Datei backup_iobroker.sh (auf github unter /Files abgelegt) direkt nach /home/pi/ kopieren
chmod +x backup_iobroker.sh

# Backup Aufabenplanung
# Backup bei mir jeden Montag um 00:05
# https://crontab.guru/#5_0_*_*_1
crontab –e
5 0 * * 1 /home/pi/backup_iobroker.sh
```

###### Backup-Skript

```
#!/bin/bash

cd /opt/iobroker
iobroker stop
sleep 5

# jetzt wird das iobroker Verzeichnis in das backup_tmp/iobroker Verzeichnis kopiert (mirror mode)
rsync --delete -aLvzh -P /opt/iobroker /backup_tmp/

# das gleiche für die iobroker History die bei mir separat auf einer USB-SSD liegt
# standardmässig ist sie unter /opt/iobroker/iobroker-data/history zu finden und wird vom ersten rsync command oben mitgesichert
rsync --delete -aLvzh -P /media/usb/history /backup_tmp/

# iobroker sobald als möglich wieder starten
#cd /opt/iobroker
iobroker start

# Erzeugen des Archivs in komprimierter Form, Ablegen in /media/usb/backup
DATE=$(date +"%Y%m%d-%H%M%S")
# use relative paths for tar, see "tar: Removing leading '/' from member names"
cd /
tar -zcf media/usb/backup/iobroker-$DATE.tgz backup_tmp/iobroker
tar -zcf media/usb/backup/history-$DATE.tgz backup_tmp/history

# ToDo alte Backups löschen, kommt demnächst

# Optional
# Zyklisch den Inhalt von /media/usb/backup als extra Backup auf einen PC, NAS etc. kopieren
# Beispiel:
# smbcredentials erzeugen (einmal)
#   sudo touch ~/.smbcredentials
#   sudo chown -R pi:pi ~/.smbcredentials
#   sudo chmod 600 ~/.smbcredentials
#   nano ~/.smbcredentials
#       username=...
#       password=...
# ggf. cifs-utils installieren (einmal)
#   sudo apt-get install cifs-utils
# mount point erstellen (einmal)
#   sudo mkdir /mnt/local_share
# mounten/kopieren/unmounten (zyklisch)
#   sudo mount -t cifs -o credentials=~/.smbcredentials //<IP-Address>/<Export-Share-Name> /mnt/local_share
#   cp media/usb/backup/iobroker-$DATE.tgz /mnt/local_share
#   cp media/usb/backup/history-$DATE.tgz /mnt/local_share
#   sudo umount /mnt/local_share
```


## SSH-Login mittels keyfile

- .ssh Verzeichnis im homedir anlegen falls noch nicht vorhanden
```
cd ~
mkdir .ssh
chmod 700 .ssh
```
- mittels puttygen ein Schlüsselpaar erzeugen
- Achtung, dabei für das public keyfile nicht das Putty Save Format verwenden (Button "Save public key"), sondern in puttygen den key direkt aus der Box "Public key for pasting into OpenSSH authorized_keys file" kopieren und in eien neue Datei (z.B. iobroker.pub) schreiben!
- public keyfile iobroker.pub nach ~/.ssh/ kopieren, z.B. mittels WinSCP
```
cd .ssh
cat iobroker.pub >> authorized_keys
chmod 600 .ssh/authorized_keys
rm iobroker.pub
```
- anschliessend kann das SSH-Login ohne Password unter Verwendung des private keyfiles erfolgen

