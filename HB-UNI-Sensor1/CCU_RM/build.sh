#!/bin/sh
source ./src/addon/params 
rm ${DEVICE}-addon.tgz
#find . -name ".DS_Store" -exec rm -rf {} \;
cd src
echo ${ADDON_VERSION} > ./addon/VERSION

rm ./addon/update-check.cgi
if [ -n "${CHECK_URL}" ]; then
echo "#!/bin/tclsh" > ./addon/update-check.cgi
echo "set checkURL    \"${CHECK_URL}\"" >> ./addon/update-check.cgi
echo "set downloadURL \"${DOWNLOAD_URL}\"" >> ./addon/update-check.cgi
cat ../update-check.cgi.skel >> ./addon/update-check.cgi
fi 

#find ./rc.d/ -type f -exec mv {} ./rc.d/${ADDON_NAME} \;
        
chmod +x update_script
chmod +x addon/install*
chmod +x addon/uninstall*
chmod +x addon/update-check.cgi
chmod +x addon/functions
#chmod +x addon/extra
chmod +x rc.d/*

tar -zcvf ../${DEVICE}-addon.tgz *
cd ..
