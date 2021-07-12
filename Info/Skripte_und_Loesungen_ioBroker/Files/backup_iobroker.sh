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
