#!/bin/sh

# String to include the functions.js for Programm/s
SSTRINGZWO="setFooter(s);"
RSTRINGZWO="setFooter(s); var scriptpp = document.createElement(\"script\"); scriptpp.type = \"text/javascript\"; scriptpp.src = \"/addons/print/functions.js\"; \$(\"body\").appendChild(scriptpp);"

# String to include the print button in single "Programm"
SSTRING="<td style='text-align:center; vertical-align:middle;'><div class='FooterButton CLASS04801' onclick='new HMScriptExecutor();'>\${footerBtnTestScript}</div></td>"
RSTRING="<td style='text-align:center; vertical-align:middle;'><div class='FooterButton CLASS04801' onclick='new HMScriptExecutor();'>\${footerBtnTestScript}</div></td><td style='text-align:center; vertical-align:middle;'><div class='FooterButton' onclick='PrintPage();'>Drucken</div></td>"

FNAME=/www/rega/pages/tabs/admin/views/programs.htm
FNAMESAVE=${FNAME%.htm}.save

sed -i "s|$RSTRING|$SSTRING|g" $FNAME
sed -i "s|$RSTRINGZWO|$SSTRINGZWO|g" $FNAME
