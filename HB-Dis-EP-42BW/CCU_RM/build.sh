#!/bin/sh
#COPYFILE_DISABLE=1; export COPYFILE_DISABLE
ADDON_NAME=hb-dis-ep-42bw

#wget -q -O ./patchsource/www/webui/webui.js.orig https://github.com/eq-3/occu/raw/master/WebUI/www/webui/webui.js
#wget -q -O ./patchsource/www/webui/webui.js.orig https://github.com/eq-3/occu/raw/b_3_41/WebUI/www/webui/webui.js
#echo "wget webui.js      exited with $?"
#wget -q -O ./patchsource/www/rega/esp/functions.fn.orig https://raw.githubusercontent.com/eq-3/occu/master/WebUI/www/rega/esp/functions.fn
#echo "wget functions.fn  exited with $?"
#wget -q -O ./patchsource/www/rega/esp/side.inc.orig https://raw.githubusercontent.com/eq-3/occu/master/WebUI/www/rega/esp/side.inc
#echo "wget side.inc      exited with $?"
#wget -q -O ./patchsource/www/rega/pages/tabs/admin/views/programs.htm.orig https://raw.githubusercontent.com/eq-3/occu/master/WebUI/www/rega/pages/tabs/admin/views/programs.htm
#echo "wget programs.htm  exited with $?"
#wget -q -O ./patchsource/www/config/ic_common.tcl.orig https://raw.githubusercontent.com/jens-maus/RaspberryMatic/master/buildroot-external/patches/occu/0000-CCU-Firmware-Diff/occu/WebUI/www/config/ic_common.tcl
#echo "wget ic_common.tcl exited with $?"

#diff -Naur ./patchsource/www/rega/esp/side.inc.orig ./patchsource/www/rega/esp/side.inc > ./src/addon/patch/hb-dis-ep-42bw.patch
#diff -Naur ./patchsource/www/rega/esp/functions.fn.orig ./patchsource/www/rega/esp/functions.fn >> ./src/addon/patch/hb-dis-ep-42bw.patch
#diff -Naur ./patchsource/www/webui/webui.js.orig ./patchsource/www/webui/webui.js >> ./src/addon/patch/hb-dis-ep-42bw.patch
#diff -Naur ./patchsource/www/rega/pages/tabs/admin/views/programs.htm.orig ./patchsource/www/rega/pages/tabs/admin/views/programs.htm >> ./src/addon/patch/hb-dis-ep-42bw.patch
#diff -Naur ./patchsource/www/config/ic_common.tcl.orig ./patchsource/www/config/ic_common.tcl >> ./src/addon/patch/hb-dis-ep-42bw.patch
#diff -Naur ./patchsource/www/rega/esp/datapointconfigurator.fn.orig ./patchsource/www/rega/esp/datapointconfigurator.fn >> ./src/addon/patch/hb-dis-ep-42bw.patch
#diff -Naur ./patchsource/www/webui/js/lang/de/translate.lang.js.orig ./patchsource/www/webui/js/lang/de/translate.lang.js >> ./src/addon/patch/hb-dis-ep-42bw.patch

#echo "Patch created."

rm ${ADDON_NAME}-addon.tgz
cd src
chmod +x update_script
chmod +x addon/install_*
chmod +x addon/uninstall_*
chmod +x addon/update-check.cgi
chmod +x rc.d/*
#find . -name ".DS_Store" -exec rm -rf {} \;
#find . -name "._*" -exec rm -rf {} \;
#dot_clean .
tar --exclude='untitled.txt' -zcf ../${ADDON_NAME}-addon.tgz * 
echo "tar created."
cd ..
