#!/bin/sh
rm HB-UNI-Sensor1_CCURM-addon.tgz
find . -name ".DS_Store" -exec rm -rf {} \;
cd HB-UNI-Sensor1_CCURM-addon-src
chmod +x update_script
chmod +x addon/installHBUNISENSOR1
chmod +x addon/update-check.cgi
chmod +x rc.d/hbunisensor1
tar -zcvf ../HB-UNI-Sensor1_CCURM-addon.tgz *
cd ..
