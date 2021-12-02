#!/bin/tclsh
#
# -------------------------------------
# TCL Skript für HB-ES-S0-CNT6
# Version 0.20
# 2019-08-30 Tom Major (Creative Commons)
# https://creativecommons.org/licenses/by-nc-sa/4.0/
# You are free to Share & Adapt under the following terms:
# Give Credit, NonCommercial, ShareAlike
# -------------------------------------
# Die folgenden HomeMatic Systemvariablen müssen für diesen TCL Skript angelegt werden:
# STROM.Cnt1 .. STROM.Cnt6  Typ: Zahl, Bereich: 0..2000000000
# STROM.ErrCnt              Typ: Zahl, Bereich: 0..2000000000
# STROM.ErrMsg              Typ: String
#
# Beschreibung siehe STROM.Update.hms
# ---------------------------------------------------------

# =========================================================
# START benutzerspezifische Konfiguration
set cDEVICE_IP      "192.168.1.227"
set cCHANNEL_COUNT  5
# ENDE benutzerspezifische Konfiguration
# =========================================================

load tclrega.so

# -------------------------------------
proc main {argc argv} {
    global cDEVICE_IP
	global cCHANNEL_COUNT
    
    # first test if device reachable, 2x ping
    puts "Trying to reach $cDEVICE_IP"
    set ipReachable 0
    catch {
        set pingRet [exec ping -c 2 -W 1 $cDEVICE_IP]
		set pos [string first " packets received," $pingRet]
		if { $pos > 10 } {
			set ipReachable [string range $pingRet [expr $pos - 1] [expr $pos - 1]]
		}
    }
	puts "ipReachable: $ipReachable"
    if { $ipReachable == 0 } {
        set errorMsg "Error: Device at $cDEVICE_IP not reachable"
        setHomeMaticErrorMsg $errorMsg
        return
    }

    # get counter values
    set wgetRet ""
    catch {
        set url "$cDEVICE_IP/counter"
        set wgetRet [exec wget --no-check-certificate --quiet --timeout=5 -O - $url]
        # puts "result:\n$wgetRet"
    }
    
    #puts "wgetRet length: [string length $wgetRet]"
    if { [string length $wgetRet] < 100 } {	;# Html Stringlänge: 213 Byte für 6 Counter + CRC
        set errorMsg "Error: Invalid wget return data"
        setHomeMaticErrorMsg $errorMsg
        return
    }

    # parse counter values, convert to Int, calc CRC
    set crc 0xFFFFFFFF  ;# CRC-32 ISO 3309
    for {set i 1} {$i <= 6} {incr i} {        ;# hier nicht $cCHANNEL_COUNT sondern 6 verwenden da der CRC sich auf alle 6 counter bezieht!
        set pos [string first "Counter_$i," $wgetRet]
        if { $pos > 0 } {
            #puts $pos
            set counterStr [string range $wgetRet [expr $pos + 10] [expr $pos + 19]] ;# 10 digits
            set counterInt [forceInteger $counterStr]
            #puts $counterInt
    
            # CRC for all 4 bytes from 32bit word, MSB first
			for {set b 3} {$b >= 0} {incr b -1} {
            	set value [expr { ($counterInt >> ($b * 8)) & 0xFF }]
				#puts $value
            	set crc [crcCalc $crc $value]
            }

            set counterValue_$i $counterInt
            puts "$i [set counterValue_$i]"
        }
    }
    set crc [expr ~$crc]  ;# CRC-32 ISO 3309

    # parse CRC
    set crcInt 0
	set pos [string first "CRC," $wgetRet]
    if { $pos > 0 } {
        set crcStr [string range $wgetRet [expr $pos + 4] [expr $pos + 13]] ;# 10 digits
        set crcInt [forceInteger $crcStr]
		#puts "crcInt: $crcInt [format %u $crcInt]"
	}

    #puts "CRC: [format %u $crc]"
	if { $crc != $crcInt } {
        set errorMsg "Error: Invalid CRC"
        setHomeMaticErrorMsg $errorMsg
        return
	}

    set rega_cmd ""
    if { $cCHANNEL_COUNT >= 1 } {
        append rega_cmd "dom.GetObject('STROM.Cnt1').State('$counterValue_1');"
    }
    if { $cCHANNEL_COUNT >= 2 } {
        append rega_cmd "dom.GetObject('STROM.Cnt2').State('$counterValue_2');"
    }
    if { $cCHANNEL_COUNT >= 3 } {
        append rega_cmd "dom.GetObject('STROM.Cnt3').State('$counterValue_3');"
    }
    if { $cCHANNEL_COUNT >= 4 } {
        append rega_cmd "dom.GetObject('STROM.Cnt4').State('$counterValue_4');"
    }
    if { $cCHANNEL_COUNT >= 5 } {
        append rega_cmd "dom.GetObject('STROM.Cnt5').State('$counterValue_5');"
    }
    if { $cCHANNEL_COUNT >= 6 } {
        append rega_cmd "dom.GetObject('STROM.Cnt6').State('$counterValue_6');"
    }
	
    # STROM.ErrMsg nur schreiben wenn sich was ändert, wegen HomeMatic Systemprotokollierung
	append rega_cmd "string errorMsg = dom.GetObject('STROM.ErrMsg').Value();"
    append rega_cmd "if (errorMsg != 'none') { dom.GetObject('STROM.ErrMsg').State('none'); }"
	array set regaRet [rega_script $rega_cmd]
    #puts "regaRet (counters): $regaRet(STDOUT)"
}

# -------------------------------------
proc forceInteger { x } {
    set count [scan $x %u%s n rest]
    if { $count <= 0 || ( $count == 2 && ![string is space $rest] ) } {
        #return -code error "not an integer: \"$x\""
		return 0
    }
    return $n
}

# -------------------------------------
proc crcCalc { crcstart byte } {
    #puts "input: [format %x $crcstart] [format %x $byte]"
	set crc [expr {$crcstart ^ $byte}]
	#puts [format %x $crc]

	for {set i 0} {$i < 8} {incr i} {
		set mask [expr { -1 * ($crc & 1) }]
		set crc [expr { (($crc >> 1) & 0x7FFFFFFF) ^ (0xEDB88320 & $mask) }]
		#puts "mask: [format %x $mask] crc: [format %x $crc]"
	}
    return $crc
}

# -------------------------------------
proc setHomeMaticErrorMsg { errorMsg } {
    puts $errorMsg
    set rega_cmd ""
	append rega_cmd "integer errorCnt = dom.GetObject('STROM.ErrCnt').Value();"
	append rega_cmd "errorCnt = errorCnt + 1;"
	append rega_cmd "dom.GetObject('STROM.ErrCnt').State(errorCnt);"
    append rega_cmd "dom.GetObject('STROM.ErrMsg').State('$errorMsg');"
    array set regaRet [rega_script $rega_cmd]
    #puts "regaRet (error): $regaRet(STDOUT)"
}

main $argc $argv

#
#if {![string is integer -strict $counter]} {
#	puts "\"$counter\" isn't a proper integer"
#}
