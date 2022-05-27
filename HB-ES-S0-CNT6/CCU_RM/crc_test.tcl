#!/bin/tclsh
#
# -------------------------------------
# CRC Test für HB-ES-S0-CNT6
# Version 0.10
# (C) 2019-2022 Tom Major (Creative Commons)
# https://creativecommons.org/licenses/by-nc-sa/4.0/
# You are free to Share & Adapt under the following terms:
# Give Credit, NonCommercial, ShareAlike
# -------------------------------------

load tclrega.so

# -------------------------------------
proc main {argc argv} {
    
    set wgetRet "<html> Counter_1,0009226523, <br> Counter_2,0000000000, <br> Counter_3,0001623832, <br> Counter_4,0003868202, <br> Counter_5,0000000000, <br> Counter_6,0000000000, <br> CRC,1158841290, </html>"
    puts "\nwget result:\n$wgetRet"

    # parse counter values, convert to Int, calc CRC
    set crc 0xFFFFFFFF  ;# CRC-32 ISO 3309
    for {set i 1} {$i <= 6} {incr i} {        ;# hier nicht $cCHANNEL_COUNT sondern 6 verwenden da der CRC sich auf alle 6 counter bezieht!
        set pos [string first "Counter_$i," $wgetRet]
        if { $pos > 0 } {
            #puts $pos
            set counterStr [string range $wgetRet [expr $pos + 10] [expr $pos + 19]] ;# 10 digits
            set counterInt [forceInteger $counterStr]
            set counterValue_$i $counterInt
            puts "$i [set counterValue_$i]"

            # CRC for all 4 bytes from 32bit word, MSB first
			for {set b 3} {$b >= 0} {incr b -1} {
            	set value [expr { ($counterInt >> ($b * 8)) & 0xFF }]
				#puts "DBG $value $crc"
            	set crc [crcCalc $crc $value]
            }

        }
    }
    set crc [expr ~$crc]  ;# CRC-32 ISO 3309
    # ab RaspberryMatic 3.63.8.20220330 kann es auch 64bit TCL geben, dann Maskierung auf 32bit CRC notwendig
    set crc [expr $crc & 0xFFFFFFFF]
    puts "CRC: [format %u $crc]"

    # parse CRC
    set crcInt 0
	set pos [string first "CRC," $wgetRet]
    if { $pos > 0 } {
        set crcStr [string range $wgetRet [expr $pos + 4] [expr $pos + 13]] ;# 10 digits
        set crcInt [forceInteger $crcStr]
		puts "crcInt: [format %u $crcInt]"
	}

	if { $crc != $crcInt } {
        puts "Error: Invalid CRC"
        return
	}
    puts "CRC OK"
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

main $argc $argv
