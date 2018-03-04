#!/bin/sh
rm HB-UNI-Sensor1_CCURM-addon.tgz
find . -name ".*" -exec rm -rf {} \;
cd HB-UNI-Sensor1_CCURM-addon-src
chmod +x update_script
tar -zcvf ../HB-UNI-Sensor1_CCURM-addon.tgz *
cd ..
