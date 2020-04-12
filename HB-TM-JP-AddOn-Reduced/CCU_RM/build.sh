#!/bin/sh
#COPYFILE_DISABLE=1; export COPYFILE_DISABLE
ADDON_NAME=hb-tm-jp-addon-reduced

rm ${ADDON_NAME}.tgz
cd src
chmod 755 update_script
chmod 755 addon/install_*
chmod 755 addon/uninstall_*
chmod 755 addon/update-check.cgi
chmod 755 addon/patchworker
chmod 755 addon/epaper42.tcl
chmod 755 rc.d/*
#find . -name ".DS_Store" -exec rm -rf {} \;
#find . -name "._*" -exec rm -rf {} \;
#dot_clean .
tar --exclude='untitled.txt' -zcf ../${ADDON_NAME}.tgz *
echo "tar created."
cd ..
