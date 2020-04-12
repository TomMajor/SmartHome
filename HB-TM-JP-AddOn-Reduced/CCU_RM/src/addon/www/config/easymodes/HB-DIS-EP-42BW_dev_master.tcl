#!/bin/tclsh
source [file join /www/config/easymodes/em_common.tcl]

set PROFILES_MAP(0)  "Experte"
set PROFILES_MAP(1)  "TheOneAndOnlyEasyMode"

proc getHelpIcon {topic x y} {
  set ret "<img src=\"/ise/img/help.png\" style=\"cursor: pointer; width:18px; height:18px; position:relative; top:2px\" onclick=\"showParamHelp('$topic', '$x', '210')\">"
  return $ret
}

proc getIconOverviewIcon {topic x y} {
  set ret "<img src=\"/ise/img/visible.png\" style=\"cursor: pointer; width:18px; height:18px; position:relative; top:2px\" onclick=\"showParamHelp('$topic', '$x', '210')\">"
  return $ret
}

proc getCheckBox {param value prn} {
  set checked ""
  if { $value } then { set checked "checked=\"checked\"" }
  set s "<input id='separate_DEVICE_$prn' type='checkbox' $checked value='dummy' name=$param/>"
  return $s
}

proc getComboBox {arOptions prn param val} {
  upvar $arOptions options

  set s "<select id=\"separate_DEVICE\_$prn\" name=$param>"
  foreach value [lsort -real [array names options]] {
    set selected ""
    if {$value == $val} {set selected "selected=\"selected\""}
    append s "<option value=$value $selected>$options($value)</option>"
  }
  append s "</select>"
  return $s
}

proc getMinMaxValueDescr {param} {
  global psDescr
  upvar psDescr descr
  array_clear param_descr
  array set param_descr $descr($param)
  set min $param_descr(MIN)
  set max $param_descr(MAX)

  # Limit float to 2 decimal places
  if {[llength [split $min "."]] == 2} {
    set min [format {%1.2f} $min]
    set max [format {%1.2f} $max]
  }
  return "($min - $max)"
}

proc getTextField {prn param val} {
  global psDescr
  upvar psDescr descr
  array_clear param_descr
  array set param_descr $descr($param)
  set minValue [format {%1.1f} $param_descr(MIN)]
  set maxValue [format {%1.1f} $param_descr(MAX)]

  set elemId 'separate_DEVICE\_$prn'
  # Limit float to 2 decimal places
  if {[llength [split $val "."]] == 2} {
    set val [format {%1.2f} $val]
  }

  set s "<input id=$elemId type=\"text\" size=\"5\" value=$val name=$param onblur=\"ProofAndSetValue(this.id, this.id, $minValue, $maxValue, 1)\">"
  return $s
}

proc set_htmlParams {iface address pps pps_descr special_input_id peer_type} {
  global iface_url psDescr
  
  upvar PROFILES_MAP  PROFILES_MAP
  upvar HTML_PARAMS   HTML_PARAMS
  upvar PROFILE_PNAME PROFILE_PNAME
  upvar $pps          ps

  set hlpBoxWidth 450
  set hlpBoxHeight 160

  puts "<script type=\"text/javascript\">load_JSFunc('/config/easymodes/MASTER_LANG/HB-DIS-EP-42BW_HELP.js')</script>"

  array set psDescr [xmlrpc $iface_url($iface) getParamsetDescription [list string $address] [list string MASTER]]
  array set devDescr [xmlrpc $iface_url($iface) getDeviceDescription [list string $address]]                                                       
  set DEVICE $devDescr(TYPE) 
  
  append HTML_PARAMS(separate_1) "<table class=\"ProfileTbl\">"
  set prn 1
  set param DISPLAY_INVERTING
  append HTML_PARAMS(separate_1) "<tr>"
    append HTML_PARAMS(separate_1) "<td>\${stringTableDisplayInverting}</td>"
    append HTML_PARAMS(separate_1) "<td>"
     append HTML_PARAMS(separate_1) "[getCheckBox '$param' $ps($param) $prn]"
    append HTML_PARAMS(separate_1) "</td>"
  append HTML_PARAMS(separate_1) "</tr>"

  incr prn
  set param DEVICE_LED_MODE
  append HTML_PARAMS(separate_1) "<tr>"
    array_clear options
    set options(0) "\${stringTableDeviceLEDModeOff}"
    set options(1) "\${stringTableDeviceLEDModeOn}"
    append HTML_PARAMS(separate_1) "<td>\${stringTableDeviceLEDMode}</td>"
    append HTML_PARAMS(separate_1) "<td>[getComboBox options $prn '$param' $ps($param)]</td>"
  append HTML_PARAMS(separate_1) "</tr>"

  if { $DEVICE == "HB-DIS-EP-42BW" } {
    incr prn
    set param LOW_BAT_LIMIT
    append HTML_PARAMS(separate_1) "<tr>"
      append HTML_PARAMS(separate_1) "<td>\${stringTableBatteryLowBatLimit}</td>"
      append HTML_PARAMS(separate_1) "<td>[getTextField $prn $param $ps($param)]&nbsp;V&nbsp;[getMinMaxValueDescr $param]</td>"
    append HTML_PARAMS(separate_1) "</tr>"
  }
  
  if { $DEVICE == "HB-DIS-EP-42BW" } {
    incr prn
    set param HB_CRITICAL_BAT_LIMIT
    append HTML_PARAMS(separate_1) "<tr>"
      append HTML_PARAMS(separate_1) "<td>\${stringTableHbCriticalBatLimit}</td>"
      append HTML_PARAMS(separate_1) "<td>[getTextField $prn $param $ps($param)]&nbsp;V&nbsp;[getMinMaxValueDescr $param]</td>"
    append HTML_PARAMS(separate_1) "</tr>"
  }
  
  incr prn
  set param HB_DISPLAY_REFRESH_WAIT_TIME
  append HTML_PARAMS(separate_1) "<tr>"
    append HTML_PARAMS(separate_1) "<td>\${stringTableHbDisplayRefreshWaitTime}</td>"
    append HTML_PARAMS(separate_1) "<td>[getTextField $prn $param $ps($param)]&nbsp;s&nbsp;[getMinMaxValueDescr $param]&nbsp;[getHelpIcon $param $hlpBoxWidth $hlpBoxHeight]</td>"
  append HTML_PARAMS(separate_1) "</tr>"
  
  incr prn
  set param POWERUP_ACTION
  append HTML_PARAMS(separate_1) "<tr>"
    array_clear options
    set options(0) "\${stringTablePowerUpOFF}"
    set options(1) "\${stringTableKeyPressShort}"
    set options(2) "\${stringTableKeyPressLong}"
    append HTML_PARAMS(separate_1) "<td>\${stringTableDimmerPowerUpAction}</td>"
    append HTML_PARAMS(separate_1) "<td>[getComboBox options $prn '$param' $ps($param)]</td>"
  incr prn
  set param KEY_TRANSCEIVER
      array_clear options
    set options(0) "1"
    set options(1) "2"
    set options(2) "3"
    set options(3) "4"
    set options(4) "5"
    set options(5) "6"
    set options(6) "7"
    set options(7) "8"
    set options(8) "9"
    set options(9) "10"
    append HTML_PARAMS(separate_1) "<td>\${stringTableKeyTranseiverTitle}</td>"
    append HTML_PARAMS(separate_1) "<td>[getComboBox options $prn '$param' $ps($param)]&nbsp;(1-10)&nbsp;[getHelpIcon $param $hlpBoxWidth $hlpBoxHeight]</td>"
    
  append HTML_PARAMS(separate_1) "</tr>"
  
  append HTML_PARAMS(separate_1) "<tr>"
    append HTML_PARAMS(separate_1) "<td>\${stringTableHbIconOverview}</td>"
    append HTML_PARAMS(separate_1) "<td>&nbsp;[getIconOverviewIcon AVAILABLE_ICONS $hlpBoxWidth $hlpBoxHeight]</td>"
  append HTML_PARAMS(separate_1) "</tr>"

  append HTML_PARAMS(separate_1) "</table>"
}
constructor
