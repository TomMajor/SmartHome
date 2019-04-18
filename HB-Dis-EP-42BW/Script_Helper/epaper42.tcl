#!/bin/tclsh
#
# =================================================
# epaper42.tcl
# HB-Dis-EP-42BW script helper
# Version 0.41
# 2019-03-28 Tom Major (Creative Commons)
# https://creativecommons.org/licenses/by-nc-sa/3.0/
# You are free to Share & Adapt under the following terms:
# Give Credit, NonCommercial, ShareAlike
#
# usage: epaper42 serial /line text [icon number] [/nextline text [icon number]] ...
#
# Der erste Parameter ist die Seriennummer des Displays, z.B. JPDISEP000
# Jede neue Zeile beginnt mit einem / gefolgt von der Zeilennummer.
# Danach folgt der anzuzeigende Text, enthält der Text Leerzeichen muss man den ganzen Text in '' einschliessen, andernfalls geht es auch ohne.
# Die im WebUI vordefinierten Texte werden mit dem Code §xx erzeugt, wobei xx zwischen 01 und 20 liegen kann und immer 2 Stellen haben muss.
# Der 3. Parameter ist die Iconnummer oder den Parameter weglassen wenn man kein Icon in der Zeile haben will.
# Die Iconnummer braucht nur eine Stelle bei Icons < 10.
# CUxD/CMD_EXEC braucht man dabei nicht zwingend. Man kann das auch mit system.Exec() aufrufen.
#
# Beispiel 1 - variabler Text in einer Zeile:
# Zeigt den Text 'Test ABC ÄÖÜäöüß' in Zeile 5 mit Icon 1 auf dem ePaper mit Serial JPDISEP000 an:
# string displayCmd = "JPDISEP000 /5 'Test ABC ÄÖÜäöüß' 1";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
# oder
# system.Exec("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 2 - variabler Text in mehreren Zeilen:
# Zeigt 3 Zeilen Text in den Zeilen 5, 7, 10 an, dabei Zeile 5 und 10 mit Icons, 7 ohne Icon
# string displayCmd = "JPDISEP000 /5 'Test ABC 123' 1 /7 Textzeile_7 /10 Textzeile_10 12";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 3 - Sensorwert
# Zeigt die Temperatur vom Gerät UNISENS077 in Zeile 2 an
# string temp = dom.GetObject('BidCos-RF.UNISENS077:1.TEMPERATURE').Value().ToString(1) # "°C";
# string displayCmd = "JPDISEP000 /2 'Temperatur " # temp # "'";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 4 - vordefinierte Texte
# Zeigt den vordef. Text 4 in Zeile 1, den vordef. Text 19 in Zeile 9 und den vordef. Text 20 in Zeile1 10 an, Zeile 1 zusätzlich mit Icon
# string displayCmd = "JPDISEP000 /1 §04 6 /9 §19 /10 §20";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# Beispiel 5 - variable und vordefinierte Texte gemischt in einer Zeile
# Zeigt den vordef. Text 2 gemischt mit variablen Text in Zeile 1 an
# string displayCmd = "JPDISEP000 /1 abcd§02efgh";
# dom.GetObject("CUxD.CUX2801001:1.CMD_EXEC").State("tclsh /usr/local/addons/epaper42.tcl " # displayCmd);
#
# =================================================

load tclrega.so

# -------------------------------------
proc main { argc argv } {
    
    if { $argc < 3 } {
        return
    }
    
    for { set i 0 }  { $i <= 10 }  { incr i } {
        set DISPLAY($i) ""
    }
    
    set SERIAL [lindex $argv 0]

    set i 0
    foreach subCmd [split $argv "/"] {
        if { $i > 0 && $i <= 10 } {
            set LINE [lindex $subCmd 0]
            set TEXT [lindex $subCmd 1]
            set ICON [lindex $subCmd 2]

            # process each display line
            if { $LINE >= 1 && $LINE <= 10 && [string length $TEXT] > 0 } {

                #debugLog "<$LINE><$TEXT><$ICON>"

                set txtOut "0x12,"

                # fixed or variable text, can be combined in one line
                for { set n 0 } { $n < [string length $TEXT] } { incr n } {
                    set char [string index $TEXT $n]
                    scan $char "%c" numDec
                    set numHex [format %x $numDec]
                    if { $numHex == "a7" } {	# §, code for fixed text, 2 digits required
                        set indexFixText [string range $TEXT [expr $n + 1] [expr $n + 2]]
                        if { ([string length $indexFixText] == 2) &&
                            ([string is integer -strict $indexFixText]) &&
                            ($indexFixText >= 1) &&
                            ($indexFixText <= 20) } {
                            set textDec [expr 127 + $indexFixText]
                            set textHex [format %x $textDec]
                            append txtOut "0x$textHex,"
                        }
                        incr n 2
                    } else { # variable text
                        # hex 30..5A, 61..7A
                        if { ($numDec >= 48 && $numDec <= 90) ||
                            ($numDec >= 97 && $numDec <= 122) } {
                            append txtOut "0x$numHex,"
                        } else {
                            append txtOut "0x[decodeSpecialChar $numHex],"
                        }
                    }
                }

                # optional icon
                if { $ICON >= 1 && $ICON <= 21 } {
                    append txtOut "0x13,"
                    set iconDec [expr 127 + $ICON]
                    set iconHex [format %x $iconDec]
                    append txtOut "0x$iconHex,"
                }
                
                # store line cmd
                set DISPLAY($LINE) $txtOut
            }
        }
        incr i
    }
    
    # build complete display command
    set displayCmd "0x02,"
    for { set i 1 } { $i <= 10 } { incr i } {
        if { [string length $DISPLAY($i)] > 0 } {
            append displayCmd $DISPLAY($i)
        } 
        # line end
        append displayCmd "0x0A,"
    }
    append displayCmd "0x03"
    #debugLog $displayCmd
    
    set rega_cmd ""
    append rega_cmd "dom.GetObject('BidCos-RF.$SERIAL:11.SUBMIT').State('$displayCmd');"
    array set regaRet [rega_script $rega_cmd]
}

# -------------------------------------
proc decodeSpecialChar { numHex } {
	switch $numHex {
    
    	20 		{ return "20" }     # space
    	21 		{ return "21" }     # !
    	25 		{ return "25" }     # %
    	27 		{ return "27" }     # =
    	2a 		{ return "2A" }     # *
    	2b 		{ return "2B" }     # +
    	2c 		{ return "2C" }     # ,
    	2d 		{ return "2D" }     # -
    	2e 		{ return "2E" }     # .
    	b0 		{ return "B0" }     # °
    	5f 		{ return "5F" }     # _

    	c4 		{ return "5B" }     # Ä
    	d6 		{ return "23" }     # Ö
    	dc 		{ return "24" }     # Ü
    	e4 		{ return "7B" }     # ä
    	f6 		{ return "7C" }     # ö
    	fc 		{ return "7D" }     # ü
    	df 		{ return "7E" }     # ß
        
    	default {
            #debugLog "Unknown: $numHex"
            return "2E" 
        }
	}
}

# -------------------------------------
proc debugLog { text } {
    #set fileId [open "/media/usb1/debug.log" "a+"]
	#puts $fileId $text
	#close $fileId
}

main $argc $argv
