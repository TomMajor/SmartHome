
set src=c:\Temp\JP-HB-Devices-addon-master\src\addon
set dest=e:\_user7\Documents\Arduino\_Repository\SmartHome\HB-TM-JP-AddOn-Reduced\CCU_RM\src\addon

rem install
xcopy /Y %src%\install_hb-dis-ep-xxx %dest%\
xcopy /Y %src%\install_hb-ou-mp3-led %dest%\
xcopy /Y %src%\install_hb-rc-12-ep %dest%\

rem uninstall
xcopy /Y %src%\uninstall_hb-dis-ep-xxx %dest%\
xcopy /Y %src%\uninstall_hb-ou-mp3-led %dest%\
xcopy /Y %src%\uninstall_hb-rc-12-ep %dest%\

rem xml
xcopy /Y %src%\firmware\rftypes\hb-dis-ep-42bw*.xml %dest%\firmware\rftypes\
xcopy /Y %src%\firmware\rftypes\hb-ou-mp3-led*.xml %dest%\firmware\rftypes\
xcopy /Y %src%\firmware\rftypes\hb-rc-12-ep*.xml %dest%\firmware\rftypes\

rem js, log
xcopy /Y /E %src%\js\* %dest%\js\
xcopy /Y /E %src%\log\* %dest%\log\

rem patches common
rem not needed: channels.fn.patch
rem not needed: datapointconfigurator.fn.patch
rem not needed: sliderControl.js.patch
xcopy /Y %src%\patch\common\functions.fn.patch %dest%\patch\common\
xcopy /Y %src%\patch\common\ic_common.tcl.patch %dest%\patch\common\
xcopy /Y %src%\patch\common\webui.js.patch %dest%\patch\common\

rem patches ge_345, le_343
xcopy /Y /E %src%\patch\ge_345\* %dest%\patch\ge_345\
xcopy /Y /E %src%\patch\le_343\* %dest%\patch\le_343\

rem tcl
xcopy /Y %src%\www\config\easymodes\HB-DIS-EP-42BW_dev_master.tcl %dest%\www\config\easymodes\
xcopy /Y %src%\www\config\easymodes\signal_hbchime_ch_link.tcl %dest%\www\config\easymodes\
xcopy /Y %src%\www\config\easymodes\signal_hbled_ch_link.tcl %dest%\www\config\easymodes\

rem js
xcopy /Y %src%\www\config\easymodes\MASTER_LANG\HB-DIS-EP-42BW_HELP.js %dest%\www\config\easymodes\MASTER_LANG\

rem SIGNAL_CHIMEHB, SIGNAL_LEDHB
xcopy /Y /E %src%\www\config\easymodes\SIGNAL_CHIMEHB\* %dest%\www\config\easymodes\SIGNAL_CHIMEHB\
xcopy /Y /E %src%\www\config\easymodes\SIGNAL_LEDHB\* %dest%\www\config\easymodes\SIGNAL_LEDHB\

rem png 250
xcopy /Y %src%\www\config\img\devices\250\hb-dis-ep-42bw*.png %dest%\www\config\img\devices\250\
xcopy /Y %src%\www\config\img\devices\250\hb-ou-mp3-led*.png %dest%\www\config\img\devices\250\
xcopy /Y %src%\www\config\img\devices\250\hb-rc-12-ep*.png %dest%\www\config\img\devices\250\

rem png 50
xcopy /Y %src%\www\config\img\devices\50\hb-dis-ep-42bw*.png %dest%\www\config\img\devices\50\
xcopy /Y %src%\www\config\img\devices\50\hb-ou-mp3-led*.png %dest%\www\config\img\devices\50\
xcopy /Y %src%\www\config\img\devices\50\hb-rc-12-ep*.png %dest%\www\config\img\devices\50\

rem icons_hm_dis_ep_wm55
xcopy /Y /E %src%\www\ise\img\icons_hm_dis_ep_wm55\* %dest%\www\ise\img\icons_hm_dis_ep_wm55\

rem htm
xcopy /Y %src%\www\rega\pages\msg\setHBOUCFMMode.htm %dest%\www\rega\pages\msg\setHBOUCFMMode.htm
